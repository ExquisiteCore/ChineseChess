#include "ChessBoardModel.h"
#include <QDebug>

ChessBoardModel::ChessBoardModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_liftedPieceIndex(-1)
{
    // 初始化为开局局面
    m_position.board().initializeStartPosition();
    rebuildPiecesList();
}

int ChessBoardModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_piecesList.count();
}

QVariant ChessBoardModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_piecesList.count())
        return QVariant();

    const ChessPiece &piece = m_piecesList[index.row()];

    switch (role) {
    case PieceTypeRole:
        return piece.chineseName();
    case IsRedRole:
        return piece.color() == PieceColor::Red;
    case RowRole:
        return piece.row();
    case ColRole:
        return piece.col();
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

bool ChessBoardModel::isRedTurn() const
{
    return m_position.currentTurn() == PieceColor::Red;
}

void ChessBoardModel::setIsRedTurn(bool turn)
{
    PieceColor newTurn = turn ? PieceColor::Red : PieceColor::Black;
    if (m_position.currentTurn() != newTurn) {
        m_position.setCurrentTurn(newTurn);
        emit isRedTurnChanged();
        emit fenStringChanged();

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
    if (index < 0 || index >= m_piecesList.count())
        return false;

    const ChessPiece &piece = m_piecesList[index];

    // 检查是否为当前回合的棋子
    return piece.color() == m_position.currentTurn();
}

void ChessBoardModel::selectPiece(int index)
{
    if (!canSelectPiece(index)) {
        qDebug() << "不是你的回合！当前是" << (isRedTurn() ? "红方" : "黑方") << "回合";
        return;
    }

    // 单选逻辑：如果点击的是当前悬浮的棋子，则放下；否则切换到新棋子
    if (m_liftedPieceIndex == index) {
        setLiftedPieceIndex(-1);  // 放下当前棋子
    } else {
        setLiftedPieceIndex(index);  // 提起新棋子
    }

    const ChessPiece &piece = m_piecesList[index];
    qDebug() << "点击了棋子:" << piece.chineseName() << "位置:" << piece.row() << piece.col();
}

void ChessBoardModel::resetBoard()
{
    beginResetModel();
    m_position.board().initializeStartPosition();
    m_position.setCurrentTurn(PieceColor::Red);
    m_position.setHalfMoveClock(0);
    m_position.setFullMoveNumber(1);
    rebuildPiecesList();
    setLiftedPieceIndex(-1);
    endResetModel();

    emit isRedTurnChanged();
    emit fenStringChanged();
    emit boardChanged();
}

bool ChessBoardModel::loadFromFen(const QString &fen)
{
    Position newPosition;
    if (!newPosition.fromFen(fen)) {
        qDebug() << "加载 FEN 失败:" << fen;
        return false;
    }

    beginResetModel();
    m_position = newPosition;
    rebuildPiecesList();
    setLiftedPieceIndex(-1);
    endResetModel();

    emit isRedTurnChanged();
    emit fenStringChanged();
    emit boardChanged();

    qDebug() << "成功加载 FEN:" << fen;
    return true;
}

void ChessBoardModel::printDebugInfo()
{
    qDebug() << "========== 调试信息 ==========";
    m_position.print();
}

void ChessBoardModel::rebuildPiecesList()
{
    m_piecesList = m_position.board().getAllPieces();
}
