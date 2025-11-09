#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "ChessBoardModel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // 创建棋盘模型实例
    ChessBoardModel *boardModel = new ChessBoardModel(&app);

    // 将模型暴露给 QML
    engine.rootContext()->setContextProperty("chessBoardModel", boardModel);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("ChineseChess", "Main");

    return app.exec();
}
