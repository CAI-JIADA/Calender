#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QTranslator>
#include <QLocale>
#include <QIcon>
#include "ui/MainWindow.h"
#include "storage/DatabaseManager.h"

int main(int argc, char *argv[])
{
    // 設定高 DPI 縮放
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);

    // 設定應用程式資訊
    app.setApplicationName(QStringLiteral("CalendarIntegration"));
    app.setApplicationDisplayName(QObject::tr("行事曆整合工具"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));
    app.setOrganizationName(QStringLiteral("CalendarIntegration"));
    app.setOrganizationDomain(QStringLiteral("calendar.local"));

    // 設定 Quick Controls 樣式
    QQuickStyle::setStyle(QStringLiteral("Material"));

    // 載入翻譯
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "CalendarIntegration_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    // 建立主視窗
    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
