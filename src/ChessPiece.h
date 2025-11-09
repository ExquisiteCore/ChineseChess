#ifndef CHESSPIECE_H
#define CHESSPIECE_H

#include <QString>
#include <QPoint>

// 棋子类型枚举
enum class PieceType {
    None = 0,    // 空位
    King,        // 将/帅
    Advisor,     // 士/仕
    Elephant,    // 象/相
    Horse,       // 马
    Rook,        // 车/車
    Cannon,      // 炮
    Pawn         // 兵/卒
};

// 棋子颜色
enum class PieceColor {
    None = 0,
    Red,         // 红方
    Black        // 黑方
};

// 棋子类
class ChessPiece
{
public:
    ChessPiece();
    ChessPiece(PieceType type, PieceColor color, int row, int col);

    // 属性访问器
    PieceType type() const { return m_type; }
    PieceColor color() const { return m_color; }
    int row() const { return m_row; }
    int col() const { return m_col; }
    QPoint position() const { return QPoint(m_col, m_row); }
    bool hasMoved() const { return m_hasMoved; }
    int id() const { return m_id; }
    bool isValid() const { return m_type != PieceType::None; }

    // 属性设置器
    void setPosition(int row, int col);
    void setHasMoved(bool moved) { m_hasMoved = moved; }

    // 获取棋子的中文名称
    QString chineseName() const;

    // 获取棋子的 FEN 字符（用于序列化）
    QChar fenChar() const;

    // 从 FEN 字符创建棋子
    static ChessPiece fromFenChar(QChar ch, int row, int col);

    // 判断两个棋子是否为同色
    bool isSameColor(const ChessPiece &other) const;

    // 运算符重载
    bool operator==(const ChessPiece &other) const {
        return m_id == other.m_id;
    }

    bool operator!=(const ChessPiece &other) const {
        return !(*this == other);
    }

private:
    PieceType m_type;      // 棋子类型
    PieceColor m_color;    // 棋子颜色
    int m_row;             // 行坐标 (0-9)
    int m_col;             // 列坐标 (0-8)
    bool m_hasMoved;       // 是否已移动过（用于判断某些规则）
    int m_id;              // 唯一ID

    static int s_nextId;   // 全局ID计数器
};

#endif // CHESSPIECE_H
