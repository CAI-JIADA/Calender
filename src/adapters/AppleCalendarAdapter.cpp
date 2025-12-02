#include "AppleCalendarAdapter.h"
#include <QNetworkReply>
#include <QAuthenticator>
#include <QRegularExpression>

namespace {
    const QString CALDAV_HOST = QStringLiteral("caldav.icloud.com");
    const int CALDAV_PORT = 443;
}

AppleCalendarAdapter::AppleCalendarAdapter(QObject* parent)
    : CalendarAdapter(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    // 設定認證處理
    connect(m_networkManager, &QNetworkAccessManager::authenticationRequired,
            this, [this](QNetworkReply* /*reply*/, QAuthenticator* authenticator) {
        authenticator->setUser(m_userId);
        authenticator->setPassword(m_appPassword);
    });
}

AppleCalendarAdapter::~AppleCalendarAdapter()
{
}

void AppleCalendarAdapter::setCredentials(const QString& userId, const QString& appPassword)
{
    m_userId = userId;
    m_appPassword = appPassword;
}

void AppleCalendarAdapter::authenticate()
{
    if (m_userId.isEmpty() || m_appPassword.isEmpty()) {
        emit authenticationFailed(tr("Please set credentials first."));
        return;
    }

    // 使用 PROPFIND 探索行事曆
    discoverCalendars();
}

void AppleCalendarAdapter::logout()
{
    m_isAuthenticated = false;
    m_calendarHome.clear();
    m_calendarUrls.clear();
}

void AppleCalendarAdapter::sendCalDAVRequest(const QString& method, const QByteArray& body,
                                              const std::function<void(const QByteArray&)>& callback)
{
    QUrl url;
    url.setScheme("https");
    url.setHost(CALDAV_HOST);
    url.setPort(CALDAV_PORT);
    url.setPath(m_calendarHome.isEmpty() ? "/" : m_calendarHome);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml; charset=utf-8");
    request.setRawHeader("Depth", "1");

    QNetworkReply* reply = m_networkManager->sendCustomRequest(request, method.toUtf8(), body);
    connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(reply->errorString());
            return;
        }
        callback(reply->readAll());
    });
}

void AppleCalendarAdapter::discoverCalendars()
{
    // CalDAV PROPFIND 請求
    QByteArray propfindBody = R"(<?xml version="1.0" encoding="utf-8"?>
<D:propfind xmlns:D="DAV:" xmlns:C="urn:ietf:params:xml:ns:caldav">
    <D:prop>
        <D:resourcetype/>
        <D:displayname/>
        <C:calendar-home-set/>
    </D:prop>
</D:propfind>)";

    QUrl url;
    url.setScheme("https");
    url.setHost(CALDAV_HOST);
    url.setPort(CALDAV_PORT);
    url.setPath("/");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml; charset=utf-8");
    request.setRawHeader("Depth", "0");

    QNetworkReply* reply = m_networkManager->sendCustomRequest(request, "PROPFIND", propfindBody);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            if (reply->error() == QNetworkReply::AuthenticationRequiredError) {
                emit authenticationFailed(tr("Invalid credentials"));
            } else {
                emit authenticationFailed(reply->errorString());
            }
            return;
        }

        QByteArray response = reply->readAll();
        
        // 解析 calendar-home-set
        QRegularExpression regex("<C:calendar-home-set>\\s*<D:href>([^<]+)</D:href>\\s*</C:calendar-home-set>");
        QRegularExpressionMatch match = regex.match(QString::fromUtf8(response));
        if (match.hasMatch()) {
            m_calendarHome = match.captured(1);
        }

        m_isAuthenticated = true;
        emit authenticated();
    });
}

void AppleCalendarAdapter::fetchEvents(const QDate& startDate, const QDate& endDate)
{
    if (!m_isAuthenticated) {
        emit errorOccurred(tr("Not authenticated"));
        return;
    }

    // CalDAV REPORT 請求取得事件
    QString reportBody = QString(R"(<?xml version="1.0" encoding="utf-8"?>
<C:calendar-query xmlns:D="DAV:" xmlns:C="urn:ietf:params:xml:ns:caldav">
    <D:prop>
        <D:getetag/>
        <C:calendar-data/>
    </D:prop>
    <C:filter>
        <C:comp-filter name="VCALENDAR">
            <C:comp-filter name="VEVENT">
                <C:time-range start="%1" end="%2"/>
            </C:comp-filter>
        </C:comp-filter>
    </C:filter>
</C:calendar-query>)")
        .arg(startDate.toString("yyyyMMdd") + "T000000Z")
        .arg(endDate.toString("yyyyMMdd") + "T235959Z");

    sendCalDAVRequest("REPORT", reportBody.toUtf8(), [this](const QByteArray& data) {
        QList<CalendarEvent> events = parseICalendar(data);
        emit eventsReceived(events);
    });
}

QList<CalendarEvent> AppleCalendarAdapter::parseICalendar(const QByteArray& data)
{
    QList<CalendarEvent> events;
    QString content = QString::fromUtf8(data);

    // 簡化的 iCalendar 解析
    QRegularExpression eventRegex("BEGIN:VEVENT([\\s\\S]*?)END:VEVENT");
    QRegularExpressionMatchIterator it = eventRegex.globalMatch(content);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString eventData = match.captured(1);

        CalendarEvent event;
        event.platform = Platform::Apple;

        // 解析 UID
        QRegularExpression uidRegex("UID:(.+)");
        QRegularExpressionMatch uidMatch = uidRegex.match(eventData);
        if (uidMatch.hasMatch()) {
            event.id = uidMatch.captured(1).trimmed();
        }

        // 解析 SUMMARY
        QRegularExpression summaryRegex("SUMMARY:(.+)");
        QRegularExpressionMatch summaryMatch = summaryRegex.match(eventData);
        if (summaryMatch.hasMatch()) {
            event.title = summaryMatch.captured(1).trimmed();
        }

        // 解析 DESCRIPTION
        QRegularExpression descRegex("DESCRIPTION:(.+)");
        QRegularExpressionMatch descMatch = descRegex.match(eventData);
        if (descMatch.hasMatch()) {
            event.description = descMatch.captured(1).trimmed();
        }

        // 解析 LOCATION
        QRegularExpression locRegex("LOCATION:(.+)");
        QRegularExpressionMatch locMatch = locRegex.match(eventData);
        if (locMatch.hasMatch()) {
            event.location = locMatch.captured(1).trimmed();
        }

        // 解析 DTSTART
        QRegularExpression startRegex("DTSTART(?:;[^:]*)?:(.+)");
        QRegularExpressionMatch startMatch = startRegex.match(eventData);
        if (startMatch.hasMatch()) {
            QString dtStart = startMatch.captured(1).trimmed();
            if (dtStart.length() == 8) {
                event.isAllDay = true;
                event.startTime = QDateTime(QDate::fromString(dtStart, "yyyyMMdd"), QTime());
            } else {
                event.startTime = QDateTime::fromString(dtStart, "yyyyMMdd'T'HHmmss'Z'");
                if (!event.startTime.isValid()) {
                    event.startTime = QDateTime::fromString(dtStart, "yyyyMMdd'T'HHmmss");
                }
            }
        }

        // 解析 DTEND
        QRegularExpression endRegex("DTEND(?:;[^:]*)?:(.+)");
        QRegularExpressionMatch endMatch = endRegex.match(eventData);
        if (endMatch.hasMatch()) {
            QString dtEnd = endMatch.captured(1).trimmed();
            if (dtEnd.length() == 8) {
                event.endTime = QDateTime(QDate::fromString(dtEnd, "yyyyMMdd"), QTime());
            } else {
                event.endTime = QDateTime::fromString(dtEnd, "yyyyMMdd'T'HHmmss'Z'");
                if (!event.endTime.isValid()) {
                    event.endTime = QDateTime::fromString(dtEnd, "yyyyMMdd'T'HHmmss");
                }
            }
        }

        // 解析 RRULE
        QRegularExpression rruleRegex("RRULE:(.+)");
        QRegularExpressionMatch rruleMatch = rruleRegex.match(eventData);
        if (rruleMatch.hasMatch()) {
            event.recurrenceRule = rruleMatch.captured(1).trimmed();
        }

        if (event.isValid()) {
            events.append(event);
        }
    }

    return events;
}

void AppleCalendarAdapter::fetchTasks()
{
    if (!m_isAuthenticated) {
        emit errorOccurred(tr("Not authenticated"));
        return;
    }

    // Apple 的提醒事項需要使用 CalDAV 的 VTODO
    QString reportBody = R"(<?xml version="1.0" encoding="utf-8"?>
<C:calendar-query xmlns:D="DAV:" xmlns:C="urn:ietf:params:xml:ns:caldav">
    <D:prop>
        <D:getetag/>
        <C:calendar-data/>
    </D:prop>
    <C:filter>
        <C:comp-filter name="VCALENDAR">
            <C:comp-filter name="VTODO"/>
        </C:comp-filter>
    </C:filter>
</C:calendar-query>)";

    sendCalDAVRequest("REPORT", reportBody.toUtf8(), [this](const QByteArray& data) {
        QList<Task> tasks;
        QString content = QString::fromUtf8(data);

        // 解析 VTODO
        QRegularExpression todoRegex("BEGIN:VTODO([\\s\\S]*?)END:VTODO");
        QRegularExpressionMatchIterator it = todoRegex.globalMatch(content);

        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            QString todoData = match.captured(1);

            Task task;
            task.platform = Platform::Apple;

            // 解析 UID
            QRegularExpression uidRegex("UID:(.+)");
            QRegularExpressionMatch uidMatch = uidRegex.match(todoData);
            if (uidMatch.hasMatch()) {
                task.id = uidMatch.captured(1).trimmed();
            }

            // 解析 SUMMARY
            QRegularExpression summaryRegex("SUMMARY:(.+)");
            QRegularExpressionMatch summaryMatch = summaryRegex.match(todoData);
            if (summaryMatch.hasMatch()) {
                task.title = summaryMatch.captured(1).trimmed();
            }

            // 解析 STATUS
            QRegularExpression statusRegex("STATUS:(.+)");
            QRegularExpressionMatch statusMatch = statusRegex.match(todoData);
            if (statusMatch.hasMatch()) {
                task.isCompleted = (statusMatch.captured(1).trimmed() == "COMPLETED");
            }

            // 解析 DUE
            QRegularExpression dueRegex("DUE(?:;[^:]*)?:(.+)");
            QRegularExpressionMatch dueMatch = dueRegex.match(todoData);
            if (dueMatch.hasMatch()) {
                QString due = dueMatch.captured(1).trimmed();
                task.dueDate = QDateTime::fromString(due, "yyyyMMdd'T'HHmmss'Z'");
                if (!task.dueDate.isValid()) {
                    task.dueDate = QDateTime::fromString(due, "yyyyMMdd'T'HHmmss");
                }
                if (!task.dueDate.isValid()) {
                    task.dueDate = QDateTime(QDate::fromString(due, "yyyyMMdd"), QTime());
                }
            }

            // 解析 PRIORITY
            QRegularExpression priorityRegex("PRIORITY:(.+)");
            QRegularExpressionMatch priorityMatch = priorityRegex.match(todoData);
            if (priorityMatch.hasMatch()) {
                int p = priorityMatch.captured(1).trimmed().toInt();
                // iCalendar priority 1-9, 轉換為 1-5
                task.priority = qBound(1, (p + 1) / 2, 5);
            }

            if (task.isValid()) {
                tasks.append(task);
            }
        }

        emit tasksReceived(tasks);
    });
}

void AppleCalendarAdapter::fetchSharedCalendars()
{
    if (!m_isAuthenticated) {
        emit errorOccurred(tr("Not authenticated"));
        return;
    }

    // 列出所有行事曆 (包含共享的)
    QByteArray propfindBody = R"(<?xml version="1.0" encoding="utf-8"?>
<D:propfind xmlns:D="DAV:" xmlns:C="urn:ietf:params:xml:ns:caldav" xmlns:CS="http://calendarserver.org/ns/">
    <D:prop>
        <D:resourcetype/>
        <D:displayname/>
        <CS:getctag/>
    </D:prop>
</D:propfind>)";

    sendCalDAVRequest("PROPFIND", propfindBody, [this](const QByteArray& data) {
        // 解析行事曆列表
        Q_UNUSED(data);
    });
}
