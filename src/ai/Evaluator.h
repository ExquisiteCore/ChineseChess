#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "../core/Board.h"
#include "../core/Position.h"
#include "../core/ChessPiece.h"
#include <QList>
#include <QPoint>

// 棋子-位置评估器（增强版）
class Evaluator
{
public:
    Evaluator();

    // 评估局面（正数表示红方优势，负数表示黑方优势）
    int evaluatePosition(const Position &position);

    // 快速评估（仅材料+位置）
    int evaluatePositionFast(const Position &position);

    // 完整评估（包含所有因素）
    int evaluatePositionFull(const Position &position);

    // 获取棋子基础价值
    int getPieceBaseValue(PieceType type) const;

    // 获取棋子总价值（基础价值+位置价值）
    int getPieceValue(PieceType type, int row, int col, PieceColor color) const;

    // 启用/禁用高级评估
    void setAdvancedEvaluationEnabled(bool enabled) { m_useAdvancedEval = enabled; }
    bool isAdvancedEvaluationEnabled() const { return m_useAdvancedEval; }

private:
    // === 高级评估因素 ===

    // 评估棋子灵活性（mobility）
    int evaluateMobility(const Position &position);

    // 评估控制力（对关键格子的控制）
    int evaluateControl(const Position &position);

    // 评估棋子保护关系
    int evaluateProtection(const Position &position);

    // 评估将帅安全性
    int evaluateKingSafety(const Position &position);

    // 评估棋型��特殊棋型奖励）
    int evaluatePatterns(const Position &position);

    // 辅助函数
    int countAttackers(const Board &board, int row, int col, PieceColor attackColor);
    int countDefenders(const Board &board, int row, int col, PieceColor defendColor);
    bool isKeySquare(int row, int col);  // 判断是否是关键格子

    // 选项
    bool m_useAdvancedEval;

    // 位置价值表（红方视角，黑方需要翻转）
    static const int PAWN_POS_VALUE[10][9];
    static const int ADVISOR_POS_VALUE[10][9];
    static const int ELEPHANT_POS_VALUE[10][9];
    static const int HORSE_POS_VALUE[10][9];
    static const int ROOK_POS_VALUE[10][9];
    static const int CANNON_POS_VALUE[10][9];
    static const int KING_POS_VALUE[10][9];

    // 关键格子定义（九宫、河口等）
    static const bool KEY_SQUARES[10][9];
};

#endif // EVALUATOR_H
