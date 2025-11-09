#ifndef CHESSRULES_H
#define CHESSRULES_H

#include "Board.h"
#include <QList>
#include <QPoint>

// 走棋规则引擎（静态类）
class ChessRules
{
public:
    // 检查移动是否合法（不考虑将军）
    static bool isValidMove(const Board &board, int fromRow, int fromCol, int toRow, int toCol);

    // 获取指定棋子的所有合法移动
    static QList<QPoint> getLegalMoves(const Board &board, int row, int col);

    // 检查是否将军
    static bool isInCheck(const Board &board, PieceColor kingColor);

    // 检查移动后是否会导致自己被将军
    static bool wouldBeInCheck(Board &board, int fromRow, int fromCol, int toRow, int toCol);

private:
    // 各个棋子的移动规则
    static bool isValidKingMove(const Board &board, int fromRow, int fromCol, int toRow, int toCol, PieceColor color);
    static bool isValidAdvisorMove(const Board &board, int fromRow, int fromCol, int toRow, int toCol, PieceColor color);
    static bool isValidElephantMove(const Board &board, int fromRow, int fromCol, int toRow, int toCol, PieceColor color);
    static bool isValidHorseMove(const Board &board, int fromRow, int fromCol, int toRow, int toCol);
    static bool isValidRookMove(const Board &board, int fromRow, int fromCol, int toRow, int toCol);
    static bool isValidCannonMove(const Board &board, int fromRow, int fromCol, int toRow, int toCol);
    static bool isValidPawnMove(const Board &board, int fromRow, int fromCol, int toRow, int toCol, PieceColor color);

    // 辅助函数
    static bool isPathClear(const Board &board, int fromRow, int fromCol, int toRow, int toCol);
    static int countPiecesBetween(const Board &board, int fromRow, int fromCol, int toRow, int toCol);
    static bool canKingsSeeEachOther(const Board &board);
};

#endif // CHESSRULES_H
