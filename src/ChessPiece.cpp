#include "ChessPiece.h"

int ChessPiece::s_nextId = 1;

ChessPiece::ChessPiece()
    : m_type(PieceType::None)
    , m_color(PieceColor::None)
    , m_row(0)
    , m_col(0)
    , m_hasMoved(false)
    , m_id(0)
{
}

ChessPiece::ChessPiece(PieceType type, PieceColor color, int row, int col)
    : m_type(type)
    , m_color(color)
    , m_row(row)
    , m_col(col)
    , m_hasMoved(false)
    , m_id(s_nextId++)
{
}

void ChessPiece::setPosition(int row, int col)
{
    m_row = row;
    m_col = col;
    m_hasMoved = true;
}

QString ChessPiece::chineseName() const
{
    if (!isValid())
        return "";

    // 红方棋子名称
    if (m_color == PieceColor::Red) {
        switch (m_type) {
        case PieceType::King:     return "帥";
        case PieceType::Advisor:  return "仕";
        case PieceType::Elephant: return "相";
        case PieceType::Horse:    return "馬";
        case PieceType::Rook:     return "車";
        case PieceType::Cannon:   return "炮";
        case PieceType::Pawn:     return "兵";
        default: return "";
        }
    }
    // 黑方棋子名称
    else {
        switch (m_type) {
        case PieceType::King:     return "将";
        case PieceType::Advisor:  return "士";
        case PieceType::Elephant: return "象";
        case PieceType::Horse:    return "马";
        case PieceType::Rook:     return "车";
        case PieceType::Cannon:   return "炮";
        case PieceType::Pawn:     return "卒";
        default: return "";
        }
    }
}

QChar ChessPiece::fenChar() const
{
    if (!isValid())
        return '.';

    QChar ch;
    switch (m_type) {
    case PieceType::King:     ch = 'k'; break;
    case PieceType::Advisor:  ch = 'a'; break;
    case PieceType::Elephant: ch = 'e'; break;
    case PieceType::Horse:    ch = 'h'; break;
    case PieceType::Rook:     ch = 'r'; break;
    case PieceType::Cannon:   ch = 'c'; break;
    case PieceType::Pawn:     ch = 'p'; break;
    default: return '.';
    }

    // 红方用大写，黑方用小写
    return (m_color == PieceColor::Red) ? ch.toUpper() : ch;
}

ChessPiece ChessPiece::fromFenChar(QChar ch, int row, int col)
{
    if (ch == '.' || ch == ' ')
        return ChessPiece();

    PieceColor color = ch.isUpper() ? PieceColor::Red : PieceColor::Black;
    ch = ch.toLower();

    PieceType type = PieceType::None;
    switch (ch.toLatin1()) {
    case 'k': type = PieceType::King; break;
    case 'a': type = PieceType::Advisor; break;
    case 'e': type = PieceType::Elephant; break;
    case 'h': type = PieceType::Horse; break;
    case 'r': type = PieceType::Rook; break;
    case 'c': type = PieceType::Cannon; break;
    case 'p': type = PieceType::Pawn; break;
    default: return ChessPiece();
    }

    return ChessPiece(type, color, row, col);
}

bool ChessPiece::isSameColor(const ChessPiece &other) const
{
    return isValid() && other.isValid() && m_color == other.m_color;
}
