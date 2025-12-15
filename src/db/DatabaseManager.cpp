#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

QString DatabaseManager::getDatabasePath() const
{
    // 获取应用数据目录
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    // 确保目录存在
    QDir dir;
    if (!dir.exists(appDataPath)) {
        dir.mkpath(appDataPath);
    }

    return appDataPath + "/chess_saves.db";
}

bool DatabaseManager::initialize()
{
    QString dbPath = getDatabasePath();
    qDebug() << "数据库路径:" << dbPath;

    // 创建数据库连接
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(dbPath);

    if (!m_database.open()) {
        qCritical() << "无法打开数据库:" << m_database.lastError().text();
        return false;
    }

    qDebug() << "数据库连接成功";

    // 创建表结构
    if (!createTables()) {
        qCritical() << "创建表失败";
        return false;
    }

    return true;
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_database);

    // 创建游戏存档表
    QString createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS game_saves (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            fen_string TEXT NOT NULL,
            game_mode TEXT NOT NULL,
            is_red_turn INTEGER NOT NULL,
            move_count INTEGER NOT NULL,
            move_history TEXT,
            save_time DATETIME DEFAULT CURRENT_TIMESTAMP,
            description TEXT,
            is_auto_save INTEGER DEFAULT 0,
            is_game_over INTEGER DEFAULT 0
        )
    )";

    if (!query.exec(createTableSQL)) {
        qCritical() << "创建表失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "数据库表创建成功";
    return true;
}

bool DatabaseManager::saveGame(const QString &fenString,
                                const QString &gameMode,
                                bool isRedTurn,
                                int moveCount,
                                const QString &moveHistory,
                                const QString &description)
{
    QSqlQuery query(m_database);

    query.prepare(R"(
        INSERT INTO game_saves (fen_string, game_mode, is_red_turn, move_count,
                                move_history, description, is_auto_save)
        VALUES (:fen, :mode, :turn, :count, :history, :desc, 0)
    )");

    query.bindValue(":fen", fenString);
    query.bindValue(":mode", gameMode);
    query.bindValue(":turn", isRedTurn ? 1 : 0);
    query.bindValue(":count", moveCount);
    query.bindValue(":history", moveHistory);
    query.bindValue(":desc", description);

    if (!query.exec()) {
        qCritical() << "保存游戏失败:" << query.lastError().text();
        emit saveCompleted(false);
        return false;
    }

    qDebug() << "游戏保存成功, ID:" << query.lastInsertId();
    emit saveCompleted(true);
    return true;
}

bool DatabaseManager::autoSaveGame(const QString &fenString,
                                    const QString &gameMode,
                                    bool isRedTurn,
                                    int moveCount,
                                    const QString &moveHistory,
                                    bool isGameOver)
{
    QSqlQuery query(m_database);

    // 先删除该游戏模式的旧自动存档
    query.prepare("DELETE FROM game_saves WHERE is_auto_save = 1 AND game_mode = :mode");
    query.bindValue(":mode", gameMode);
    query.exec();

    // 插入新的自动存档
    query.prepare(R"(
        INSERT INTO game_saves (fen_string, game_mode, is_red_turn, move_count,
                                move_history, description, is_auto_save, is_game_over)
        VALUES (:fen, :mode, :turn, :count, :history, '自动存档', 1, :gameover)
    )");

    query.bindValue(":fen", fenString);
    query.bindValue(":mode", gameMode);
    query.bindValue(":turn", isRedTurn ? 1 : 0);
    query.bindValue(":count", moveCount);
    query.bindValue(":history", moveHistory);
    query.bindValue(":gameover", isGameOver ? 1 : 0);

    if (!query.exec()) {
        qCritical() << "自动保存失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "自动保存成功 [模式:" << gameMode << ", 游戏结束:" << isGameOver << "]";
    return true;
}

bool DatabaseManager::hasAutoSave(const QString &gameMode) const
{
    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM game_saves WHERE is_auto_save = 1 AND game_mode = :mode AND is_game_over = 0");
    query.bindValue(":mode", gameMode);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

GameSave DatabaseManager::loadAutoSave(const QString &gameMode)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT id, fen_string, game_mode, is_red_turn, move_count,
               move_history, save_time, description, is_game_over
        FROM game_saves
        WHERE is_auto_save = 1 AND game_mode = :mode AND is_game_over = 0
        ORDER BY save_time DESC
        LIMIT 1
    )");
    query.bindValue(":mode", gameMode);

    GameSave save;

    if (query.exec() && query.next()) {
        save.id = query.value(0).toInt();
        save.fenString = query.value(1).toString();
        save.gameMode = query.value(2).toString();
        save.isRedTurn = query.value(3).toInt() == 1;
        save.moveCount = query.value(4).toInt();
        save.moveHistory = query.value(5).toString();
        save.saveTime = query.value(6).toDateTime();
        save.description = query.value(7).toString();
        save.isGameOver = query.value(8).toInt() == 1;

        qDebug() << "加载自动存档成功 [模式:" << gameMode << ", ID:" << save.id << "]";
        emit loadCompleted(true);
    } else {
        qWarning() << "没有找到自动存档 [模式:" << gameMode << "]";
        emit loadCompleted(false);
    }

    return save;
}

GameSave DatabaseManager::loadGame(int saveId)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT id, fen_string, game_mode, is_red_turn, move_count,
               move_history, save_time, description
        FROM game_saves
        WHERE id = :id
    )");

    query.bindValue(":id", saveId);

    GameSave save;

    if (query.exec() && query.next()) {
        save.id = query.value(0).toInt();
        save.fenString = query.value(1).toString();
        save.gameMode = query.value(2).toString();
        save.isRedTurn = query.value(3).toInt() == 1;
        save.moveCount = query.value(4).toInt();
        save.moveHistory = query.value(5).toString();
        save.saveTime = query.value(6).toDateTime();
        save.description = query.value(7).toString();

        qDebug() << "加载存档成功:" << save.id;
        emit loadCompleted(true);
    } else {
        qCritical() << "加载存档失败:" << query.lastError().text();
        emit loadCompleted(false);
    }

    return save;
}

QList<GameSave> DatabaseManager::getAllSaves()
{
    QList<GameSave> saves;

    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT id, fen_string, game_mode, is_red_turn, move_count,
               move_history, save_time, description
        FROM game_saves
        WHERE is_auto_save = 0
        ORDER BY save_time DESC
    )");

    if (query.exec()) {
        while (query.next()) {
            GameSave save;
            save.id = query.value(0).toInt();
            save.fenString = query.value(1).toString();
            save.gameMode = query.value(2).toString();
            save.isRedTurn = query.value(3).toInt() == 1;
            save.moveCount = query.value(4).toInt();
            save.moveHistory = query.value(5).toString();
            save.saveTime = query.value(6).toDateTime();
            save.description = query.value(7).toString();

            saves.append(save);
        }

        qDebug() << "获取存档列表成功，共" << saves.size() << "个";
    } else {
        qCritical() << "获取存档列表失败:" << query.lastError().text();
    }

    return saves;
}

bool DatabaseManager::deleteSave(int saveId)
{
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM game_saves WHERE id = :id");
    query.bindValue(":id", saveId);

    if (!query.exec()) {
        qCritical() << "删除存档失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "删除存档成功:" << saveId;
    return true;
}

bool DatabaseManager::clearAllSaves()
{
    QSqlQuery query(m_database);

    if (!query.exec("DELETE FROM game_saves")) {
        qCritical() << "清空存档失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "所有存档已清空";
    return true;
}

bool DatabaseManager::clearAutoSave(const QString &gameMode)
{
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM game_saves WHERE is_auto_save = 1 AND game_mode = :mode");
    query.bindValue(":mode", gameMode);

    if (!query.exec()) {
        qCritical() << "清除自动存档失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "自动存档已清除 [模式:" << gameMode << "]";
    return true;
}
