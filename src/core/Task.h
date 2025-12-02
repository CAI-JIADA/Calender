#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QDateTime>
#include <QStringList>
#include "Platform.h"

/**
 * @brief 任務/待辦事項資料結構
 * 
 * 用於表示來自不同平台的待辦事項
 */
struct Task {
    QString id;              ///< 任務唯一識別碼
    QString title;           ///< 任務標題
    QString description;     ///< 任務描述
    QDateTime dueDate;       ///< 到期日期
    Platform platform;       ///< 來源平台
    QString ownerId;         ///< 任務擁有者
    bool isCompleted;        ///< 是否已完成
    int priority;            ///< 優先級 (1-5, 1為最高)
    QStringList tags;        ///< 標籤列表

    /**
     * @brief 預設建構函式
     */
    Task()
        : platform(Platform::Google)
        , isCompleted(false)
        , priority(3)
    {}

    /**
     * @brief 建構函式
     * @param id 任務識別碼
     * @param title 任務標題
     * @param dueDate 到期日期
     * @param platform 來源平台
     */
    Task(const QString& id, const QString& title,
         const QDateTime& dueDate, Platform platform)
        : id(id)
        , title(title)
        , dueDate(dueDate)
        , platform(platform)
        , isCompleted(false)
        , priority(3)
    {}

    /**
     * @brief 檢查任務是否有效
     * @return true 如果任務有效
     */
    bool isValid() const {
        return !id.isEmpty() && !title.isEmpty();
    }

    /**
     * @brief 檢查任務是否已過期
     * @return true 如果任務已過期
     */
    bool isOverdue() const {
        if (!dueDate.isValid()) {
            return false;
        }
        return dueDate < QDateTime::currentDateTime() && !isCompleted;
    }

    /**
     * @brief 檢查任務是否在指定日期到期
     * @param date 目標日期
     * @return true 如果任務在該日期到期
     */
    bool isDueOn(const QDate& date) const {
        if (!dueDate.isValid()) {
            return false;
        }
        return dueDate.date() == date;
    }

    /**
     * @brief 比較運算子 (依到期日期和優先級排序)
     */
    bool operator<(const Task& other) const {
        // 未完成的任務優先
        if (isCompleted != other.isCompleted) {
            return !isCompleted;
        }
        // 優先級高的優先 (數字小的優先)
        if (priority != other.priority) {
            return priority < other.priority;
        }
        // 到期日早的優先
        return dueDate < other.dueDate;
    }

    /**
     * @brief 相等運算子
     */
    bool operator==(const Task& other) const {
        return id == other.id && platform == other.platform;
    }
};

#endif // TASK_H
