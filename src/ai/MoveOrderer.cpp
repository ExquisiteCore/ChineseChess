#include "MoveOrderer.h"
#include <algorithm>

MoveOrderer::MoveOrderer(Evaluator *evaluator)
    : m_evaluator(evaluator)
{
    reset();
}

void MoveOrderer::sortMoves(QList<AIMove> &moves, const Position &position, int depth, const AIMove *ttMove)
{
    // 使用快速评估给每个移动打分
    for (AIMove &move : moves) {
        move.score = quickEvaluateMove(position, move, depth, ttMove);
    }

    // 按分数降序排序
    std::sort(moves.begin(), moves.end(), [](const AIMove &a, const AIMove &b) {
        return a.score > b.score;
    });
}

int MoveOrderer::quickEvaluateMove(const Position &position, const AIMove &move, int depth, const AIMove *ttMove)
{
    int score = 0;

    // 1. 置换表移动（最高优先级）
    if (ttMove && ttMove->fromRow == move.fromRow && ttMove->fromCol == move.fromCol &&
        ttMove->toRow == move.toRow && ttMove->toCol == move.toCol) {
        return 1000000;
    }

    // 2. 杀手移动
    if (depth < 10) {
        if ((m_killerMoves[depth][0].fromRow == move.fromRow && m_killerMoves[depth][0].fromCol == move.fromCol &&
             m_killerMoves[depth][0].toRow == move.toRow && m_killerMoves[depth][0].toCol == move.toCol) ||
            (m_killerMoves[depth][1].fromRow == move.fromRow && m_killerMoves[depth][1].fromCol == move.fromCol &&
             m_killerMoves[depth][1].toRow == move.toRow && m_killerMoves[depth][1].toCol == move.toCol)) {
            score += 500000;
        }
    }

    // 3. MVV-LVA（Most Valuable Victim - Least Valuable Attacker）
    const ChessPiece *target = position.board().pieceAt(move.toRow, move.toCol);
    if (target && target->isValid()) {
        const ChessPiece *attacker = position.board().pieceAt(move.fromRow, move.fromCol);
        if (attacker && attacker->isValid()) {
            score += m_evaluator->getPieceBaseValue(target->type()) * 10
                   - m_evaluator->getPieceBaseValue(attacker->type());
        }
    }

    // 4. 历史启发
    score += m_historyTable[move.fromRow][move.fromCol][move.toRow][move.toCol];

    return score;
}

void MoveOrderer::updateKillerMove(const AIMove &move, int depth)
{
    if (depth >= 10) return;

    // 如果不是当前第一个杀手移动，则更新
    if (m_killerMoves[depth][0].fromRow != move.fromRow ||
        m_killerMoves[depth][0].fromCol != move.fromCol ||
        m_killerMoves[depth][0].toRow != move.toRow ||
        m_killerMoves[depth][0].toCol != move.toCol) {

        m_killerMoves[depth][1] = m_killerMoves[depth][0];
        m_killerMoves[depth][0] = move;
    }
}

void MoveOrderer::updateHistory(const AIMove &move, int depth)
{
    m_historyTable[move.fromRow][move.fromCol][move.toRow][move.toCol] += depth * depth;
}

void MoveOrderer::reset()
{
    // 初始化历史表和杀手移动
    memset(m_historyTable, 0, sizeof(m_historyTable));
    for (int i = 0; i < 10; i++) {
        m_killerMoves[i][0] = AIMove();
        m_killerMoves[i][1] = AIMove();
    }
}
