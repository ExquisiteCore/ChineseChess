#ifndef OPENINGBOOK_H
#define OPENINGBOOK_H

#include "TranspositionTable.h"
#include "../core/Position.h"
#include <QHash>
#include <QList>

// 开局库条目
struct BookEntry {
    AIMove move;
    int weight;      // 权重（出现次数或质量评分）
    int winRate;     // 胜率（百分比）

    BookEntry() : weight(1), winRate(50) {}
    BookEntry(const AIMove &m, int w = 1, int wr = 50)
        : move(m), weight(w), winRate(wr) {}
};

// 开局库管理器
class OpeningBook
{
public:
    OpeningBook(TranspositionTable *tt);

    // 选择最佳开局走法（根据权重随机选择）
    AIMove selectMove(quint64 zobristKey);

    // 添加开局走法
    void addMove(quint64 zobristKey, const AIMove &move, int weight = 1, int winRate = 50);

    // 初始化常见开局
    void initializeCommonOpenings();

    // 是否启用
    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled) { m_enabled = enabled; }

private:
    // 开局库：zobrist_key -> 可能的走法列表
    QHash<quint64, QList<BookEntry>> m_book;
    bool m_enabled;
    TranspositionTable *m_transpositionTable;

    // 辅助函数：添加对称位置
    void addSymmetricMoves(const Position &pos, const AIMove &move, int weight, int winRate);
};

#endif // OPENINGBOOK_H
