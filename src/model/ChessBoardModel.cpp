#include "ChessBoardModel.h"
#include <QDebug>
#include <QRandomGenerator>
#include <QtConcurrent/QtConcurrent>

ChessBoardModel::ChessBoardModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_liftedPieceIndex(-1)
    , m_gameStatus("进行中")
    , m_gameController(this)
    , m_ai(this)
    , m_aiEnabled(false)
    , m_aiThinking(false)
    , m_isTwoPlayerMode(false)
    , m_boardRotation(0)
{
    // 初始化为开局局面
    m_position.board().initializeStartPosition();
    rebuildPiecesList();

    // 创建AI定时器
    m_aiTimer = new QTimer(this);
    m_aiTimer->setSingleShot(true);
    connect(m_aiTimer, &QTimer::timeout, this, &ChessBoardModel::executeAIMove);

    // 创建AI任务监视器
    m_aiWatcher = new QFutureWatcher<AIMove>(this);
    connect(m_aiWatcher, &QFutureWatcher<AIMove>::finished, this, &ChessBoardModel::onAIFinished);

    // 连接游戏控制器信号
    connect(&m_gameController, &GameController::undoAvailableChanged, this, &ChessBoardModel::canUndoChanged);
    connect(&m_gameController, &GameController::redoAvailableChanged, this, &ChessBoardModel::canRedoChanged);
    connect(&m_gameController, &GameController::moveHistoryChanged, this, &ChessBoardModel::moveCountChanged);

    // 设置AI默认难度
    m_ai.setDifficulty(AIDifficulty::Medium);

    // 开始新游戏（传入当前局面的FEN）
    m_gameController.startNewGame(m_position.toFen());
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

        // 更新可走位置
        updateValidMoves();
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
    // 如果AI正在思考，禁止用户操作
    if (m_aiThinking) {
        qDebug() << "AI正在思考，请等待...";
        return;
    }

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

bool ChessBoardModel::movePieceToPosition(int fromIndex, int toRow, int toCol)
{
    // 如果AI正在思考，禁止用户操作
    if (m_aiThinking) {
        qDebug() << "AI正在思考，请等待...";
        return false;
    }

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

    // 记录移动棋子和目标位置的棋子（用于历史记录）
    const ChessPiece *movedPiece = m_position.board().pieceAt(fromRow, fromCol);
    const ChessPiece *targetPiece = m_position.board().pieceAt(toRow, toCol);
    QString capturedPiece = targetPiece ? targetPiece->chineseName() : "";
    bool isCapture = (targetPiece != nullptr);  // 记录是否吃子

    // 执行移动
    m_position.board().movePiece(fromRow, fromCol, toRow, toCol);

    // 发射棋子移动信号（用于音效）
    emit pieceMoved(isCapture);

    // 切换回合
    m_position.switchTurn();
    m_position.incrementFullMoveNumber();

    // 记录走棋历史（使用移动前保存的棋子信息）
    if (movedPiece) {
        m_gameController.recordMove(m_position, *movedPiece, fromRow, fromCol, toRow, toCol, capturedPiece);
    }

    // 更新模型（使用辅助方法）
    updateModelAfterMove(fromRow, fromCol, toRow, toCol, movedPiece, targetPiece);

    // 检查游戏状态（将军、将死、困毙）
    checkGameStatus();

    qDebug() << "移动成功: " << fromRow << fromCol << "->" << toRow << toCol;

    // 如果AI启用且切换到黑方回合，触发AI走棋
    if (m_aiEnabled && !isRedTurn()) {
        triggerAIMove();
    }

    return true;
}

void ChessBoardModel::resetBoardState()
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
    emit moveHistoryChanged();  // 通知历史记录已重置

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

    // 重置游戏控制器（使用新加载的局面作为起点）
    m_gameController.startNewGame(m_position.toFen());

    emit isRedTurnChanged();
    emit fenStringChanged();
    emit boardChanged();
    emit moveHistoryChanged();  // 通知历史记录已重置

    // 检查加载后的游戏状态
    checkGameStatus();

    qDebug() << "成功加载 FEN:" << fen;
    return true;
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

    // 首先检查双方的将/帅是否还存在
    bool redKingExists = false;
    bool blackKingExists = false;

    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const ChessPiece *piece = m_position.board().pieceAt(row, col);
            if (piece && piece->type() == PieceType::King) {
                if (piece->color() == PieceColor::Red) {
                    redKingExists = true;
                } else if (piece->color() == PieceColor::Black) {
                    blackKingExists = true;
                }
            }
        }
    }

    // 如果某一方的将/帅被吃掉，游戏结束
    if (!redKingExists) {
        m_gameStatus = "黑方胜 - 红方帅被吃";
        qDebug() << "游戏结束:" << m_gameStatus;
        emit checkmateDetected();  // 发射将死信号（用于音效）
        emit gameOver(m_gameStatus);
        emit gameStatusChanged();
        return;
    }

    if (!blackKingExists) {
        m_gameStatus = "红方胜 - 黑方将被吃";
        qDebug() << "游戏结束:" << m_gameStatus;
        emit checkmateDetected();  // 发射将死信号（用于音效）
        emit gameOver(m_gameStatus);
        emit gameStatusChanged();
        return;
    }

    // 检查将死
    if (ChessRules::isCheckmate(m_position.board(), currentColor)) {
        m_gameStatus = opponentName + "胜 - " + colorName + "被将死";
        qDebug() << "游戏结束:" << m_gameStatus;
        emit checkmateDetected();  // 发射将死信号（用于音效）
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
        // currentColor 是当前回合方（刚被走棋后的一方）
        // 如果他被将军了，说明对方（刚走完棋的一方）将了他的军
        m_gameStatus = opponentName + "将军！" + colorName + "被将军";
        qDebug() << m_gameStatus;
        emit checkDetected();  // 发射将军信号（用于音效）
        emit gameStatusChanged();
        return;
    }

    // 正常进行中
    m_gameStatus = colorName + "走棋";
    emit gameStatusChanged();
}

void ChessBoardModel::updateValidMoves()
{
    m_validMovePositions.clear();

    // 如果没有选中棋子，清空可走位置
    if (m_liftedPieceIndex < 0 || m_liftedPieceIndex >= m_piecesList.count()) {
        emit validMovePositionsChanged();
        return;
    }

    const ChessPiece &piece = m_piecesList[m_liftedPieceIndex];

    // 获取该棋子的所有合法走法
    m_validMovePositions = ChessRules::getLegalMoves(m_position.board(),
                                                     piece.row(),
                                                     piece.col());

    qDebug() << "棋子" << piece.chineseName()
             << "在位置(" << piece.row() << "," << piece.col() << ")"
             << "有" << m_validMovePositions.size() << "个合法走法";

    emit validMovePositionsChanged();
}

bool ChessBoardModel::canUndo() const
{
    return m_gameController.canUndo();
}

bool ChessBoardModel::canRedo() const
{
    return m_gameController.canRedo();
}

int ChessBoardModel::moveCount() const
{
    return m_gameController.getCurrentMoveNumber();
}

QStringList ChessBoardModel::moveHistory() const
{
    return m_gameController.getMoveHistoryList();
}

void ChessBoardModel::undoMove()
{
    // 在人机对战模式下，悔棋需要回退2步（玩家和AI各一步）
    // 在双人对战模式下，悔棋只需回退1步
    int undoSteps = (m_aiEnabled && !m_isTwoPlayerMode) ? 2 : 1;

    // 检查是否有足够的步数可以悔棋
    if (m_gameController.getCurrentMoveNumber() < undoSteps) {
        qDebug() << "无法悔棋：历史记录不足" << undoSteps << "步";
        return;
    }

    // 执行悔棋
    for (int i = 0; i < undoSteps; ++i) {
        if (!m_gameController.undo(m_position)) {
            qDebug() << "悔棋失败在第" << (i + 1) << "步";
            break;
        }
    }

    // 重建棋盘显示
    beginResetModel();
    rebuildPiecesList();
    setLiftedPieceIndex(-1);
    endResetModel();

    emit isRedTurnChanged();
    emit fenStringChanged();
    emit boardChanged();
    emit moveHistoryChanged();
    checkGameStatus();

    qDebug() << "悔棋成功，回退了" << undoSteps << "步，当前步数:" << moveCount()
             << "当前回合:" << (m_position.currentTurn() == PieceColor::Red ? "红方" : "黑方");
}

void ChessBoardModel::redoMove()
{
    // 在人机对战模式下，重做需要恢复2步（玩家和AI各一步）
    // 在双人对战模式下，重做只需恢复1步
    int redoSteps = (m_aiEnabled && !m_isTwoPlayerMode) ? 2 : 1;

    // 检查是否有足够的步数可以重做
    if (!m_gameController.canRedo()) {
        qDebug() << "无法重做：没有可重做的历史";
        return;
    }

    // 执行重做
    for (int i = 0; i < redoSteps; ++i) {
        if (!m_gameController.redo(m_position)) {
            qDebug() << "重做失败在第" << (i + 1) << "步";
            break;
        }
    }

    // 重建棋盘显示
    beginResetModel();
    rebuildPiecesList();
    setLiftedPieceIndex(-1);
    endResetModel();

    emit isRedTurnChanged();
    emit fenStringChanged();
    emit boardChanged();
    emit moveHistoryChanged();
    checkGameStatus();

    qDebug() << "重做成功，恢复了" << redoSteps << "步，当前步数:" << moveCount()
             << "当前回合:" << (m_position.currentTurn() == PieceColor::Red ? "红方" : "黑方");
}

void ChessBoardModel::startNewGame()
{
    // 重置棋盘状态
    resetBoardState();

    // 清空游戏控制器的历史记录并重置（传入当前局面的FEN）
    m_gameController.clearHistory();  // 先清空历史
    m_gameController.startNewGame(m_position.toFen());

    // 重置棋盘旋转角度
    m_boardRotation = 0;
    emit boardRotationChanged();

    // 发出历史记录改变信号，确保UI更新
    emit moveHistoryChanged();
    emit canUndoChanged();
    emit canRedoChanged();
    emit moveCountChanged();

    qDebug() << "开始新游戏";
}

QString ChessBoardModel::saveGame() const
{
    // 返回当前局面的FEN字符串
    return m_position.toFen();
}

bool ChessBoardModel::loadGame(const QString &fen)
{
    // 从FEN字符串加载局面
    return loadFromFen(fen);
}

void ChessBoardModel::showHint()
{
    // 如果当前没有选中棋子，选中第一个可移动的己方棋子
    if (m_liftedPieceIndex < 0) {
        PieceColor currentColor = m_position.currentTurn();

        // 查找第一个有合法走法的棋子
        for (int i = 0; i < m_piecesList.count(); ++i) {
            const ChessPiece &piece = m_piecesList[i];
            if (piece.color() == currentColor) {
                QList<QPoint> moves = ChessRules::getLegalMoves(m_position.board(), piece.row(), piece.col());
                if (!moves.isEmpty()) {
                    setLiftedPieceIndex(i);
                    qDebug() << "提示：选中" << piece.chineseName() << "位置:" << piece.row() << piece.col();
                    emit hintShown();
                    return;
                }
            }
        }

        qDebug() << "提示：无可走棋子";
    } else {
        // 如果已经选中棋子，显示可走位置（已经通过updateValidMoves自动显示）
        qDebug() << "提示：显示当前棋子可走位置";
        emit hintShown();
    }
}

void ChessBoardModel::offerDraw()
{
    QString colorName = isRedTurn() ? "红方" : "黑方";

    // 在人机模式下，让AI决定
    if (m_aiEnabled && !m_isTwoPlayerMode) {
        // AI自动决定是否接受和棋
        // 简单逻辑：如果局势对AI不利，接受和棋；否则拒绝
        // 这里可以根据评估函数来决定
        qDebug() << colorName << "提出和棋，AI正在考虑...";

        // 简单策略：50%概率接受（实际应该基于局势评估）
        bool aiAccepts = (QRandomGenerator::global()->bounded(2) == 0);

        if (aiAccepts) {
            QString message = "AI接受和棋";
            emit drawOffered(message);

            // 延迟一下再结束游戏，让玩家看到消息
            QTimer::singleShot(1500, this, [this]() {
                acceptDraw();
            });
        } else {
            QString message = "AI拒绝和棋";
            emit drawDeclined(message);
        }
    } else {
        // 双人模式，发送请求让对方选择
        qDebug() << colorName << "提出和棋";
        emit drawRequested();
    }
}

void ChessBoardModel::acceptDraw()
{
    m_gameStatus = "和棋 - 双方同意";
    qDebug() << "游戏结束:" << m_gameStatus;
    emit gameOver(m_gameStatus);
    emit gameStatusChanged();
}

void ChessBoardModel::declineDraw()
{
    QString opponentName = isRedTurn() ? "黑方" : "红方";
    QString message = opponentName + "拒绝和棋";
    qDebug() << message;
    emit drawDeclined(message);
}

void ChessBoardModel::resign()
{
    QString colorName = isRedTurn() ? "红方" : "黑方";
    QString opponentName = isRedTurn() ? "黑方" : "红方";
    m_gameStatus = opponentName + "胜 - " + colorName + "认输";
    qDebug() << "游戏结束:" << m_gameStatus;
    emit checkmateDetected();  // 发射将死信号（用于音效）
    emit gameOver(m_gameStatus);
    emit gameStatusChanged();
}

void ChessBoardModel::setAiEnabled(bool enabled)
{
    if (m_aiEnabled != enabled) {
        m_aiEnabled = enabled;
        emit aiEnabledChanged();

        qDebug() << "AI对手" << (enabled ? "启用" : "禁用");

        // 如果启用AI且当前是黑方回合，触发AI走棋
        if (enabled && !isRedTurn() && !m_aiThinking) {
            triggerAIMove();
        }
    }
}

void ChessBoardModel::setAiDifficulty(int difficulty)
{
    AIDifficulty aiDiff = static_cast<AIDifficulty>(difficulty);
    m_ai.setDifficulty(aiDiff);
    emit aiDifficultyChanged();
    qDebug() << "AI难度设置为:" << difficulty;
}

void ChessBoardModel::setIsTwoPlayerMode(bool enabled)
{
    if (m_isTwoPlayerMode != enabled) {
        m_isTwoPlayerMode = enabled;
        emit isTwoPlayerModeChanged();

        qDebug() << "游戏模式:" << (enabled ? "双人对战" : "人机对战");

        // 双人模式时自动禁用AI
        if (enabled && m_aiEnabled) {
            setAiEnabled(false);
        }

        // 重置棋盘旋转
        if (!enabled) {
            m_boardRotation = 0;
            emit boardRotationChanged();
        }
    }
}

void ChessBoardModel::rotateBoardIfNeeded()
{
    // 只在双人模式下旋转棋盘
    if (m_isTwoPlayerMode) {
        m_boardRotation = (m_boardRotation == 0) ? 180 : 0;
        emit boardRotationChanged();
        qDebug() << "棋盘旋转至:" << m_boardRotation << "度";
    }
}

void ChessBoardModel::triggerAIMove()
{
    if (!m_aiEnabled || m_aiThinking) {
        return;
    }

    // 检查游戏是否结束
    if (m_gameStatus.contains("胜") || m_gameStatus.contains("和棋")) {
        return;
    }

    // 检查是否是AI的回合（黑方）
    if (isRedTurn()) {
        return;  // AI是黑方，红方回合不走棋
    }

    qDebug() << "触发AI走棋...";

    // 延迟500ms再走棋，让用户能看清楚红方的走法
    m_aiTimer->start(500);
}

void ChessBoardModel::executeAIMove()
{
    if (m_aiThinking) {
        qDebug() << "AI正在思考，忽略重复请求";
        return;
    }

    m_aiThinking = true;
    emit aiThinkingChanged();

    qDebug() << "AI开始思考...";

    // 在后台线程中执行AI思考
    QFuture<AIMove> future = QtConcurrent::run([this]() {
        return m_ai.getBestMove(m_position);
    });

    m_aiWatcher->setFuture(future);
}

void ChessBoardModel::onAIFinished()
{
    // 获取AI思考结果
    AIMove bestMove = m_aiWatcher->result();

    m_aiThinking = false;
    emit aiThinkingChanged();

    if (!bestMove.isValid()) {
        qDebug() << "AI无法找到有效移动";
        return;
    }

    qDebug() << "AI选择移动:" << bestMove.fromRow << bestMove.fromCol
             << "->" << bestMove.toRow << bestMove.toCol;

    // 执行AI的移动
    int fromRow = bestMove.fromRow;
    int fromCol = bestMove.fromCol;
    int toRow = bestMove.toRow;
    int toCol = bestMove.toCol;

    // 记录移动棋子和目标位置的棋子
    const ChessPiece *movedPiece = m_position.board().pieceAt(fromRow, fromCol);
    const ChessPiece *targetPiece = m_position.board().pieceAt(toRow, toCol);
    QString capturedPiece = targetPiece ? targetPiece->chineseName() : "";
    bool isCapture = (targetPiece != nullptr);  // 记录是否吃子

    // 执行移动
    m_position.board().movePiece(fromRow, fromCol, toRow, toCol);

    // 发射棋子移动信号（用于音效）
    emit pieceMoved(isCapture);

    // 切换回合
    m_position.switchTurn();
    m_position.incrementFullMoveNumber();

    // 记录走棋历史
    if (movedPiece) {
        m_gameController.recordMove(m_position, *movedPiece, fromRow, fromCol, toRow, toCol, capturedPiece);
    }

    // 更新模型（使用辅助方法）
    updateModelAfterMove(fromRow, fromCol, toRow, toCol, movedPiece, targetPiece);

    // 检查游戏状态
    checkGameStatus();

    qDebug() << "AI走棋完成";
}

// 辅助方法：执行移动并更新模型
void ChessBoardModel::updateModelAfterMove(int fromRow, int fromCol, int toRow, int toCol,
                                            const ChessPiece *movedPiece, const ChessPiece *targetPiece)
{
    // 找到移动的棋子在列表中的索引
    int fromIndex = -1;
    for (int i = 0; i < m_piecesList.count(); ++i) {
        if (m_piecesList[i].row() == fromRow && m_piecesList[i].col() == fromCol) {
            fromIndex = i;
            break;
        }
    }

    // 更新模型 - 使用细粒度更新而不是重建
    // 1. 如果有被吃的棋子，先从列表中移除
    if (targetPiece) {
        // 找到被吃棋子在列表中的索引
        int capturedIndex = -1;
        for (int i = 0; i < m_piecesList.count(); ++i) {
            if (m_piecesList[i].row() == toRow && m_piecesList[i].col() == toCol) {
                capturedIndex = i;
                break;
            }
        }

        if (capturedIndex >= 0) {
            beginRemoveRows(QModelIndex(), capturedIndex, capturedIndex);
            m_piecesList.removeAt(capturedIndex);
            endRemoveRows();

            // 如果移动的棋子索引在被吃棋子之后，需要调整索引
            if (fromIndex > capturedIndex) {
                fromIndex--;
            }
        }
    }

    // 2. 更新移动棋子的位置
    if (fromIndex >= 0 && fromIndex < m_piecesList.count() && movedPiece) {
        m_piecesList[fromIndex] = ChessPiece(movedPiece->type(), movedPiece->color(), toRow, toCol);
        QModelIndex changedIndex = index(fromIndex);
        emit dataChanged(changedIndex, changedIndex);
    }

    setLiftedPieceIndex(-1);

    emit isRedTurnChanged();
    emit fenStringChanged();
    emit boardChanged();
    emit moveHistoryChanged();

    // 双人模式下旋转棋盘
    rotateBoardIfNeeded();
}
