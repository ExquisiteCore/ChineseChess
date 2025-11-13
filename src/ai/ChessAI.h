#ifndef CHESSAI_H
#define CHESSAI_H

#include "../core/Board.h"
#include "../core/Position.h"
#include "TranspositionTable.h"
#include "Evaluator.h"
#include "MoveOrderer.h"
#include "SearchEngine.h"
#include <QObject>
#include <memory>

// AI难度级别
enum class AIDifficulty {
    Easy = 1,      // 搜索深度 2
    Medium = 2,    // 搜索深度 3
    Hard = 3,      // 搜索深度 4
    Expert = 4     // 搜索深度 5
};

// 中国象棋AI引擎（主控制器）
class ChessAI : public QObject
{
    Q_OBJECT

public:
    explicit ChessAI(QObject *parent = nullptr);
    ~ChessAI();

    // 设置AI难度
    void setDifficulty(AIDifficulty difficulty);
    AIDifficulty getDifficulty() const { return m_difficulty; }

    // 获取最佳移动
    AIMove getBestMove(const Position &position);

    // 获取搜索统计信息
    int getNodesSearched() const;
    int getPruneCount() const;
    void resetStatistics();

signals:
    void searchProgress(int depth, int nodes);
    void moveFound(int fromRow, int fromCol, int toRow, int toCol, int score);

private:
    // 难度配置
    AIDifficulty m_difficulty;
    int m_maxDepth;

    // 模块组件
    std::unique_ptr<TranspositionTable> m_transpositionTable;
    std::unique_ptr<Evaluator> m_evaluator;
    std::unique_ptr<MoveOrderer> m_moveOrderer;
    std::unique_ptr<SearchEngine> m_searchEngine;
};

#endif // CHESSAI_H
