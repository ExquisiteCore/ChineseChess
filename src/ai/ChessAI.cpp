#include "ChessAI.h"
#include <QDebug>
#include <algorithm>

ChessAI::ChessAI(QObject *parent)
    : QObject(parent)
    , m_difficulty(AIDifficulty::Medium)
    , m_maxDepth(3)
    , m_nodesSearched(0)
    , m_pruneCount(0)
{
}

void ChessAI::setDifficulty(AIDifficulty difficulty)
{
    m_difficulty = difficulty;
    m_maxDepth = static_cast<int>(difficulty) + 1;  // Easy=2, Medium=3, Hard=4, Expert=5
    qDebug() << "AI难度设置为:" << m_maxDepth << "层搜索";
}

void ChessAI::resetStatistics()
{
    m_nodesSearched = 0;
    m_pruneCount = 0;
}

AIMove ChessAI::getBestMove(const Position &position)
{
    resetStatistics();
    qDebug() << "=== AI开始思考 ===";
    qDebug() << "搜索深度:" << m_maxDepth;

    PieceColor aiColor = position.currentTurn();
    bool isMaximizing = (aiColor == PieceColor::Red);

    // 创建位置的副本用于搜索
    Position searchPos = position;

    // 生成所有可能的移动
    QList<AIMove> allMoves = generateAllMoves(searchPos, aiColor);

    if (allMoves.isEmpty()) {
        qDebug() << "没有可用的移动";
        return AIMove();
    }

    // 对移动进行排序以提高剪枝效率
    sortMoves(allMoves, searchPos);

    AIMove bestMove;
    int bestScore = isMaximizing ? -INF : INF;

    qDebug() << "评估" << allMoves.size() << "个可能的移动...";

    // 遍历所有移动
    for (int i = 0; i < allMoves.size(); ++i) {
        AIMove &move = allMoves[i];

        // 创建临时局面
        Position tempPos = searchPos;

        // 执行移动
        tempPos.board().movePiece(move.fromRow, move.fromCol, move.toRow, move.toCol);
        tempPos.switchTurn();

        // Minimax搜索
        int score = minimax(tempPos, m_maxDepth - 1, -INF, INF, !isMaximizing);

        move.score = score;

        // 更新最佳移动
        if (isMaximizing) {
            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
        } else {
            if (score < bestScore) {
                bestScore = score;
                bestMove = move;
            }
        }

        // 发射进度信号
        if ((i + 1) % 5 == 0 || i == allMoves.size() - 1) {
            emit searchProgress(m_maxDepth, m_nodesSearched);
        }
    }

    qDebug() << "最佳移动:" << bestMove.fromRow << bestMove.fromCol
             << "->" << bestMove.toRow << bestMove.toCol
             << "评分:" << bestScore;
    qDebug() << "搜索节点数:" << m_nodesSearched;
    qDebug() << "剪枝次数:" << m_pruneCount;

    emit moveFound(bestMove.fromRow, bestMove.fromCol, bestMove.toRow, bestMove.toCol, bestScore);

    return bestMove;
}

int ChessAI::minimax(Position &position, int depth, int alpha, int beta, bool isMaximizing)
{
    m_nodesSearched++;

    // 叶子节点：返回评估值
    if (depth == 0) {
        return evaluatePosition(position);
    }

    PieceColor currentColor = position.currentTurn();

    // 检查游戏结束状态
    if (ChessRules::isCheckmate(position.board(), currentColor)) {
        // 将死：对搜索方来说是最坏的情况
        return isMaximizing ? -MATE_SCORE + (m_maxDepth - depth) : MATE_SCORE - (m_maxDepth - depth);
    }

    if (ChessRules::isStalemate(position.board(), currentColor)) {
        // 困毙：和棋
        return 0;
    }

    // 生成所有可能的移动
    QList<AIMove> moves = generateAllMoves(position, currentColor);

    if (moves.isEmpty()) {
        return 0;  // 无棋可走，和棋
    }

    // 移动排序
    sortMoves(moves, position);

    if (isMaximizing) {
        // 最大化节点（红方）
        int maxEval = -INF;

        for (const AIMove &move : moves) {
            // 创建临时局面
            Position tempPos = position;

            // 执行移动
            tempPos.board().movePiece(move.fromRow, move.fromCol, move.toRow, move.toCol);
            tempPos.switchTurn();

            // 递归搜索
            int eval = minimax(tempPos, depth - 1, alpha, beta, false);

            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);

            // Beta剪枝
            if (beta <= alpha) {
                m_pruneCount++;
                break;
            }
        }

        return maxEval;
    } else {
        // 最小化节点（黑方）
        int minEval = INF;

        for (const AIMove &move : moves) {
            // 创建临时局面
            Position tempPos = position;

            // 执行移动
            tempPos.board().movePiece(move.fromRow, move.fromCol, move.toRow, move.toCol);
            tempPos.switchTurn();

            // 递归搜索
            int eval = minimax(tempPos, depth - 1, alpha, beta, true);

            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);

            // Alpha剪枝
            if (beta <= alpha) {
                m_pruneCount++;
                break;
            }
        }

        return minEval;
    }
}

QList<AIMove> ChessAI::generateAllMoves(const Position &position, PieceColor color)
{
    QList<AIMove> moves;

    // 遍历棋盘上的所有己方棋子
    for (int fromRow = 0; fromRow < Board::ROWS; ++fromRow) {
        for (int fromCol = 0; fromCol < Board::COLS; ++fromCol) {
            const ChessPiece *piece = position.board().pieceAt(fromRow, fromCol);

            if (!piece || !piece->isValid() || piece->color() != color) {
                continue;
            }

            // 获取该棋子的所有合法移动
            QList<QPoint> legalMoves = ChessRules::getLegalMoves(position.board(), fromRow, fromCol);

            for (const QPoint &dest : legalMoves) {
                moves.append(AIMove(fromRow, fromCol, dest.y(), dest.x()));
            }
        }
    }

    return moves;
}

int ChessAI::evaluatePosition(const Position &position)
{
    int score = 0;

    // 1. 棋子材料价值
    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const ChessPiece *piece = position.board().pieceAt(row, col);
            if (piece && piece->isValid()) {
                int value = getPieceValue(piece);
                int posValue = getPositionValue(piece);

                if (piece->color() == PieceColor::Red) {
                    score += value + posValue;
                } else {
                    score -= value + posValue;
                }
            }
        }
    }

    // 2. 灵活性（可走步数）
    score += getMobilityScore(position, PieceColor::Red);
    score -= getMobilityScore(position, PieceColor::Black);

    // 3. 将帅安全性
    score += getKingSafetyScore(position, PieceColor::Red);
    score -= getKingSafetyScore(position, PieceColor::Black);

    // 4. 控制中心
    score += getCenterControlScore(position, PieceColor::Red);
    score -= getCenterControlScore(position, PieceColor::Black);

    return score;
}

int ChessAI::getPieceValue(const ChessPiece *piece) const
{
    if (!piece || !piece->isValid()) {
        return 0;
    }

    // 中国象棋棋子价值（单位：分）
    switch (piece->type()) {
    case PieceType::King:
        return 10000;  // 将/帅：无价（但需要一个值用于计算）
    case PieceType::Rook:
        return 600;    // 车：最强大的棋子
    case PieceType::Cannon:
        return 300;    // 炮：开局强，残局弱
    case PieceType::Horse:
        return 300;    // 马：灵活但易被蹩
    case PieceType::Advisor:
        return 120;    // 士：保护将帅
    case PieceType::Elephant:
        return 120;    // 象/相：防守棋子
    case PieceType::Pawn:
        return 100;    // 兵/卒：过河后价值增加
    default:
        return 0;
    }
}

int ChessAI::getPositionValue(const ChessPiece *piece) const
{
    if (!piece || !piece->isValid()) {
        return 0;
    }

    int row = piece->row();
    int col = piece->col();
    int posValue = 0;

    // 根据棋子类型和位置给予额外分数
    switch (piece->type()) {
    case PieceType::Pawn: {
        // 兵/卒：过河后价值翻倍，越靠近敌方越有价值
        bool crossed = !Board::isInOwnHalf(row, col, piece->color());
        if (crossed) {
            posValue += 50;  // 过河奖励
            // 越靠近敌方底线越有价值
            int advanceBonus = (piece->color() == PieceColor::Red) ? (9 - row) * 10 : row * 10;
            posValue += advanceBonus;
        }
        // 中路兵更有价值
        if (col >= 3 && col <= 5) {
            posValue += 10;
        }
        break;
    }
    case PieceType::Horse:
    case PieceType::Cannon:
        // 马和炮：中心位置更有价值
        if (row >= 3 && row <= 6 && col >= 3 && col <= 5) {
            posValue += 20;
        }
        break;
    case PieceType::Rook:
        // 车：在敌方半场更有威胁
        if (!Board::isInOwnHalf(row, col, piece->color())) {
            posValue += 30;
        }
        break;
    default:
        break;
    }

    return posValue;
}

int ChessAI::getMobilityScore(const Position &position, PieceColor color)
{
    int mobility = 0;

    // 计算该方所有棋子的可走步数
    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const ChessPiece *piece = position.board().pieceAt(row, col);
            if (piece && piece->isValid() && piece->color() == color) {
                QList<QPoint> moves = ChessRules::getLegalMoves(position.board(), row, col);
                mobility += moves.size();
            }
        }
    }

    // 灵活性得分（每个可走步2分）
    return mobility * 2;
}

int ChessAI::getKingSafetyScore(const Position &position, PieceColor color)
{
    int safety = 0;

    ChessPiece *king = const_cast<Board&>(position.board()).findKing(color);
    if (!king) {
        return -10000;  // 没有将/帅，极度危险
    }

    // 检查是否被将军
    if (ChessRules::isInCheck(position.board(), color)) {
        safety -= 500;  // 被将军惩罚
    }

    int kingRow = king->row();
    int kingCol = king->col();

    // 检查将/帅周围的保护
    int protectors = 0;
    const int dr[] = {-1, 1, 0, 0};
    const int dc[] = {0, 0, -1, 1};

    for (int i = 0; i < 4; ++i) {
        int newRow = kingRow + dr[i];
        int newCol = kingCol + dc[i];

        if (Board::isValidPosition(newRow, newCol)) {
            const ChessPiece *neighbor = position.board().pieceAt(newRow, newCol);
            if (neighbor && neighbor->isValid() && neighbor->color() == color) {
                if (neighbor->type() == PieceType::Advisor) {
                    protectors += 2;  // 士的保护价值更高
                } else {
                    protectors += 1;
                }
            }
        }
    }

    safety += protectors * 20;

    return safety;
}

int ChessAI::getCenterControlScore(const Position &position, PieceColor color)
{
    int control = 0;

    // 中心区域（河界附近）
    const int centerRows[] = {4, 5};
    const int centerCols[] = {3, 4, 5};

    for (int row : centerRows) {
        for (int col : centerCols) {
            const ChessPiece *piece = position.board().pieceAt(row, col);
            if (piece && piece->isValid() && piece->color() == color) {
                // 控制中心区域的棋子加分
                control += 15;

                // 强力棋子控制中心更有价值
                if (piece->type() == PieceType::Rook || piece->type() == PieceType::Cannon) {
                    control += 10;
                }
            }
        }
    }

    return control;
}

void ChessAI::sortMoves(QList<AIMove> &moves, const Position &position)
{
    // 使用快速评估给每个移动打分
    for (AIMove &move : moves) {
        move.score = quickEvaluateMove(position, move);
    }

    // 按分数降序排序（好的移动优先搜索）
    std::sort(moves.begin(), moves.end(), [](const AIMove &a, const AIMove &b) {
        return a.score > b.score;
    });
}

int ChessAI::quickEvaluateMove(const Position &position, const AIMove &move)
{
    int score = 0;

    const ChessPiece *movingPiece = position.board().pieceAt(move.fromRow, move.fromCol);
    const ChessPiece *targetPiece = position.board().pieceAt(move.toRow, move.toCol);

    // 吃子移动优先
    if (targetPiece && targetPiece->isValid()) {
        score += getPieceValue(targetPiece) * 10;  // 吃子得分

        // MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
        // 用小子吃大子更好
        score -= getPieceValue(movingPiece);
    }

    // 移动到中心位置
    if (move.toRow >= 3 && move.toRow <= 6 && move.toCol >= 3 && move.toCol <= 5) {
        score += 50;
    }

    // 将军的移动优先
    Position tempPos = position;
    tempPos.board().movePiece(move.fromRow, move.fromCol, move.toRow, move.toCol);
    PieceColor enemyColor = (position.currentTurn() == PieceColor::Red) ? PieceColor::Black : PieceColor::Red;
    if (ChessRules::isInCheck(tempPos.board(), enemyColor)) {
        score += 1000;  // 将军高优先级
    }

    return score;
}
