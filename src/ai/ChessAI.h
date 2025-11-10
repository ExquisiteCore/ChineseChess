#ifndef CHESSAI_H
#define CHESSAI_H

#include "../core/Board.h"
#include "../core/Position.h"
#include "../core/ChessRules.h"
#include <QObject>
#include <QPair>
#include <QList>
#include <QPoint>
#include <QHash>
#include <limits>

// AI难度级别
enum class AIDifficulty {
    Easy = 1,      // 搜索深度 3
    Medium = 2,    // 搜索深度 4
    Hard = 3,      // 搜索深度 5
    Expert = 4     // 搜索深度 6
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

// 置换表项
struct TTEntry {
    quint64 zobristKey;
    int depth;
    int score;
    enum Flag { EXACT, LOWER_BOUND, UPPER_BOUND } flag;
    AIMove bestMove;

    TTEntry() : zobristKey(0), depth(-1), score(0), flag(EXACT) {}
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

    // 静态搜索（解决水平线效应）
    int quiescence(Position &position, int alpha, int beta, bool isMaximizing);

    // 获取所有可能的移动
    QList<AIMove> generateAllMoves(const Position &position, PieceColor color);

    // 获取吃子移动（用于静态搜索）
    QList<AIMove> generateCaptureMoves(const Position &position, PieceColor color);

    // === 局面评估 ===

    // 评估局面（正数表示红方优势，负数表示黑方优势）
    int evaluatePosition(const Position &position);

    // 增量式评估（更快）
    int evaluatePositionFast(const Position &position);

    // 棋子价值评估
    int getPieceValue(PieceType type, int row, int col, PieceColor color) const;

    // 棋子基础价值
    int getPieceBaseValue(PieceType type) const;

    // === 置换表 ===
    quint64 computeZobristKey(const Position &position);
    void initZobristKeys();
    bool probeTranspositionTable(quint64 key, int depth, int alpha, int beta, int &score);
    void storeTranspositionTable(quint64 key, int depth, int score, TTEntry::Flag flag, const AIMove &bestMove);

    // === 移动排序（提高剪枝效率） ===
    void sortMoves(QList<AIMove> &moves, const Position &position, const AIMove *ttMove = nullptr);

    // 快速评估移动价值（用于排序）
    int quickEvaluateMove(const Position &position, const AIMove &move);

    // === 成员变量 ===
    AIDifficulty m_difficulty;
    int m_maxDepth;          // 最大搜索深度
    int m_nodesSearched;     // 搜索的节点数
    int m_pruneCount;        // Alpha-Beta剪枝次数
    int m_ttHits;            // 置换表命中次数
    int m_qsNodes;           // 静态搜索节点数

    // 置换表
    QHash<quint64, TTEntry> m_transpositionTable;
    static constexpr int TT_SIZE = 1000000;  // 约100万个表项

    // Zobrist哈希
    quint64 m_zobristTable[10][9][14];  // [row][col][piece]
    bool m_zobristInitialized;

    // 杀手移动启发
    AIMove m_killerMoves[10][2];  // 每层保存2个杀手移动

    // 历史启发
    int m_historyTable[10][9][10][9];  // [fromRow][fromCol][toRow][toCol]

    // 常量定义
    static constexpr int INF = std::numeric_limits<int>::max() / 2;  // 无穷大（避免溢出）
    static constexpr int MATE_SCORE = 100000;  // 将死得分

    // 位置价值表（红方视角，黑方需要翻转）
    static const int PAWN_POS_VALUE[10][9];
    static const int ADVISOR_POS_VALUE[10][9];
    static const int ELEPHANT_POS_VALUE[10][9];
    static const int HORSE_POS_VALUE[10][9];
    static const int ROOK_POS_VALUE[10][9];
    static const int CANNON_POS_VALUE[10][9];
    static const int KING_POS_VALUE[10][9];
};

#endif // CHESSAI_H
