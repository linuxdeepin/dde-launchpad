// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QSignalSpy>

#include "trashmonitor.h"

class TestTrashMonitor : public QObject
{
    Q_OBJECT
private slots:
    void constructAndDestruct();
    void trashItemCountReturnsNonNegative();
    void onTrashMonitorChangedAttributeChangedEmitsSignal();
    void onTrashMonitorChangedCreatedDoesNotEmit();
    void onTrashMonitorChangedDeletedDoesNotEmit();
    void onTrashMonitorChangedChangedDoesNotEmit();
    void slotOnTrashMonitorChangedEmitsSignal();
    void slotOnTrashMonitorChangedOtherDoesNotEmit();
};

void TestTrashMonitor::constructAndDestruct()
{
    {
        TrashMonitor monitor(nullptr);
        QVERIFY(monitor.m_trash != nullptr);
        QVERIFY(monitor.m_trashMonitor != nullptr);
    }
}

void TestTrashMonitor::trashItemCountReturnsNonNegative()
{
    TrashMonitor monitor(nullptr);
    int count = monitor.trashItemCount();
    QCOMPARE(count, 0);
}

void TestTrashMonitor::onTrashMonitorChangedAttributeChangedEmitsSignal()
{
    TrashMonitor monitor(nullptr);
    QSignalSpy spy(&monitor, &TrashMonitor::trashAttributeChanged);

    monitor.onTrashMonitorChanged(nullptr, nullptr, nullptr,
                                  G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED);

    QCOMPARE(spy.count(), 1);
}

void TestTrashMonitor::onTrashMonitorChangedCreatedDoesNotEmit()
{
    TrashMonitor monitor(nullptr);
    QSignalSpy spy(&monitor, &TrashMonitor::trashAttributeChanged);

    monitor.onTrashMonitorChanged(nullptr, nullptr, nullptr,
                                  G_FILE_MONITOR_EVENT_CREATED);

    QCOMPARE(spy.count(), 0);
}

void TestTrashMonitor::onTrashMonitorChangedDeletedDoesNotEmit()
{
    TrashMonitor monitor(nullptr);
    QSignalSpy spy(&monitor, &TrashMonitor::trashAttributeChanged);

    monitor.onTrashMonitorChanged(nullptr, nullptr, nullptr,
                                  G_FILE_MONITOR_EVENT_DELETED);

    QCOMPARE(spy.count(), 0);
}

void TestTrashMonitor::onTrashMonitorChangedChangedDoesNotEmit()
{
    TrashMonitor monitor(nullptr);
    QSignalSpy spy(&monitor, &TrashMonitor::trashAttributeChanged);

    monitor.onTrashMonitorChanged(nullptr, nullptr, nullptr,
                                  G_FILE_MONITOR_EVENT_CHANGED);

    QCOMPARE(spy.count(), 0);
}

void TestTrashMonitor::slotOnTrashMonitorChangedEmitsSignal()
{
    TrashMonitor monitor(nullptr);
    QSignalSpy spy(&monitor, &TrashMonitor::trashAttributeChanged);

    TrashMonitor::slot_onTrashMonitorChanged(nullptr, nullptr, nullptr,
                                             G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED,
                                             &monitor);

    QCOMPARE(spy.count(), 1);
}

void TestTrashMonitor::slotOnTrashMonitorChangedOtherDoesNotEmit()
{
    TrashMonitor monitor(nullptr);
    QSignalSpy spy(&monitor, &TrashMonitor::trashAttributeChanged);

    TrashMonitor::slot_onTrashMonitorChanged(nullptr, nullptr, nullptr,
                                             G_FILE_MONITOR_EVENT_PRE_UNMOUNT,
                                             &monitor);

    QCOMPARE(spy.count(), 0);
}

QTEST_MAIN(TestTrashMonitor)
#include "trashmonitortest.moc"
