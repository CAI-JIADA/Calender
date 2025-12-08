# Qt 多平台行事曆整合工具 - 實作指南

## 專案概述

本專案旨在使用 Qt 框架開發一個跨平台的行事曆整合工具，可以將 Google Calendar、Apple Calendar 和 Microsoft Outlook 等不同平台的待辦事項整合到統一的介面中。

---

## 目錄

1. [系統架構](#系統架構)
2. [支援平台整合](#支援平台整合)
3. [功能需求](#功能需求)
4. [技術實作建議](#技術實作建議)
5. [所需工具與依賴](#所需工具與依賴)
6. [使用者介面設計](#使用者介面設計)
7. [安全性考量](#安全性考量)
8. [開發時程建議](#開發時程建議)

---

## 系統架構

### 整體架構圖

```
┌─────────────────────────────────────────────────────────────┐
│                      使用者介面 (Qt QML/Widgets)             │
│                   (桌面版 + 行動裝置版)                       │
├─────────────────────────────────────────────────────────────┤
│                      業務邏輯層                              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │   事件管理   │  │   日程管理   │  │   搜索引擎   │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
├─────────────────────────────────────────────────────────────┤
│                      資料同步層                              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │ Google API   │  │ Apple API    │  │ Outlook API  │      │
│  │   適配器     │  │   適配器     │  │   適配器     │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
├─────────────────────────────────────────────────────────────┤
│                      本地儲存層                              │
│           (SQLite / JSON 快取)                              │
└─────────────────────────────────────────────────────────────┘
```

### 核心模組

1. **平台適配器模組** - 處理不同平台 API 的差異
2. **資料同步模組** - 管理雲端與本地資料同步
3. **事件管理模組** - 統一管理事件和日程
4. **搜索模組** - 提供全文搜索功能
5. **UI 模組** - 跨平台使用者介面

---

## 支援平台整合

### 1. Google Calendar

#### API 資訊
- **API 名稱**: Google Calendar API v3
- **文件連結**: https://developers.google.com/calendar/api
- **認證方式**: OAuth 2.0

#### 整合步驟
1. 在 Google Cloud Console 建立專案
2. 啟用 Google Calendar API
3. 設定 OAuth 2.0 憑證
4. 實作 OAuth 2.0 授權流程

#### 所需權限範圍 (Scopes)
```
https://www.googleapis.com/auth/calendar.readonly
https://www.googleapis.com/auth/calendar.events.readonly
https://www.googleapis.com/auth/tasks.readonly
```

#### Qt 實作建議
```cpp
// 使用 Qt Network 模組進行 API 呼叫
#include <QNetworkAccessManager>
#include <QOAuth2AuthorizationCodeFlow>

class GoogleCalendarAdapter : public CalendarAdapter {
    Q_OBJECT
public:
    void authenticate();
    void fetchEvents(const QDate& startDate, const QDate& endDate);
    void fetchTasks();
    
signals:
    void eventsReceived(const QList<CalendarEvent>& events);
    void tasksReceived(const QList<Task>& tasks);
};
```

---

### 2. Apple Calendar (iCloud)

#### 跨平台整合說明 ⭐

**重要**: Apple Calendar 整合**不需要 Mac 環境**！透過 CalDAV 協議，可以在 Windows、Linux 和其他平台上實作完整的 iCloud Calendar 整合。

#### API 資訊
- **API 名稱**: CalDAV Protocol (RFC 4791)
- **主要協議**: CalDAV (Calendar Extensions to WebDAV)
- **跨平台支援**: ✅ Windows、Linux、macOS 全平台支援
- **文件連結**: 
  - CalDAV RFC: https://tools.ietf.org/html/rfc4791
  - Apple CalDAV: https://developer.apple.com/library/archive/documentation/NetworkingInternet/Conceptual/iCloudCalendarSharingGuide/
- **認證方式**: App-specific Password (應用程式專用密碼)

> **注意**: EventKit framework 僅限 macOS/iOS 平台，但 CalDAV 協議是標準化的跨平台解決方案，無需 Apple 硬體或作業系統。

#### 整合步驟 (跨平台適用)

##### 1. 設定 Apple ID 應用程式專用密碼
1. 登入 Apple ID 帳號管理頁面 (appleid.apple.com)
2. 進入「安全性」區段
3. 選擇「應用程式專用密碼」
4. 產生新的應用程式密碼 (會顯示為 xxxx-xxxx-xxxx-xxxx 格式)
5. 記錄此密碼供應用程式使用

##### 2. CalDAV 連線設定
```cpp
// CalDAV 連線參數 (適用所有平台)
struct CalDAVConfig {
    QString server = "caldav.icloud.com";
    quint16 port = 443;
    bool useSSL = true;
    QString username; // Apple ID email
    QString password; // 應用程式專用密碼
    QString basePath; // /[AppleID]/calendars/
};
```

##### 3. 實作 CalDAV 客戶端 (跨平台)

#### CalDAV 端點
```
主機: caldav.icloud.com
端口: 443 (HTTPS)
協議: CalDAV over HTTPS
基礎路徑: /{Apple_ID_去掉@及後面部分}/calendars/

範例:
- Apple ID: user@example.com
- 基礎路徑: /user/calendars/
```

#### Qt 跨平台實作範例
```cpp
// 使用 Qt Network 實作 CalDAV 客戶端 (全平台適用)
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QXmlStreamReader>

class AppleCalendarAdapter : public CalendarAdapter {
    Q_OBJECT
public:
    // 跨平台認證 - 使用應用程式專用密碼
    void authenticate(const QString& appleId, const QString& appPassword);
    
    // CalDAV 標準操作
    void discoverCalendars();           // PROPFIND 請求
    void fetchCalendarList();           // 列出所有行事曆
    void fetchEvents(const QString& calendarId, 
                    const QDateTime& start, 
                    const QDateTime& end);  // REPORT 請求
    void createEvent(const CalendarEvent& event);    // PUT 請求
    void updateEvent(const CalendarEvent& event);    // PUT 請求
    void deleteEvent(const QString& eventId);        // DELETE 請求
    
signals:
    void authenticationSuccess();
    void authenticationFailed(const QString& error);
    void calendarsDiscovered(const QList<CalendarInfo>& calendars);
    void eventsReceived(const QList<CalendarEvent>& events);
    
private:
    QNetworkAccessManager* m_networkManager;
    QString m_baseUrl;
    QString m_username;
    QString m_password;
    
    // CalDAV HTTP 方法實作
    void sendPropfind(const QString& path, int depth = 1);
    void sendReport(const QString& path, const QByteArray& reportXml);
    void sendPut(const QString& path, const QByteArray& icsData);
    void sendDelete(const QString& path);
    
    // XML 解析
    QList<CalendarInfo> parseMultistatusResponse(const QByteArray& xml);
    QList<CalendarEvent> parseCalendarData(const QByteArray& icsData);
    
    // 身份驗證處理
    void handleAuthenticationRequired(QNetworkReply* reply, 
                                     QAuthenticator* authenticator);
};

// CalDAV PROPFIND 請求範例 (查詢行事曆列表)
void AppleCalendarAdapter::discoverCalendars() {
    QString url = QString("https://%1:%2%3")
        .arg(m_server)
        .arg(m_port)
        .arg(m_basePath);
    
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml; charset=utf-8");
    request.setRawHeader("Depth", "1");
    
    QByteArray propfindXml = 
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:propfind xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "  <d:prop>"
        "    <d:displayname />"
        "    <d:resourcetype />"
        "    <c:calendar-description />"
        "    <c:calendar-color />"
        "  </d:prop>"
        "</d:propfind>";
    
    QNetworkReply* reply = m_networkManager->sendCustomRequest(
        request, "PROPFIND", propfindXml);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QList<CalendarInfo> calendars = parseMultistatusResponse(reply->readAll());
            emit calendarsDiscovered(calendars);
        }
        reply->deleteLater();
    });
}

// CalDAV REPORT 請求範例 (查詢事件)
void AppleCalendarAdapter::fetchEvents(const QString& calendarUrl,
                                       const QDateTime& start,
                                       const QDateTime& end) {
    QNetworkRequest request(calendarUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml; charset=utf-8");
    request.setRawHeader("Depth", "1");
    
    QString reportXml = QString(
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<c:calendar-query xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "  <d:prop>"
        "    <d:getetag />"
        "    <c:calendar-data />"
        "  </d:prop>"
        "  <c:filter>"
        "    <c:comp-filter name=\"VCALENDAR\">"
        "      <c:comp-filter name=\"VEVENT\">"
        "        <c:time-range start=\"%1\" end=\"%2\" />"
        "      </c:comp-filter>"
        "    </c:comp-filter>"
        "  </c:filter>"
        "</c:calendar-query>")
        .arg(start.toString(Qt::ISODate))
        .arg(end.toString(Qt::ISODate));
    
    QNetworkReply* reply = m_networkManager->sendCustomRequest(
        request, "REPORT", reportXml.toUtf8());
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QList<CalendarEvent> events = parseCalendarData(reply->readAll());
            emit eventsReceived(events);
        }
        reply->deleteLater();
    });
}
```

#### 平台特定注意事項

##### Windows
- ✅ 完全支援 CalDAV
- 建議使用 Qt Network 模組
- 支援 Windows 7 及以上版本

##### Linux
- ✅ 完全支援 CalDAV
- 建議使用 Qt Network 模組
- 所有主流發行版均可使用

##### macOS
- ✅ 支援 CalDAV（跨平台方式）
- ✅ 可選使用 EventKit framework（原生 API，但限 macOS）
- 建議：優先使用 CalDAV 以保持跨平台一致性

#### 常見問題與解決方案

##### Q: 如何獲取 Apple ID 的基礎路徑？
```cpp
// 自動發現基礎路徑
void AppleCalendarAdapter::discoverPrincipal(const QString& appleId) {
    // Step 1: 發送 PROPFIND 到根路徑
    QString url = "https://caldav.icloud.com/";
    // Step 2: 解析回應中的 current-user-principal
    // Step 3: 使用該路徑作為基礎路徑
}
```

##### Q: 身份驗證失敗怎麼辦？
- 確認使用的是「應用程式專用密碼」而非 Apple ID 密碼
- 檢查 Apple ID 是否啟用雙重認證
- 驗證應用程式專用密碼是否過期

##### Q: 如何處理 SSL/TLS 連線？
```cpp
// Qt Network 自動處理 SSL
QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
request.setSslConfiguration(sslConfig);
```

---

### 3. Microsoft Outlook

#### API 資訊
- **API 名稱**: Microsoft Graph API
- **文件連結**: https://learn.microsoft.com/en-us/graph/api/resources/calendar
- **認證方式**: OAuth 2.0 (Azure AD)

#### 整合步驟
1. 在 Azure Portal 註冊應用程式
2. 設定 API 權限
3. 實作 OAuth 2.0 授權流程

#### 所需權限
```
Calendars.Read
Calendars.Read.Shared
Tasks.Read
User.Read
```

#### Qt 實作建議
```cpp
class OutlookCalendarAdapter : public CalendarAdapter {
    Q_OBJECT
public:
    void authenticate();
    void fetchEvents(const QDateTime& start, const QDateTime& end);
    void fetchSharedCalendars();
    void fetchTasks();
    
private:
    QString m_accessToken;
    QNetworkAccessManager* m_networkManager;
};
```

---

## 跨平台實作重點說明

### Apple Calendar 跨平台整合 (無需 Mac 環境)

本節詳細說明如何在**沒有 Mac 環境**的情況下實作 Apple Calendar (iCloud) 整合。

#### 為什麼 CalDAV 是最佳跨平台方案？

| 特性 | CalDAV 協議 | EventKit Framework |
|------|-------------|-------------------|
| 跨平台支援 | ✅ Windows/Linux/macOS | ❌ 僅 macOS/iOS |
| 需要 Mac | ❌ 不需要 | ✅ 必須 |
| 開發成本 | 低（標準協議） | 高（需 Apple 硬體） |
| 部署靈活性 | 高（任意平台） | 低（僅 Apple 平台） |
| API 穩定性 | ✅ RFC 標準 | ⚠️ Apple 專有 |

#### CalDAV 完整實作流程

##### 步驟 1: 設定開發環境（任意平台）

```bash
# Windows、Linux、macOS 均適用
# 僅需安裝 Qt 開發環境

# 安裝 Qt 6.x
# 下載: https://www.qt.io/download

# 或使用套件管理器
# Ubuntu/Debian:
sudo apt-get install qt6-base-dev qt6-networkauth-dev

# macOS (Homebrew):
brew install qt@6

# Windows: 使用 Qt Online Installer
```

##### 步驟 2: 獲取 Apple 認證資訊

```plaintext
1. 登入 https://appleid.apple.com
2. 點選「安全性」
3. 在「應用程式專用密碼」區段點選「產生密碼」
4. 輸入密碼名稱（例如：「Calendar Integration App」）
5. 記錄產生的密碼（格式：xxxx-xxxx-xxxx-xxxx）

注意事項：
- 必須啟用雙重認證才能產生應用程式專用密碼
- 每個應用程式專用密碼僅顯示一次，請妥善保存
- 可隨時撤銷不再使用的密碼
```

##### 步驟 3: 實作 CalDAV 客戶端

```cpp
// CalDAVClient.h - 跨平台 CalDAV 客戶端
#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QXmlStreamReader>
#include <QSslConfiguration>

class CalDAVClient : public QObject {
    Q_OBJECT
    
public:
    explicit CalDAVClient(QObject* parent = nullptr);
    ~CalDAVClient() override;
    
    // 認證
    void setCredentials(const QString& username, const QString& appPassword);
    
    // 服務發現
    void discoverService();
    void discoverCalendars();
    
    // 行事曆操作
    void listCalendars();
    void getEvents(const QString& calendarUrl, 
                   const QDateTime& start, 
                   const QDateTime& end);
    
    // 事件操作
    void createEvent(const QString& calendarUrl, const QString& icsData);
    void updateEvent(const QString& eventUrl, const QString& icsData);
    void deleteEvent(const QString& eventUrl);
    
signals:
    void serviceDiscovered(const QString& principalUrl);
    void calendarsListed(const QList<CalendarInfo>& calendars);
    void eventsReceived(const QList<QByteArray>& icsData);
    void operationSucceeded();
    void errorOccurred(const QString& error);
    
private slots:
    void handleAuthenticationRequired(QNetworkReply* reply, 
                                      QAuthenticator* authenticator);
    void handleSslErrors(QNetworkReply* reply, 
                        const QList<QSslError>& errors);
    
private:
    QNetworkAccessManager* m_manager;
    QString m_username;
    QString m_password;
    QString m_baseUrl;
    
    // CalDAV 請求方法
    void sendPropfind(const QUrl& url, const QByteArray& xml, int depth = 1);
    void sendReport(const QUrl& url, const QByteArray& xml);
    void sendPut(const QUrl& url, const QByteArray& data);
    void sendDelete(const QUrl& url);
    
    // XML 處理
    QList<CalendarInfo> parseCalendarList(const QByteArray& xml);
    QString parsePrincipalUrl(const QByteArray& xml);
};

// CalDAVClient.cpp - 實作範例
CalDAVClient::CalDAVClient(QObject* parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
    // 設定 SSL
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    QSslConfiguration::setDefaultConfiguration(sslConfig);
    
    // 連接信號
    connect(m_manager, &QNetworkAccessManager::authenticationRequired,
            this, &CalDAVClient::handleAuthenticationRequired);
    connect(m_manager, &QNetworkAccessManager::sslErrors,
            this, &CalDAVClient::handleSslErrors);
}

void CalDAVClient::setCredentials(const QString& username, 
                                   const QString& appPassword) {
    m_username = username;
    m_password = appPassword;
    m_baseUrl = "https://caldav.icloud.com";
}

void CalDAVClient::discoverService() {
    QUrl url(m_baseUrl + "/.well-known/caldav");
    
    QByteArray propfindXml = 
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:propfind xmlns:d=\"DAV:\">"
        "  <d:prop>"
        "    <d:current-user-principal />"
        "  </d:prop>"
        "</d:propfind>";
    
    sendPropfind(url, propfindXml, 0);
}

void CalDAVClient::discoverCalendars() {
    // 假設已知 principal URL
    QString principalUrl = m_baseUrl + "/" + 
                          m_username.left(m_username.indexOf('@')) + "/";
    
    QByteArray propfindXml = 
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:propfind xmlns:d=\"DAV:\" "
        "xmlns:c=\"urn:ietf:params:xml:ns:caldav\" "
        "xmlns:cs=\"http://calendarserver.org/ns/\" "
        "xmlns:ical=\"http://apple.com/ns/ical/\">"
        "  <d:prop>"
        "    <d:resourcetype />"
        "    <d:displayname />"
        "    <ical:calendar-color />"
        "    <c:calendar-description />"
        "    <c:supported-calendar-component-set />"
        "  </d:prop>"
        "</d:propfind>";
    
    sendPropfind(QUrl(principalUrl + "calendars/"), propfindXml, 1);
}

void CalDAVClient::getEvents(const QString& calendarUrl,
                             const QDateTime& start,
                             const QDateTime& end) {
    QByteArray reportXml = QString(
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<c:calendar-query xmlns:d=\"DAV:\" "
        "xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "  <d:prop>"
        "    <d:getetag />"
        "    <c:calendar-data />"
        "  </d:prop>"
        "  <c:filter>"
        "    <c:comp-filter name=\"VCALENDAR\">"
        "      <c:comp-filter name=\"VEVENT\">"
        "        <c:time-range start=\"%1\" end=\"%2\" />"
        "      </c:comp-filter>"
        "    </c:comp-filter>"
        "  </c:filter>"
        "</c:calendar-query>")
        .arg(start.toString(Qt::ISODate))
        .arg(end.toString(Qt::ISODate))
        .toUtf8();
    
    sendReport(QUrl(calendarUrl), reportXml);
}

void CalDAVClient::sendPropfind(const QUrl& url, 
                                const QByteArray& xml, 
                                int depth) {
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, 
                     "application/xml; charset=utf-8");
    request.setRawHeader("Depth", QByteArray::number(depth));
    request.setRawHeader("User-Agent", "Qt CalDAV Client/1.0");
    
    QNetworkReply* reply = m_manager->sendCustomRequest(
        request, "PROPFIND", xml);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            // 處理回應...
            QList<CalendarInfo> calendars = parseCalendarList(response);
            emit calendarsListed(calendars);
        } else {
            emit errorOccurred(reply->errorString());
        }
        reply->deleteLater();
    });
}

void CalDAVClient::handleAuthenticationRequired(QNetworkReply* reply,
                                                QAuthenticator* authenticator) {
    Q_UNUSED(reply);
    authenticator->setUser(m_username);
    authenticator->setPassword(m_password);
}

void CalDAVClient::handleSslErrors(QNetworkReply* reply,
                                   const QList<QSslError>& errors) {
    // 在生產環境中，應該妥善處理 SSL 錯誤
    // 這裡僅作為範例
    for (const QSslError& error : errors) {
        qWarning() << "SSL Error:" << error.errorString();
    }
    // 注意：忽略 SSL 錯誤在生產環境中不安全
    reply->ignoreSslErrors();
}
```

##### 步驟 4: 解析 iCalendar 資料

```cpp
// iCalendar 解析器
#include <QRegularExpression>

class ICalendarParser {
public:
    struct Event {
        QString uid;
        QString summary;
        QString description;
        QDateTime dtStart;
        QDateTime dtEnd;
        QString location;
        bool isAllDay;
    };
    
    static QList<Event> parseICS(const QByteArray& icsData) {
        QList<Event> events;
        QString data = QString::fromUtf8(icsData);
        
        // 簡單的 VEVENT 解析
        QRegularExpression eventRegex(
            "BEGIN:VEVENT.*?END:VEVENT",
            QRegularExpression::DotMatchesEverythingOption
        );
        
        QRegularExpressionMatchIterator it = eventRegex.globalMatch(data);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            QString eventBlock = match.captured(0);
            
            Event event;
            event.uid = extractField(eventBlock, "UID");
            event.summary = extractField(eventBlock, "SUMMARY");
            event.description = extractField(eventBlock, "DESCRIPTION");
            event.location = extractField(eventBlock, "LOCATION");
            
            // 解析日期時間
            QString dtStart = extractField(eventBlock, "DTSTART");
            event.dtStart = parseDateTime(dtStart);
            
            QString dtEnd = extractField(eventBlock, "DTEND");
            event.dtEnd = parseDateTime(dtEnd);
            
            events.append(event);
        }
        
        return events;
    }
    
private:
    static QString extractField(const QString& block, 
                               const QString& fieldName) {
        QRegularExpression regex(fieldName + ":(.*)");
        QRegularExpressionMatch match = regex.match(block);
        if (match.hasMatch()) {
            return match.captured(1).trimmed();
        }
        return QString();
    }
    
    static QDateTime parseDateTime(const QString& dtString) {
        // 處理 iCalendar 日期格式 (例如: 20240101T120000Z)
        return QDateTime::fromString(dtString, "yyyyMMddTHHmmssZ");
    }
};
```

#### 測試與驗證

```cpp
// 測試程式
#include <QCoreApplication>
#include <QDebug>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    CalDAVClient client;
    
    // 設定認證
    client.setCredentials("your-apple-id@example.com", 
                         "xxxx-xxxx-xxxx-xxxx");
    
    // 測試連線
    QObject::connect(&client, &CalDAVClient::calendarsListed,
                    [](const QList<CalendarInfo>& calendars) {
        qDebug() << "發現" << calendars.size() << "個行事曆:";
        for (const auto& cal : calendars) {
            qDebug() << "  -" << cal.displayName;
        }
    });
    
    QObject::connect(&client, &CalDAVClient::errorOccurred,
                    [](const QString& error) {
        qDebug() << "錯誤:" << error;
    });
    
    // 啟動服務發現
    client.discoverService();
    
    return app.exec();
}
```

#### 跨平台部署注意事項

##### Windows
```powershell
# 建置 Qt 專案
cd your-project
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=C:/Qt/6.x/msvc2019_64
cmake --build . --config Release

# 部署
windeployqt Release/YourApp.exe
```

##### Linux
```bash
# 建置
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/qt6
cmake --build .

# 部署（AppImage）
linuxdeployqt YourApp
```

##### macOS
```bash
# 建置
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/usr/local/opt/qt@6
cmake --build .

# 部署
macdeployqt YourApp.app -dmg
```

#### 效能優化建議

1. **快取機制**: 本地快取已下載的事件
2. **增量同步**: 僅同步變更的資料
3. **批次請求**: 合併多個 CalDAV 請求
4. **背景同步**: 使用 Qt Concurrent 進行非同步處理

```cpp
// 背景同步範例
#include <QtConcurrent>

void CalendarManager::syncInBackground() {
    QFuture<void> future = QtConcurrent::run([this]() {
        for (const auto& calendar : m_calendars) {
            QDateTime start = QDateTime::currentDateTime();
            QDateTime end = start.addDays(30);
            m_caldavClient->getEvents(calendar.url, start, end);
        }
    });
}
```

---

## 功能需求

### 1. 待辦事項整合

#### 資料模型

```cpp
// 平台類型列舉 - 使用 enum class 確保類型安全
enum class Platform {
    Google,
    Apple,
    Outlook
};

// 統一的事件資料結構
struct CalendarEvent {
    QString id;
    QString title;
    QString description;
    QDateTime startTime;
    QDateTime endTime;
    QString location;
    Platform platform;     // 使用列舉類型確保類型安全
    QString ownerId;       // 事件擁有者
    bool isAllDay;
    QStringList attendees;
    QString recurrenceRule;
    QColor color;
};

// 任務/待辦事項
struct Task {
    QString id;
    QString title;
    QString description;
    QDateTime dueDate;
    Platform platform;    // 使用列舉類型確保類型安全
    QString ownerId;
    bool isCompleted;
    int priority;         // 1-5
    QStringList tags;
};
```

### 2. 讀取他人行事曆

#### 共享行事曆支援

| 平台 | 共享方式 | 實作方法 |
|------|----------|----------|
| Google | 行事曆共享連結 / 邀請 | 使用 `calendarList.list` API |
| Apple | iCloud 家庭共享 / 行事曆共享 | CalDAV 訂閱 |
| Outlook | 共享行事曆 / 群組行事曆 | Microsoft Graph `calendarView` |

```cpp
class SharedCalendarManager {
public:
    void addSharedCalendar(const QString& url, Platform platform);
    void removeSharedCalendar(const QString& calendarId);
    QList<CalendarEvent> getSharedEvents();
    void syncSharedCalendars();
};
```

### 3. 搜索功能

#### 搜索範圍
- 事件標題
- 事件描述
- 地點
- 參與者
- 標籤

```cpp
class SearchEngine {
public:
    // 基本搜索
    QList<CalendarEvent> search(const QString& query);
    
    // 進階搜索
    QList<CalendarEvent> advancedSearch(const SearchCriteria& criteria);
    
    // 日期範圍搜索
    QList<CalendarEvent> searchByDateRange(
        const QDate& start, 
        const QDate& end,
        const QString& query = QString()
    );
    
    // 平台篩選搜索
    QList<CalendarEvent> searchByPlatform(
        const QString& query,
        const QStringList& platforms
    );
};

struct SearchCriteria {
    QString keyword;
    QDate startDate;
    QDate endDate;
    QStringList platforms;
    QStringList owners;
    bool includeCompleted;
};
```

---

## 技術實作建議

### Qt 版本與模組

#### 建議使用 Qt 6.x
```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(CalendarIntegration VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Qml
    Quick
    QuickControls2
    Network
    NetworkAuth
    Sql
    Concurrent
)
```

### 專案結構

```
CalendarIntegration/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── CalendarEvent.h
│   │   ├── Task.h
│   │   ├── CalendarManager.cpp
│   │   └── CalendarManager.h
│   ├── adapters/
│   │   ├── CalendarAdapter.h          # 抽象基類
│   │   ├── GoogleCalendarAdapter.cpp
│   │   ├── GoogleCalendarAdapter.h
│   │   ├── AppleCalendarAdapter.cpp
│   │   ├── AppleCalendarAdapter.h
│   │   ├── OutlookCalendarAdapter.cpp
│   │   └── OutlookCalendarAdapter.h
│   ├── sync/
│   │   ├── SyncManager.cpp
│   │   └── SyncManager.h
│   ├── search/
│   │   ├── SearchEngine.cpp
│   │   └── SearchEngine.h
│   ├── storage/
│   │   ├── DatabaseManager.cpp
│   │   └── DatabaseManager.h
│   └── ui/
│       ├── MainWindow.cpp
│       ├── MainWindow.h
│       └── qml/
│           ├── main.qml
│           ├── CalendarView.qml
│           ├── EventList.qml
│           └── SearchBar.qml
├── resources/
│   ├── icons/
│   └── qml.qrc
└── tests/
    ├── test_adapters.cpp
    └── test_search.cpp
```

### 本地資料庫設計 (SQLite)

```sql
-- 行事曆表
CREATE TABLE calendars (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    platform TEXT NOT NULL,
    owner_id TEXT,
    is_shared BOOLEAN DEFAULT FALSE,
    color TEXT,
    last_sync DATETIME
);

-- 事件表
CREATE TABLE events (
    id TEXT PRIMARY KEY,
    calendar_id TEXT NOT NULL,
    title TEXT NOT NULL,
    description TEXT,
    start_time DATETIME NOT NULL,
    end_time DATETIME,
    location TEXT,
    is_all_day BOOLEAN DEFAULT FALSE,
    recurrence_rule TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (calendar_id) REFERENCES calendars(id)
);

-- 任務表
CREATE TABLE tasks (
    id TEXT PRIMARY KEY,
    calendar_id TEXT NOT NULL,
    title TEXT NOT NULL,
    description TEXT,
    due_date DATETIME,
    is_completed BOOLEAN DEFAULT FALSE,
    priority INTEGER DEFAULT 3,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (calendar_id) REFERENCES calendars(id)
);

-- 搜索索引 (使用 FTS5)
CREATE VIRTUAL TABLE events_fts USING fts5(
    title, 
    description, 
    location,
    content='events',
    content_rowid='rowid'
);
```

---

## 所需工具與依賴

### 開發環境

| 工具 | 版本建議 | 用途 |
|------|----------|------|
| Qt | 6.5+ | 主要開發框架 |
| Qt Creator | 12.0+ | IDE |
| CMake | 3.16+ | 建置系統 |
| C++ Compiler | C++17 支援 | GCC 9+, Clang 10+, MSVC 2019+ |
| Git | 2.0+ | 版本控制 |

### 外部函式庫

```cmake
# 第三方依賴
# 1. JSON 處理 (Qt 內建)
# 2. OAuth 2.0 (Qt Network Auth)
# 3. CalDAV 客戶端 (需自行實作或使用第三方)
# 4. SQLite (Qt SQL)
```

### 平台 SDK

#### Google
```bash
# 不需要額外 SDK，使用 REST API
# 需要設定 Google Cloud 專案
```

#### Apple (跨平台 - 無需 Mac)
```bash
# ⭐ 跨平台方案 (推薦)
# 使用 CalDAV 協議 - 適用於 Windows、Linux、macOS
# 無需 Apple 開發環境或 Mac 硬體
# 僅需 Qt Network 模組

# 可選方案 (僅限 macOS/iOS)
# macOS/iOS: 可使用 EventKit framework (原生 API)
# 注意: EventKit 僅限 Apple 平台，不具跨平台性
```

#### Microsoft
```bash
# 使用 Microsoft Graph REST API
# 需要設定 Azure AD 應用程式
```

### 行動裝置建置

#### Android
```bash
# 需要安裝
- Android SDK
- Android NDK
- Java JDK 11+
```

#### iOS
```bash
# 需要
- Xcode
- Apple Developer 帳號
- macOS 開發環境
```

---

## 使用者介面設計

### 桌面版 (Qt Widgets / QML)

```qml
// main.qml - 主介面範例
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 1200
    height: 800
    visible: true
    title: "行事曆整合工具"
    
    RowLayout {
        anchors.fill: parent
        spacing: 0
        
        // 側邊欄 - 行事曆列表
        Rectangle {
            Layout.preferredWidth: 250
            Layout.fillHeight: true
            color: "#f5f5f5"
            
            CalendarSidebar {
                anchors.fill: parent
            }
        }
        
        // 主內容區
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0
            
            // 搜索列
            SearchBar {
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                onSearch: (query) => calendarManager.search(query)
            }
            
            // 行事曆視圖
            CalendarView {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
        
        // 右側欄 - 事件詳情
        EventDetailPanel {
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            visible: selectedEvent !== null
        }
    }
}
```

### 行動版調整

```qml
// 響應式設計
Item {
    property bool isMobile: width < 600
    
    StackView {
        id: stackView
        anchors.fill: parent
        visible: isMobile
        
        initialItem: calendarListPage
    }
    
    SplitView {
        anchors.fill: parent
        visible: !isMobile
        
        // 桌面版分欄佈局
    }
}
```

### UI 元件清單

1. **CalendarView** - 月/週/日 檢視
2. **EventList** - 事件列表
3. **SearchBar** - 搜索欄
4. **EventDetailPanel** - 事件詳情面板
5. **CalendarSidebar** - 行事曆選擇器
6. **PlatformAccountManager** - 帳號管理
7. **SyncStatusIndicator** - 同步狀態指示

---

## 安全性考量

### OAuth Token 管理

```cpp
#include <optional>

class SecureTokenStorage {
public:
    // 使用系統金鑰鏈儲存 token
    // 回傳 true 表示儲存成功，false 表示失敗
    bool saveToken(const QString& service, const QString& token);
    
    // 使用 std::optional 處理 token 可能不存在的情況
    std::optional<QString> loadToken(const QString& service);
    
    // 回傳 true 表示刪除成功，false 表示失敗
    bool deleteToken(const QString& service);
    
private:
    // 平台特定實作
    #ifdef Q_OS_MACOS
        // 使用 Keychain
    #elif defined(Q_OS_WIN)
        // 使用 Windows Credential Manager
    #elif defined(Q_OS_LINUX)
        // 使用 libsecret / KWallet
    #endif
};

// 使用範例
void authenticateUser() {
    SecureTokenStorage storage;
    auto token = storage.loadToken("google_calendar");
    if (token.has_value()) {
        // Token 存在，使用它
        useToken(token.value());
    } else {
        // Token 不存在，需要重新認證
        performOAuthFlow();
    }
}
```

### 資料加密

```cpp
// 本地敏感資料加密
class DataEncryption {
public:
    QByteArray encrypt(const QByteArray& data, const QByteArray& key);
    QByteArray decrypt(const QByteArray& data, const QByteArray& key);
};
```

### 安全建議

1. **永遠不要**在程式碼中硬編碼 API 金鑰
2. 使用 HTTPS 進行所有網路通訊
3. 實作 token 自動更新機制
4. 定期清理過期的 token
5. 加密本地儲存的敏感資料

---

## 開發時程建議

### 第一階段 - 基礎架構 

- [ ] 建立專案結構
- [ ] 實作資料模型
- [ ] 建立本地資料庫
- [ ] 基本 UI 框架

### 第二階段 - 平台整合 (跨平台重點)

- [ ] Google Calendar 整合 (OAuth 2.0)
- [ ] Microsoft Outlook 整合 (Microsoft Graph API)
- [ ] **Apple Calendar 跨平台整合** (CalDAV 協議)
  - [ ] 實作 CalDAV 客戶端（適用 Windows/Linux/macOS）
  - [ ] 應用程式專用密碼認證
  - [ ] 服務自動發現
  - [ ] iCalendar (ICS) 格式解析
  - [ ] 測試跨平台相容性

### 第三階段 - 核心功能 

- [ ] 事件同步功能
- [ ] 搜索功能實作
- [ ] 共享行事曆支援
- [ ] 離線模式

### 第四階段 - UI 完善

- [ ] 響應式 UI
- [ ] 行動裝置優化
- [ ] 主題與自訂設定

### 第五階段 - 測試與發布 

- [ ] 單元測試
- [ ] 整合測試
- [ ] 效能優化
- [ ] 文件撰寫

---

## 常見問題 (FAQ)

### Apple Calendar 跨平台整合

#### Q1: 真的不需要 Mac 就能整合 Apple Calendar 嗎？
**A**: 是的！透過標準的 CalDAV 協議，您可以在 Windows、Linux 或任何支援 HTTP/HTTPS 的平台上存取 iCloud Calendar。CalDAV 是一個開放標準（RFC 4791），不需要 Apple 硬體或作業系統。

#### Q2: CalDAV 和 EventKit 有什麼差別？
**A**: 
- **CalDAV**: 開放標準協議，跨平台支援，透過 HTTP/HTTPS 存取
- **EventKit**: Apple 專有框架，僅限 macOS 和 iOS，需要 Xcode 和 Apple 開發環境

對於跨平台應用，**強烈建議使用 CalDAV**。

#### Q3: 如何獲取 Apple 的應用程式專用密碼？
**A**: 
1. 前往 https://appleid.apple.com
2. 登入您的 Apple ID
3. 在「安全性」區段中選擇「應用程式專用密碼」
4. 點選「產生密碼」並給予名稱
5. 複製產生的密碼（格式：xxxx-xxxx-xxxx-xxxx）

**注意**: 必須先啟用雙重認證才能產生應用程式專用密碼。

#### Q4: CalDAV 支援哪些功能？
**A**: CalDAV 支援完整的日曆功能：
- ✅ 讀取事件
- ✅ 建立事件
- ✅ 更新事件
- ✅ 刪除事件
- ✅ 讀取共享行事曆
- ✅ 事件提醒
- ✅ 重複事件
- ✅ 行事曆顏色

#### Q5: 效能如何？需要持續連線嗎？
**A**: CalDAV 是基於 HTTP 的請求-回應協議：
- 無需持續連線
- 可實作本地快取以提升效能
- 支援增量同步（僅同步變更）
- 適合定期同步（如每 5-15 分鐘）

#### Q6: 如何處理多個 iCloud 帳號？
**A**: 為每個帳號建立獨立的 CalDAVClient 實例：

```cpp
CalDAVClient* account1 = new CalDAVClient();
account1->setCredentials("user1@example.com", "xxxx-xxxx-xxxx-xxxx");

CalDAVClient* account2 = new CalDAVClient();
account2->setCredentials("user2@example.com", "yyyy-yyyy-yyyy-yyyy");
```

#### Q7: 支援哪些 iCalendar 格式？
**A**: CalDAV 使用標準的 iCalendar (RFC 5545) 格式：
- VEVENT (事件)
- VTODO (待辦事項)
- VJOURNAL (日誌)
- VALARM (提醒)

#### Q8: 如何處理時區問題？
**A**: iCalendar 格式內建時區支援。Qt 的 QDateTime 可以自動處理：

```cpp
// 使用 UTC 時間
QDateTime dt = QDateTime::currentDateTimeUtc();
QString isoDate = dt.toString(Qt::ISODate); // 2024-01-01T12:00:00Z

// 使用本地時區
QDateTime local = dt.toLocalTime();
```

#### Q9: 是否需要付費或申請開發者帳號？
**A**: 不需要！CalDAV 存取僅需：
- ✅ 免費的 Apple ID 帳號
- ✅ 啟用雙重認證
- ✅ 產生應用程式專用密碼

**無需**：
- ❌ Apple Developer Program ($99/年)
- ❌ 特殊權限或審核
- ❌ Mac 電腦

#### Q10: 遇到「401 Unauthorized」錯誤怎麼辦？
**A**: 常見原因與解決方案：
1. **密碼錯誤**: 確認使用應用程式專用密碼，而非 Apple ID 密碼
2. **密碼過期**: 應用程式專用密碼不會過期，但可能被撤銷，請重新產生
3. **帳號問題**: 確認 Apple ID 已啟用雙重認證
4. **URL 錯誤**: 確認使用正確的 CalDAV 端點（caldav.icloud.com）

#### Q11: 可以在商業產品中使用嗎？
**A**: 可以！CalDAV 是開放標準，且 iCloud 免費提供 CalDAV 存取。但請注意：
- 遵守 Apple 的服務條款
- 不要過度頻繁請求（建議間隔至少 5 分鐘）
- 實作適當的錯誤處理和重試機制
- 考慮 iCloud 免費帳號的儲存限制

#### Q12: 如何除錯 CalDAV 連線問題？
**A**: 使用這些工具和技巧：

```cpp
// 啟用 Qt 網路除錯
qputenv("QT_LOGGING_RULES", "qt.network.ssl.warning=true");

// 記錄 HTTP 請求
connect(m_manager, &QNetworkAccessManager::finished,
        [](QNetworkReply* reply) {
    qDebug() << "Response:" << reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "Headers:" << reply->rawHeaderPairs();
    qDebug() << "Body:" << reply->readAll();
});
```

也可以使用外部工具如 cURL 測試：
```bash
curl -X PROPFIND \
  -H "Depth: 1" \
  -u "your-email@example.com:xxxx-xxxx-xxxx-xxxx" \
  https://caldav.icloud.com/
```

---

## 參考資源

### API 文件
- [Google Calendar API](https://developers.google.com/calendar/api/v3/reference)
- [Microsoft Graph API](https://learn.microsoft.com/en-us/graph/api/resources/calendar)
- [CalDAV RFC 4791](https://tools.ietf.org/html/rfc4791) - CalDAV 標準協議
- [iCalendar RFC 5545](https://tools.ietf.org/html/rfc5545) - iCalendar 資料格式
- [Apple CalDAV 指南](https://developer.apple.com/library/archive/documentation/NetworkingInternet/Conceptual/iCloudCalendarSharingGuide/)

### Qt 文件
- [Qt 6 Documentation](https://doc.qt.io/qt-6/)
- [Qt Network Auth](https://doc.qt.io/qt-6/qtnetworkauth-index.html)
- [Qt Network](https://doc.qt.io/qt-6/qtnetwork-index.html)
- [Qt QML](https://doc.qt.io/qt-6/qtqml-index.html)

### CalDAV 相關資源
- [CalDAV Tester Tool](https://github.com/apple/ccs-caldavtester) - Apple 的 CalDAV 測試工具
- [sabre/dav](https://sabre.io/dav/) - PHP CalDAV 參考實作
- [RFC 6638](https://tools.ietf.org/html/rfc6638) - CalDAV 行程安排擴充

### 範例專案
- [Qt OAuth2 Examples](https://doc.qt.io/qt-6/qtnetworkauth-index.html#examples)
- [Qt Calendar Example](https://doc.qt.io/qt-6/qtwidgets-widgets-calendarwidget-example.html)
- [Qt Network Examples](https://doc.qt.io/qt-6/qtnetwork-examples.html)

---

## 授權與貢獻

本專案採用 MIT 授權。歡迎提交 Issue 和 Pull Request。

---

*最後更新: 2024*
