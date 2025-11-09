#include "GameController.h"
#include "ChessRules.h"
#include <QDebug>
#include <QDate>

GameController::GameController(QObject *parent)
    : QObject(parent)
    , m_gameState(GameState::NotStarted)
{
}

void GameController::startNewGame(const QString &initialFen)
{
    m_moveHistory.clear();
    m_redoStack.clear();
    m_gameState = GameState::InProgress;

    // 保存初始局面（如果没有提供，使用标准开局）
    if (initialFen.isEmpty()) {
        m_initialFen = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1";
    } else {
        m_initialFen = initialFen;
    }

    qDebug() << "开始新游戏，初始FEN:" << m_initialFen;

    emit moveHistoryChanged();
    emit gameStateChanged(m_gameState);
    emit undoAvailableChanged(false);
    emit redoAvailableChanged(false);
}

void GameController::recordMove(const Position &position, const ChessPiece &movedPiece, int fromRow, int fromCol, int toRow, int toCol, const QString &capturedPiece)
{
    // 生成记谱法
    QString notation = generateNotation(movedPiece, fromRow, fromCol, toRow, toCol);

    // 创建移动记录
    MoveRecord record(position.toFen(), notation, fromRow, fromCol, toRow, toCol, capturedPiece);

    // 添加到历史记录
    m_moveHistory.push(record);

    // 清空重做栈（新的走棋会清除之前的重做历史）
    m_redoStack.clear();

    qDebug() << "记录走棋:" << notation << "步数:" << m_moveHistory.size();

    emit moveHistoryChanged();
    emit undoAvailableChanged(true);
    emit redoAvailableChanged(false);
}

bool GameController::undo(Position &position)
{
    if (!canUndo()) {
        qDebug() << "无法悔棋：历史记录为空";
        return false;
    }

    // 将当前移动压入重做栈
    MoveRecord currentMove = m_moveHistory.pop();
    m_redoStack.push(currentMove);

    // 恢复到前一个局面
    if (m_moveHistory.isEmpty()) {
        // 恢复到初始局面
        position.fromFen(m_initialFen);
        qDebug() << "悔棋到初始局面";
    } else {
        // 恢复到前一步
        const MoveRecord &prevMove = m_moveHistory.top();
        position.fromFen(prevMove.fen);
        qDebug() << "悔棋到第" << m_moveHistory.size() << "步";
    }

    emit moveHistoryChanged();
    emit undoAvailableChanged(canUndo());
    emit redoAvailableChanged(true);

    return true;
}

bool GameController::redo(Position &position)
{
    if (!canRedo()) {
        qDebug() << "无法重做：重做栈为空";
        return false;
    }

    // 从重做栈中取出移动
    MoveRecord redoMove = m_redoStack.pop();
    m_moveHistory.push(redoMove);

    // 恢复局面
    position.fromFen(redoMove.fen);

    qDebug() << "重做到第" << m_moveHistory.size() << "步";

    emit moveHistoryChanged();
    emit undoAvailableChanged(true);
    emit redoAvailableChanged(canRedo());

    return true;
}

bool GameController::canUndo() const
{
    return !m_moveHistory.isEmpty();
}

bool GameController::canRedo() const
{
    return !m_redoStack.isEmpty();
}

void GameController::setGameState(GameState state)
{
    if (m_gameState != state) {
        m_gameState = state;
        qDebug() << "游戏状态改变:" << getGameStateDescription();
        emit gameStateChanged(state);
    }
}

QString GameController::getGameStateDescription() const
{
    switch (m_gameState) {
    case GameState::NotStarted:
        return "未开始";
    case GameState::InProgress:
        return "进行中";
    case GameState::Check:
        return "将军";
    case GameState::Checkmate:
        return "将死";
    case GameState::Stalemate:
        return "困毙（和棋）";
    case GameState::Draw:
        return "和棋";
    default:
        return "未知状态";
    }
}

void GameController::clearHistory()
{
    m_moveHistory.clear();
    m_redoStack.clear();

    emit moveHistoryChanged();
    emit undoAvailableChanged(false);
    emit redoAvailableChanged(false);
}

QString GameController::exportToPGN() const
{
    QString pgn;
    pgn += "[Event \"中国象棋对局\"]\n";
    pgn += "[Date \"" + QDate::currentDate().toString("yyyy.MM.dd") + "\"]\n";
    pgn += "[Result \"*\"]\n\n";

    int moveNum = 1;
    for (int i = 0; i < m_moveHistory.size(); ++i) {
        const MoveRecord &record = m_moveHistory.at(i);

        if (i % 2 == 0) {
            pgn += QString::number(moveNum) + ". ";
        }

        pgn += record.notation + " ";

        if (i % 2 == 1) {
            pgn += "\n";
            moveNum++;
        }
    }

    return pgn;
}

QStringList GameController::getMoveHistoryList() const
{
    QStringList historyList;

    for (int i = 0; i < m_moveHistory.size(); i += 2) {
        QString move = QString::number(i / 2 + 1) + ". ";

        // 红方走棋
        move += m_moveHistory.at(i).notation;

        // 黑方走棋（如果存在）
        if (i + 1 < m_moveHistory.size()) {
            move += " " + m_moveHistory.at(i + 1).notation;
        }

        historyList.append(move);
    }

    return historyList;
}

QString GameController::generateNotation(const ChessPiece &piece, int fromRow, int fromCol, int toRow, int toCol) const
{
    // 简化的记谱法实现
    // 完整的中国象棋记谱法需要考虑：同类棋子区分（前/后）、方向（进/退/平）等

    QString notation = piece.chineseName();

    // 简化版：起点->终点
    int rowDiff = toRow - fromRow;

    if (rowDiff == 0) {
        notation += "平";
    } else if ((piece.color() == PieceColor::Red && rowDiff < 0) ||
               (piece.color() == PieceColor::Black && rowDiff > 0)) {
        notation += "进";
    } else {
        notation += "退";
    }

    // 添加列号（简化版）
    notation += QString::number(toCol + 1);

    return notation;
}
