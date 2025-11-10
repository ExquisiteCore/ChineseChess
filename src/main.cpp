#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "model/ChessBoardModel.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    // Windows下设置控制台输出为UTF-8编码，解决中文乱码
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // 创建棋盘模型实例
    auto *boardModel = new ChessBoardModel(&app);

    // 将模型暴露给 QML
    engine.rootContext()->setContextProperty("chessBoardModel", boardModel);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("ChineseChess", "Main");

    return QGuiApplication::exec();
}
