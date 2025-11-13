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
    qDebug() << "=== AI开始思考 ===";
    qDebug() << "搜索深度:" << m_maxDepth;

    PieceColor aiColor = position.currentTurn();
    bool isMaximizing = (aiColor == PieceColor::Red);

    // 创建位置的副本用于搜索
    Position searchPos = position;

    // 生成所有可能的移动
    QList<AIMove> allMoves = m_searchEngine->generateAllMoves(searchPos, aiColor);

    if (allMoves.isEmpty()) {
        qDebug() << "没有可用的移动";
        return AIMove();
    }

    // 检查置换表中的最佳移动
    quint64 posKey = m_transpositionTable->computeZobristKey(searchPos);
    AIMove *ttMove = m_transpositionTable->getBestMove(posKey);

    // 对移动进行排序以提高剪枝效率
    m_moveOrderer->sortMoves(allMoves, searchPos, 0, ttMove);

    AIMove bestMove;
    constexpr int INF = std::numeric_limits<int>::max() / 2;
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

        // 使用PVS搜索
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
            emit searchProgress(m_maxDepth, m_searchEngine->getNodesSearched());
        }
    }

    // 存储到置换表
    m_transpositionTable->store(posKey, m_maxDepth, bestScore, TTEntry::EXACT, bestMove);

    qDebug() << "最佳移动:" << bestMove.fromRow << bestMove.fromCol
             << "->" << bestMove.toRow << bestMove.toCol
             << "评分:" << bestScore;
    qDebug() << "搜索节点数:" << m_searchEngine->getNodesSearched()
             << "(静态搜索:" << m_searchEngine->getQsNodes() << ")";
    qDebug() << "剪枝次数:" << m_searchEngine->getPruneCount()
             << "置换表命中:" << m_transpositionTable->getHits();
    qDebug() << "空移动剪枝:" << m_searchEngine->getNullMoveCuts()
             << "LMR减少:" << m_searchEngine->getLmrReductions();

    emit moveFound(bestMove.fromRow, bestMove.fromCol, bestMove.toRow, bestMove.toCol, bestScore);

    return bestMove;
}
