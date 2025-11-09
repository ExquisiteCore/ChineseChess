#ifndef POSITION_H
#define POSITION_H

#include "Board.h"
#include <QString>

// 局面类 - 表示完整的游戏状态
class Position
{
public:
    Position();
    explicit Position(const Board &board);

    // 获取棋盘
    Board& board() { return m_board; }
    const Board& board() const { return m_board; }

    // 当前回合
    PieceColor currentTurn() const { return m_currentTurn; }
    void setCurrentTurn(PieceColor color) { m_currentTurn = color; }

    // 切换回合
    void switchTurn();

    // 半回合计数（用于判断和棋，50回合无吃子无兵移动）
    int halfMoveClock() const { return m_halfMoveClock; }
    void setHalfMoveClock(int count) { m_halfMoveClock = count; }
    void incrementHalfMoveClock() { ++m_halfMoveClock; }
    void resetHalfMoveClock() { m_halfMoveClock = 0; }

    // 全回合计数
    int fullMoveNumber() const { return m_fullMoveNumber; }
    void setFullMoveNumber(int number) { m_fullMoveNumber = number; }
    void incrementFullMoveNumber() { ++m_fullMoveNumber; }

    // ===== FEN 格式序列化 =====
    // 中国象棋 FEN 格式示例:
    // "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1"
    //
    // 格式说明：
    // 1. 棋盘布局（从黑方第0行到红方第9行，用 / 分隔）
    // 2. 当前回合 (w=红方, b=黑方)
    // 3. 保留字段（兼容国际象棋FEN）
    // 4. 保留字段
    // 5. 半回合计数
    // 6. 全回合计数

    // 导出为 FEN 字符串
    QString toFen() const;

    // 从 FEN 字符串加载
    bool fromFen(const QString &fen);

    // 导出为简化 FEN（仅棋盘布局）
    QString toBoardFen() const;

    // 调试：打印局面信息
    void print() const;

private:
    Board m_board;              // 棋盘
    PieceColor m_currentTurn;   // 当前回合
    int m_halfMoveClock;        // 半回合计数
    int m_fullMoveNumber;       // 全回合计数
};

#endif // POSITION_H
