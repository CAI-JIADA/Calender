#include "OutlookCalendarAdapter.h"
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QOAuthHttpServerReplyHandler>

namespace {
    const QString GRAPH_API = QStringLiteral("https://graph.microsoft.com/v1.0");
}

OutlookCalendarAdapter::OutlookCalendarAdapter(QObject* parent)
    : CalendarAdapter(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_oauth(nullptr)
{
}

OutlookCalendarAdapter::~OutlookCalendarAdapter()
{
}

void OutlookCalendarAdapter::setCredentials(const QString& clientId, const QString& tenantId)
{
    m_clientId = clientId;
    m_tenantId = tenantId;
    setupOAuth();
}

void OutlookCalendarAdapter::setupOAuth()
{
    if (m_oauth) {
        delete m_oauth;
    }

    m_oauth = new QOAuth2AuthorizationCodeFlow(this);
    
    QString authUrl = QString("https://login.microsoftonline.com/%1/oauth2/v2.0/authorize")
                          .arg(m_tenantId.isEmpty() ? "common" : m_tenantId);
    QString tokenUrl = QString("https://login.microsoftonline.com/%1/oauth2/v2.0/token")
                          .arg(m_tenantId.isEmpty() ? "common" : m_tenantId);

    m_oauth->setAuthorizationUrl(QUrl(authUrl));
    m_oauth->setAccessTokenUrl(QUrl(tokenUrl));
    m_oauth->setClientIdentifier(m_clientId);
    m_oauth->setScope(QStringLiteral("Calendars.Read Calendars.Read.Shared Tasks.Read User.Read offline_access"));

    auto* replyHandler = new QOAuthHttpServerReplyHandler(8081, this);
    m_oauth->setReplyHandler(replyHandler);

    connect(m_oauth, &QOAuth2AuthorizationCodeFlow::granted,
            this, &OutlookCalendarAdapter::onAuthenticationGranted);
    connect(m_oauth, &QOAuth2AuthorizationCodeFlow::error,
            this, &OutlookCalendarAdapter::onAuthenticationError);

    connect(m_oauth, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            &QDesktopServices::openUrl);
}

void OutlookCalendarAdapter::authenticate()
{
    if (!m_oauth) {
        emit authenticationFailed(tr("OAuth not configured. Please set credentials first."));
        return;
    }
    m_oauth->grant();
}

void OutlookCalendarAdapter::logout()
{
    m_accessToken.clear();
    m_isAuthenticated = false;
    if (m_oauth) {
        m_oauth->setToken(QString());
    }
}

void OutlookCalendarAdapter::onAuthenticationGranted()
{
    m_accessToken = m_oauth->token();
    m_isAuthenticated = true;
    emit authenticated();
}

void OutlookCalendarAdapter::onAuthenticationError(const QString& error,
                                                    const QString& errorDescription,
                                                    const QUrl& /*uri*/)
{
    m_isAuthenticated = false;
    emit authenticationFailed(QString("%1: %2").arg(error, errorDescription));
}

void OutlookCalendarAdapter::sendRequest(const QUrl& url,
                                          const std::function<void(const QByteArray&)>& callback)
{
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_accessToken).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(reply->errorString());
            return;
        }
        callback(reply->readAll());
    });
}

void OutlookCalendarAdapter::fetchEvents(const QDate& startDate, const QDate& endDate)
{
    if (!m_isAuthenticated) {
        emit errorOccurred(tr("Not authenticated"));
        return;
    }

    QUrl url(GRAPH_API + "/me/calendarView");
    QUrlQuery query;
    query.addQueryItem("startDateTime", startDate.startOfDay().toUTC().toString(Qt::ISODate));
    query.addQueryItem("endDateTime", endDate.endOfDay().toUTC().toString(Qt::ISODate));
    query.addQueryItem("$orderby", "start/dateTime");
    query.addQueryItem("$top", "100");
    url.setQuery(query);

    sendRequest(url, [this](const QByteArray& data) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject root = doc.object();
        QJsonArray items = root["value"].toArray();

        QList<CalendarEvent> events;
        for (const QJsonValue& item : items) {
            QJsonObject obj = item.toObject();
            CalendarEvent event;
            event.id = obj["id"].toString();
            event.title = obj["subject"].toString();
            
            QJsonObject body = obj["body"].toObject();
            event.description = body["content"].toString();
            
            QJsonObject location = obj["location"].toObject();
            event.location = location["displayName"].toString();
            
            event.platform = Platform::Outlook;
            event.isAllDay = obj["isAllDay"].toBool();

            QJsonObject start = obj["start"].toObject();
            QJsonObject end = obj["end"].toObject();

            event.startTime = QDateTime::fromString(start["dateTime"].toString(), Qt::ISODate);
            event.endTime = QDateTime::fromString(end["dateTime"].toString(), Qt::ISODate);

            // 解析參與者
            QJsonArray attendees = obj["attendees"].toArray();
            for (const QJsonValue& attendee : attendees) {
                QJsonObject emailAddress = attendee.toObject()["emailAddress"].toObject();
                event.attendees.append(emailAddress["address"].toString());
            }

            // 解析重複規則
            QJsonObject recurrence = obj["recurrence"].toObject();
            if (!recurrence.isEmpty()) {
                QJsonObject pattern = recurrence["pattern"].toObject();
                event.recurrenceRule = pattern["type"].toString();
            }

            events.append(event);
        }

        emit eventsReceived(events);
    });
}

void OutlookCalendarAdapter::fetchTasks()
{
    if (!m_isAuthenticated) {
        emit errorOccurred(tr("Not authenticated"));
        return;
    }

    // 使用 Microsoft To Do API (Graph API)
    QUrl url(GRAPH_API + "/me/todo/lists");

    sendRequest(url, [this](const QByteArray& data) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject root = doc.object();
        QJsonArray lists = root["value"].toArray();

        if (lists.isEmpty()) {
            emit tasksReceived(QList<Task>());
            return;
        }

        // 取得預設任務清單的任務
        QString listId = lists.first().toObject()["id"].toString();
        QUrl tasksUrl(GRAPH_API + QString("/me/todo/lists/%1/tasks").arg(listId));

        sendRequest(tasksUrl, [this](const QByteArray& tasksData) {
            QJsonDocument tasksDoc = QJsonDocument::fromJson(tasksData);
            QJsonObject tasksRoot = tasksDoc.object();
            QJsonArray taskItems = tasksRoot["value"].toArray();

            QList<Task> tasks;
            for (const QJsonValue& item : taskItems) {
                QJsonObject obj = item.toObject();
                Task task;
                task.id = obj["id"].toString();
                task.title = obj["title"].toString();
                
                QJsonObject body = obj["body"].toObject();
                task.description = body["content"].toString();
                
                task.platform = Platform::Outlook;
                task.isCompleted = (obj["status"].toString() == "completed");

                QJsonObject dueDateTime = obj["dueDateTime"].toObject();
                if (!dueDateTime.isEmpty()) {
                    task.dueDate = QDateTime::fromString(dueDateTime["dateTime"].toString(), Qt::ISODate);
                }

                // 解析重要性
                QString importance = obj["importance"].toString();
                if (importance == "high") {
                    task.priority = 1;
                } else if (importance == "normal") {
                    task.priority = 3;
                } else {
                    task.priority = 5;
                }

                tasks.append(task);
            }

            emit tasksReceived(tasks);
        });
    });
}

void OutlookCalendarAdapter::fetchSharedCalendars()
{
    if (!m_isAuthenticated) {
        emit errorOccurred(tr("Not authenticated"));
        return;
    }

    QUrl url(GRAPH_API + "/me/calendars");

    sendRequest(url, [this](const QByteArray& data) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject root = doc.object();
        QJsonArray calendars = root["value"].toArray();

        // 處理行事曆列表 (包含共享行事曆)
        // 可以進一步取得每個行事曆的事件
        Q_UNUSED(calendars);
    });
}
