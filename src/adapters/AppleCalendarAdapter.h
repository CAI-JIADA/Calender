#ifndef APPLECALENDARADAPTER_H
#define APPLECALENDARADAPTER_H

#include "CalendarAdapter.h"
#include <QNetworkAccessManager>

/**
 * @brief Apple Calendar 適配器
 * 
 * 實作 Apple Calendar (iCloud) 透過 CalDAV 協議的整合
 */
class AppleCalendarAdapter : public CalendarAdapter
{
    Q_OBJECT

public:
    /**
     * @brief 建構函式
     * @param parent 父物件
     */
    explicit AppleCalendarAdapter(QObject* parent = nullptr);

    /**
     * @brief 解構函式
     */
    ~AppleCalendarAdapter() override;

    /**
     * @brief 取得平台類型
     * @return Platform::Apple
     */
    Platform platform() const override { return Platform::Apple; }

    /**
     * @brief 設定應用程式專用密碼
     * @param userId Apple ID
     * @param appPassword 應用程式專用密碼
     */
    void setCredentials(const QString& userId, const QString& appPassword);

    /**
     * @brief 認證
     */
    void authenticate() override;

    /**
     * @brief 登出
     */
    void logout() override;

    /**
     * @brief 取得事件
     * @param startDate 開始日期
     * @param endDate 結束日期
     */
    void fetchEvents(const QDate& startDate, const QDate& endDate) override;

    /**
     * @brief 取得任務
     */
    void fetchTasks() override;

    /**
     * @brief 取得共享行事曆
     */
    void fetchSharedCalendars() override;

private:
    void sendCalDAVRequest(const QString& method, const QByteArray& body,
                           const std::function<void(const QByteArray&)>& callback);
    void discoverCalendars();
    QList<CalendarEvent> parseICalendar(const QByteArray& data);

    QNetworkAccessManager* m_networkManager;
    QString m_userId;
    QString m_appPassword;
    QString m_calendarHome;
    QStringList m_calendarUrls;
};

#endif // APPLECALENDARADAPTER_H
