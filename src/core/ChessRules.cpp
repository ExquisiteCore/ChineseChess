#include "ChessRules.h"
#include <QDebug>
#include <cmath>

bool ChessRules::isValidMove(const Board &board, int fromRow, int fromCol, int toRow, int toCol)
{
    // 检查坐标是否有效
    if (!Board::isValidPosition(fromRow, fromCol) || !Board::isValidPosition(toRow, toCol))
        return false;

    // 起点必须有棋子
    const ChessPiece *piece = board.pieceAt(fromRow, fromCol);
    if (!piece || !piece->isValid())
        return false;

    // 不能移动到原位置
    if (fromRow == toRow && fromCol == toCol)
        return false;

    // 不能吃自己的棋子
    const ChessPiece *targetPiece = board.pieceAt(toRow, toCol);
    if (targetPiece && targetPiece->isValid() && piece->isSameColor(*targetPiece))
        return false;

    // 根据棋子类型检查移动规则
    switch (piece->type()) {
    case PieceType::King:
        return isValidKingMove(board, fromRow, fromCol, toRow, toCol, piece->color());
    case PieceType::Advisor:
        return isValidAdvisorMove(board, fromRow, fromCol, toRow, toCol, piece->color());
    case PieceType::Elephant:
        return isValidElephantMove(board, fromRow, fromCol, toRow, toCol, piece->color());
    case PieceType::Horse:
        return isValidHorseMove(board, fromRow, fromCol, toRow, toCol);
    case PieceType::Rook:
        return isValidRookMove(board, fromRow, fromCol, toRow, toCol);
    case PieceType::Cannon:
        return isValidCannonMove(board, fromRow, fromCol, toRow, toCol);
    case PieceType::Pawn:
        return isValidPawnMove(board, fromRow, fromCol, toRow, toCol, piece->color());
    default:
        return false;
    }
}

// 将/帅：只能在九宫内移动，每次一步，不能斜走
bool ChessRules::isValidKingMove(const Board &board, int fromRow, int fromCol, int toRow, int toCol, PieceColor color)
{
    // 必须在九宫内
    if (!Board::isInPalace(toRow, toCol, color))
        return false;

    // 只能移动一格
    int rowDiff = std::abs(toRow - fromRow);
    int colDiff = std::abs(toCol - fromCol);

    // 只能直走（不能斜走）
    if ((rowDiff == 1 && colDiff == 0) || (rowDiff == 0 && colDiff == 1))
        return true;

    // 特殊规则：飞将（将帅对面）
    const ChessPiece *targetPiece = board.pieceAt(toRow, toCol);
    if (targetPiece && targetPiece->type() == PieceType::King) {
        // 检查是否在同一列且中间无棋子
        if (fromCol == toCol && isPathClear(board, fromRow, fromCol, toRow, toCol)) {
            return true;
        }
    }

    return false;
}

// 士/仕：只能在九宫内斜走，每次一步
bool ChessRules::isValidAdvisorMove(const Board &board, int fromRow, int fromCol, int toRow, int toCol, PieceColor color)
{
    // 必须在九宫内
    if (!Board::isInPalace(toRow, toCol, color))
        return false;

    // 只能斜走一格
    int rowDiff = std::abs(toRow - fromRow);
    int colDiff = std::abs(toCol - fromCol);

    return rowDiff == 1 && colDiff == 1;
}

// 象/相：只能在己方半场斜走，每次两格，不能越子（塞象眼）
bool ChessRules::isValidElephantMove(const Board &board, int fromRow, int fromCol, int toRow, int toCol, PieceColor color)
{
    // 必须在己方半场
    if (!Board::isInOwnHalf(toRow, toCol, color))
        return false;

    // 必须斜走两格
    int rowDiff = toRow - fromRow;
    int colDiff = toCol - fromCol;

    if (std::abs(rowDiff) != 2 || std::abs(colDiff) != 2)
        return false;

    // 检查象眼是否被塞
    int eyeRow = fromRow + rowDiff / 2;
    int eyeCol = fromCol + colDiff / 2;
    const ChessPiece *eyePiece = board.pieceAt(eyeRow, eyeCol);

    return !eyePiece || !eyePiece->isValid();
}

// 马：走日字，不能蹩马腿
bool ChessRules::isValidHorseMove(const Board &board, int fromRow, int fromCol, int toRow, int toCol)
{
    int rowDiff = std::abs(toRow - fromRow);
    int colDiff = std::abs(toCol - fromCol);

    // 必须走日字（2+1 或 1+2）
    if (!((rowDiff == 2 && colDiff == 1) || (rowDiff == 1 && colDiff == 2)))
        return false;

    // 检查马腿
    int legRow = fromRow;
    int legCol = fromCol;

    if (rowDiff == 2) {
        legRow += (toRow > fromRow) ? 1 : -1;
    } else {
        legCol += (toCol > fromCol) ? 1 : -1;
    }

    const ChessPiece *legPiece = board.pieceAt(legRow, legCol);
    return !legPiece || !legPiece->isValid();
}

// 车/車：横竖直走，路径不能有棋子
bool ChessRules::isValidRookMove(const Board &board, int fromRow, int fromCol, int toRow, int toCol)
{
    // 必须直线移动
    if (fromRow != toRow && fromCol != toCol)
        return false;

    // 路径必须畅通
    return isPathClear(board, fromRow, fromCol, toRow, toCol);
}

// 炮：横竖直走，吃子时中间必须有且仅有一个棋子（炮架）
bool ChessRules::isValidCannonMove(const Board &board, int fromRow, int fromCol, int toRow, int toCol)
{
    // 必须直线移动
    if (fromRow != toRow && fromCol != toCol)
        return false;

    const ChessPiece *targetPiece = board.pieceAt(toRow, toCol);
    int piecesBetween = countPiecesBetween(board, fromRow, fromCol, toRow, toCol);

    // 不吃子：路径必须畅通
    if (!targetPiece || !targetPiece->isValid()) {
        return piecesBetween == 0;
    }
    // 吃子：中间必须有且仅有一个棋子
    else {
        return piecesBetween == 1;
    }
}

// 兵/卒：过河前只能前进，过河后可以左右移动
bool ChessRules::isValidPawnMove(const Board &board, int fromRow, int fromCol, int toRow, int toCol, PieceColor color)
{
    int rowDiff = toRow - fromRow;
    int colDiff = std::abs(toCol - fromCol);

    // 只能移动一格
    if (std::abs(rowDiff) + colDiff != 1)
        return false;

    // 红方向上走（row减小），黑方向下走（row增大）
    bool isForward = (color == PieceColor::Red && rowDiff < 0) ||
                     (color == PieceColor::Black && rowDiff > 0);

    // 判断是否过河
    bool hasCrossedRiver = !Board::isInOwnHalf(fromRow, fromCol, color);

    // 过河前只能前进
    if (!hasCrossedRiver) {
        return isForward && colDiff == 0;
    }
    // 过河后可以前进或左右移动
    else {
        return (isForward && colDiff == 0) || (rowDiff == 0 && colDiff == 1);
    }
}

// 检查路径是否畅通（不包括起点和终点）
bool ChessRules::isPathClear(const Board &board, int fromRow, int fromCol, int toRow, int toCol)
{
    return countPiecesBetween(board, fromRow, fromCol, toRow, toCol) == 0;
}

// 计算路径上的棋子数量（不包括起点和终点）
int ChessRules::countPiecesBetween(const Board &board, int fromRow, int fromCol, int toRow, int toCol)
{
    int count = 0;

    // 确定移动方向
    int rowStep = (toRow == fromRow) ? 0 : ((toRow > fromRow) ? 1 : -1);
    int colStep = (toCol == fromCol) ? 0 : ((toCol > fromCol) ? 1 : -1);

    int currentRow = fromRow + rowStep;
    int currentCol = fromCol + colStep;

    // 遍历路径（不包括终点）
    while (currentRow != toRow || currentCol != toCol) {
        const ChessPiece *piece = board.pieceAt(currentRow, currentCol);
        if (piece && piece->isValid()) {
            ++count;
        }
        currentRow += rowStep;
        currentCol += colStep;
    }

    return count;
}

bool ChessRules::isInCheck(const Board &board, PieceColor kingColor)
{
    // 找到己方将/帅
    ChessPiece *king = const_cast<Board&>(board).findKing(kingColor);
    if (!king)
        return false;

    int kingRow = king->row();
    int kingCol = king->col();

    // 检查是否有对方棋子可以吃掉将/帅
    QList<ChessPiece> allPieces = board.getAllPieces();
    for (const ChessPiece &piece : allPieces) {
        if (piece.color() != kingColor) {
            if (isValidMove(board, piece.row(), piece.col(), kingRow, kingCol)) {
                return true;
            }
        }
    }

    return false;
}

bool ChessRules::wouldBeInCheck(Board &board, int fromRow, int fromCol, int toRow, int toCol)
{
    const ChessPiece *movingPiece = board.pieceAt(fromRow, fromCol);
    if (!movingPiece)
        return true;

    PieceColor color = movingPiece->color();

    // 保存目标位置的棋子
    ChessPiece *targetPiece = board.pieceAt(toRow, toCol);
    ChessPiece savedTarget;
    if (targetPiece) {
        savedTarget = *targetPiece;
    }

    // 临时移动
    board.movePiece(fromRow, fromCol, toRow, toCol);

    // 检查是否被将军
    bool inCheck = isInCheck(board, color);

    // 恢复棋盘
    board.movePiece(toRow, toCol, fromRow, fromCol);
    if (savedTarget.isValid()) {
        board.setPiece(toRow, toCol, savedTarget);
    }

    return inCheck;
}

QList<QPoint> ChessRules::getLegalMoves(const Board &board, int row, int col)
{
    QList<QPoint> moves;

    const ChessPiece *piece = board.pieceAt(row, col);
    if (!piece || !piece->isValid())
        return moves;

    // 遍历棋盘所有位置
    for (int toRow = 0; toRow < Board::ROWS; ++toRow) {
        for (int toCol = 0; toCol < Board::COLS; ++toCol) {
            if (isValidMove(board, row, col, toRow, toCol)) {
                // 还需要检查是否会导致自己被将军
                Board tempBoard = board;
                if (!wouldBeInCheck(tempBoard, row, col, toRow, toCol)) {
                    moves.append(QPoint(toCol, toRow));
                }
            }
        }
    }

    return moves;
}

bool ChessRules::hasLegalMoves(const Board &board, PieceColor color)
{
    // 遍历所有己方棋子
    QList<ChessPiece> allPieces = board.getAllPieces();
    for (const ChessPiece &piece : allPieces) {
        if (piece.color() == color) {
            // 检查这个棋子是否有合法走法
            QList<QPoint> moves = getLegalMoves(board, piece.row(), piece.col());
            if (!moves.isEmpty()) {
                return true;  // 找到至少一个合法走法
            }
        }
    }
    return false;  // 没有任何合法走法
}

bool ChessRules::isCheckmate(const Board &board, PieceColor kingColor)
{
    // 将死 = 被将军 + 没有合法走法可以解将
    if (!isInCheck(board, kingColor)) {
        return false;  // 没有被将军，不是将死
    }

    // 被将军了，检查是否有走法可以解将
    return !hasLegalMoves(board, kingColor);
}

bool ChessRules::isStalemate(const Board &board, PieceColor currentTurn)
{
    // 困毙 = 没有被将军 + 没有合法走法（和棋）
    if (isInCheck(board, currentTurn)) {
        return false;  // 被将军了，不是困毙
    }

    // 没有被将军，检查是否有合法走法
    return !hasLegalMoves(board, currentTurn);
}
