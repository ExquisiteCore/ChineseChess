#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "../core/Board.h"
#include "../core/Position.h"
#include "../core/ChessPiece.h"

// 棋子-位置评估器
class Evaluator
{
public:
    Evaluator();

    // 评估局面（正数表示红方优势，负数表示黑方优势）
    int evaluatePosition(const Position &position);

    // 快速评估（仅材料+位置）
    int evaluatePositionFast(const Position &position);

    // 获取棋子基础价值
    int getPieceBaseValue(PieceType type) const;

    // 获取棋子总价值（基础价值+位置价值）
    int getPieceValue(PieceType type, int row, int col, PieceColor color) const;

private:
    // 位置价值表（红方视角，黑方需要翻转）
    static const int PAWN_POS_VALUE[10][9];
    static const int ADVISOR_POS_VALUE[10][9];
    static const int ELEPHANT_POS_VALUE[10][9];
    static const int HORSE_POS_VALUE[10][9];
    static const int ROOK_POS_VALUE[10][9];
    static const int CANNON_POS_VALUE[10][9];
    static const int KING_POS_VALUE[10][9];
};

#endif // EVALUATOR_H
