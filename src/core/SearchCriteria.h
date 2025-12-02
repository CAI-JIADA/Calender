#ifndef SEARCHCRITERIA_H
#define SEARCHCRITERIA_H

#include <QString>
#include <QDate>
#include <QStringList>

/**
 * @brief 搜索條件結構
 * 
 * 用於定義進階搜索的各項條件
 */
struct SearchCriteria {
    QString keyword;           ///< 搜索關鍵字
    QDate startDate;           ///< 開始日期
    QDate endDate;             ///< 結束日期
    QStringList platforms;     ///< 平台篩選 (空表示全部)
    QStringList owners;        ///< 擁有者篩選 (空表示全部)
    bool includeCompleted;     ///< 是否包含已完成的任務

    /**
     * @brief 預設建構函式
     */
    SearchCriteria()
        : includeCompleted(false)
    {}

    /**
     * @brief 建構函式
     * @param keyword 搜索關鍵字
     */
    explicit SearchCriteria(const QString& keyword)
        : keyword(keyword)
        , includeCompleted(false)
    {}

    /**
     * @brief 檢查是否有設定日期範圍
     * @return true 如果有設定日期範圍
     */
    bool hasDateRange() const {
        return startDate.isValid() && endDate.isValid();
    }

    /**
     * @brief 檢查是否有設定平台篩選
     * @return true 如果有設定平台篩選
     */
    bool hasPlatformFilter() const {
        return !platforms.isEmpty();
    }

    /**
     * @brief 檢查是否有設定擁有者篩選
     * @return true 如果有設定擁有者篩選
     */
    bool hasOwnerFilter() const {
        return !owners.isEmpty();
    }

    /**
     * @brief 檢查搜索條件是否為空
     * @return true 如果所有條件都為空
     */
    bool isEmpty() const {
        return keyword.isEmpty() &&
               !hasDateRange() &&
               !hasPlatformFilter() &&
               !hasOwnerFilter();
    }

    /**
     * @brief 重設所有搜索條件
     */
    void clear() {
        keyword.clear();
        startDate = QDate();
        endDate = QDate();
        platforms.clear();
        owners.clear();
        includeCompleted = false;
    }
};

#endif // SEARCHCRITERIA_H
