#ifndef CALENDARADAPTER_H
#define CALENDARADAPTER_H

#include <QObject>
#include <QList>
#include <QDate>
#include "core/CalendarEvent.h"
#include "core/Task.h"
#include "core/Platform.h"

/**
 * @brief 行事曆適配器抽象基類
 * 
 * 定義所有平台適配器必須實作的介面
 */
class CalendarAdapter : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 建構函式
     * @param parent 父物件
     */
    explicit CalendarAdapter(QObject* parent = nullptr)
        : QObject(parent)
        , m_isAuthenticated(false)
    {}

    /**
     * @brief 虛擬解構函式
     */
    virtual ~CalendarAdapter() = default;

    /**
     * @brief 取得平台類型
     * @return 平台類型
     */
    virtual Platform platform() const = 0;

    /**
     * @brief 認證
     */
    virtual void authenticate() = 0;

    /**
     * @brief 檢查是否已認證
     * @return true 如果已認證
     */
    bool isAuthenticated() const { return m_isAuthenticated; }

    /**
     * @brief 登出
     */
    virtual void logout() = 0;

    /**
     * @brief 取得事件
     * @param startDate 開始日期
     * @param endDate 結束日期
     */
    virtual void fetchEvents(const QDate& startDate, const QDate& endDate) = 0;

    /**
     * @brief 取得任務
     */
    virtual void fetchTasks() = 0;

    /**
     * @brief 取得共享行事曆
     */
    virtual void fetchSharedCalendars() = 0;

signals:
    /**
     * @brief 認證成功信號
     */
    void authenticated();

    /**
     * @brief 認證失敗信號
     * @param error 錯誤訊息
     */
    void authenticationFailed(const QString& error);

    /**
     * @brief 收到事件信號
     * @param events 事件列表
     */
    void eventsReceived(const QList<CalendarEvent>& events);

    /**
     * @brief 收到任務信號
     * @param tasks 任務列表
     */
    void tasksReceived(const QList<Task>& tasks);

    /**
     * @brief 錯誤信號
     * @param error 錯誤訊息
     */
    void errorOccurred(const QString& error);

protected:
    bool m_isAuthenticated;  ///< 認證狀態
};

#endif // CALENDARADAPTER_H
