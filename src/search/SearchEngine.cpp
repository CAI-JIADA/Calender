#include "SearchEngine.h"
#include <algorithm>

SearchEngine::SearchEngine(CalendarManager* calendarManager, QObject* parent)
    : QObject(parent)
    , m_calendarManager(calendarManager)
{
}

SearchEngine::~SearchEngine()
{
}

bool SearchEngine::eventMatchesKeyword(const CalendarEvent& event, const QString& keyword) const
{
    return event.title.toLower().contains(keyword) ||
           event.description.toLower().contains(keyword) ||
           event.location.toLower().contains(keyword) ||
           std::any_of(event.attendees.begin(), event.attendees.end(),
                       [&keyword](const QString& attendee) {
                           return attendee.toLower().contains(keyword);
                       });
}

bool SearchEngine::taskMatchesKeyword(const Task& task, const QString& keyword) const
{
    return task.title.toLower().contains(keyword) ||
           task.description.toLower().contains(keyword) ||
           std::any_of(task.tags.begin(), task.tags.end(),
                       [&keyword](const QString& tag) {
                           return tag.toLower().contains(keyword);
                       });
}

QList<CalendarEvent> SearchEngine::searchEvents(const QString& query) const
{
    if (query.isEmpty()) {
        return m_calendarManager->getAllEvents();
    }

    QList<CalendarEvent> result;
    QString keyword = query.toLower();
    
    for (const CalendarEvent& event : m_calendarManager->getAllEvents()) {
        if (eventMatchesKeyword(event, keyword)) {
            result.append(event);
        }
    }

    // 依開始時間排序
    std::sort(result.begin(), result.end());
    
    return result;
}

QList<CalendarEvent> SearchEngine::advancedSearchEvents(const SearchCriteria& criteria) const
{
    return m_calendarManager->advancedSearch(criteria);
}

QList<CalendarEvent> SearchEngine::searchEventsByDateRange(const QDate& start,
                                                            const QDate& end,
                                                            const QString& query) const
{
    QList<CalendarEvent> events = m_calendarManager->getEventsByDateRange(start, end);

    if (query.isEmpty()) {
        std::sort(events.begin(), events.end());
        return events;
    }

    QString keyword = query.toLower();
    QList<CalendarEvent> result;

    for (const CalendarEvent& event : events) {
        if (eventMatchesKeyword(event, keyword)) {
            result.append(event);
        }
    }

    std::sort(result.begin(), result.end());
    return result;
}

QList<CalendarEvent> SearchEngine::searchEventsByPlatform(const QString& query,
                                                           const QStringList& platforms) const
{
    QList<CalendarEvent> result;
    QString keyword = query.toLower();

    for (const CalendarEvent& event : m_calendarManager->getAllEvents()) {
        QString platformStr = platformToString(event.platform);
        if (!platforms.contains(platformStr)) {
            continue;
        }

        if (query.isEmpty() || eventMatchesKeyword(event, keyword)) {
            result.append(event);
        }
    }

    std::sort(result.begin(), result.end());
    return result;
}

QList<Task> SearchEngine::searchTasks(const QString& query) const
{
    QList<Task> result;
    QString keyword = query.toLower();

    for (const Task& task : m_calendarManager->getAllTasks()) {
        if (query.isEmpty() || taskMatchesKeyword(task, keyword)) {
            result.append(task);
        }
    }

    // 依優先級和到期日排序
    std::sort(result.begin(), result.end());

    return result;
}

QList<Task> SearchEngine::advancedSearchTasks(const SearchCriteria& criteria) const
{
    QList<Task> result;
    QString keyword = criteria.keyword.toLower();

    for (const Task& task : m_calendarManager->getAllTasks()) {
        // 排除已完成的任務 (如果設定了)
        if (!criteria.includeCompleted && task.isCompleted) {
            continue;
        }

        // 關鍵字篩選
        if (!criteria.keyword.isEmpty() && !taskMatchesKeyword(task, keyword)) {
            continue;
        }

        // 日期範圍篩選
        if (criteria.hasDateRange() && task.dueDate.isValid()) {
            if (task.dueDate.date() < criteria.startDate ||
                task.dueDate.date() > criteria.endDate) {
                continue;
            }
        }

        // 平台篩選
        if (criteria.hasPlatformFilter()) {
            QString platformStr = platformToString(task.platform);
            if (!criteria.platforms.contains(platformStr)) {
                continue;
            }
        }

        // 擁有者篩選
        if (criteria.hasOwnerFilter()) {
            if (!criteria.owners.contains(task.ownerId)) {
                continue;
            }
        }

        result.append(task);
    }

    std::sort(result.begin(), result.end());
    return result;
}

void SearchEngine::searchAll(const QString& query,
                              QList<CalendarEvent>& outEvents,
                              QList<Task>& outTasks) const
{
    outEvents = searchEvents(query);
    outTasks = searchTasks(query);
}
