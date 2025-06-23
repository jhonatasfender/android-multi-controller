#include <QGuiApplication>
#include <QIcon>
#include "core/logging/logger.h"
#include "presentation/main_window.h"

int main(int argc, char* argv[])
{
    const QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icons/app_icon.png"));

    Logger::getInstance()->initialize();

    QQmlApplicationEngine engine;

    presentation::MainWindow mainWindow;
    mainWindow.initialize(engine);

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app, [url](const QObject* obj, const QUrl& objUrl)
        {
            if (!obj && url == objUrl)
            {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection
    );
    engine.load(url);

    return app.exec();
}
