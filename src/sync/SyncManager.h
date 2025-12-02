#ifndef SYNCMANAGER_H
#define SYNCMANAGER_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include "adapters/CalendarAdapter.h"
#include "core/CalendarManager.h"

/**
 * @brief 同步管理器
 * 
 * 負責管理各平台的資料同步
 */
class SyncManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isSyncing READ isSyncing NOTIFY syncingChanged)
    Q_PROPERTY(QDateTime lastSyncTime READ lastSyncTime NOTIFY lastSyncTimeChanged)

public:
    /**
     * @brief 建構函式
     * @param calendarManager 行事曆管理器
     * @param parent 父物件
     */
    explicit SyncManager(CalendarManager* calendarManager, QObject* parent = nullptr);

    /**
     * @brief 解構函式
     */
    ~SyncManager() override;

    /**
     * @brief 註冊適配器
     * @param adapter 行事曆適配器
     */
    void registerAdapter(CalendarAdapter* adapter);

    /**
     * @brief 取消註冊適配器
     * @param platform 平台類型
     */
    void unregisterAdapter(Platform platform);

    /**
     * @brief 取得適配器
     * @param platform 平台類型
     * @return 適配器指標，如果不存在則返回 nullptr
     */
    CalendarAdapter* adapter(Platform platform) const;

    /**
     * @brief 檢查是否正在同步
     * @return true 如果正在同步
     */
    bool isSyncing() const { return m_isSyncing; }

    /**
     * @brief 取得最後同步時間
     * @return 最後同步時間
     */
    QDateTime lastSyncTime() const { return m_lastSyncTime; }

    /**
     * @brief 設定自動同步間隔
     * @param minutes 分鐘數
     */
    void setAutoSyncInterval(int minutes);

    /**
     * @brief 啟用/停用自動同步
     * @param enabled 是否啟用
     */
    void setAutoSyncEnabled(bool enabled);

public slots:
    /**
     * @brief 同步所有平台
     */
    void syncAll();

    /**
     * @brief 同步指定平台
     * @param platform 平台類型
     */
    void syncPlatform(Platform platform);

signals:
    /**
     * @brief 同步開始信號
     */
    void syncStarted();

    /**
     * @brief 同步完成信號
     */
    void syncCompleted();

    /**
     * @brief 同步錯誤信號
     * @param error 錯誤訊息
     */
    void syncError(const QString& error);

    /**
     * @brief 同步進度信號
     * @param platform 平台名稱
     * @param progress 進度 (0-100)
     */
    void syncProgress(const QString& platform, int progress);

    /**
     * @brief 同步狀態變更信號
     */
    void syncingChanged();

    /**
     * @brief 最後同步時間變更信號
     */
    void lastSyncTimeChanged();

private slots:
    void onEventsReceived(const QList<CalendarEvent>& events);
    void onTasksReceived(const QList<Task>& tasks);
    void onAdapterError(const QString& error);

private:
    CalendarManager* m_calendarManager;
    QMap<Platform, CalendarAdapter*> m_adapters;
    QTimer* m_autoSyncTimer;
    bool m_isSyncing;
    QDateTime m_lastSyncTime;
    int m_pendingSyncs;
    QDate m_syncStartDate;
    QDate m_syncEndDate;
};

#endif // SYNCMANAGER_H
