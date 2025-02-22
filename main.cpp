#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    qmlRegisterType<MainWindow>("BinarySplitter", 1, 0, "MainWindow");

    MainWindow mainWindow;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("mainWindow", &mainWindow);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
