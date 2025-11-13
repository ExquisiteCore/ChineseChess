#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include "TranspositionTable.h"
#include "Evaluator.h"
#include "MoveOrderer.h"
#include "../core/Position.h"
#include "../core/ChessRules.h"
#include <QList>
#include <limits>

// 搜索引擎（负责所有搜索算法）
class SearchEngine
{
public:
    SearchEngine(TranspositionTable *tt, Evaluator *evaluator, MoveOrderer *orderer);

    // 迭代加深搜索（主入口）
    AIMove iterativeDeepening(Position &position, int maxDepth, bool isMaximizing, AIMove *bestMove = nullptr);

    // PVS搜索（主要变例搜索）
    int pvs(Position &position, int depth, int alpha, int beta, bool isMaximizing, bool isPV, int maxDepth);

    // 静态搜索（解决水平线效应）
    int quiescence(Position &position, int alpha, int beta, bool isMaximizing, int qsDepth = 0);

    // 生成所有可能的移动
    QList<AIMove> generateAllMoves(const Position &position, PieceColor color);

    // 生成吃子移动（用于静态搜索）
    QList<AIMove> generateCaptureMoves(const Position &position, PieceColor color);

    // 获取统计信息
    int getNodesSearched() const { return m_nodesSearched; }
    int getPruneCount() const { return m_pruneCount; }
    int getQsNodes() const { return m_qsNodes; }
    int getNullMoveCuts() const { return m_nullMoveCuts; }
    int getLmrReductions() const { return m_lmrReductions; }
    int getCurrentDepth() const { return m_currentDepth; }

    // 启用/禁用迭代加深
    void setIterativeDeepeningEnabled(bool enabled) { m_useIterativeDeepening = enabled; }
    bool isIterativeDeepeningEnabled() const { return m_useIterativeDeepening; }

    // 重置统计信息
    void resetStatistics();

private:
    // 空移动剪枝
    int nullMoveSearch(Position &position, int depth, int beta, bool isMaximizing, int maxDepth);

    TranspositionTable *m_transpositionTable;
    Evaluator *m_evaluator;
    MoveOrderer *m_moveOrderer;

    // 统计信息
    int m_nodesSearched;
    int m_pruneCount;
    int m_qsNodes;
    int m_nullMoveCuts;
    int m_lmrReductions;
    int m_currentDepth;

    // 选项
    bool m_useIterativeDeepening;

    // 常量定义
    static constexpr int INF = std::numeric_limits<int>::max() / 2;
    static constexpr int MATE_SCORE = 100000;
};

#endif // SEARCHENGINE_H
