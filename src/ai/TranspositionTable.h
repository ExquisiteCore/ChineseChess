#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#include "../core/Position.h"
#include <QHash>
#include <QtTypes>

// 移动结构（包含评分）
struct AIMove {
    int fromRow, fromCol;
    int toRow, toCol;
    int score;

    AIMove() : fromRow(-1), fromCol(-1), toRow(-1), toCol(-1), score(0) {}
    AIMove(int fr, int fc, int tr, int tc, int s = 0)
        : fromRow(fr), fromCol(fc), toRow(tr), toCol(tc), score(s) {}

    bool isValid() const { return fromRow >= 0 && fromCol >= 0 && toRow >= 0 && toCol >= 0; }
};

// 置换表项
struct TTEntry {
    quint64 zobristKey;
    int depth;
    int score;
    enum Flag { EXACT, LOWER_BOUND, UPPER_BOUND } flag;
    AIMove bestMove;

    TTEntry() : zobristKey(0), depth(-1), score(0), flag(EXACT) {}
};

// 置换表管理器（使用Zobrist哈希）
class TranspositionTable
{
public:
    TranspositionTable();

    // 初始化Zobrist键值
    void initialize();

    // 计算局面的Zobrist哈希值
    quint64 computeZobristKey(const Position &position);

    // 查询置换表
    bool probe(quint64 key, int depth, int alpha, int beta, int &score);

    // 存储到置换表
    void store(quint64 key, int depth, int score, TTEntry::Flag flag, const AIMove &bestMove);

    // 获取最佳移动（如果有）
    AIMove* getBestMove(quint64 key);

    // 清空置换表
    void clear();

    // 获取命中次数
    int getHits() const { return m_hits; }

    // 重置统计信息
    void resetStatistics();

private:
    QHash<quint64, TTEntry> m_table;
    quint64 m_zobristTable[10][9][14];  // [row][col][piece]
    bool m_initialized;
    int m_hits;

    static constexpr int TT_SIZE = 1000000;  // 约100万个表项
};

#endif // TRANSPOSITIONTABLE_H
