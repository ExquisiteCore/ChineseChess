#ifndef BOARD_H
#define BOARD_H

#include "ChessPiece.h"
#include <QVector>
#include <QList>
#include <QString>
#include <QSharedPointer>

// 棋盘类 - 9×10 格位
class Board
{
public:
    static const int ROWS = 10;    // 10条横线（0-9）
    static const int COLS = 9;     // 9条竖线（0-8）

    Board();

    // 初始化棋盘到初始局面
    void initializeStartPosition();

    // 清空棋盘
    void clear();

    // 获取指定位置的棋子（返回指针，nullptr 表示空位）
    ChessPiece* pieceAt(int row, int col);
    const ChessPiece* pieceAt(int row, int col) const;

    // 设置指定位置的棋子
    void setPiece(int row, int col, const ChessPiece &piece);

    // 移除指定位置的棋子
    void removePiece(int row, int col);

    // 移动棋子（会自动处理吃子）
    bool movePiece(int fromRow, int fromCol, int toRow, int toCol);

    // 检查坐标是否有效
    static bool isValidPosition(int row, int col);

    // 检查位置是否在九宫内
    static bool isInPalace(int row, int col, PieceColor color);

    // 检查位置是否在己方半场
    static bool isInOwnHalf(int row, int col, PieceColor color);

    // 获取所有棋子列表
    QList<ChessPiece> getAllPieces() const;

    // 查找指定类型和颜色的棋子
    ChessPiece* findKing(PieceColor color);

    // 调试：打印棋盘
    void print() const;

private:
    // 二维数组存储棋盘，使用智能指针管理棋子
    QSharedPointer<ChessPiece> m_board[ROWS][COLS];

    // 棋子池（实际存储棋子对象的智能指针）
    QList<QSharedPointer<ChessPiece>> m_pieces;

    // 辅助函数：添加棋子到棋盘
    void addPiece(const ChessPiece &piece);
};

#endif // BOARD_H
