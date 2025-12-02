#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QQmlApplicationEngine>
#include "core/CalendarManager.h"
#include "sync/SyncManager.h"
#include "search/SearchEngine.h"

class QQuickWidget;

/**
 * @brief 主視窗
 * 
 * 應用程式的主視窗，整合 QML UI
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 建構函式
     * @param parent 父物件
     */
    explicit MainWindow(QWidget* parent = nullptr);

    /**
     * @brief 解構函式
     */
    ~MainWindow() override;

    /**
     * @brief 取得行事曆管理器
     * @return 行事曆管理器
     */
    CalendarManager* calendarManager() const { return m_calendarManager; }

    /**
     * @brief 取得同步管理器
     * @return 同步管理器
     */
    SyncManager* syncManager() const { return m_syncManager; }

    /**
     * @brief 取得搜索引擎
     * @return 搜索引擎
     */
    SearchEngine* searchEngine() const { return m_searchEngine; }

private:
    void setupUI();
    void setupConnections();
    void registerQmlTypes();

    CalendarManager* m_calendarManager;
    SyncManager* m_syncManager;
    SearchEngine* m_searchEngine;
    QQmlApplicationEngine* m_qmlEngine;
};

#endif // MAINWINDOW_H
