#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include "Position.h"
#include <QObject>
#include <QStack>
#include <QString>

// 游戏状态枚举
enum class GameState {
    NotStarted,    // 未开始
    InProgress,    // 进行中
    Check,         // 将军
    Checkmate,     // 将死
    Stalemate,     // 困毙（和棋）
    Draw           // 其他和棋情况（如长将等）
};

// 移动记录结构
struct MoveRecord {
    QString fen;           // 移动后的 FEN 字符串
    QString notation;      // 移动的记谱法表示（如"炮二平五"）
    int fromRow, fromCol;  // 起始位置
    int toRow, toCol;      // 目标位置
    QString capturedPiece; // 被吃掉的棋子（如果有）

    MoveRecord() : fromRow(0), fromCol(0), toRow(0), toCol(0) {}

    MoveRecord(const QString &f, const QString &n, int fr, int fc, int tr, int tc, const QString &cp)
        : fen(f), notation(n), fromRow(fr), fromCol(fc), toRow(tr), toCol(tc), capturedPiece(cp) {}
};

// 游戏控制器类
class GameController : public QObject
{
    Q_OBJECT

public:
    explicit GameController(QObject *parent = nullptr);

    // 开始新游戏
    void startNewGame(const QString &initialFen = "");

    // 记录一步棋
    void recordMove(const Position &position, const ChessPiece &movedPiece, int fromRow, int fromCol, int toRow, int toCol, const QString &capturedPiece = "");

    // 悔棋
    bool undo(Position &position);

    // 重做（撤销悔棋）
    bool redo(Position &position);

    // 判断是否可以悔棋
    bool canUndo() const;

    // 判断是否可以重做
    bool canRedo() const;

    // 获取历史记录
    const QStack<MoveRecord>& getMoveHistory() const { return m_moveHistory; }

    // 获取当前步数
    int getCurrentMoveNumber() const { return m_moveHistory.size(); }

    // 获取历史记录列表（用于显示）
    QStringList getMoveHistoryList() const;

    // 获取游戏状态
    GameState getGameState() const { return m_gameState; }

    // 设置游戏状态
    void setGameState(GameState state);

    // 获取状态描述
    QString getGameStateDescription() const;

    // 清空历史记录
    void clearHistory();

    // 导出 PGN 格式（便于保存对局）
    QString exportToPGN() const;

signals:
    void moveHistoryChanged();
    void gameStateChanged(GameState state);
    void undoAvailableChanged(bool available);
    void redoAvailableChanged(bool available);

private:
    QStack<MoveRecord> m_moveHistory;  // 移动历史栈
    QStack<MoveRecord> m_redoStack;    // 重做栈
    GameState m_gameState;              // 游戏状态
    QString m_initialFen;               // 初始局面 FEN

    // 生成记谱法表示
    QString generateNotation(const ChessPiece &piece, int fromRow, int fromCol, int toRow, int toCol) const;
};

#endif // GAMECONTROLLER_H
