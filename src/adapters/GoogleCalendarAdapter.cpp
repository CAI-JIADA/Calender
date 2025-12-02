#include "GoogleCalendarAdapter.h"
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QOAuthHttpServerReplyHandler>

namespace {
    const QString GOOGLE_AUTH_URL = QStringLiteral("https://accounts.google.com/o/oauth2/v2/auth");
    const QString GOOGLE_TOKEN_URL = QStringLiteral("https://oauth2.googleapis.com/token");
    const QString GOOGLE_CALENDAR_API = QStringLiteral("https://www.googleapis.com/calendar/v3");
    const QString GOOGLE_TASKS_API = QStringLiteral("https://tasks.googleapis.com/tasks/v1");
}

GoogleCalendarAdapter::GoogleCalendarAdapter(QObject* parent)
    : CalendarAdapter(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_oauth(nullptr)
{
}

GoogleCalendarAdapter::~GoogleCalendarAdapter()
{
}

void GoogleCalendarAdapter::setCredentials(const QString& clientId, const QString& clientSecret)
{
    m_clientId = clientId;
    m_clientSecret = clientSecret;
    setupOAuth();
}

void GoogleCalendarAdapter::setupOAuth()
{
    if (m_oauth) {
        delete m_oauth;
    }

    m_oauth = new QOAuth2AuthorizationCodeFlow(this);
    m_oauth->setAuthorizationUrl(QUrl(GOOGLE_AUTH_URL));
    m_oauth->setAccessTokenUrl(QUrl(GOOGLE_TOKEN_URL));
    m_oauth->setClientIdentifier(m_clientId);
    m_oauth->setClientIdentifierSharedKey(m_clientSecret);
    m_oauth->setScope(QStringLiteral("https://www.googleapis.com/auth/calendar.readonly "
                                      "https://www.googleapis.com/auth/tasks.readonly"));

    auto* replyHandler = new QOAuthHttpServerReplyHandler(8080, this);
    m_oauth->setReplyHandler(replyHandler);

    connect(m_oauth, &QOAuth2AuthorizationCodeFlow::granted,
            this, &GoogleCalendarAdapter::onAuthenticationGranted);
    connect(m_oauth, &QOAuth2AuthorizationCodeFlow::error,
            this, &GoogleCalendarAdapter::onAuthenticationError);

    connect(m_oauth, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            &QDesktopServices::openUrl);
}

void GoogleCalendarAdapter::authenticate()
{
    if (!m_oauth) {
        emit authenticationFailed(tr("OAuth not configured. Please set credentials first."));
        return;
    }
    m_oauth->grant();
}

void GoogleCalendarAdapter::logout()
{
    m_accessToken.clear();
    m_isAuthenticated = false;
    if (m_oauth) {
        m_oauth->setToken(QString());
    }
}

void GoogleCalendarAdapter::onAuthenticationGranted()
{
    m_accessToken = m_oauth->token();
    m_isAuthenticated = true;
    emit authenticated();
}

void GoogleCalendarAdapter::onAuthenticationError(const QString& error,
                                                   const QString& errorDescription,
                                                   const QUrl& /*uri*/)
{
    m_isAuthenticated = false;
    emit authenticationFailed(QString("%1: %2").arg(error, errorDescription));
}

void GoogleCalendarAdapter::sendRequest(const QUrl& url,
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

void GoogleCalendarAdapter::fetchEvents(const QDate& startDate, const QDate& endDate)
{
    if (!m_isAuthenticated) {
        emit errorOccurred(tr("Not authenticated"));
        return;
    }

    QUrl url(GOOGLE_CALENDAR_API + "/calendars/primary/events");
    QUrlQuery query;
    query.addQueryItem("timeMin", startDate.startOfDay().toUTC().toString(Qt::ISODate));
    query.addQueryItem("timeMax", endDate.endOfDay().toUTC().toString(Qt::ISODate));
    query.addQueryItem("singleEvents", "true");
    query.addQueryItem("orderBy", "startTime");
    url.setQuery(query);

    sendRequest(url, [this](const QByteArray& data) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject root = doc.object();
        QJsonArray items = root["items"].toArray();

        QList<CalendarEvent> events;
        for (const QJsonValue& item : items) {
            QJsonObject obj = item.toObject();
            CalendarEvent event;
            event.id = obj["id"].toString();
            event.title = obj["summary"].toString();
            event.description = obj["description"].toString();
            event.location = obj["location"].toString();
            event.platform = Platform::Google;

            QJsonObject start = obj["start"].toObject();
            QJsonObject end = obj["end"].toObject();

            if (start.contains("date")) {
                event.isAllDay = true;
                event.startTime = QDateTime(QDate::fromString(start["date"].toString(), Qt::ISODate), QTime());
                event.endTime = QDateTime(QDate::fromString(end["date"].toString(), Qt::ISODate), QTime());
            } else {
                event.isAllDay = false;
                event.startTime = QDateTime::fromString(start["dateTime"].toString(), Qt::ISODate);
                event.endTime = QDateTime::fromString(end["dateTime"].toString(), Qt::ISODate);
            }

            // 解析參與者
            QJsonArray attendees = obj["attendees"].toArray();
            for (const QJsonValue& attendee : attendees) {
                event.attendees.append(attendee.toObject()["email"].toString());
            }

            // 解析重複規則
            QJsonArray recurrence = obj["recurrence"].toArray();
            if (!recurrence.isEmpty()) {
                event.recurrenceRule = recurrence.first().toString();
            }

            events.append(event);
        }

        emit eventsReceived(events);
    });
}

void GoogleCalendarAdapter::fetchTasks()
{
    if (!m_isAuthenticated) {
        emit errorOccurred(tr("Not authenticated"));
        return;
    }

    // 首先取得任務清單
    QUrl url(GOOGLE_TASKS_API + "/users/@me/lists");

    sendRequest(url, [this](const QByteArray& data) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject root = doc.object();
        QJsonArray items = root["items"].toArray();

        if (items.isEmpty()) {
            emit tasksReceived(QList<Task>());
            return;
        }

        // 取得第一個任務清單的任務
        QString listId = items.first().toObject()["id"].toString();
        QUrl tasksUrl(GOOGLE_TASKS_API + QString("/lists/%1/tasks").arg(listId));

        sendRequest(tasksUrl, [this](const QByteArray& tasksData) {
            QJsonDocument tasksDoc = QJsonDocument::fromJson(tasksData);
            QJsonObject tasksRoot = tasksDoc.object();
            QJsonArray taskItems = tasksRoot["items"].toArray();

            QList<Task> tasks;
            for (const QJsonValue& item : taskItems) {
                QJsonObject obj = item.toObject();
                Task task;
                task.id = obj["id"].toString();
                task.title = obj["title"].toString();
                task.description = obj["notes"].toString();
                task.platform = Platform::Google;
                task.isCompleted = (obj["status"].toString() == "completed");

                if (obj.contains("due")) {
                    task.dueDate = QDateTime::fromString(obj["due"].toString(), Qt::ISODate);
                }

                tasks.append(task);
            }

            emit tasksReceived(tasks);
        });
    });
}

void GoogleCalendarAdapter::fetchSharedCalendars()
{
    if (!m_isAuthenticated) {
        emit errorOccurred(tr("Not authenticated"));
        return;
    }

    QUrl url(GOOGLE_CALENDAR_API + "/users/me/calendarList");

    sendRequest(url, [this](const QByteArray& data) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject root = doc.object();
        QJsonArray items = root["items"].toArray();

        // 處理行事曆列表 (包含共享行事曆)
        // 這裡可以進一步處理取得共享行事曆的事件
        Q_UNUSED(items);
    });
}
