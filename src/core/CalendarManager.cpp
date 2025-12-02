#include "CalendarManager.h"
#include <algorithm>

CalendarManager::CalendarManager(QObject* parent)
    : QObject(parent)
{
}

CalendarManager::~CalendarManager()
{
}

// 事件管理實作

QList<CalendarEvent> CalendarManager::getAllEvents() const
{
    return m_events;
}

QList<CalendarEvent> CalendarManager::getEventsByDateRange(const QDate& start, const QDate& end) const
{
    QList<CalendarEvent> result;
    for (const auto& event : m_events) {
        if (event.isInDateRange(start, end)) {
            result.append(event);
        }
    }
    return result;
}

QList<CalendarEvent> CalendarManager::getEventsByPlatform(Platform platform) const
{
    QList<CalendarEvent> result;
    for (const auto& event : m_events) {
        if (event.platform == platform) {
            result.append(event);
        }
    }
    return result;
}

bool CalendarManager::addEvent(const CalendarEvent& event)
{
    if (!event.isValid()) {
        return false;
    }

    // 檢查是否已存在
    for (const auto& existing : m_events) {
        if (existing.id == event.id && existing.platform == event.platform) {
            return false;
        }
    }

    m_events.append(event);
    emit eventAdded(event);
    emit dataChanged();
    return true;
}

bool CalendarManager::updateEvent(const CalendarEvent& event)
{
    if (!event.isValid()) {
        return false;
    }

    for (int i = 0; i < m_events.size(); ++i) {
        if (m_events[i].id == event.id && m_events[i].platform == event.platform) {
            m_events[i] = event;
            emit eventUpdated(event);
            emit dataChanged();
            return true;
        }
    }
    return false;
}

bool CalendarManager::removeEvent(const QString& eventId, Platform platform)
{
    for (int i = 0; i < m_events.size(); ++i) {
        if (m_events[i].id == eventId && m_events[i].platform == platform) {
            m_events.removeAt(i);
            emit eventRemoved(eventId, platform);
            emit dataChanged();
            return true;
        }
    }
    return false;
}

// 任務管理實作

QList<Task> CalendarManager::getAllTasks() const
{
    return m_tasks;
}

QList<Task> CalendarManager::getTasksByDueDate(const QDate& date) const
{
    QList<Task> result;
    for (const auto& task : m_tasks) {
        if (task.isDueOn(date)) {
            result.append(task);
        }
    }
    return result;
}

QList<Task> CalendarManager::getTasksByPlatform(Platform platform) const
{
    QList<Task> result;
    for (const auto& task : m_tasks) {
        if (task.platform == platform) {
            result.append(task);
        }
    }
    return result;
}

bool CalendarManager::addTask(const Task& task)
{
    if (!task.isValid()) {
        return false;
    }

    // 檢查是否已存在
    for (const auto& existing : m_tasks) {
        if (existing.id == task.id && existing.platform == task.platform) {
            return false;
        }
    }

    m_tasks.append(task);
    emit taskAdded(task);
    emit dataChanged();
    return true;
}

bool CalendarManager::updateTask(const Task& task)
{
    if (!task.isValid()) {
        return false;
    }

    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].id == task.id && m_tasks[i].platform == task.platform) {
            m_tasks[i] = task;
            emit taskUpdated(task);
            emit dataChanged();
            return true;
        }
    }
    return false;
}

bool CalendarManager::removeTask(const QString& taskId, Platform platform)
{
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].id == taskId && m_tasks[i].platform == platform) {
            m_tasks.removeAt(i);
            emit taskRemoved(taskId, platform);
            emit dataChanged();
            return true;
        }
    }
    return false;
}

bool CalendarManager::setTaskCompleted(const QString& taskId, Platform platform, bool completed)
{
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].id == taskId && m_tasks[i].platform == platform) {
            m_tasks[i].isCompleted = completed;
            emit taskUpdated(m_tasks[i]);
            emit dataChanged();
            return true;
        }
    }
    return false;
}

// 搜索功能實作

QList<CalendarEvent> CalendarManager::searchEvents(const QString& query) const
{
    if (query.isEmpty()) {
        return m_events;
    }

    QList<CalendarEvent> result;
    QString lowerQuery = query.toLower();

    for (const auto& event : m_events) {
        if (event.title.toLower().contains(lowerQuery) ||
            event.description.toLower().contains(lowerQuery) ||
            event.location.toLower().contains(lowerQuery)) {
            result.append(event);
        }
    }
    return result;
}

QList<CalendarEvent> CalendarManager::advancedSearch(const SearchCriteria& criteria) const
{
    QList<CalendarEvent> result;
    QString lowerKeyword = criteria.keyword.toLower();

    for (const auto& event : m_events) {
        // 關鍵字篩選
        if (!criteria.keyword.isEmpty()) {
            if (!event.title.toLower().contains(lowerKeyword) &&
                !event.description.toLower().contains(lowerKeyword) &&
                !event.location.toLower().contains(lowerKeyword)) {
                continue;
            }
        }

        // 日期範圍篩選
        if (criteria.hasDateRange()) {
            if (!event.isInDateRange(criteria.startDate, criteria.endDate)) {
                continue;
            }
        }

        // 平台篩選
        if (criteria.hasPlatformFilter()) {
            QString platformStr = platformToString(event.platform);
            if (!criteria.platforms.contains(platformStr)) {
                continue;
            }
        }

        // 擁有者篩選
        if (criteria.hasOwnerFilter()) {
            if (!criteria.owners.contains(event.ownerId)) {
                continue;
            }
        }

        result.append(event);
    }

    return result;
}
