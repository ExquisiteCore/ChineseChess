#include "ChessBoardModel.h"
#include <QDebug>

ChessBoardModel::ChessBoardModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_isRedTurn(true)
    , m_liftedPieceIndex(-1)
{
    initializeBoard();
}

int ChessBoardModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_pieces.count();
}

QVariant ChessBoardModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_pieces.count())
        return QVariant();

    const ChessPieceData &piece = m_pieces[index.row()];

    switch (role) {
    case PieceTypeRole:
        return piece.type;
    case IsRedRole:
        return piece.isRed;
    case RowRole:
        return piece.row;
    case ColRole:
        return piece.col;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ChessBoardModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[PieceTypeRole] = "pieceType";
    roles[IsRedRole] = "isRed";
    roles[RowRole] = "row";
    roles[ColRole] = "col";
    return roles;
}

void ChessBoardModel::setIsRedTurn(bool turn)
{
    if (m_isRedTurn != turn) {
        m_isRedTurn = turn;
        emit isRedTurnChanged();

        // 切换回合时放下棋子
        setLiftedPieceIndex(-1);
    }
}

void ChessBoardModel::setLiftedPieceIndex(int index)
{
    if (m_liftedPieceIndex != index) {
        m_liftedPieceIndex = index;
        emit liftedPieceIndexChanged();
    }
}

bool ChessBoardModel::canSelectPiece(int index) const
{
    if (index < 0 || index >= m_pieces.count())
        return false;

    const ChessPieceData &piece = m_pieces[index];

    // 检查是否为当前回合的棋子
    return piece.isRed == m_isRedTurn;
}

void ChessBoardModel::selectPiece(int index)
{
    if (!canSelectPiece(index)) {
        qDebug() << "不是你的回合！当前是" << (m_isRedTurn ? "红方" : "黑方") << "回合";
        return;
    }

    // 单选逻辑：如果点击的是当前悬浮的棋子，则放下；否则切换到新棋子
    if (m_liftedPieceIndex == index) {
        setLiftedPieceIndex(-1);  // 放下当前棋子
    } else {
        setLiftedPieceIndex(index);  // 提起新棋子
    }

    const ChessPieceData &piece = m_pieces[index];
    qDebug() << "点击了棋子:" << piece.type << "位置:" << piece.row << piece.col;
}

void ChessBoardModel::initializeBoard()
{
    // 黑方（上方）
    // 第0行：车 马 象 士 将 士 象 马 车
    m_pieces.append({"车", false, 0, 0});
    m_pieces.append({"马", false, 0, 1});
    m_pieces.append({"象", false, 0, 2});
    m_pieces.append({"士", false, 0, 3});
    m_pieces.append({"将", false, 0, 4});
    m_pieces.append({"士", false, 0, 5});
    m_pieces.append({"象", false, 0, 6});
    m_pieces.append({"马", false, 0, 7});
    m_pieces.append({"车", false, 0, 8});

    // 第2行：炮
    m_pieces.append({"炮", false, 2, 1});
    m_pieces.append({"炮", false, 2, 7});

    // 第3行：卒
    m_pieces.append({"卒", false, 3, 0});
    m_pieces.append({"卒", false, 3, 2});
    m_pieces.append({"卒", false, 3, 4});
    m_pieces.append({"卒", false, 3, 6});
    m_pieces.append({"卒", false, 3, 8});

    // 红方（下方）
    // 第6行：兵
    m_pieces.append({"兵", true, 6, 0});
    m_pieces.append({"兵", true, 6, 2});
    m_pieces.append({"兵", true, 6, 4});
    m_pieces.append({"兵", true, 6, 6});
    m_pieces.append({"兵", true, 6, 8});

    // 第7行：炮
    m_pieces.append({"炮", true, 7, 1});
    m_pieces.append({"炮", true, 7, 7});

    // 第9行：車 馬 相 仕 帥 仕 相 馬 車
    m_pieces.append({"車", true, 9, 0});
    m_pieces.append({"馬", true, 9, 1});
    m_pieces.append({"相", true, 9, 2});
    m_pieces.append({"仕", true, 9, 3});
    m_pieces.append({"帥", true, 9, 4});
    m_pieces.append({"仕", true, 9, 5});
    m_pieces.append({"相", true, 9, 6});
    m_pieces.append({"馬", true, 9, 7});
    m_pieces.append({"車", true, 9, 8});
}
