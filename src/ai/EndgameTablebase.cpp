#include "EndgameTablebase.h"
#include "../core/ChessRules.h"
#include <QDebug>

EndgameTablebase::EndgameTablebase()
    : m_enabled(true)
{
    qDebug() << "残局库初始化完成";
}

EndgameEntry EndgameTablebase::probe(const Position &position)
{
    if (!m_enabled || !isEndgame(position)) {
        return EndgameEntry();
    }

    // 首先尝试识别特殊残局
    return recognizeSpecialEndgame(position);
}

bool EndgameTablebase::isEndgame(const Position &position) const
{
    // 判断是否进入残局阶段
    // 规则：总棋子数 <= 10 或者没有重子（车、炮）
    const Board &board = position.board();
    int totalPieces = countTotalPieces(board);

    if (totalPieces <= 10) {
        return true;
    }

    // 检查是否有重子
    int heavyPieces = 0;
    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const ChessPiece *piece = board.pieceAt(row, col);
            if (piece && piece->isValid()) {
                if (piece->type() == PieceType::Rook || piece->type() == PieceType::Cannon) {
                    heavyPieces++;
                }
            }
        }
    }

    // 如果重子数量 <= 2，也认为是残局
    return heavyPieces <= 2;
}

int EndgameTablebase::evaluateEndgame(const Position &position)
{
    if (!isEndgame(position)) {
        return 0;
    }

    // 根据残局类型评估
    int totalPieces = countTotalPieces(position.board());

    if (totalPieces <= 6) {
        return evaluateKingPawnEndgame(position);
    } else {
        return evaluateRookEndgame(position);
    }
}

EndgameEntry EndgameTablebase::recognizeSpecialEndgame(const Position &position)
{
    const Board &board = position.board();
    int totalPieces = countTotalPieces(board);

    // 只剩两个将帅 - 和棋
    if (totalPieces == 2) {
        return handleKingVsKing(position);
    }

    // 单车对单子残局
    if (totalPieces <= 4) {
        return handleRookVsMinor(position);
    }

    // 兵类残局
    if (totalPieces <= 8) {
        return handlePawnEndgame(position);
    }

    return EndgameEntry();
}

int EndgameTablebase::countPieces(const Board &board, PieceColor color) const
{
    int count = 0;
    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const ChessPiece *piece = board.pieceAt(row, col);
            if (piece && piece->isValid() && piece->color() == color) {
                count++;
            }
        }
    }
    return count;
}

int EndgameTablebase::countTotalPieces(const Board &board) const
{
    int count = 0;
    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const ChessPiece *piece = board.pieceAt(row, col);
            if (piece && piece->isValid()) {
                count++;
            }
        }
    }
    return count;
}

EndgameEntry EndgameTablebase::handleKingVsKing(const Position &position)
{
    // 只剩两个将帅，必然和棋
    return EndgameEntry(EndgameResult::Draw, 0);
}

EndgameEntry EndgameTablebase::handleRookVsMinor(const Position &position)
{
    // 车对小子通常是胜势
    const Board &board = position.board();
    bool redHasRook = false, blackHasRook = false;

    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const ChessPiece *piece = board.pieceAt(row, col);
            if (piece && piece->isValid() && piece->type() == PieceType::Rook) {
                if (piece->color() == PieceColor::Red) {
                    redHasRook = true;
                } else {
                    blackHasRook = true;
                }
            }
        }
    }

    PieceColor currentColor = position.currentTurn();

    if (redHasRook && !blackHasRook) {
        // 红方有车，黑方没有
        return EndgameEntry(currentColor == PieceColor::Red ? EndgameResult::Win : EndgameResult::Loss, 20);
    } else if (blackHasRook && !redHasRook) {
        // 黑方有车，红方没有
        return EndgameEntry(currentColor == PieceColor::Black ? EndgameResult::Win : EndgameResult::Loss, 20);
    }

    return EndgameEntry();
}

EndgameEntry EndgameTablebase::handlePawnEndgame(const Position &position)
{
    // 兵类残局需要复杂的评估
    // 这里简化处理：根据兵的数量和位置判断优势
    return EndgameEntry();
}

int EndgameTablebase::evaluateKingPawnEndgame(const Position &position)
{
    const Board &board = position.board();
    int score = 0;

    // 兵的位置价值在残局中更重要
    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const ChessPiece *piece = board.pieceAt(row, col);
            if (piece && piece->isValid() && piece->type() == PieceType::Pawn) {
                // 过河兵价值更高
                bool isCrossed = (piece->color() == PieceColor::Red && row <= 4) ||
                                (piece->color() == PieceColor::Black && row >= 5);

                int pawnValue = isCrossed ? 150 : 80;

                // 接近对方九宫的兵价值更高
                if (piece->color() == PieceColor::Red && row <= 2) {
                    pawnValue += 50;
                } else if (piece->color() == PieceColor::Black && row >= 7) {
                    pawnValue += 50;
                }

                if (piece->color() == PieceColor::Red) {
                    score += pawnValue;
                } else {
                    score -= pawnValue;
                }
            }
        }
    }

    // 将帅的活动性在残局中很重要
    int redKingMoves = 0, blackKingMoves = 0;
    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const ChessPiece *piece = board.pieceAt(row, col);
            if (piece && piece->isValid() && piece->type() == PieceType::King) {
                QList<QPoint> moves = ChessRules::getLegalMoves(board, row, col);
                if (piece->color() == PieceColor::Red) {
                    redKingMoves = moves.size();
                } else {
                    blackKingMoves = moves.size();
                }
            }
        }
    }

    score += (redKingMoves - blackKingMoves) * 10;

    return score;
}

int EndgameTablebase::evaluateRookEndgame(const Position &position)
{
    const Board &board = position.board();
    int score = 0;

    // 车的活动性非常重要
    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const ChessPiece *piece = board.pieceAt(row, col);
            if (piece && piece->isValid() && piece->type() == PieceType::Rook) {
                QList<QPoint> moves = ChessRules::getLegalMoves(board, row, col);
                int mobility = moves.size();

                if (piece->color() == PieceColor::Red) {
                    score += mobility * 15;
                } else {
                    score -= mobility * 15;
                }
            }
        }
    }

    return score;
}
