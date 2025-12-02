#include "SyncManager.h"
#include <QDate>

SyncManager::SyncManager(CalendarManager* calendarManager, QObject* parent)
    : QObject(parent)
    , m_calendarManager(calendarManager)
    , m_autoSyncTimer(new QTimer(this))
    , m_isSyncing(false)
    , m_pendingSyncs(0)
{
    connect(m_autoSyncTimer, &QTimer::timeout, this, &SyncManager::syncAll);
}

SyncManager::~SyncManager()
{
}

void SyncManager::registerAdapter(CalendarAdapter* adapter)
{
    if (!adapter) {
        return;
    }

    Platform platform = adapter->platform();
    
    // 如果已有該平台的適配器，先移除
    if (m_adapters.contains(platform)) {
        unregisterAdapter(platform);
    }

    m_adapters[platform] = adapter;

    // 連接信號
    connect(adapter, &CalendarAdapter::eventsReceived,
            this, &SyncManager::onEventsReceived);
    connect(adapter, &CalendarAdapter::tasksReceived,
            this, &SyncManager::onTasksReceived);
    connect(adapter, &CalendarAdapter::errorOccurred,
            this, &SyncManager::onAdapterError);
}

void SyncManager::unregisterAdapter(Platform platform)
{
    if (!m_adapters.contains(platform)) {
        return;
    }

    CalendarAdapter* adapter = m_adapters.take(platform);
    disconnect(adapter, nullptr, this, nullptr);
}

CalendarAdapter* SyncManager::adapter(Platform platform) const
{
    return m_adapters.value(platform, nullptr);
}

void SyncManager::setAutoSyncInterval(int minutes)
{
    if (minutes > 0) {
        m_autoSyncTimer->setInterval(minutes * 60 * 1000);
    }
}

void SyncManager::setAutoSyncEnabled(bool enabled)
{
    if (enabled) {
        if (!m_autoSyncTimer->isActive()) {
            m_autoSyncTimer->start();
        }
    } else {
        m_autoSyncTimer->stop();
    }
}

void SyncManager::syncAll()
{
    if (m_isSyncing) {
        return;
    }

    m_isSyncing = true;
    emit syncingChanged();
    emit syncStarted();

    // 設定同步日期範圍 (前後各一個月)
    m_syncStartDate = QDate::currentDate().addMonths(-1);
    m_syncEndDate = QDate::currentDate().addMonths(1);

    m_pendingSyncs = 0;

    for (auto it = m_adapters.begin(); it != m_adapters.end(); ++it) {
        if (it.value()->isAuthenticated()) {
            m_pendingSyncs += 2; // 事件 + 任務
            it.value()->fetchEvents(m_syncStartDate, m_syncEndDate);
            it.value()->fetchTasks();
            emit syncProgress(platformToString(it.key()), 0);
        }
    }

    if (m_pendingSyncs == 0) {
        m_isSyncing = false;
        emit syncingChanged();
        emit syncCompleted();
    }
}

void SyncManager::syncPlatform(Platform platform)
{
    if (m_isSyncing) {
        return;
    }

    CalendarAdapter* adapter = m_adapters.value(platform, nullptr);
    if (!adapter || !adapter->isAuthenticated()) {
        emit syncError(tr("Adapter not available or not authenticated"));
        return;
    }

    m_isSyncing = true;
    emit syncingChanged();
    emit syncStarted();

    m_syncStartDate = QDate::currentDate().addMonths(-1);
    m_syncEndDate = QDate::currentDate().addMonths(1);
    m_pendingSyncs = 2;

    adapter->fetchEvents(m_syncStartDate, m_syncEndDate);
    adapter->fetchTasks();
    emit syncProgress(platformToString(platform), 0);
}

void SyncManager::onEventsReceived(const QList<CalendarEvent>& events)
{
    CalendarAdapter* adapter = qobject_cast<CalendarAdapter*>(sender());
    if (!adapter) {
        return;
    }

    Platform platform = adapter->platform();
    emit syncProgress(platformToString(platform), 50);

    // 更新事件到 CalendarManager
    for (const CalendarEvent& event : events) {
        if (!m_calendarManager->addEvent(event)) {
            m_calendarManager->updateEvent(event);
        }
    }

    m_pendingSyncs--;
    if (m_pendingSyncs <= 0) {
        m_isSyncing = false;
        m_lastSyncTime = QDateTime::currentDateTime();
        emit syncingChanged();
        emit lastSyncTimeChanged();
        emit syncCompleted();
    }
}

void SyncManager::onTasksReceived(const QList<Task>& tasks)
{
    CalendarAdapter* adapter = qobject_cast<CalendarAdapter*>(sender());
    if (!adapter) {
        return;
    }

    Platform platform = adapter->platform();
    emit syncProgress(platformToString(platform), 100);

    // 更新任務到 CalendarManager
    for (const Task& task : tasks) {
        if (!m_calendarManager->addTask(task)) {
            m_calendarManager->updateTask(task);
        }
    }

    m_pendingSyncs--;
    if (m_pendingSyncs <= 0) {
        m_isSyncing = false;
        m_lastSyncTime = QDateTime::currentDateTime();
        emit syncingChanged();
        emit lastSyncTimeChanged();
        emit syncCompleted();
    }
}

void SyncManager::onAdapterError(const QString& error)
{
    CalendarAdapter* adapter = qobject_cast<CalendarAdapter*>(sender());
    if (adapter) {
        emit syncError(QString("%1: %2").arg(platformToString(adapter->platform()), error));
    } else {
        emit syncError(error);
    }

    m_pendingSyncs--;
    if (m_pendingSyncs <= 0) {
        m_isSyncing = false;
        emit syncingChanged();
        emit syncCompleted();
    }
}
