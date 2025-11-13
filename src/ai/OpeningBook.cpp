#include "OpeningBook.h"
#include "../core/Board.h"
#include <QRandomGenerator>
#include <QDebug>
#include <QtMath>

OpeningBook::OpeningBook(TranspositionTable *tt)
    : m_enabled(true)
    , m_transpositionTable(tt)
{
    initializeCommonOpenings();
}

AIMove OpeningBook::selectMove(quint64 zobristKey)
{
    if (!m_enabled) {
        qDebug() << "[开局库] 已禁用";
        return AIMove();
    }

    // 查询开局库
    if (!m_book.contains(zobristKey)) {
        qDebug() << "[开局库] 当前局面不在开局库中, key =" << zobristKey;
        return AIMove(); // 不在开局库中
    }

    qDebug() << "[开局库] 找到开局库条目, key =" << zobristKey;

    QList<BookEntry> entries = m_book[zobristKey];
    if (entries.isEmpty()) {
        qDebug() << "[开局库] 条目列表为空";
        return AIMove();
    }

    // 计算总权重
    int totalWeight = 0;
    for (const BookEntry &entry : entries) {
        totalWeight += entry.weight;
    }

    // 根据权重随机选择
    int randomValue = QRandomGenerator::global()->bounded(totalWeight);
    int currentWeight = 0;

    for (const BookEntry &entry : entries) {
        currentWeight += entry.weight;
        if (randomValue < currentWeight) {
            // 安全检查：拒绝送炮的走法（炮纵向进攻2步或更多）
            int rowDiff = qAbs(entry.move.toRow - entry.move.fromRow);
            bool isCannonAdvance = (entry.move.fromCol == entry.move.toCol) && (rowDiff >= 2);

            if (isCannonAdvance) {
                qDebug() << "WARNING: 拒绝危险的炮进走法:" << entry.move.fromRow << entry.move.fromCol
                         << "->" << entry.move.toRow << entry.move.toCol;
                continue; // 跳过这个走法，选择下一个
            }

            qDebug() << "使用开局库走法:" << entry.move.fromRow << entry.move.fromCol
                     << "->" << entry.move.toRow << entry.move.toCol
                     << "权重:" << entry.weight << "胜率:" << entry.winRate << "%";
            return entry.move;
        }
    }

    // 如果所有走法都被拒绝，返回第一个非危险走法
    for (const BookEntry &entry : entries) {
        int rowDiff = qAbs(entry.move.toRow - entry.move.fromRow);
        bool isCannonAdvance = (entry.move.fromCol == entry.move.toCol) && (rowDiff >= 2);
        if (!isCannonAdvance) {
            return entry.move;
        }
    }

    return AIMove(); // 如果所有走法都危险，返回无效走法让搜索引擎处理
}

void OpeningBook::addMove(quint64 zobristKey, const AIMove &move, int weight, int winRate)
{
    if (!m_book.contains(zobristKey)) {
        m_book[zobristKey] = QList<BookEntry>();
    }

    // 检查是否已存在
    for (BookEntry &entry : m_book[zobristKey]) {
        if (entry.move.fromRow == move.fromRow && entry.move.fromCol == move.fromCol &&
            entry.move.toRow == move.toRow && entry.move.toCol == move.toCol) {
            // 更新权重
            entry.weight += weight;
            return;
        }
    }

    // 添加新条目
    m_book[zobristKey].append(BookEntry(move, weight, winRate));
}

void OpeningBook::initializeCommonOpenings()
{
    qDebug() << "初始化常见开局库...";

    // 创建初始局面
    Position initialPos;
    quint64 initKey = m_transpositionTable->computeZobristKey(initialPos);

    // ========== 红方第一步（红先） ==========

    // 1. 中炮开局 - 炮二平五（最流行）
    addSymmetricMoves(initialPos, AIMove(2, 1, 2, 4), 100, 54); // 炮二平五

    // 2. 起马局 - 马二进三 / 马八进七
    addSymmetricMoves(initialPos, AIMove(0, 1, 2, 2), 90, 52);  // 马二进三

    // 3. 仙人指路 - 兵三进一 / 兵七进一
    addSymmetricMoves(initialPos, AIMove(3, 2, 4, 2), 85, 51);  // 兵三进一

    // 4. 飞相局 - 相三进五 / 相七进五
    addSymmetricMoves(initialPos, AIMove(0, 2, 2, 4), 70, 50);  // 相三进五

    // 5. 过宫炮 - 炮二平六
    addMove(initKey, AIMove(2, 1, 2, 5), 60, 50);  // 炮二平六
    addMove(initKey, AIMove(2, 7, 2, 3), 60, 50);  // 炮八平四（镜像）

    // 6. 士角炮
    addMove(initKey, AIMove(2, 1, 2, 3), 55, 49);  // 炮二平四
    addMove(initKey, AIMove(2, 7, 2, 5), 55, 49);  // 炮八平六（镜像）

    // ========== 应对中炮（炮二平五后的局面） ==========

    Position afterCenterCannon = initialPos;
    afterCenterCannon.board().movePiece(2, 1, 2, 4);
    afterCenterCannon.switchTurn();
    quint64 afterCenterCannonKey = m_transpositionTable->computeZobristKey(afterCenterCannon);

    // 黑方应对中炮：
    // 1. 屏风马 - 马8进7（最常见）
    addSymmetricMoves(afterCenterCannon, AIMove(9, 7, 7, 6), 100, 53); // 马8进7

    // 2. 反攻中炮 - 炮8平5
    addSymmetricMoves(afterCenterCannon, AIMove(7, 7, 7, 4), 95, 52);  // 炮8平5

    // 3. 飞象局 - 象7进5
    addSymmetricMoves(afterCenterCannon, AIMove(9, 6, 7, 4), 80, 50);  // 象7进5

    // 4. 进卒 - 卒7进1
    addSymmetricMoves(afterCenterCannon, AIMove(6, 6, 5, 6), 75, 50);  // 卒7进1

    // ========== 应对起马（马二进三后的局面） ==========

    Position afterHorseMove = initialPos;
    afterHorseMove.board().movePiece(0, 1, 2, 2);
    afterHorseMove.switchTurn();
    quint64 afterHorseMoveKey = m_transpositionTable->computeZobristKey(afterHorseMove);

    // 黑方应对：
    // 1. 对跳马 - 马8进7
    addSymmetricMoves(afterHorseMove, AIMove(9, 7, 7, 6), 100, 52);  // 马8进7

    // 2. 飞象 - 象7进5
    addSymmetricMoves(afterHorseMove, AIMove(9, 6, 7, 4), 90, 51);   // 象7进5

    // 3. 出炮 - 炮8平6
    addMove(afterHorseMoveKey, AIMove(7, 7, 7, 5), 85, 50);  // 炮8平6
    addMove(afterHorseMoveKey, AIMove(7, 1, 7, 3), 85, 50);  // 炮2平4（镜像）

    // ========== 应对仙人指路（兵三进一后的局面） ==========

    Position afterPawnMove = initialPos;
    afterPawnMove.board().movePiece(3, 2, 4, 2);
    afterPawnMove.switchTurn();
    quint64 afterPawnMoveKey = m_transpositionTable->computeZobristKey(afterPawnMove);

    // 黑方应对：
    // 1. 对进卒 - 卒7进1
    addSymmetricMoves(afterPawnMove, AIMove(6, 6, 5, 6), 100, 51);  // 卒7进1

    // 2. 飞象 - 象7进5
    addSymmetricMoves(afterPawnMove, AIMove(9, 6, 7, 4), 90, 50);   // 象7进5

    // 3. 起马 - 马8进7
    addSymmetricMoves(afterPawnMove, AIMove(9, 7, 7, 6), 85, 50);   // 马8进7

    // ========== 中炮对屏风马（经典对局） ==========

    Position centerCannonVsScreen = afterCenterCannon;
    centerCannonVsScreen.board().movePiece(9, 7, 7, 6);
    centerCannonVsScreen.switchTurn();
    quint64 centerCannonVsScreenKey = m_transpositionTable->computeZobristKey(centerCannonVsScreen);

    // 红方继续：
    // 1. 马二进三（标准）
    addSymmetricMoves(centerCannonVsScreen, AIMove(0, 1, 2, 2), 100, 54);  // 马二进三

    // 2. 兵三进一（兵炮配合）
    addSymmetricMoves(centerCannonVsScreen, AIMove(3, 2, 4, 2), 90, 52);   // 兵三进一

    // 3. 车一平二（快速出车）
    addSymmetricMoves(centerCannonVsScreen, AIMove(0, 0, 0, 1), 85, 51);   // 车一平二

    // 4. 兵七进一
    addSymmetricMoves(centerCannonVsScreen, AIMove(3, 6, 4, 6), 80, 50);   // 兵七进一

    qDebug() << "开局库初始化完成："
             << m_book.size() << "个局面";
}

void OpeningBook::addSymmetricMoves(const Position &pos, const AIMove &move, int weight, int winRate)
{
    // 1. 添加原始移动
    quint64 key = m_transpositionTable->computeZobristKey(pos);
    addMove(key, move, weight, winRate);

    // 2. 中国象棋左右对称，添加镜像移动
    // 镜像关系：col -> 8 - col
    AIMove mirrorMove(move.fromRow, 8 - move.fromCol, move.toRow, 8 - move.toCol);

    // 3. 创建镜像局面并计算其 Zobrist Key
    Position mirrorPos;

    // 遍历原始局面的所有棋子，创建镜像局面
    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const ChessPiece *piece = pos.board().pieceAt(row, col);
            if (piece && piece->isValid()) {
                // 在镜像位置放置相同的棋子
                int mirrorCol = 8 - col;
                // 注意：我们需要复制棋子，但 Board 类可能不支持直接设置棋子
                // 这里假设镜像局面初始化时已经是对称的
                // 对于初始局面，镜像局面实际上就是自己
            }
        }
    }

    // 对于初始局面和大多数对称局面，我们可以简化处理：
    // 如果移动本身不在中线（列4），则添加镜像移动
    if (move.fromCol != 4 || move.toCol != 4) {
        // 计算执行镜像移动后的局面
        Position tempMirrorPos = pos;

        // 检查镜像移动是否合法
        const ChessPiece *mirrorPiece = tempMirrorPos.board().pieceAt(mirrorMove.fromRow, mirrorMove.fromCol);
        if (mirrorPiece && mirrorPiece->isValid()) {
            quint64 mirrorKey = m_transpositionTable->computeZobristKey(tempMirrorPos);
            addMove(mirrorKey, mirrorMove, weight, winRate);
        }
    }
}
