#ifndef CALENDARMANAGER_H
#define CALENDARMANAGER_H

#include <QObject>
#include <QList>
#include <QDate>
#include <QMap>
#include "CalendarEvent.h"
#include "Task.h"
#include "SearchCriteria.h"

/**
 * @brief 行事曆管理器
 * 
 * 負責管理所有平台的事件和任務，提供統一的存取介面
 */
class CalendarManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 建構函式
     * @param parent 父物件
     */
    explicit CalendarManager(QObject* parent = nullptr);

    /**
     * @brief 解構函式
     */
    ~CalendarManager() override;

    // 事件管理
    /**
     * @brief 取得所有事件
     * @return 事件列表
     */
    QList<CalendarEvent> getAllEvents() const;

    /**
     * @brief 取得指定日期範圍的事件
     * @param start 開始日期
     * @param end 結束日期
     * @return 事件列表
     */
    QList<CalendarEvent> getEventsByDateRange(const QDate& start, const QDate& end) const;

    /**
     * @brief 取得指定平台的事件
     * @param platform 平台類型
     * @return 事件列表
     */
    QList<CalendarEvent> getEventsByPlatform(Platform platform) const;

    /**
     * @brief 新增事件
     * @param event 事件
     * @return true 如果新增成功
     */
    bool addEvent(const CalendarEvent& event);

    /**
     * @brief 更新事件
     * @param event 事件
     * @return true 如果更新成功
     */
    bool updateEvent(const CalendarEvent& event);

    /**
     * @brief 刪除事件
     * @param eventId 事件識別碼
     * @param platform 平台類型
     * @return true 如果刪除成功
     */
    bool removeEvent(const QString& eventId, Platform platform);

    // 任務管理
    /**
     * @brief 取得所有任務
     * @return 任務列表
     */
    QList<Task> getAllTasks() const;

    /**
     * @brief 取得指定日期到期的任務
     * @param date 目標日期
     * @return 任務列表
     */
    QList<Task> getTasksByDueDate(const QDate& date) const;

    /**
     * @brief 取得指定平台的任務
     * @param platform 平台類型
     * @return 任務列表
     */
    QList<Task> getTasksByPlatform(Platform platform) const;

    /**
     * @brief 新增任務
     * @param task 任務
     * @return true 如果新增成功
     */
    bool addTask(const Task& task);

    /**
     * @brief 更新任務
     * @param task 任務
     * @return true 如果更新成功
     */
    bool updateTask(const Task& task);

    /**
     * @brief 刪除任務
     * @param taskId 任務識別碼
     * @param platform 平台類型
     * @return true 如果刪除成功
     */
    bool removeTask(const QString& taskId, Platform platform);

    /**
     * @brief 標記任務為已完成
     * @param taskId 任務識別碼
     * @param platform 平台類型
     * @param completed 是否完成
     * @return true 如果操作成功
     */
    bool setTaskCompleted(const QString& taskId, Platform platform, bool completed);

    // 搜索功能
    /**
     * @brief 搜索事件
     * @param query 搜索關鍵字
     * @return 符合條件的事件列表
     */
    QList<CalendarEvent> searchEvents(const QString& query) const;

    /**
     * @brief 進階搜索事件
     * @param criteria 搜索條件
     * @return 符合條件的事件列表
     */
    QList<CalendarEvent> advancedSearch(const SearchCriteria& criteria) const;

signals:
    /**
     * @brief 事件新增信號
     * @param event 新增的事件
     */
    void eventAdded(const CalendarEvent& event);

    /**
     * @brief 事件更新信號
     * @param event 更新的事件
     */
    void eventUpdated(const CalendarEvent& event);

    /**
     * @brief 事件刪除信號
     * @param eventId 刪除的事件識別碼
     * @param platform 平台類型
     */
    void eventRemoved(const QString& eventId, Platform platform);

    /**
     * @brief 任務新增信號
     * @param task 新增的任務
     */
    void taskAdded(const Task& task);

    /**
     * @brief 任務更新信號
     * @param task 更新的任務
     */
    void taskUpdated(const Task& task);

    /**
     * @brief 任務刪除信號
     * @param taskId 刪除的任務識別碼
     * @param platform 平台類型
     */
    void taskRemoved(const QString& taskId, Platform platform);

    /**
     * @brief 資料變更信號
     */
    void dataChanged();

private:
    QList<CalendarEvent> m_events;   ///< 事件列表
    QList<Task> m_tasks;             ///< 任務列表
};

#endif // CALENDARMANAGER_H
