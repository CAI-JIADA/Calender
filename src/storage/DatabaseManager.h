#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>
#include "core/CalendarEvent.h"
#include "core/Task.h"

/**
 * @brief 資料庫管理器
 * 
 * 負責管理本地 SQLite 資料庫
 */
class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 取得單例實例
     * @return DatabaseManager 實例
     */
    static DatabaseManager& instance();

    /**
     * @brief 初始化資料庫
     * @param dbPath 資料庫檔案路徑
     * @return true 如果初始化成功
     */
    bool initialize(const QString& dbPath = QString());

    /**
     * @brief 關閉資料庫
     */
    void close();

    /**
     * @brief 檢查資料庫是否已開啟
     * @return true 如果資料庫已開啟
     */
    bool isOpen() const;

    // 行事曆操作
    /**
     * @brief 儲存行事曆
     * @param id 行事曆 ID
     * @param name 行事曆名稱
     * @param platform 平台類型
     * @param ownerId 擁有者 ID
     * @param isShared 是否為共享行事曆
     * @param color 顏色
     * @return true 如果儲存成功
     */
    bool saveCalendar(const QString& id, const QString& name, Platform platform,
                      const QString& ownerId, bool isShared, const QString& color);

    /**
     * @brief 刪除行事曆
     * @param id 行事曆 ID
     * @return true 如果刪除成功
     */
    bool deleteCalendar(const QString& id);

    // 事件操作
    /**
     * @brief 儲存事件
     * @param event 事件
     * @return true 如果儲存成功
     */
    bool saveEvent(const CalendarEvent& event);

    /**
     * @brief 刪除事件
     * @param id 事件 ID
     * @return true 如果刪除成功
     */
    bool deleteEvent(const QString& id);

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

    // 任務操作
    /**
     * @brief 儲存任務
     * @param task 任務
     * @return true 如果儲存成功
     */
    bool saveTask(const Task& task);

    /**
     * @brief 刪除任務
     * @param id 任務 ID
     * @return true 如果刪除成功
     */
    bool deleteTask(const QString& id);

    /**
     * @brief 取得所有任務
     * @return 任務列表
     */
    QList<Task> getAllTasks() const;

    /**
     * @brief 取得指定平台的任務
     * @param platform 平台類型
     * @return 任務列表
     */
    QList<Task> getTasksByPlatform(Platform platform) const;

    // 搜索操作
    /**
     * @brief 全文搜索事件
     * @param query 搜索關鍵字
     * @return 符合條件的事件列表
     */
    QList<CalendarEvent> searchEvents(const QString& query) const;

    // 同步狀態
    /**
     * @brief 更新行事曆最後同步時間
     * @param calendarId 行事曆 ID
     * @param syncTime 同步時間
     * @return true 如果更新成功
     */
    bool updateLastSync(const QString& calendarId, const QDateTime& syncTime);

signals:
    /**
     * @brief 資料變更信號
     */
    void dataChanged();

    /**
     * @brief 錯誤信號
     * @param error 錯誤訊息
     */
    void errorOccurred(const QString& error);

private:
    /**
     * @brief 私有建構函式 (單例模式)
     */
    explicit DatabaseManager(QObject* parent = nullptr);

    /**
     * @brief 建立資料表
     * @return true 如果建立成功
     */
    bool createTables();

    /**
     * @brief 建立全文搜索索引
     * @return true 如果建立成功
     */
    bool createFTSIndex();

    /**
     * @brief 從查詢結果建立事件
     * @param query SQL 查詢
     * @return 事件
     */
    CalendarEvent eventFromQuery(const QSqlQuery& query) const;

    /**
     * @brief 從查詢結果建立任務
     * @param query SQL 查詢
     * @return 任務
     */
    Task taskFromQuery(const QSqlQuery& query) const;

    QSqlDatabase m_database;
    QString m_connectionName;
};

#endif // DATABASEMANAGER_H
