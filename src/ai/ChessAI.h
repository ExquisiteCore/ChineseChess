#ifndef CHESSAI_H
#define CHESSAI_H

#include "../core/Board.h"
#include "../core/Position.h"
#include "TranspositionTable.h"
#include "Evaluator.h"
#include "MoveOrderer.h"
#include "SearchEngine.h"
#include "OpeningBook.h"
#include "EndgameTablebase.h"
#include <QObject>
#include <memory>

// AI难度级别
enum class AIDifficulty {
    Easy = 1,      // 搜索深度 2
    Medium = 2,    // 搜索深度 3
    Hard = 3,      // 搜索深度 4
    Expert = 4     // 搜索深度 5
};

// 中国象棋AI引擎（主控制器 - 增强版）
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

    // === 高级功能配置 ===

    // 开局库
    void setOpeningBookEnabled(bool enabled);
    bool isOpeningBookEnabled() const;

    // 残局库
    void setEndgameTablebaseEnabled(bool enabled);
    bool isEndgameTablebaseEnabled() const;

    // 迭代加深
    void setIterativeDeepeningEnabled(bool enabled);
    bool isIterativeDeepeningEnabled() const;

    // 高级评估
    void setAdvancedEvaluationEnabled(bool enabled);
    bool isAdvancedEvaluationEnabled() const;

    // 并行搜索
    void setParallelSearchEnabled(bool enabled);
    bool isParallelSearchEnabled() const;
    void setThreadCount(int count);
    int getThreadCount() const;

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
    std::unique_ptr<OpeningBook> m_openingBook;
    std::unique_ptr<EndgameTablebase> m_endgameTablebase;
};

#endif // CHESSAI_H
