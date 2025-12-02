#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include <QObject>
#include "core/CalendarEvent.h"
#include "core/Task.h"
#include "core/SearchCriteria.h"
#include "core/CalendarManager.h"

/**
 * @brief 搜索引擎
 * 
 * 提供事件和任務的搜索功能
 */
class SearchEngine : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 建構函式
     * @param calendarManager 行事曆管理器
     * @param parent 父物件
     */
    explicit SearchEngine(CalendarManager* calendarManager, QObject* parent = nullptr);

    /**
     * @brief 解構函式
     */
    ~SearchEngine() override;

    /**
     * @brief 基本搜索事件
     * @param query 搜索關鍵字
     * @return 符合條件的事件列表
     */
    QList<CalendarEvent> searchEvents(const QString& query) const;

    /**
     * @brief 進階搜索事件
     * @param criteria 搜索條件
     * @return 符合條件的事件列表
     */
    QList<CalendarEvent> advancedSearchEvents(const SearchCriteria& criteria) const;

    /**
     * @brief 日期範圍搜索事件
     * @param start 開始日期
     * @param end 結束日期
     * @param query 搜索關鍵字 (可選)
     * @return 符合條件的事件列表
     */
    QList<CalendarEvent> searchEventsByDateRange(const QDate& start,
                                                  const QDate& end,
                                                  const QString& query = QString()) const;

    /**
     * @brief 平台篩選搜索事件
     * @param query 搜索關鍵字
     * @param platforms 平台列表
     * @return 符合條件的事件列表
     */
    QList<CalendarEvent> searchEventsByPlatform(const QString& query,
                                                 const QStringList& platforms) const;

    /**
     * @brief 基本搜索任務
     * @param query 搜索關鍵字
     * @return 符合條件的任務列表
     */
    QList<Task> searchTasks(const QString& query) const;

    /**
     * @brief 進階搜索任務
     * @param criteria 搜索條件
     * @return 符合條件的任務列表
     */
    QList<Task> advancedSearchTasks(const SearchCriteria& criteria) const;

    /**
     * @brief 搜索所有項目 (事件 + 任務)
     * @param query 搜索關鍵字
     * @param outEvents 輸出事件列表
     * @param outTasks 輸出任務列表
     */
    void searchAll(const QString& query,
                   QList<CalendarEvent>& outEvents,
                   QList<Task>& outTasks) const;

private:
    /**
     * @brief 檢查事件是否符合關鍵字
     * @param event 事件
     * @param keyword 關鍵字 (小寫)
     * @return true 如果符合
     */
    bool eventMatchesKeyword(const CalendarEvent& event, const QString& keyword) const;

    /**
     * @brief 檢查任務是否符合關鍵字
     * @param task 任務
     * @param keyword 關鍵字 (小寫)
     * @return true 如果符合
     */
    bool taskMatchesKeyword(const Task& task, const QString& keyword) const;

    CalendarManager* m_calendarManager;
};

#endif // SEARCHENGINE_H
