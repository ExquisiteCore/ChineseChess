#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QString>
#include <QSqlDatabase>
#include <QDateTime>
#include <QList>

// 游戏存档结构
struct GameSave {
    int id;                      // 存档ID
    QString fenString;           // 棋局FEN字符串
    QString gameMode;            // 游戏模式 (single/two)
    bool isRedTurn;              // 是否红方回合
    int moveCount;               // 步数
    QString moveHistory;         // 移动历史（JSON格式）
    QDateTime saveTime;          // 保存时间
    QString description;         // 存档描述

    GameSave() : id(0), isRedTurn(true), moveCount(0) {}
};

// 数据库管理器类
class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    // 初始化数据库
    bool initialize();

    // 保存游戏
    bool saveGame(const QString &fenString,
                  const QString &gameMode,
                  bool isRedTurn,
                  int moveCount,
                  const QString &moveHistory,
                  const QString &description = QString());

    // 自动保存（覆盖最近的自动存档）
    bool autoSaveGame(const QString &fenString,
                      const QString &gameMode,
                      bool isRedTurn,
                      int moveCount,
                      const QString &moveHistory);

    // 加载最近的自动存档
    GameSave loadAutoSave();

    // 加载指定存档
    GameSave loadGame(int saveId);

    // 获取所有手动存档列表
    QList<GameSave> getAllSaves();

    // 删除存档
    bool deleteSave(int saveId);

    // 清空所有存档
    bool clearAllSaves();

    // 检查是否有自动存档
    bool hasAutoSave() const;

signals:
    void saveCompleted(bool success);
    void loadCompleted(bool success);

private:
    QSqlDatabase m_database;

    // 创建表结构
    bool createTables();

    // 获取数据库路径
    QString getDatabasePath() const;
};

#endif // DATABASEMANAGER_H
