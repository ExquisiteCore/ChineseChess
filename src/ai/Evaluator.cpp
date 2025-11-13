#include "Evaluator.h"
#include "../core/ChessRules.h"
#include <algorithm>

Evaluator::Evaluator()
    : m_useAdvancedEval(true)
{
}

int Evaluator::evaluatePosition(const Position &position)
{
    if (m_useAdvancedEval) {
        return evaluatePositionFull(position);
    } else {
        return evaluatePositionFast(position);
    }
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

// === 高级评估函数实现 ===

int Evaluator::evaluatePositionFull(const Position &position)
{
    int score = 0;

    // 基础评估（材料+位置）
    score += evaluatePositionFast(position);

    // 高级评估因素
    score += evaluateMobility(position);
    score += evaluateControl(position);
    score += evaluateProtection(position);
    score += evaluateKingSafety(position);
    score += evaluatePatterns(position);

    return score;
}

int Evaluator::evaluateMobility(const Position &position)
{
    const Board &board = position.board();
    int redMobility = 0, blackMobility = 0;

    // 计算每个棋子的合法移动数
    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const ChessPiece *piece = board.pieceAt(row, col);
            if (piece && piece->isValid()) {
                QList<QPoint> moves = ChessRules::getLegalMoves(board, row, col);
                int mobility = moves.size();

                // 根据棋子类型调整权重
                int weight = 1;
                if (piece->type() == PieceType::Rook) {
                    weight = 3;  // 车的灵活性最重要
                } else if (piece->type() == PieceType::Horse || piece->type() == PieceType::Cannon) {
                    weight = 2;  // 马炮次之
                }

                if (piece->color() == PieceColor::Red) {
                    redMobility += mobility * weight;
                } else {
                    blackMobility += mobility * weight;
                }
            }
        }
    }

    return (redMobility - blackMobility) * 2;  // 灵活性权重系数
}

int Evaluator::evaluateControl(const Position &position)
{
    const Board &board = position.board();
    int redControl = 0, blackControl = 0;

    // 评估对关键格子的控制
    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            if (!isKeySquare(row, col)) {
                continue;
            }

            // 计算攻击这个格子的双方棋子数
            int redAttackers = countAttackers(board, row, col, PieceColor::Red);
            int blackAttackers = countAttackers(board, row, col, PieceColor::Black);

            redControl += redAttackers;
            blackControl += blackAttackers;
        }
    }

    return (redControl - blackControl) * 3;  // 控制力权重系数
}

int Evaluator::evaluateProtection(const Position &position)
{
    const Board &board = position.board();
    int score = 0;

    // 评估每个棋子的保护情况
    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const ChessPiece *piece = board.pieceAt(row, col);
            if (!piece || !piece->isValid()) {
                continue;
            }

            int defenders = countDefenders(board, row, col, piece->color());
            int attackers = countAttackers(board, row, col,
                                          piece->color() == PieceColor::Red ? PieceColor::Black : PieceColor::Red);

            // 如果被攻击且无保护，扣分
            if (attackers > 0 && defenders == 0) {
                int penalty = getPieceBaseValue(piece->type()) / 10;
                if (piece->color() == PieceColor::Red) {
                    score -= penalty;
                } else {
                    score += penalty;
                }
            }
            // 如果有保护，加分
            else if (defenders > attackers) {
                int bonus = 5 * (defenders - attackers);
                if (piece->color() == PieceColor::Red) {
                    score += bonus;
                } else {
                    score -= bonus;
                }
            }
        }
    }

    return score;
}

int Evaluator::evaluateKingSafety(const Position &position)
{
    const Board &board = position.board();
    int score = 0;

    // 找到双方的将帅
    int redKingRow = -1, redKingCol = -1;
    int blackKingRow = -1, blackKingCol = -1;

    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const ChessPiece *piece = board.pieceAt(row, col);
            if (piece && piece->isValid() && piece->type() == PieceType::King) {
                if (piece->color() == PieceColor::Red) {
                    redKingRow = row;
                    redKingCol = col;
                } else {
                    blackKingRow = row;
                    blackKingCol = col;
                }
            }
        }
    }

    // 评估红方将帅安全
    if (redKingRow >= 0) {
        int defenders = countDefenders(board, redKingRow, redKingCol, PieceColor::Red);
        int attackers = countAttackers(board, redKingRow, redKingCol, PieceColor::Black);

        score += defenders * 10;
        score -= attackers * 15;

        // 九宫完整性（士象齐全更安全）
        int advisors = 0, elephants = 0;
        for (int row = 7; row < 10; ++row) {
            for (int col = 3; col <= 5; ++col) {
                const ChessPiece *piece = board.pieceAt(row, col);
                if (piece && piece->isValid() && piece->color() == PieceColor::Red) {
                    if (piece->type() == PieceType::Advisor) advisors++;
                    if (piece->type() == PieceType::Elephant) elephants++;
                }
            }
        }
        score += advisors * 8 + elephants * 6;
    }

    // 评估黑方将帅安全
    if (blackKingRow >= 0) {
        int defenders = countDefenders(board, blackKingRow, blackKingCol, PieceColor::Black);
        int attackers = countAttackers(board, blackKingRow, blackKingCol, PieceColor::Red);

        score -= defenders * 10;
        score += attackers * 15;

        int advisors = 0, elephants = 0;
        for (int row = 0; row < 3; ++row) {
            for (int col = 3; col <= 5; ++col) {
                const ChessPiece *piece = board.pieceAt(row, col);
                if (piece && piece->isValid() && piece->color() == PieceColor::Black) {
                    if (piece->type() == PieceType::Advisor) advisors++;
                    if (piece->type() == PieceType::Elephant) elephants++;
                }
            }
        }
        score -= advisors * 8 + elephants * 6;
    }

    return score;
}

int Evaluator::evaluatePatterns(const Position &position)
{
    const Board &board = position.board();
    int score = 0;

    // 识别特殊棋型（马后炮、重炮等）
    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const ChessPiece *piece = board.pieceAt(row, col);
            if (!piece || !piece->isValid()) {
                continue;
            }

            // 马后炮：炮在马后面
            if (piece->type() == PieceType::Cannon) {
                // 检查炮前方是否有己方马
                int direction = (piece->color() == PieceColor::Red) ? -1 : 1;
                for (int r = row + direction; r >= 0 && r < Board::ROWS; r += direction) {
                    const ChessPiece *front = board.pieceAt(r, col);
                    if (front && front->isValid()) {
                        if (front->color() == piece->color() && front->type() == PieceType::Horse) {
                            int bonus = 30;
                            if (piece->color() == PieceColor::Red) {
                                score += bonus;
                            } else {
                                score -= bonus;
                            }
                        }
                        break;
                    }
                }
            }

            // 双车联动
            if (piece->type() == PieceType::Rook) {
                // 检查同一行或列是否有另一个己方车
                for (int c = 0; c < Board::COLS; ++c) {
                    if (c == col) continue;
                    const ChessPiece *other = board.pieceAt(row, c);
                    if (other && other->isValid() &&
                        other->color() == piece->color() &&
                        other->type() == PieceType::Rook) {
                        int bonus = 40;
                        if (piece->color() == PieceColor::Red) {
                            score += bonus;
                        } else {
                            score -= bonus;
                        }
                        break;
                    }
                }
            }
        }
    }

    return score;
}

int Evaluator::countAttackers(const Board &board, int row, int col, PieceColor attackColor)
{
    int count = 0;

    // 检查所有敌方棋子是否能攻击到这个位置
    for (int r = 0; r < Board::ROWS; ++r) {
        for (int c = 0; c < Board::COLS; ++c) {
            const ChessPiece *piece = board.pieceAt(r, c);
            if (!piece || !piece->isValid() || piece->color() != attackColor) {
                continue;
            }

            QList<QPoint> moves = ChessRules::getLegalMoves(board, r, c);
            for (const QPoint &move : moves) {
                if (move.y() == row && move.x() == col) {
                    count++;
                    break;
                }
            }
        }
    }

    return count;
}

int Evaluator::countDefenders(const Board &board, int row, int col, PieceColor defendColor)
{
    return countAttackers(board, row, col, defendColor);
}

bool Evaluator::isKeySquare(int row, int col)
{
    return KEY_SQUARES[row][col];
}

// 关键格子定义
const bool Evaluator::KEY_SQUARES[10][9] = {
    {0, 0, 0, 1, 1, 1, 0, 0, 0},  // 黑方九宫
    {0, 0, 0, 1, 1, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 1, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 1, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 1},  // 河界
    {1, 1, 1, 1, 1, 1, 1, 1, 1},  // 河界
    {0, 1, 0, 0, 0, 0, 0, 1, 0},
    {0, 0, 0, 1, 1, 1, 0, 0, 0},  // 红方九宫
    {0, 0, 0, 1, 1, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 1, 0, 0, 0}
};
