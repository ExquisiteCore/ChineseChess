#include "ChessAI.h"
#include <QDebug>
#include <limits>

ChessAI::ChessAI(QObject *parent)
    : QObject(parent)
    , m_difficulty(AIDifficulty::Medium)
    , m_maxDepth(3)
{
    // 创建各个模块
    m_transpositionTable = std::make_unique<TranspositionTable>();
    m_evaluator = std::make_unique<Evaluator>();
    m_moveOrderer = std::make_unique<MoveOrderer>(m_evaluator.get());
    m_searchEngine = std::make_unique<SearchEngine>(m_transpositionTable.get(),
                                                      m_evaluator.get(),
                                                      m_moveOrderer.get());
    m_openingBook = std::make_unique<OpeningBook>(m_transpositionTable.get());
    m_endgameTablebase = std::make_unique<EndgameTablebase>();

    qDebug() << "ChessAI 增强版初始化完成";
    qDebug() << "- 开局库: 启用";
    qDebug() << "- 残局库: 启用";
    qDebug() << "- 迭代加深: 启用";
    qDebug() << "- 高级评估: 启用";
    qDebug() << "- 并行搜索: 启用 (线程数:" << (m_searchEngine->getThreadCount() == 0 ? "自动" : QString::number(m_searchEngine->getThreadCount())) << ")";
}

ChessAI::~ChessAI()
{
}

void ChessAI::setDifficulty(AIDifficulty difficulty)
{
    m_difficulty = difficulty;
    m_maxDepth = static_cast<int>(difficulty) + 1;
    qDebug() << "AI难度设置为:" << m_maxDepth << "层搜索";
}

void ChessAI::resetStatistics()
{
    m_searchEngine->resetStatistics();
    m_transpositionTable->resetStatistics();
    m_moveOrderer->reset();
}

int ChessAI::getNodesSearched() const
{
    return m_searchEngine->getNodesSearched();
}

int ChessAI::getPruneCount() const
{
    return m_searchEngine->getPruneCount();
}

AIMove ChessAI::getBestMove(const Position &position)
{
    resetStatistics();
    qDebug() << "=== AI开始思考（增强版） ===";
    qDebug() << "搜索深度:" << m_maxDepth;

    PieceColor aiColor = position.currentTurn();
    bool isMaximizing = (aiColor == PieceColor::Red);

    // 创建位置的副本用于搜索
    Position searchPos = position;

    // 1. 先尝试查询开局库
    if (m_openingBook && m_openingBook->isEnabled()) {
        quint64 posKey = m_transpositionTable->computeZobristKey(searchPos);
        AIMove bookMove = m_openingBook->selectMove(posKey);
        if (bookMove.isValid()) {
            qDebug() << "使用开局库走法";
            emit moveFound(bookMove.fromRow, bookMove.fromCol, bookMove.toRow, bookMove.toCol, 0);
            return bookMove;
        }
    }

    // 2. 检查是否进入残局
    if (m_endgameTablebase && m_endgameTablebase->isEnabled()) {
        if (m_endgameTablebase->isEndgame(searchPos)) {
            qDebug() << "进入残局阶段，使用残局评估";
            // 残局评估会影响Evaluator的评分
        }
    }

    AIMove bestMove;

    // 3. 使用并行搜索、迭代加深或普通搜索
    if (m_searchEngine->isParallelSearchEnabled()) {
        qDebug() << "使用并行搜索";
        bestMove = m_searchEngine->parallelSearch(searchPos, m_maxDepth, isMaximizing);
    } else if (m_searchEngine->isIterativeDeepeningEnabled()) {
        qDebug() << "使用迭代加深搜索";
        bestMove = m_searchEngine->iterativeDeepening(searchPos, m_maxDepth, isMaximizing);
    } else {
        // 传统搜索方式
        qDebug() << "使用传统搜索";

        QList<AIMove> allMoves = m_searchEngine->generateAllMoves(searchPos, aiColor);

        if (allMoves.isEmpty()) {
            qDebug() << "没有可用的移动";
            return AIMove();
        }

        quint64 posKey = m_transpositionTable->computeZobristKey(searchPos);
        std::optional<AIMove> ttMove = m_transpositionTable->getBestMove(posKey);

        m_moveOrderer->sortMoves(allMoves, searchPos, 0, ttMove);

        constexpr int INF = std::numeric_limits<int>::max() / 2;
        int bestScore = isMaximizing ? -INF : INF;

        qDebug() << "评估" << allMoves.size() << "个可能的移动...";

        for (int i = 0; i < allMoves.size(); ++i) {
            AIMove &move = allMoves[i];

            Position tempPos = searchPos;
            tempPos.board().movePiece(move.fromRow, move.fromCol, move.toRow, move.toCol);
            tempPos.switchTurn();

            int score;
            if (i == 0) {
                score = m_searchEngine->pvs(tempPos, m_maxDepth - 1, -INF, INF, !isMaximizing, true, m_maxDepth);
            } else {
                score = m_searchEngine->pvs(tempPos, m_maxDepth - 1,
                                           isMaximizing ? bestScore : -INF,
                                           isMaximizing ? INF : bestScore,
                                           !isMaximizing, false, m_maxDepth);
            }

            move.score = score;

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

            if ((i + 1) % 5 == 0 || i == allMoves.size() - 1) {
                emit searchProgress(m_maxDepth, m_searchEngine->getNodesSearched());
            }
        }

        m_transpositionTable->store(posKey, m_maxDepth, bestScore, TTEntry::EXACT, bestMove);

        qDebug() << "最佳移动:" << bestMove.fromRow << bestMove.fromCol
                 << "->" << bestMove.toRow << bestMove.toCol
                 << "评分:" << bestScore;

        emit moveFound(bestMove.fromRow, bestMove.fromCol, bestMove.toRow, bestMove.toCol, bestScore);
    }

    // 统计信息
    qDebug() << "搜索节点数:" << m_searchEngine->getNodesSearched()
             << "(静态搜索:" << m_searchEngine->getQsNodes() << ")";
    qDebug() << "剪枝次数:" << m_searchEngine->getPruneCount()
             << "置换表命中:" << m_transpositionTable->getHits();
    qDebug() << "空移动剪枝:" << m_searchEngine->getNullMoveCuts()
             << "LMR减少:" << m_searchEngine->getLmrReductions();

    return bestMove;
}

// === 高级功能配置实现 ===

void ChessAI::setOpeningBookEnabled(bool enabled)
{
    if (m_openingBook) {
        m_openingBook->setEnabled(enabled);
        qDebug() << "开局库:" << (enabled ? "启用" : "禁用");
    }
}

bool ChessAI::isOpeningBookEnabled() const
{
    return m_openingBook && m_openingBook->isEnabled();
}

void ChessAI::setEndgameTablebaseEnabled(bool enabled)
{
    if (m_endgameTablebase) {
        m_endgameTablebase->setEnabled(enabled);
        qDebug() << "残局库:" << (enabled ? "启用" : "禁用");
    }
}

bool ChessAI::isEndgameTablebaseEnabled() const
{
    return m_endgameTablebase && m_endgameTablebase->isEnabled();
}

void ChessAI::setIterativeDeepeningEnabled(bool enabled)
{
    if (m_searchEngine) {
        m_searchEngine->setIterativeDeepeningEnabled(enabled);
        qDebug() << "迭代加深:" << (enabled ? "启用" : "禁用");
    }
}

bool ChessAI::isIterativeDeepeningEnabled() const
{
    return m_searchEngine && m_searchEngine->isIterativeDeepeningEnabled();
}

void ChessAI::setAdvancedEvaluationEnabled(bool enabled)
{
    if (m_evaluator) {
        m_evaluator->setAdvancedEvaluationEnabled(enabled);
        qDebug() << "高级评估:" << (enabled ? "启用" : "禁用");
    }
}

bool ChessAI::isAdvancedEvaluationEnabled() const
{
    return m_evaluator && m_evaluator->isAdvancedEvaluationEnabled();
}

void ChessAI::setParallelSearchEnabled(bool enabled)
{
    if (m_searchEngine) {
        m_searchEngine->setParallelSearchEnabled(enabled);
        qDebug() << "并行搜索:" << (enabled ? "启用" : "禁用");
    }
}

bool ChessAI::isParallelSearchEnabled() const
{
    return m_searchEngine && m_searchEngine->isParallelSearchEnabled();
}

void ChessAI::setThreadCount(int count)
{
    if (m_searchEngine) {
        m_searchEngine->setThreadCount(count);
        qDebug() << "线程数设置为:" << (count == 0 ? "自动" : QString::number(count));
    }
}

int ChessAI::getThreadCount() const
{
    return m_searchEngine ? m_searchEngine->getThreadCount() : 0;
}
