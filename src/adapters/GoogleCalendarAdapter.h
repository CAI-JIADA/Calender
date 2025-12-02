#ifndef GOOGLECALENDARADAPTER_H
#define GOOGLECALENDARADAPTER_H

#include "CalendarAdapter.h"
#include <QNetworkAccessManager>
#include <QOAuth2AuthorizationCodeFlow>

/**
 * @brief Google Calendar 適配器
 * 
 * 實作 Google Calendar API 的整合
 */
class GoogleCalendarAdapter : public CalendarAdapter
{
    Q_OBJECT

public:
    /**
     * @brief 建構函式
     * @param parent 父物件
     */
    explicit GoogleCalendarAdapter(QObject* parent = nullptr);

    /**
     * @brief 解構函式
     */
    ~GoogleCalendarAdapter() override;

    /**
     * @brief 取得平台類型
     * @return Platform::Google
     */
    Platform platform() const override { return Platform::Google; }

    /**
     * @brief 設定 OAuth 2.0 憑證
     * @param clientId 客戶端 ID
     * @param clientSecret 客戶端密鑰
     */
    void setCredentials(const QString& clientId, const QString& clientSecret);

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

private slots:
    void onAuthenticationGranted();
    void onAuthenticationError(const QString& error, const QString& errorDescription, const QUrl& uri);

private:
    void setupOAuth();
    void sendRequest(const QUrl& url, const std::function<void(const QByteArray&)>& callback);

    QNetworkAccessManager* m_networkManager;
    QOAuth2AuthorizationCodeFlow* m_oauth;
    QString m_clientId;
    QString m_clientSecret;
    QString m_accessToken;
};

#endif // GOOGLECALENDARADAPTER_H
