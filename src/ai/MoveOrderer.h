#ifndef MOVEORDERER_H
#define MOVEORDERER_H

#include "TranspositionTable.h"
#include "Evaluator.h"
#include "../core/Position.h"
#include <QList>
#include <cstring>
#include <optional>

// 移动排序器（使用多种启发式）
class MoveOrderer
{
public:
    MoveOrderer(Evaluator *evaluator);

    // 对移动列表进行排序
    void sortMoves(QList<AIMove> &moves, const Position &position, int depth, const std::optional<AIMove> &ttMove = std::nullopt);

    // 更新杀手移动
    void updateKillerMove(const AIMove &move, int depth);

    // 更新历史表
    void updateHistory(const AIMove &move, int depth);

    // 重置所有启发式数据
    void reset();

private:
    // 快速评估移动价值（用于排序）
    int quickEvaluateMove(const Position &position, const AIMove &move, int depth, const std::optional<AIMove> &ttMove);

    // 杀手移动启发（每层保存2个杀手移动）
    AIMove m_killerMoves[10][2];

    // 历史启发
    int m_historyTable[10][9][10][9];  // [fromRow][fromCol][toRow][toCol]

    Evaluator *m_evaluator;
};

#endif // MOVEORDERER_H
