#include "SearchEngine.h"
#include <algorithm>

SearchEngine::SearchEngine(TranspositionTable *tt, Evaluator *evaluator, MoveOrderer *orderer)
    : m_transpositionTable(tt)
    , m_evaluator(evaluator)
    , m_moveOrderer(orderer)
    , m_nodesSearched(0)
    , m_pruneCount(0)
    , m_qsNodes(0)
    , m_nullMoveCuts(0)
    , m_lmrReductions(0)
{
}

void SearchEngine::resetStatistics()
{
    m_nodesSearched = 0;
    m_pruneCount = 0;
    m_qsNodes = 0;
    m_nullMoveCuts = 0;
    m_lmrReductions = 0;
}

int SearchEngine::pvs(Position &position, int depth, int alpha, int beta, bool isMaximizing, bool isPV, int maxDepth)
{
    m_nodesSearched++;

    // 检查置换表
    quint64 posKey = m_transpositionTable->computeZobristKey(position);
    int ttScore;
    if (m_transpositionTable->probe(posKey, depth, alpha, beta, ttScore)) {
        return ttScore;
    }

    PieceColor currentColor = position.currentTurn();

    // 检查游戏结束状态
    if (ChessRules::isCheckmate(position.board(), currentColor)) {
        int score = isMaximizing ? -MATE_SCORE + (maxDepth - depth) : MATE_SCORE - (maxDepth - depth);
        m_transpositionTable->store(posKey, depth, score, TTEntry::EXACT, AIMove());
        return score;
    }

    if (ChessRules::isStalemate(position.board(), currentColor)) {
        m_transpositionTable->store(posKey, depth, 0, TTEntry::EXACT, AIMove());
        return 0;
    }

    // 叶子节点：进入静态搜索
    if (depth <= 0) {
        int score = quiescence(position, alpha, beta, isMaximizing);
        m_transpositionTable->store(posKey, 0, score, TTEntry::EXACT, AIMove());
        return score;
    }

    // 空移动剪枝（Null Move Pruning）
    if (!isPV && depth >= 3 && !ChessRules::isInCheck(position.board(), currentColor)) {
        int nullScore = nullMoveSearch(position, depth, beta, isMaximizing, maxDepth);
        if ((isMaximizing && nullScore >= beta) || (!isMaximizing && nullScore <= alpha)) {
            m_nullMoveCuts++;
            return nullScore;
        }
    }

    // 生成所有可能的移动
    QList<AIMove> moves = generateAllMoves(position, currentColor);

    if (moves.isEmpty()) {
        m_transpositionTable->store(posKey, depth, 0, TTEntry::EXACT, AIMove());
        return 0;
    }

    // 移动排序
    AIMove *ttMove = m_transpositionTable->getBestMove(posKey);
    m_moveOrderer->sortMoves(moves, position, maxDepth - depth, ttMove);

    AIMove bestMove;
    TTEntry::Flag flag = TTEntry::UPPER_BOUND;
    bool isFirstMove = true;

    if (isMaximizing) {
        int maxEval = -INF;

        for (int i = 0; i < moves.size(); ++i) {
            const AIMove &move = moves[i];
            Position tempPos = position;
            tempPos.board().movePiece(move.fromRow, move.fromCol, move.toRow, move.toCol);
            tempPos.switchTurn();

            int eval;
            int newDepth = depth - 1;

            // Late Move Reduction (LMR)
            if (!isPV && i >= 4 && depth >= 3 && !ChessRules::isInCheck(position.board(), currentColor)) {
                newDepth = depth - 2;
                m_lmrReductions++;
            }

            if (isFirstMove) {
                eval = pvs(tempPos, newDepth, alpha, beta, false, isPV, maxDepth);
                isFirstMove = false;
            } else {
                eval = pvs(tempPos, newDepth, alpha, alpha + 1, false, false, maxDepth);

                if (eval > alpha && eval < beta) {
                    eval = pvs(tempPos, newDepth, alpha, beta, false, isPV, maxDepth);
                }
            }

            if (eval > maxEval) {
                maxEval = eval;
                bestMove = move;
            }
            alpha = std::max(alpha, eval);

            // Beta剪枝
            if (beta <= alpha) {
                m_pruneCount++;
                m_moveOrderer->updateKillerMove(move, maxDepth - depth);
                m_moveOrderer->updateHistory(move, depth);
                flag = TTEntry::LOWER_BOUND;
                break;
            }
        }

        if (maxEval > alpha) flag = TTEntry::EXACT;
        m_transpositionTable->store(posKey, depth, maxEval, flag, bestMove);
        return maxEval;
    } else {
        int minEval = INF;

        for (int i = 0; i < moves.size(); ++i) {
            const AIMove &move = moves[i];
            Position tempPos = position;
            tempPos.board().movePiece(move.fromRow, move.fromCol, move.toRow, move.toCol);
            tempPos.switchTurn();

            int eval;
            int newDepth = depth - 1;

            // Late Move Reduction (LMR)
            if (!isPV && i >= 4 && depth >= 3 && !ChessRules::isInCheck(position.board(), currentColor)) {
                newDepth = depth - 2;
                m_lmrReductions++;
            }

            if (isFirstMove) {
                eval = pvs(tempPos, newDepth, alpha, beta, true, isPV, maxDepth);
                isFirstMove = false;
            } else {
                eval = pvs(tempPos, newDepth, beta - 1, beta, true, false, maxDepth);

                if (eval > alpha && eval < beta) {
                    eval = pvs(tempPos, newDepth, alpha, beta, true, isPV, maxDepth);
                }
            }

            if (eval < minEval) {
                minEval = eval;
                bestMove = move;
            }
            beta = std::min(beta, eval);

            // Alpha剪枝
            if (beta <= alpha) {
                m_pruneCount++;
                m_moveOrderer->updateKillerMove(move, maxDepth - depth);
                m_moveOrderer->updateHistory(move, depth);
                flag = TTEntry::LOWER_BOUND;
                break;
            }
        }

        if (minEval < beta) flag = TTEntry::EXACT;
        m_transpositionTable->store(posKey, depth, minEval, flag, bestMove);
        return minEval;
    }
}

int SearchEngine::nullMoveSearch(Position &position, int depth, int beta, bool isMaximizing, int maxDepth)
{
    Position tempPos = position;
    tempPos.switchTurn();

    int R = 2;
    int score = pvs(tempPos, depth - 1 - R, beta - 1, beta, !isMaximizing, false, maxDepth);

    return score;
}

int SearchEngine::quiescence(Position &position, int alpha, int beta, bool isMaximizing, int qsDepth)
{
    m_qsNodes++;

    // 限制静态搜索深度
    if (qsDepth >= 4) {
        return m_evaluator->evaluatePositionFast(position);
    }

    // 站立评估
    int standPat = m_evaluator->evaluatePositionFast(position);

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

    // Delta剪枝
    if (!captureMoves.isEmpty()) {
        int biggestCapture = 0;
        for (const AIMove &move : captureMoves) {
            const ChessPiece *target = position.board().pieceAt(move.toRow, move.toCol);
            if (target && target->isValid()) {
                biggestCapture = std::max(biggestCapture, m_evaluator->getPieceBaseValue(target->type()));
            }
        }

        const int DELTA_MARGIN = 200;
        if (isMaximizing && standPat + biggestCapture + DELTA_MARGIN < alpha) {
            return alpha;
        }
        if (!isMaximizing && standPat - biggestCapture - DELTA_MARGIN > beta) {
            return beta;
        }
    }

    // 只检查高价值吃子（SEE简化版）
    QList<AIMove> goodCaptures;
    for (const AIMove &move : captureMoves) {
        const ChessPiece *target = position.board().pieceAt(move.toRow, move.toCol);
        const ChessPiece *attacker = position.board().pieceAt(move.fromRow, move.fromCol);
        if (target && target->isValid() && attacker && attacker->isValid()) {
            if (m_evaluator->getPieceBaseValue(target->type()) >= m_evaluator->getPieceBaseValue(attacker->type()) - 100) {
                goodCaptures.append(move);
            }
        }
    }

    if (goodCaptures.isEmpty()) {
        return standPat;
    }

    // 对吃子移动排序
    m_moveOrderer->sortMoves(goodCaptures, position, 0);

    if (isMaximizing) {
        for (const AIMove &move : goodCaptures) {
            Position tempPos = position;
            tempPos.board().movePiece(move.fromRow, move.fromCol, move.toRow, move.toCol);
            tempPos.switchTurn();

            int score = quiescence(tempPos, alpha, beta, false, qsDepth + 1);

            if (score >= beta) return beta;
            if (score > alpha) alpha = score;
        }
        return alpha;
    } else {
        for (const AIMove &move : goodCaptures) {
            Position tempPos = position;
            tempPos.board().movePiece(move.fromRow, move.fromCol, move.toRow, move.toCol);
            tempPos.switchTurn();

            int score = quiescence(tempPos, alpha, beta, true, qsDepth + 1);

            if (score <= alpha) return alpha;
            if (score < beta) beta = score;
        }
        return beta;
    }
}

QList<AIMove> SearchEngine::generateAllMoves(const Position &position, PieceColor color)
{
    QList<AIMove> moves;

    for (int fromRow = 0; fromRow < Board::ROWS; ++fromRow) {
        for (int fromCol = 0; fromCol < Board::COLS; ++fromCol) {
            const ChessPiece *piece = position.board().pieceAt(fromRow, fromCol);

            if (!piece || !piece->isValid() || piece->color() != color) {
                continue;
            }

            QList<QPoint> legalMoves = ChessRules::getLegalMoves(position.board(), fromRow, fromCol);

            for (const QPoint &dest : legalMoves) {
                moves.append(AIMove(fromRow, fromCol, dest.y(), dest.x()));
            }
        }
    }

    return moves;
}

QList<AIMove> SearchEngine::generateCaptureMoves(const Position &position, PieceColor color)
{
    QList<AIMove> moves;

    for (int fromRow = 0; fromRow < Board::ROWS; ++fromRow) {
        for (int fromCol = 0; fromCol < Board::COLS; ++fromCol) {
            const ChessPiece *piece = position.board().pieceAt(fromRow, fromCol);

            if (!piece || !piece->isValid() || piece->color() != color) {
                continue;
            }

            QList<QPoint> legalMoves = ChessRules::getLegalMoves(position.board(), fromRow, fromCol);

            for (const QPoint &dest : legalMoves) {
                const ChessPiece *target = position.board().pieceAt(dest.y(), dest.x());
                if (target && target->isValid() && target->color() != color) {
                    moves.append(AIMove(fromRow, fromCol, dest.y(), dest.x()));
                }
            }
        }
    }

    return moves;
}
