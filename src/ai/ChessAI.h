#ifndef CHESSAI_H
#define CHESSAI_H

#include "../core/Board.h"
#include "../core/Position.h"
#include "../core/ChessRules.h"
#include <QObject>
#include <QPair>
#include <QList>
#include <QPoint>
#include <limits>

// AI难度级别
enum class AIDifficulty {
    Easy = 1,      // 搜索深度 2
    Medium = 2,    // 搜索深度 3
    Hard = 3,      // 搜索深度 4
    Expert = 4     // 搜索深度 5
};

// 移动结构（包含评分）
struct AIMove {
    int fromRow, fromCol;
    int toRow, toCol;
    int score;

    AIMove() : fromRow(-1), fromCol(-1), toRow(-1), toCol(-1), score(0) {}
    AIMove(int fr, int fc, int tr, int tc, int s = 0)
        : fromRow(fr), fromCol(fc), toRow(tr), toCol(tc), score(s) {}

    bool isValid() const { return fromRow >= 0 && fromCol >= 0 && toRow >= 0 && toCol >= 0; }
};

// 中国象棋AI引擎
class ChessAI : public QObject
{
    Q_OBJECT

public:
    explicit ChessAI(QObject *parent = nullptr);

    // 设置AI难度
    void setDifficulty(AIDifficulty difficulty);
    AIDifficulty getDifficulty() const { return m_difficulty; }

    // 获取最佳移动
    AIMove getBestMove(const Position &position);

    // 获取搜索统计信息
    int getNodesSearched() const { return m_nodesSearched; }
    int getPruneCount() const { return m_pruneCount; }
    void resetStatistics();

signals:
    void searchProgress(int depth, int nodes);  // 搜索进度信号
    void moveFound(int fromRow, int fromCol, int toRow, int toCol, int score);

private:
    // === 核心搜索算法 ===

    // Minimax算法（带Alpha-Beta剪枝）
    int minimax(Position &position, int depth, int alpha, int beta, bool isMaximizing);

    // 获取所有可能的移动
    QList<AIMove> generateAllMoves(const Position &position, PieceColor color);

    // === 局面评估 ===

    // 评估局面（正数表示红方优势，负数表示黑方优势）
    int evaluatePosition(const Position &position);

    // 棋子价值评估
    int getPieceValue(const ChessPiece *piece) const;

    // 位置价值评估（棋子在不同位置的价值不同）
    int getPositionValue(const ChessPiece *piece) const;

    // 灵活性评估（可走步数）
    int getMobilityScore(const Position &position, PieceColor color);

    // 将帅安全性评估
    int getKingSafetyScore(const Position &position, PieceColor color);

    // 控制中心评估
    int getCenterControlScore(const Position &position, PieceColor color);

    // === 移动排序（提高剪枝效率） ===
    void sortMoves(QList<AIMove> &moves, const Position &position);

    // 快速评估移动价值（用于排序）
    int quickEvaluateMove(const Position &position, const AIMove &move);

    // === 成员变量 ===
    AIDifficulty m_difficulty;
    int m_maxDepth;          // 最大搜索深度
    int m_nodesSearched;     // 搜索的节点数
    int m_pruneCount;        // Alpha-Beta剪枝次数

    // 常量定义
    static constexpr int INF = std::numeric_limits<int>::max() / 2;  // 无穷大（避免溢出）
    static constexpr int MATE_SCORE = 100000;  // 将死得分
};

#endif // CHESSAI_H
