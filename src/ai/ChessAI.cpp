#include "ChessAI.h"
#include <QDebug>
#include <algorithm>
#include <QRandomGenerator>

ChessAI::ChessAI(QObject *parent)
    : QObject(parent)
    , m_difficulty(AIDifficulty::Medium)
    , m_maxDepth(4)
    , m_nodesSearched(0)
    , m_pruneCount(0)
    , m_ttHits(0)
    , m_qsNodes(0)
    , m_zobristInitialized(false)
{
    initZobristKeys();
    // 初始化历史表和杀手移动
    memset(m_historyTable, 0, sizeof(m_historyTable));
    for (int i = 0; i < 10; i++) {
        m_killerMoves[i][0] = AIMove();
        m_killerMoves[i][1] = AIMove();
    }
}

void ChessAI::setDifficulty(AIDifficulty difficulty)
{
    m_difficulty = difficulty;
    m_maxDepth = static_cast<int>(difficulty) + 2;  // Easy=3, Medium=4, Hard=5, Expert=6
    qDebug() << "AI难度设置为:" << m_maxDepth << "层搜索";
}

void ChessAI::resetStatistics()
{
    m_nodesSearched = 0;
    m_pruneCount = 0;
    m_ttHits = 0;
    m_qsNodes = 0;
    m_transpositionTable.clear();
    // 清空历史表
    memset(m_historyTable, 0, sizeof(m_historyTable));
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

    // 检查置换表中的最佳移动
    quint64 posKey = computeZobristKey(searchPos);
    AIMove *ttMove = nullptr;
    if (m_transpositionTable.contains(posKey)) {
        TTEntry &entry = m_transpositionTable[posKey];
        if (entry.bestMove.isValid()) {
            ttMove = &entry.bestMove;
        }
    }

    // 对移动进行排序以提高剪枝效率
    sortMoves(allMoves, searchPos, ttMove);

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

    // 存储到置换表
    storeTranspositionTable(posKey, m_maxDepth, bestScore, TTEntry::EXACT, bestMove);

    qDebug() << "最佳移动:" << bestMove.fromRow << bestMove.fromCol
             << "->" << bestMove.toRow << bestMove.toCol
             << "评分:" << bestScore;
    qDebug() << "搜索节点数:" << m_nodesSearched << "(静态搜索:" << m_qsNodes << ")";
    qDebug() << "剪枝次数:" << m_pruneCount << "置换表命中:" << m_ttHits;

    emit moveFound(bestMove.fromRow, bestMove.fromCol, bestMove.toRow, bestMove.toCol, bestScore);

    return bestMove;
}

int ChessAI::minimax(Position &position, int depth, int alpha, int beta, bool isMaximizing)
{
    m_nodesSearched++;

    // 检查置换表
    quint64 posKey = computeZobristKey(position);
    int ttScore;
    if (probeTranspositionTable(posKey, depth, alpha, beta, ttScore)) {
        m_ttHits++;
        return ttScore;
    }

    PieceColor currentColor = position.currentTurn();

    // 检查游戏结束状态
    if (ChessRules::isCheckmate(position.board(), currentColor)) {
        // 将死：对搜索方来说是最坏的情况
        int score = isMaximizing ? -MATE_SCORE + (m_maxDepth - depth) : MATE_SCORE - (m_maxDepth - depth);
        storeTranspositionTable(posKey, depth, score, TTEntry::EXACT, AIMove());
        return score;
    }

    if (ChessRules::isStalemate(position.board(), currentColor)) {
        // 困毙：和棋
        storeTranspositionTable(posKey, depth, 0, TTEntry::EXACT, AIMove());
        return 0;
    }

    // 叶子节点：进入静态搜索
    if (depth == 0) {
        int score = quiescence(position, alpha, beta, isMaximizing);
        storeTranspositionTable(posKey, 0, score, TTEntry::EXACT, AIMove());
        return score;
    }

    // 生成所有可能的移动
    QList<AIMove> moves = generateAllMoves(position, currentColor);

    if (moves.isEmpty()) {
        storeTranspositionTable(posKey, depth, 0, TTEntry::EXACT, AIMove());
        return 0;  // 无棋可走，和棋
    }

    // 移动排序（使用置换表、杀手移动、历史启发）
    AIMove *ttMove = nullptr;
    if (m_transpositionTable.contains(posKey)) {
        TTEntry &entry = m_transpositionTable[posKey];
        if (entry.bestMove.isValid()) {
            ttMove = &entry.bestMove;
        }
    }
    sortMoves(moves, position, ttMove);

    AIMove bestMove;
    TTEntry::Flag flag = TTEntry::UPPER_BOUND;

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

            if (eval > maxEval) {
                maxEval = eval;
                bestMove = move;
            }
            alpha = std::max(alpha, eval);

            // Beta剪枝
            if (beta <= alpha) {
                m_pruneCount++;
                // 更新杀手移动和历史表
                if (m_killerMoves[depth][0].fromRow != move.fromRow || m_killerMoves[depth][0].fromCol != move.fromCol) {
                    m_killerMoves[depth][1] = m_killerMoves[depth][0];
                    m_killerMoves[depth][0] = move;
                }
                m_historyTable[move.fromRow][move.fromCol][move.toRow][move.toCol] += depth * depth;
                flag = TTEntry::LOWER_BOUND;
                break;
            }
        }

        if (maxEval > alpha) flag = TTEntry::EXACT;
        storeTranspositionTable(posKey, depth, maxEval, flag, bestMove);
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

            if (eval < minEval) {
                minEval = eval;
                bestMove = move;
            }
            beta = std::min(beta, eval);

            // Alpha剪枝
            if (beta <= alpha) {
                m_pruneCount++;
                // 更新杀手移动和历史表
                if (m_killerMoves[depth][0].fromRow != move.fromRow || m_killerMoves[depth][0].fromCol != move.fromCol) {
                    m_killerMoves[depth][1] = m_killerMoves[depth][0];
                    m_killerMoves[depth][0] = move;
                }
                m_historyTable[move.fromRow][move.fromCol][move.toRow][move.toCol] += depth * depth;
                flag = TTEntry::LOWER_BOUND;
                break;
            }
        }

        if (minEval < beta) flag = TTEntry::EXACT;
        storeTranspositionTable(posKey, depth, minEval, flag, bestMove);
        return minEval;
    }
}

// 静态搜索（解决水平线效应）
int ChessAI::quiescence(Position &position, int alpha, int beta, bool isMaximizing)
{
    m_qsNodes++;

    // 站立评估
    int standPat = evaluatePositionFast(position);

    if (isMaximizing) {
        if (standPat >= beta) return beta;
        if (standPat > alpha) alpha = standPat;
    } else {
        if (standPat <= alpha) return alpha;
        if (standPat < beta) beta = standPat;
    }

    // 只搜索吃子移动
    PieceColor currentColor = position.currentTurn();
    QList<AIMove> captureMoves = generateCaptureMoves(position, currentColor);

    if (captureMoves.isEmpty()) {
        return standPat;
    }

    // 对吃子移动排序
    sortMoves(captureMoves, position);

    if (isMaximizing) {
        for (const AIMove &move : captureMoves) {
            Position tempPos = position;
            tempPos.board().movePiece(move.fromRow, move.fromCol, move.toRow, move.toCol);
            tempPos.switchTurn();

            int score = quiescence(tempPos, alpha, beta, false);

            if (score >= beta) return beta;
            if (score > alpha) alpha = score;
        }
        return alpha;
    } else {
        for (const AIMove &move : captureMoves) {
            Position tempPos = position;
            tempPos.board().movePiece(move.fromRow, move.fromCol, move.toRow, move.toCol);
            tempPos.switchTurn();

            int score = quiescence(tempPos, alpha, beta, true);

            if (score <= alpha) return alpha;
            if (score < beta) beta = score;
        }
        return beta;
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

QList<AIMove> ChessAI::generateCaptureMoves(const Position &position, PieceColor color)
{
    QList<AIMove> moves;

    // 遍历棋盘上的所有己方棋子，只生成吃子移动
    for (int fromRow = 0; fromRow < Board::ROWS; ++fromRow) {
        for (int fromCol = 0; fromCol < Board::COLS; ++fromCol) {
            const ChessPiece *piece = position.board().pieceAt(fromRow, fromCol);

            if (!piece || !piece->isValid() || piece->color() != color) {
                continue;
            }

            QList<QPoint> legalMoves = ChessRules::getLegalMoves(position.board(), fromRow, fromCol);

            for (const QPoint &dest : legalMoves) {
                const ChessPiece *target = position.board().pieceAt(dest.y(), dest.x());
                // 只添加吃子移动
                if (target && target->isValid() && target->color() != color) {
                    moves.append(AIMove(fromRow, fromCol, dest.y(), dest.x()));
                }
            }
        }
    }

    return moves;
}

// 完整评估（带开销较大的计算）
int ChessAI::evaluatePosition(const Position &position)
{
    return evaluatePositionFast(position);
}

// 快速评估（只计算材料+位置价值，不计算灵活性）
int ChessAI::evaluatePositionFast(const Position &position)
{
    int score = 0;

    // 检查将军状态
    if (ChessRules::isInCheck(position.board(), PieceColor::Red)) {
        score -= 50;
    }
    if (ChessRules::isInCheck(position.board(), PieceColor::Black)) {
        score += 50;
    }

    // 计算材料和位置价值
    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const ChessPiece *piece = position.board().pieceAt(row, col);
            if (piece && piece->isValid()) {
                int value = getPieceValue(piece->type(), row, col, piece->color());

                if (piece->color() == PieceColor::Red) {
                    score += value;
                } else {
                    score -= value;
                }
            }
        }
    }

    return score;
}

int ChessAI::getPieceBaseValue(PieceType type) const
{
    switch (type) {
    case PieceType::King:
        return 10000;  // 将/帅
    case PieceType::Rook:
        return 1000;   // 车
    case PieceType::Horse:
        return 350;    // 马
    case PieceType::Cannon:
        return 350;    // 炮
    case PieceType::Advisor:
        return 200;    // 士
    case PieceType::Elephant:
        return 200;    // 象
    case PieceType::Pawn:
        return 100;    // 兵
    default:
        return 0;
    }
}

// 获取棋子价值（基础价值+位置价值）
int ChessAI::getPieceValue(PieceType type, int row, int col, PieceColor color) const
{
    int baseValue = getPieceBaseValue(type);

    // 黑方需要翻转行坐标
    int posRow = (color == PieceColor::Black) ? (9 - row) : row;

    int posValue = 0;
    switch (type) {
    case PieceType::Pawn:
        posValue = PAWN_POS_VALUE[posRow][col];
        break;
    case PieceType::Advisor:
        posValue = ADVISOR_POS_VALUE[posRow][col];
        break;
    case PieceType::Elephant:
        posValue = ELEPHANT_POS_VALUE[posRow][col];
        break;
    case PieceType::Horse:
        posValue = HORSE_POS_VALUE[posRow][col];
        break;
    case PieceType::Rook:
        posValue = ROOK_POS_VALUE[posRow][col];
        break;
    case PieceType::Cannon:
        posValue = CANNON_POS_VALUE[posRow][col];
        break;
    case PieceType::King:
        posValue = KING_POS_VALUE[posRow][col];
        break;
    default:
        break;
    }

    return baseValue + posValue;
}

// 移动排序（使用多种启发）
void ChessAI::sortMoves(QList<AIMove> &moves, const Position &position, const AIMove *ttMove)
{
    int depth = m_maxDepth - (m_nodesSearched % m_maxDepth);  // 近似深度

    // 使用快速评估给每个移动打分
    for (AIMove &move : moves) {
        int score = 0;

        // 1. 置换表移动（最高优先级）
        if (ttMove && ttMove->fromRow == move.fromRow && ttMove->fromCol == move.fromCol &&
            ttMove->toRow == move.toRow && ttMove->toCol == move.toCol) {
            score = 1000000;
        }
        // 2. 杀手移动
        else if (depth < 10) {
            if ((m_killerMoves[depth][0].fromRow == move.fromRow && m_killerMoves[depth][0].fromCol == move.fromCol &&
                 m_killerMoves[depth][0].toRow == move.toRow && m_killerMoves[depth][0].toCol == move.toCol) ||
                (m_killerMoves[depth][1].fromRow == move.fromRow && m_killerMoves[depth][1].fromCol == move.fromCol &&
                 m_killerMoves[depth][1].toRow == move.toRow && m_killerMoves[depth][1].toCol == move.toCol)) {
                score = 500000;
            }
        }

        // 3. MVV-LVA（吃子价值）
        const ChessPiece *target = position.board().pieceAt(move.toRow, move.toCol);
        if (target && target->isValid()) {
            const ChessPiece *attacker = position.board().pieceAt(move.fromRow, move.fromCol);
            if (attacker && attacker->isValid()) {
                score += getPieceBaseValue(target->type()) * 10 - getPieceBaseValue(attacker->type());
            }
        }

        // 4. 历史启发
        score += m_historyTable[move.fromRow][move.fromCol][move.toRow][move.toCol];

        move.score = score;
    }

    // 按分数降序排序
    std::sort(moves.begin(), moves.end(), [](const AIMove &a, const AIMove &b) {
        return a.score > b.score;
    });
}

int ChessAI::quickEvaluateMove(const Position &position, const AIMove &move)
{
    int score = 0;

    const ChessPiece *movingPiece = position.board().pieceAt(move.fromRow, move.fromCol);
    const ChessPiece *targetPiece = position.board().pieceAt(move.toRow, move.toCol);

    // 吃子移动
    if (targetPiece && targetPiece->isValid()) {
        score += getPieceBaseValue(targetPiece->type()) * 10;
        if (movingPiece && movingPiece->isValid()) {
            score -= getPieceBaseValue(movingPiece->type());
        }
    }

    return score;
}

// ============ 置换表和Zobrist哈希实现 ============

void ChessAI::initZobristKeys()
{
    if (m_zobristInitialized) return;

    QRandomGenerator *rng = QRandomGenerator::global();

    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 9; col++) {
            for (int piece = 0; piece < 14; piece++) {
                m_zobristTable[row][col][piece] = (quint64(rng->generate()) << 32) | rng->generate();
            }
        }
    }

    m_zobristInitialized = true;
}

quint64 ChessAI::computeZobristKey(const Position &position)
{
    quint64 key = 0;

    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const ChessPiece *piece = position.board().pieceAt(row, col);
            if (piece && piece->isValid()) {
                int pieceIndex = static_cast<int>(piece->type()) + (piece->color() == PieceColor::Black ? 7 : 0);
                key ^= m_zobristTable[row][col][pieceIndex];
            }
        }
    }

    return key;
}

bool ChessAI::probeTranspositionTable(quint64 key, int depth, int alpha, int beta, int &score)
{
    if (!m_transpositionTable.contains(key)) {
        return false;
    }

    const TTEntry &entry = m_transpositionTable[key];

    if (entry.depth < depth) {
        return false;
    }

    if (entry.flag == TTEntry::EXACT) {
        score = entry.score;
        return true;
    }

    if (entry.flag == TTEntry::LOWER_BOUND && entry.score >= beta) {
        score = entry.score;
        return true;
    }

    if (entry.flag == TTEntry::UPPER_BOUND && entry.score <= alpha) {
        score = entry.score;
        return true;
    }

    return false;
}

void ChessAI::storeTranspositionTable(quint64 key, int depth, int score, TTEntry::Flag flag, const AIMove &bestMove)
{
    if (m_transpositionTable.size() >= TT_SIZE) {
        if (!m_transpositionTable.contains(key) || m_transpositionTable[key].depth < depth) {
            auto it = m_transpositionTable.begin();
            m_transpositionTable.erase(it);
        }
    }

    TTEntry entry;
    entry.zobristKey = key;
    entry.depth = depth;
    entry.score = score;
    entry.flag = flag;
    entry.bestMove = bestMove;

    m_transpositionTable[key] = entry;
}

// ============ 位置价值表定义 ============

const int ChessAI::PAWN_POS_VALUE[10][9] = {
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0, -2,  0,  4,  0, -2,  0,  0},
    { 2,  0,  8,  0,  8,  0,  8,  0,  2},
    { 6,  12, 18, 18, 20, 18, 18, 12, 6},
    { 10, 20, 30, 34, 40, 34, 30, 20, 10},
    { 14, 26, 42, 60, 80, 60, 42, 26, 14},
    { 18, 36, 56, 80, 120, 80, 56, 36, 18},
    { 0,  3,  6,  9,  12,  9,  6,  3,  0}
};

const int ChessAI::ADVISOR_POS_VALUE[10][9] = {
    { 0,  0,  0, 20,  0, 20,  0,  0,  0},
    { 0,  0,  0,  0, 23,  0,  0,  0,  0},
    { 0,  0,  0, 20,  0, 20,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0}
};

const int ChessAI::ELEPHANT_POS_VALUE[10][9] = {
    { 0,  0, 20,  0,  0,  0, 20,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    {18,  0,  0,  0, 23,  0,  0,  0, 18},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0, 20,  0,  0,  0, 20,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0}
};

const int ChessAI::HORSE_POS_VALUE[10][9] = {
    { 0, -3,  5,  4,  2,  4,  5, -3,  0},
    {-3,  2,  4,  6,  10, 6,  4,  2, -3},
    { 4,  6, 12, 11, 15, 11, 12, 6,  4},
    { 2,  6,  8, 11, 11, 11,  8,  6,  2},
    { 2,  12, 11, 15, 16, 15, 11, 12, 2},
    { 0,  5,  7,  7,  14,  7,  7,  5,  0},
    {-5,  2,  4,  8,  8,  8,  4,  2, -5},
    {-6,  3,  2,  5,  4,  5,  2,  3, -6},
    {-8, -3,  1,  4,  4,  4,  1, -3, -8},
    {-10,-8, -6, -3, -1, -3, -6, -8, -10}
};

const int ChessAI::ROOK_POS_VALUE[10][9] = {
    {-6,  5,  8,  8,  8,  8,  8,  5, -6},
    { 6, 8,   10, 14, 15, 14, 10,  8,  6},
    { 4,  6,  8,  12, 12, 12,  8,  6,  4},
    {12, 16, 16, 20, 20, 20, 16, 16, 12},
    {10, 14, 15, 17, 20, 17, 15, 14, 10},
    { 6, 11, 13, 15, 16, 15, 13, 11,  6},
    { 4, 6,   9,  10, 11, 10, 9,   6,  4},
    { 2, 4,   7,  7,  8,  7,  7,   4,  2},
    { 0, 3,   5,  5,  6,  5,  5,   3,  0},
    {-4, 2,   4,  4,  5,  4,  4,   2, -4}
};

const int ChessAI::CANNON_POS_VALUE[10][9] = {
    { 0,  0,  1,  0,  3,  0,  1,  0,  0},
    { 0,  2,  4,  3,  4,  3,  4,  2,  0},
    { 1,  0,  7,  4,  4,  4,  7,  0,  1},
    { 0,  0,  7,  4,  4,  4,  7,  0,  0},
    { 0,  1,  6,  7,  7,  7,  6,  1,  0},
    {-1,  1,  2,  7,  8,  7,  2,  1, -1},
    { 0,  3,  4,  4,  3,  4,  4,  3,  0},
    { 0,  2,  2,  2,  2,  2,  2,  2,  0},
    { 0,  1,  2,  3,  3,  3,  2,  1,  0},
    { 0,  0,  1,  1,  2,  1,  1,  0,  0}
};

const int ChessAI::KING_POS_VALUE[10][9] = {
    { 0,  0,  0,  8,  8,  8,  0,  0,  0},
    { 0,  0,  0,  9,  9,  9,  0,  0,  0},
    { 0,  0,  0, 10, 10, 10,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0,  0}
};

