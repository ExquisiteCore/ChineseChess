#include "Evaluator.h"
#include "../core/ChessRules.h"
#include <algorithm>

Evaluator::Evaluator()
{
}

int Evaluator::evaluatePosition(const Position &position)
{
    return evaluatePositionFast(position);
}

int Evaluator::evaluatePositionFast(const Position &position)
{
    int score = 0;

    // 检查将军状态
    if (ChessRules::isInCheck(position.board(), PieceColor::Red)) {
        score -= 50;
    }
    if (ChessRules::isInCheck(position.board(), PieceColor::Black)) {
        score += 50;
    }

    // 计算材料和位置价值
    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const ChessPiece *piece = position.board().pieceAt(row, col);
            if (piece && piece->isValid()) {
                int value = getPieceValue(piece->type(), row, col, piece->color());

                if (piece->color() == PieceColor::Red) {
                    score += value;
                } else {
                    score -= value;
                }
            }
        }
    }

    return score;
}

int Evaluator::getPieceBaseValue(PieceType type) const
{
    switch (type) {
    case PieceType::King:
        return 10000;  // 将/帅
    case PieceType::Rook:
        return 1000;   // 车
    case PieceType::Horse:
        return 350;    // 马
    case PieceType::Cannon:
        return 350;    // 炮
    case PieceType::Advisor:
        return 200;    // 士
    case PieceType::Elephant:
        return 200;    // 象
    case PieceType::Pawn:
        return 100;    // 兵
    default:
        return 0;
    }
}

int Evaluator::getPieceValue(PieceType type, int row, int col, PieceColor color) const
{
    int baseValue = getPieceBaseValue(type);

    // 黑方需要翻转行坐标
    int posRow = (color == PieceColor::Black) ? (9 - row) : row;

    int posValue = 0;
    switch (type) {
    case PieceType::Pawn:
        posValue = PAWN_POS_VALUE[posRow][col];
        break;
    case PieceType::Advisor:
        posValue = ADVISOR_POS_VALUE[posRow][col];
        break;
    case PieceType::Elephant:
        posValue = ELEPHANT_POS_VALUE[posRow][col];
        break;
    case PieceType::Horse:
        posValue = HORSE_POS_VALUE[posRow][col];
        break;
    case PieceType::Rook:
        posValue = ROOK_POS_VALUE[posRow][col];
        break;
    case PieceType::Cannon:
        posValue = CANNON_POS_VALUE[posRow][col];
        break;
    case PieceType::King:
        posValue = KING_POS_VALUE[posRow][col];
        break;
    default:
        break;
    }

    return baseValue + posValue;
}

// ============ 位置价值表定义 ============

const int Evaluator::PAWN_POS_VALUE[10][9] = {
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0, -2,  0,  4,  0, -2,  0,  0},
    { 2,  0,  8,  0,  8,  0,  8,  0,  2},
    { 6,  12, 18, 18, 20, 18, 18, 12, 6},
    { 10, 20, 30, 34, 40, 34, 30, 20, 10},
    { 14, 26, 42, 60, 80, 60, 42, 26, 14},
    { 18, 36, 56, 80, 120, 80, 56, 36, 18},
    { 0,  3,  6,  9,  12,  9,  6,  3,  0}
};

const int Evaluator::ADVISOR_POS_VALUE[10][9] = {
    { 0,  0,  0, 20,  0, 20,  0,  0,  0},
    { 0,  0,  0,  0, 23,  0,  0,  0,  0},
    { 0,  0,  0, 20,  0, 20,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0}
};

const int Evaluator::ELEPHANT_POS_VALUE[10][9] = {
    { 0,  0, 20,  0,  0,  0, 20,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    {18,  0,  0,  0, 23,  0,  0,  0, 18},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0, 20,  0,  0,  0, 20,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0}
};

const int Evaluator::HORSE_POS_VALUE[10][9] = {
    { 0, -3,  5,  4,  2,  4,  5, -3,  0},
    {-3,  2,  4,  6,  10, 6,  4,  2, -3},
    { 4,  6, 12, 11, 15, 11, 12, 6,  4},
    { 2,  6,  8, 11, 11, 11,  8,  6,  2},
    { 2,  12, 11, 15, 16, 15, 11, 12, 2},
    { 0,  5,  7,  7,  14,  7,  7,  5,  0},
    {-5,  2,  4,  8,  8,  8,  4,  2, -5},
    {-6,  3,  2,  5,  4,  5,  2,  3, -6},
    {-8, -3,  1,  4,  4,  4,  1, -3, -8},
    {-10,-8, -6, -3, -1, -3, -6, -8, -10}
};

const int Evaluator::ROOK_POS_VALUE[10][9] = {
    {-6,  5,  8,  8,  8,  8,  8,  5, -6},
    { 6, 8,   10, 14, 15, 14, 10,  8,  6},
    { 4,  6,  8,  12, 12, 12,  8,  6,  4},
    {12, 16, 16, 20, 20, 20, 16, 16, 12},
    {10, 14, 15, 17, 20, 17, 15, 14, 10},
    { 6, 11, 13, 15, 16, 15, 13, 11,  6},
    { 4, 6,   9,  10, 11, 10, 9,   6,  4},
    { 2, 4,   7,  7,  8,  7,  7,   4,  2},
    { 0, 3,   5,  5,  6,  5,  5,   3,  0},
    {-4, 2,   4,  4,  5,  4,  4,   2, -4}
};

const int Evaluator::CANNON_POS_VALUE[10][9] = {
    { 0,  0,  1,  0,  3,  0,  1,  0,  0},
    { 0,  2,  4,  3,  4,  3,  4,  2,  0},
    { 1,  0,  7,  4,  4,  4,  7,  0,  1},
    { 0,  0,  7,  4,  4,  4,  7,  0,  0},
    { 0,  1,  6,  7,  7,  7,  6,  1,  0},
    {-1,  1,  2,  7,  8,  7,  2,  1, -1},
    { 0,  3,  4,  4,  3,  4,  4,  3,  0},
    { 0,  2,  2,  2,  2,  2,  2,  2,  0},
    { 0,  1,  2,  3,  3,  3,  2,  1,  0},
    { 0,  0,  1,  1,  2,  1,  1,  0,  0}
};

const int Evaluator::KING_POS_VALUE[10][9] = {
    { 0,  0,  0,  8,  8,  8,  0,  0,  0},
    { 0,  0,  0,  9,  9,  9,  0,  0,  0},
    { 0,  0,  0, 10, 10, 10,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0}
};
