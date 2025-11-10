#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include "model/ChessBoardModel.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

// 自定义消息处理器 - 输出到文件和控制台
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QFile logFile("chess_debug.log");
    static bool opened = false;

    if (!opened) {
        if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            opened = true;
        }
    }

    QTextStream out(&logFile);

    // 添加时间戳
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

    QString typeStr;
    switch (type) {
    case QtDebugMsg:
        typeStr = "[DEBUG]";
        break;
    case QtInfoMsg:
        typeStr = "[INFO]";
        break;
    case QtWarningMsg:
        typeStr = "[WARN]";
        break;
    case QtCriticalMsg:
        typeStr = "[CRITICAL]";
        break;
    case QtFatalMsg:
        typeStr = "[FATAL]";
        break;
    }

    // 输出到文件
    QString fullMsg = QString("%1 %2 %3").arg(timestamp, typeStr, msg);
    out << fullMsg << "\n";
    out.flush();

    // 同时输出到标准输出（控制台）
    fprintf(stdout, "%s\n", fullMsg.toUtf8().constData());
    fflush(stdout);
}

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    // Windows下设置控制台输出为UTF-8编码，解决中文乱码
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // 设置Qt Quick Controls样式为Basic，避免样式自定义警告
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");

    // 安装消息处理器，将qDebug输出到文件
    qInstallMessageHandler(messageHandler);

    qDebug() << "========== 中国象棋游戏启动 ==========";

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

    qDebug() << "========== QML引擎加载完成 ==========";

    return QGuiApplication::exec();
}
