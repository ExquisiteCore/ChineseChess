#include "TranspositionTable.h"
#include "../core/Board.h"
#include <QRandomGenerator>

TranspositionTable::TranspositionTable()
    : m_initialized(false)
    , m_hits(0)
    , m_threadSafe(true)
{
    initialize();
}

void TranspositionTable::initialize()
{
    if (m_initialized) return;

    QRandomGenerator *rng = QRandomGenerator::global();

    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 9; col++) {
            for (int piece = 0; piece < 14; piece++) {
                m_zobristTable[row][col][piece] = (quint64(rng->generate()) << 32) | rng->generate();
            }
        }
    }

    m_initialized = true;
}

quint64 TranspositionTable::computeZobristKey(const Position &position)
{
    quint64 key = 0;

    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const ChessPiece *piece = position.board().pieceAt(row, col);
            if (piece && piece->isValid()) {
                int pieceIndex = static_cast<int>(piece->type()) + (piece->color() == PieceColor::Black ? 7 : 0);
                key ^= m_zobristTable[row][col][pieceIndex];
            }
        }
    }

    return key;
}

bool TranspositionTable::probe(quint64 key, int depth, int alpha, int beta, int &score)
{
    if (m_threadSafe) {
        QMutexLocker locker(&m_mutex);
        return probeImpl(key, depth, alpha, beta, score);
    } else {
        return probeImpl(key, depth, alpha, beta, score);
    }
}

bool TranspositionTable::probeImpl(quint64 key, int depth, int alpha, int beta, int &score)
{
    if (!m_table.contains(key)) {
        return false;
    }

    const TTEntry &entry = m_table[key];

    if (entry.depth < depth) {
        return false;
    }

    if (entry.flag == TTEntry::EXACT) {
        score = entry.score;
        m_hits++;
        return true;
    }

    if (entry.flag == TTEntry::LOWER_BOUND && entry.score >= beta) {
        score = entry.score;
        m_hits++;
        return true;
    }

    if (entry.flag == TTEntry::UPPER_BOUND && entry.score <= alpha) {
        score = entry.score;
        m_hits++;
        return true;
    }

    return false;
}

void TranspositionTable::store(quint64 key, int depth, int score, TTEntry::Flag flag, const AIMove &bestMove)
{
    if (m_threadSafe) {
        QMutexLocker locker(&m_mutex);
        storeImpl(key, depth, score, flag, bestMove);
    } else {
        storeImpl(key, depth, score, flag, bestMove);
    }
}

void TranspositionTable::storeImpl(quint64 key, int depth, int score, TTEntry::Flag flag, const AIMove &bestMove)
{
    // 如果已存在该键
    if (m_table.contains(key)) {
        TTEntry &existing = m_table[key];
        // 只有当新结果深度更深或相等时才替换（深度优先策略）
        if (depth >= existing.depth) {
            existing.zobristKey = key;
            existing.depth = depth;
            existing.score = score;
            existing.flag = flag;
            existing.bestMove = bestMove;
        }
        return;
    }

    // 如果表满了，使用简单的替换策略
    if (m_table.size() >= TT_SIZE) {
        // 直接删除第一个元素（FIFO策略，简单高效）
        auto it = m_table.begin();
        m_table.erase(it);
    }

    // 存储新项
    TTEntry entry;
    entry.zobristKey = key;
    entry.depth = depth;
    entry.score = score;
    entry.flag = flag;
    entry.bestMove = bestMove;

    m_table[key] = entry;
}

std::optional<AIMove> TranspositionTable::getBestMove(quint64 key)
{
    if (m_threadSafe) {
        QMutexLocker locker(&m_mutex);
        return getBestMoveImpl(key);
    } else {
        return getBestMoveImpl(key);
    }
}

std::optional<AIMove> TranspositionTable::getBestMoveImpl(quint64 key)
{
    if (!m_table.contains(key)) {
        return std::nullopt;
    }

    TTEntry &entry = m_table[key];
    if (entry.bestMove.isValid()) {
        return entry.bestMove;  // 返回值拷贝，线程安全
    }

    return std::nullopt;
}

void TranspositionTable::clear()
{
    m_table.clear();
}

void TranspositionTable::resetStatistics()
{
    m_hits = 0;
}
