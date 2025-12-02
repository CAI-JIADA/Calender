#ifndef OUTLOOKCALENDARADAPTER_H
#define OUTLOOKCALENDARADAPTER_H

#include "CalendarAdapter.h"
#include <QNetworkAccessManager>
#include <QOAuth2AuthorizationCodeFlow>

/**
 * @brief Microsoft Outlook 適配器
 * 
 * 實作 Microsoft Graph API 的整合
 */
class OutlookCalendarAdapter : public CalendarAdapter
{
    Q_OBJECT

public:
    /**
     * @brief 建構函式
     * @param parent 父物件
     */
    explicit OutlookCalendarAdapter(QObject* parent = nullptr);

    /**
     * @brief 解構函式
     */
    ~OutlookCalendarAdapter() override;

    /**
     * @brief 取得平台類型
     * @return Platform::Outlook
     */
    Platform platform() const override { return Platform::Outlook; }

    /**
     * @brief 設定 Azure AD 憑證
     * @param clientId 應用程式 ID
     * @param tenantId 租戶 ID
     */
    void setCredentials(const QString& clientId, const QString& tenantId);

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
    QString m_tenantId;
    QString m_accessToken;
};

#endif // OUTLOOKCALENDARADAPTER_H
