#include "MainWindow.h"
#include "storage/DatabaseManager.h"
#include "adapters/GoogleCalendarAdapter.h"
#include "adapters/AppleCalendarAdapter.h"
#include "adapters/OutlookCalendarAdapter.h"
#include <QQmlContext>
#include <QQuickWidget>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QApplication>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_calendarManager(new CalendarManager(this))
    , m_syncManager(new SyncManager(m_calendarManager, this))
    , m_searchEngine(new SearchEngine(m_calendarManager, this))
    , m_qmlEngine(new QQmlApplicationEngine(this))
{
    // 初始化資料庫
    DatabaseManager::instance().initialize();

    // 註冊 QML 類型
    registerQmlTypes();

    // 設定 UI
    setupUI();

    // 設定信號連接
    setupConnections();

    // 設定視窗屬性
    setWindowTitle(tr("行事曆整合工具"));
    setMinimumSize(1200, 800);
}

MainWindow::~MainWindow()
{
    DatabaseManager::instance().close();
}

void MainWindow::setupUI()
{
    // 建立選單列
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    // 檔案選單
    QMenu* fileMenu = menuBar->addMenu(tr("檔案(&F)"));
    
    QAction* syncAction = fileMenu->addAction(tr("同步(&S)"));
    syncAction->setShortcut(QKeySequence("Ctrl+S"));
    connect(syncAction, &QAction::triggered, m_syncManager, &SyncManager::syncAll);

    fileMenu->addSeparator();

    QAction* quitAction = fileMenu->addAction(tr("結束(&Q)"));
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    // 檢視選單
    QMenu* viewMenu = menuBar->addMenu(tr("檢視(&V)"));
    
    QAction* todayAction = viewMenu->addAction(tr("今天(&T)"));
    todayAction->setShortcut(QKeySequence("Ctrl+T"));

    viewMenu->addSeparator();

    QAction* monthViewAction = viewMenu->addAction(tr("月檢視(&M)"));
    monthViewAction->setShortcut(QKeySequence("Ctrl+M"));

    QAction* weekViewAction = viewMenu->addAction(tr("週檢視(&W)"));
    weekViewAction->setShortcut(QKeySequence("Ctrl+W"));

    QAction* dayViewAction = viewMenu->addAction(tr("日檢視(&D)"));
    dayViewAction->setShortcut(QKeySequence("Ctrl+D"));

    // 帳號選單
    QMenu* accountMenu = menuBar->addMenu(tr("帳號(&A)"));
    
    QAction* addGoogleAction = accountMenu->addAction(tr("新增 Google 帳號"));
    QAction* addAppleAction = accountMenu->addAction(tr("新增 Apple 帳號"));
    QAction* addOutlookAction = accountMenu->addAction(tr("新增 Outlook 帳號"));

    // 說明選單
    QMenu* helpMenu = menuBar->addMenu(tr("說明(&H)"));
    
    QAction* aboutAction = helpMenu->addAction(tr("關於(&A)"));

    // 建立 QML 視圖
    QQuickWidget* quickWidget = new QQuickWidget(m_qmlEngine, this);
    quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    quickWidget->setSource(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    setCentralWidget(quickWidget);

    // 建立狀態列
    statusBar()->showMessage(tr("就緒"));

    // 連接同步狀態
    connect(m_syncManager, &SyncManager::syncStarted, this, [this]() {
        statusBar()->showMessage(tr("正在同步..."));
    });

    connect(m_syncManager, &SyncManager::syncCompleted, this, [this]() {
        statusBar()->showMessage(tr("同步完成 - %1")
            .arg(m_syncManager->lastSyncTime().toString("yyyy-MM-dd HH:mm:ss")));
    });

    connect(m_syncManager, &SyncManager::syncError, this, [this](const QString& error) {
        statusBar()->showMessage(tr("同步錯誤: %1").arg(error));
    });
}

void MainWindow::setupConnections()
{
    // 資料變更時的處理
    connect(m_calendarManager, &CalendarManager::dataChanged, this, [this]() {
        // 更新 UI
    });
}

void MainWindow::registerQmlTypes()
{
    // 註冊 C++ 物件到 QML
    m_qmlEngine->rootContext()->setContextProperty("calendarManager", m_calendarManager);
    m_qmlEngine->rootContext()->setContextProperty("syncManager", m_syncManager);
    m_qmlEngine->rootContext()->setContextProperty("searchEngine", m_searchEngine);
}
