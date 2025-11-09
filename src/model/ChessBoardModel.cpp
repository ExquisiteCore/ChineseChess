#include "ChessBoardModel.h"
#include <QDebug>

ChessBoardModel::ChessBoardModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_liftedPieceIndex(-1)
    , m_gameStatus("进行中")
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
    const ChessPiece &piece = m_piecesList[index];

    // 如果已经有悬浮的棋子，尝试走棋（可能是吃子）
    if (m_liftedPieceIndex >= 0 && m_liftedPieceIndex != index) {
        // 尝试移动到该位置（可能是吃子）
        if (movePieceToPosition(m_liftedPieceIndex, piece.row(), piece.col())) {
            qDebug() << "移动成功";
            return;
        }
        // 如果移动失败，继续下面的逻辑（可能想切换选择己方其他棋子）
    }

    // 检查是否能选择这个棋子（必须是己方棋子）
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

    qDebug() << "点击了棋子:" << piece.chineseName() << "位置:" << piece.row() << piece.col();
}

bool ChessBoardModel::movePiece(int fromIndex, int toIndex)
{
    if (fromIndex < 0 || fromIndex >= m_piecesList.count())
        return false;
    if (toIndex < 0 || toIndex >= m_piecesList.count())
        return false;

    const ChessPiece &fromPiece = m_piecesList[fromIndex];
    const ChessPiece &toPiece = m_piecesList[toIndex];

    int fromRow = fromPiece.row();
    int fromCol = fromPiece.col();
    int toRow = toPiece.row();
    int toCol = toPiece.col();

    // 检查移动是否合法
    if (!ChessRules::isValidMove(m_position.board(), fromRow, fromCol, toRow, toCol)) {
        qDebug() << "非法移动";
        return false;
    }

    // 检查是否会导致自己被将军
    if (ChessRules::wouldBeInCheck(m_position.board(), fromRow, fromCol, toRow, toCol)) {
        qDebug() << "此移动会导致被将军，非法";
        return false;
    }

    // 执行移动
    m_position.board().movePiece(fromRow, fromCol, toRow, toCol);

    // 切换回合
    m_position.switchTurn();
    m_position.incrementFullMoveNumber();

    // 重建模型
    beginResetModel();
    rebuildPiecesList();
    setLiftedPieceIndex(-1);
    endResetModel();

    emit isRedTurnChanged();
    emit fenStringChanged();
    emit boardChanged();

    // 检查游戏状态（将军、将死、困毙）
    checkGameStatus();

    qDebug() << "移动成功: " << fromRow << fromCol << "->" << toRow << toCol;
    return true;
}

bool ChessBoardModel::movePieceToPosition(int fromIndex, int toRow, int toCol)
{
    if (fromIndex < 0 || fromIndex >= m_piecesList.count())
        return false;
    if (!Board::isValidPosition(toRow, toCol))
        return false;

    const ChessPiece &fromPiece = m_piecesList[fromIndex];
    int fromRow = fromPiece.row();
    int fromCol = fromPiece.col();

    // 检查移动是否合法
    if (!ChessRules::isValidMove(m_position.board(), fromRow, fromCol, toRow, toCol)) {
        qDebug() << "非法移动";
        return false;
    }

    // 检查是否会导致自己被将军
    if (ChessRules::wouldBeInCheck(m_position.board(), fromRow, fromCol, toRow, toCol)) {
        qDebug() << "此移动会导致被将军，非法";
        return false;
    }

    // 执行移动
    m_position.board().movePiece(fromRow, fromCol, toRow, toCol);

    // 切换回合
    m_position.switchTurn();
    m_position.incrementFullMoveNumber();

    // 重建模型
    beginResetModel();
    rebuildPiecesList();
    setLiftedPieceIndex(-1);
    endResetModel();

    emit isRedTurnChanged();
    emit fenStringChanged();
    emit boardChanged();

    // 检查游戏状态（将军、将死、困毙）
    checkGameStatus();

    qDebug() << "移动成功: " << fromRow << fromCol << "->" << toRow << toCol;
    return true;
}

QList<int> ChessBoardModel::getValidMoves(int index) const
{
    QList<int> validIndices;

    if (index < 0 || index >= m_piecesList.count())
        return validIndices;

    const ChessPiece &piece = m_piecesList[index];
    QList<QPoint> validPositions = ChessRules::getLegalMoves(m_position.board(), piece.row(), piece.col());

    // 将位置转换为棋子索引
    for (const QPoint &pos : validPositions) {
        for (int i = 0; i < m_piecesList.count(); ++i) {
            if (m_piecesList[i].row() == pos.y() && m_piecesList[i].col() == pos.x()) {
                validIndices.append(i);
                break;
            }
        }
    }

    return validIndices;
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

    // 重置后检查游戏状态
    checkGameStatus();
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

void ChessBoardModel::checkGameStatus()
{
    PieceColor currentColor = m_position.currentTurn();
    QString colorName = (currentColor == PieceColor::Red) ? "红方" : "黑方";
    QString opponentName = (currentColor == PieceColor::Red) ? "黑方" : "红方";

    // 检查将死
    if (ChessRules::isCheckmate(m_position.board(), currentColor)) {
        m_gameStatus = opponentName + "胜 - " + colorName + "被将死";
        qDebug() << "游戏结束:" << m_gameStatus;
        emit gameOver(m_gameStatus);
        emit gameStatusChanged();
        return;
    }

    // 检查困毙（和棋）
    if (ChessRules::isStalemate(m_position.board(), currentColor)) {
        m_gameStatus = "和棋 - " + colorName + "被困毙";
        qDebug() << "游戏结束:" << m_gameStatus;
        emit gameOver(m_gameStatus);
        emit gameStatusChanged();
        return;
    }

    // 检查将军
    if (ChessRules::isInCheck(m_position.board(), currentColor)) {
        m_gameStatus = colorName + "被将军！";
        qDebug() << m_gameStatus;
        emit gameStatusChanged();
        return;
    }

    // 正常进行中
    m_gameStatus = colorName + "走棋";
    emit gameStatusChanged();
}
