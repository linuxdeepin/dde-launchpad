// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appwiz.h"

#include "DaemonLauncher1.h"

#include <QThreadPool>
#include <DDesktopEntry>

using DaemonLauncher1 = __DaemonLauncher1;

DCORE_USE_NAMESPACE

AppWiz::AppWiz(QObject *parent)
    : QObject(parent)
    , m_dbusDaemonLauncherIface(new DaemonLauncher1(DaemonLauncher1::staticInterfaceName(), "/org/deepin/dde/daemon/Launcher1",
                                                    QDBusConnection::sessionBus(), this))
{

}

AppWiz::~AppWiz()
{

}

// TODO: remove this and the whole m_dbusDaemonLauncherIface thing once we have a modern "appwiz" service for uninstalling apps.
void AppWiz::legacyRequestUninstall(const QString &desktopFileFullPath)
{
    qDebug() << "uninstall" << desktopFileFullPath;
    qDebug() << m_dbusDaemonLauncherIface->lastError();

    QThreadPool::globalInstance()->start([desktopFileFullPath, this](){

        DDesktopEntry desktopEntry(desktopFileFullPath);
        if (desktopEntry.status() != DDesktopEntry::NoError) {
            qDebug() << "Desktop file" << desktopFileFullPath << "is invalid.";
            return;
        }

        if (!desktopEntry.stringValue("X-Deepin-PreUninstall").isEmpty()) {
            QFileInfo desktopFileInfo(desktopFileFullPath);
            bool writable = desktopFileInfo.isWritable();
            if (writable) {
                qDebug() << "Desktop file" << desktopFileFullPath << "is writable, it might be a user-level .desktop file, avoiding execute the PreUninstall command.";
            } else {
                const QString & preUninstallScript = desktopEntry.stringValue("X-Deepin-PreUninstall");
                // The script is usually a shell script, we need to execute it and check the return code.
                // We don't need pkexec, execute it directly.
                // If error, we should print the stderr and return.
                QStringList args = QProcess::splitCommand(preUninstallScript);
                QProcess process;
                if (args.size() < 1) {
                    qDebug() << "Pre-uninstall script" << preUninstallScript << "is invalid, aborting uninstallation for" << desktopFileFullPath;
                    return;
                } else if (args.size() == 1) {
                    process.start(args[0]);
                } else {
                    process.start(args[0], args.mid(1));
                }
                bool succ = process.waitForFinished(-1);
                if (!succ || process.exitCode() != 0) {
                    const int exitCode = process.exitCode();
                    qDebug() << "Pre-uninstall script" << preUninstallScript << "exited with exit code:" << exitCode << process.error();
                    switch (exitCode) {
                    case 101:
                        qDebug() << "Which means user canceled uninstallation for" << desktopFileFullPath;
                        qDebug() << "Thus aborting the uninstallation.";
                        return;
                    case 103:
                        qDebug() << "Which means there is a running instance of the pre-uninstall script for" << desktopFileFullPath;
                        qDebug() << "Thus aborting the uninstallation.";
                        return;
                    default:
                        qDebug() << "stderr:" << process.readAllStandardError();
                        qDebug() << "stdout:" << process.readAllStandardOutput();
                        qDebug() << "Will continue uninstallation for" << desktopFileFullPath;
                    }
                }
                qDebug() << "Pre-uninstall script" << preUninstallScript << "succeeded.";
            }
        }
    
        QDBusPendingReply rpy = m_dbusDaemonLauncherIface->RequestUninstall(desktopFileFullPath, true);
        if (rpy.isError()) {
            qDebug() << rpy.error();
        }
    });
}
