#ifndef CALENDAREVENT_H
#define CALENDAREVENT_H

#include <QString>
#include <QDateTime>
#include <QStringList>
#include <QColor>
#include "Platform.h"

/**
 * @brief 統一的事件資料結構
 * 
 * 用於表示來自不同平台的行事曆事件
 */
struct CalendarEvent {
    QString id;              ///< 事件唯一識別碼
    QString title;           ///< 事件標題
    QString description;     ///< 事件描述
    QDateTime startTime;     ///< 開始時間
    QDateTime endTime;       ///< 結束時間
    QString location;        ///< 地點
    Platform platform;       ///< 來源平台
    QString ownerId;         ///< 事件擁有者
    bool isAllDay;           ///< 是否為全天事件
    QStringList attendees;   ///< 參與者列表
    QString recurrenceRule;  ///< 重複規則 (RFC 5545 RRULE)
    QColor color;            ///< 事件顏色

    /**
     * @brief 預設建構函式
     */
    CalendarEvent()
        : platform(Platform::Google)
        , isAllDay(false)
        , color(Qt::blue)
    {}

    /**
     * @brief 建構函式
     * @param id 事件識別碼
     * @param title 事件標題
     * @param startTime 開始時間
     * @param endTime 結束時間
     * @param platform 來源平台
     */
    CalendarEvent(const QString& id, const QString& title,
                  const QDateTime& startTime, const QDateTime& endTime,
                  Platform platform)
        : id(id)
        , title(title)
        , startTime(startTime)
        , endTime(endTime)
        , platform(platform)
        , isAllDay(false)
        , color(Qt::blue)
    {}

    /**
     * @brief 檢查事件是否有效
     * @return true 如果事件有效
     */
    bool isValid() const {
        return !id.isEmpty() && !title.isEmpty() && startTime.isValid();
    }

    /**
     * @brief 檢查事件是否在指定日期範圍內
     * @param start 開始日期
     * @param end 結束日期
     * @return true 如果事件在範圍內
     */
    bool isInDateRange(const QDate& start, const QDate& end) const {
        QDate eventStart = startTime.date();
        QDate eventEnd = endTime.isValid() ? endTime.date() : eventStart;
        return (eventStart <= end) && (eventEnd >= start);
    }

    /**
     * @brief 比較運算子 (依開始時間排序)
     */
    bool operator<(const CalendarEvent& other) const {
        return startTime < other.startTime;
    }

    /**
     * @brief 相等運算子
     */
    bool operator==(const CalendarEvent& other) const {
        return id == other.id && platform == other.platform;
    }
};

#endif // CALENDAREVENT_H
