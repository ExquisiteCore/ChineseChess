#include "Board.h"
#include <QDebug>

Board::Board()
{
    clear();
}

void Board::clear()
{
    // 清空棋盘（-1 表示空位）
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            m_board[row][col] = -1;
        }
    }
    m_pieces.clear();
}

void Board::initializeStartPosition()
{
    clear();

    // 黑方（上方，第0-4行）
    // 第0行：车 马 象 士 将 士 象 马 车
    addPiece(ChessPiece(PieceType::Rook, PieceColor::Black, 0, 0));
    addPiece(ChessPiece(PieceType::Horse, PieceColor::Black, 0, 1));
    addPiece(ChessPiece(PieceType::Elephant, PieceColor::Black, 0, 2));
    addPiece(ChessPiece(PieceType::Advisor, PieceColor::Black, 0, 3));
    addPiece(ChessPiece(PieceType::King, PieceColor::Black, 0, 4));
    addPiece(ChessPiece(PieceType::Advisor, PieceColor::Black, 0, 5));
    addPiece(ChessPiece(PieceType::Elephant, PieceColor::Black, 0, 6));
    addPiece(ChessPiece(PieceType::Horse, PieceColor::Black, 0, 7));
    addPiece(ChessPiece(PieceType::Rook, PieceColor::Black, 0, 8));

    // 第2行：炮
    addPiece(ChessPiece(PieceType::Cannon, PieceColor::Black, 2, 1));
    addPiece(ChessPiece(PieceType::Cannon, PieceColor::Black, 2, 7));

    // 第3行：卒
    for (int col = 0; col < COLS; col += 2) {
        addPiece(ChessPiece(PieceType::Pawn, PieceColor::Black, 3, col));
    }

    // 红方（下方，第5-9行）
    // 第6行：兵
    for (int col = 0; col < COLS; col += 2) {
        addPiece(ChessPiece(PieceType::Pawn, PieceColor::Red, 6, col));
    }

    // 第7行：炮
    addPiece(ChessPiece(PieceType::Cannon, PieceColor::Red, 7, 1));
    addPiece(ChessPiece(PieceType::Cannon, PieceColor::Red, 7, 7));

    // 第9行：車 馬 相 仕 帥 仕 相 馬 車
    addPiece(ChessPiece(PieceType::Rook, PieceColor::Red, 9, 0));
    addPiece(ChessPiece(PieceType::Horse, PieceColor::Red, 9, 1));
    addPiece(ChessPiece(PieceType::Elephant, PieceColor::Red, 9, 2));
    addPiece(ChessPiece(PieceType::Advisor, PieceColor::Red, 9, 3));
    addPiece(ChessPiece(PieceType::King, PieceColor::Red, 9, 4));
    addPiece(ChessPiece(PieceType::Advisor, PieceColor::Red, 9, 5));
    addPiece(ChessPiece(PieceType::Elephant, PieceColor::Red, 9, 6));
    addPiece(ChessPiece(PieceType::Horse, PieceColor::Red, 9, 7));
    addPiece(ChessPiece(PieceType::Rook, PieceColor::Red, 9, 8));
}

ChessPiece* Board::pieceAt(int row, int col)
{
    if (!isValidPosition(row, col))
        return nullptr;

    int index = m_board[row][col];
    if (index < 0 || index >= m_pieces.size())
        return nullptr;

    return &m_pieces[index];
}

const ChessPiece* Board::pieceAt(int row, int col) const
{
    if (!isValidPosition(row, col))
        return nullptr;

    int index = m_board[row][col];
    if (index < 0 || index >= m_pieces.size())
        return nullptr;

    return &m_pieces[index];
}

void Board::setPiece(int row, int col, const ChessPiece &piece)
{
    if (!isValidPosition(row, col))
        return;

    // 移除旧棋子
    removePiece(row, col);

    // 添加新棋子
    m_pieces.append(piece);
    int newIndex = m_pieces.size() - 1;
    m_pieces[newIndex].setPosition(row, col);
    m_board[row][col] = newIndex;
}

void Board::removePiece(int row, int col)
{
    if (!isValidPosition(row, col))
        return;

    int index = m_board[row][col];
    if (index >= 0 && index < m_pieces.size()) {
        // 标记棋子为无效（不从列表中删除以保持索引稳定）
        m_pieces[index] = ChessPiece();  // 设置为空棋子
        m_board[row][col] = -1;
    }
}

bool Board::movePiece(int fromRow, int fromCol, int toRow, int toCol)
{
    if (!isValidPosition(fromRow, fromCol) || !isValidPosition(toRow, toCol))
        return false;

    int fromIndex = m_board[fromRow][fromCol];
    if (fromIndex < 0 || fromIndex >= m_pieces.size())
        return false;

    if (!m_pieces[fromIndex].isValid())
        return false;

    // 移除目标位置的棋子（吃子）
    removePiece(toRow, toCol);

    // 移动棋子
    m_pieces[fromIndex].setPosition(toRow, toCol);
    m_board[toRow][toCol] = fromIndex;
    m_board[fromRow][fromCol] = -1;

    return true;
}

bool Board::isValidPosition(int row, int col)
{
    return row >= 0 && row < ROWS && col >= 0 && col < COLS;
}

bool Board::isInPalace(int row, int col, PieceColor color)
{
    if (!isValidPosition(row, col))
        return false;

    // 红方九宫：第7-9行，第3-5列
    if (color == PieceColor::Red) {
        return row >= 7 && row <= 9 && col >= 3 && col <= 5;
    }
    // 黑方九宫：第0-2行，第3-5列
    else {
        return row >= 0 && row <= 2 && col >= 3 && col <= 5;
    }
}

bool Board::isInOwnHalf(int row, int col, PieceColor color)
{
    if (!isValidPosition(row, col))
        return false;

    // 红方在下半场（第5-9行）
    if (color == PieceColor::Red) {
        return row >= 5;
    }
    // 黑方在上半场（第0-4行）
    else {
        return row <= 4;
    }
}

QList<ChessPiece> Board::getAllPieces() const
{
    // 只返回有效的棋子
    QList<ChessPiece> validPieces;
    for (const ChessPiece &piece : m_pieces) {
        if (piece.isValid()) {
            validPieces.append(piece);
        }
    }
    return validPieces;
}

ChessPiece* Board::findKing(PieceColor color)
{
    for (ChessPiece &piece : m_pieces) {
        if (piece.isValid() && piece.type() == PieceType::King && piece.color() == color) {
            return &piece;
        }
    }
    return nullptr;
}

void Board::print() const
{
    qDebug() << "=== 棋盘 ===";
    for (int row = 0; row < ROWS; ++row) {
        QString line;
        for (int col = 0; col < COLS; ++col) {
            const ChessPiece *piece = pieceAt(row, col);
            if (piece) {
                line += piece->chineseName() + " ";
            } else {
                line += "·  ";
            }
        }
        qDebug().noquote() << QString("%1: %2").arg(row).arg(line);
    }
    qDebug() << "============";
}

void Board::addPiece(const ChessPiece &piece)
{
    m_pieces.append(piece);
    int newIndex = m_pieces.size() - 1;
    m_board[piece.row()][piece.col()] = newIndex;
}
