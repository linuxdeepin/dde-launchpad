// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appwiz.h"

#include "DaemonLauncher1.h"

#include <QThreadPool>
#include <DDesktopEntry>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDdeIntegration)

using DaemonLauncher1 = __DaemonLauncher1;

DCORE_USE_NAMESPACE

AppWiz::AppWiz(QObject *parent)
    : QObject(parent)
    , m_dbusDaemonLauncherIface(new DaemonLauncher1(DaemonLauncher1::staticInterfaceName(), "/org/deepin/dde/daemon/Launcher1",
                                                    QDBusConnection::sessionBus(), this))
{
    qCDebug(logDdeIntegration) << "Initializing AppWiz";
}

AppWiz::~AppWiz()
{
    qCDebug(logDdeIntegration) << "Destroying AppWiz";
}

// TODO: remove this and the whole m_dbusDaemonLauncherIface thing once we have a modern "appwiz" service for uninstalling apps.
void AppWiz::legacyRequestUninstall(const QString &desktopFileFullPath)
{

    QThreadPool::globalInstance()->start([desktopFileFullPath, this](){

        DDesktopEntry desktopEntry(desktopFileFullPath);
        if (desktopEntry.status() != DDesktopEntry::NoError) {
            qCWarning(logDdeIntegration) << "Desktop file is invalid:" << desktopFileFullPath;
            return;
        }

        if (!desktopEntry.stringValue("X-Deepin-PreUninstall").isEmpty()) {
            QFileInfo desktopFileInfo(desktopFileFullPath);
            bool writable = desktopFileInfo.isWritable();
            if (writable) {
                qCDebug(logDdeIntegration) << "Desktop file" << desktopFileFullPath << "is writable, it might be a user-level .desktop file, avoiding execute the PreUninstall command.";
            } else {
                const QString & preUninstallScript = desktopEntry.stringValue("X-Deepin-PreUninstall");
                // The script is usually a shell script, we need to execute it and check the return code.
                // We don't need pkexec, execute it directly.
                // If error, we should print the stderr and return.
                QStringList args = QProcess::splitCommand(preUninstallScript);
                QProcess process;
                if (args.size() < 1) {
                    qCDebug(logDdeIntegration) << "Pre-uninstall script" << preUninstallScript << "is invalid, aborting uninstallation for" << desktopFileFullPath;
                    return;
                }

                // Security: validate the script path to prevent command injection (CWE-78).
                // Only allow absolute paths to executables that exist and are executable.
                QFileInfo scriptInfo(args[0]);
                if (!scriptInfo.isAbsolute()) {
                    qCWarning(logDdeIntegration) << "Pre-uninstall script path is not absolute, aborting:" << args[0];
                    return;
                }
                if (!scriptInfo.exists() || !scriptInfo.isExecutable()) {
                    qCWarning(logDdeIntegration) << "Pre-uninstall script does not exist or is not executable, aborting:" << args[0];
                    return;
                }

                if (args.size() == 1) {
                    process.start(args[0]);
                } else {
                    process.start(args[0], args.mid(1));
                }
                bool succ = process.waitForFinished(-1);
                if (!succ || process.exitCode() != 0) {
                    const int exitCode = process.exitCode();
                    qCDebug(logDdeIntegration) << "Pre-uninstall script" << preUninstallScript << "exited with exit code:" << exitCode << process.error();
                    switch (exitCode) {
                    case 101:
                        qCDebug(logDdeIntegration) << "Which means user canceled uninstallation for" << desktopFileFullPath;
                        qCDebug(logDdeIntegration) << "Thus aborting the uninstallation.";
                        return;
                    case 103:
                        qCDebug(logDdeIntegration) << "Which means there is a running instance of the pre-uninstall script for" << desktopFileFullPath;
                        qCDebug(logDdeIntegration) << "Thus aborting the uninstallation.";
                        return;
                    default:
                        qCDebug(logDdeIntegration) << "stderr:" << process.readAllStandardError();
                        qCDebug(logDdeIntegration) << "stdout:" << process.readAllStandardOutput();
                        qCDebug(logDdeIntegration) << "Will continue uninstallation for" << desktopFileFullPath;
                    }
                }
                qCDebug(logDdeIntegration) << "Pre-uninstall script" << preUninstallScript << "succeeded.";
            }
        }
    
        QDBusPendingReply rpy = m_dbusDaemonLauncherIface->RequestUninstall(desktopFileFullPath, true);
        if (rpy.isError()) {
            qCDebug(logDdeIntegration) << rpy.error();
        }
    });
}
