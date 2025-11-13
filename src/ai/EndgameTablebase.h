#ifndef ENDGAMETABLEBASE_H
#define ENDGAMETABLEBASE_H

#include "TranspositionTable.h"
#include "../core/Position.h"
#include "../core/Board.h"
#include <QHash>
#include <QPair>

// 残局结果
enum class EndgameResult {
    Unknown,        // 未知
    Win,           // 胜
    Loss,          // 负
    Draw,          // 和
    WinInN,        // N步内胜
    LossInN        // N步内负
};

// 残局库条目
struct EndgameEntry {
    EndgameResult result;
    int movesToMate;  // 将死步数（正数表示我方胜，负数表示我方负）
    AIMove bestMove;

    EndgameEntry() : result(EndgameResult::Unknown), movesToMate(0) {}
    EndgameEntry(EndgameResult r, int moves = 0, const AIMove &m = AIMove())
        : result(r), movesToMate(moves), bestMove(m) {}
};

// 残局库管理器
class EndgameTablebase
{
public:
    EndgameTablebase();

    // 查询残局库
    EndgameEntry probe(const Position &position);

    // 判断是否是残局
    bool isEndgame(const Position &position) const;

    // 评估残局（返回精确分数）
    int evaluateEndgame(const Position &position);

    // 是否启用
    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled) { m_enabled = enabled; }

    // 统计信息
    int getCacheSize() const { return m_cache.size(); }

private:
    // 残局库缓存
    QHash<quint64, EndgameEntry> m_cache;
    bool m_enabled;

    // 特殊残局识别和处理
    EndgameEntry recognizeSpecialEndgame(const Position &position);

    // 计算棋子数量
    int countPieces(const Board &board, PieceColor color) const;
    int countTotalPieces(const Board &board) const;

    // 特定残局处理函数
    EndgameEntry handleKingVsKing(const Position &position);
    EndgameEntry handleRookVsMinor(const Position &position);
    EndgameEntry handlePawnEndgame(const Position &position);

    // 评估残局分数
    int evaluateKingPawnEndgame(const Position &position);
    int evaluateRookEndgame(const Position &position);
};

#endif // ENDGAMETABLEBASE_H
