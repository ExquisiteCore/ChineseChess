#include "OpeningBook.h"
#include <QRandomGenerator>
#include <QTextStream>
#include <QDebug>

OpeningBook::OpeningBook()
    : m_enabled(true)
{
    initializeCommonOpenings();
}

bool OpeningBook::loadFromFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开开局库文件:" << filename;
        return false;
    }

    QTextStream in(&file);
    m_book.clear();

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        // 格式: zobrist_key from_row from_col to_row to_col weight winrate comment
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        if (parts.size() >= 6) {
            quint64 key = parts[0].toULongLong();
            int fromRow = parts[1].toInt();
            int fromCol = parts[2].toInt();
            int toRow = parts[3].toInt();
            int toCol = parts[4].toInt();
            int weight = parts[5].toInt();
            int winRate = parts.size() > 6 ? parts[6].toInt() : 50;
            QString comment = parts.size() > 7 ? parts.mid(7).join(' ') : "";

            AIMove move(fromRow, fromCol, toRow, toCol);
            addMove(key, move, weight, winRate);
        }
    }

    file.close();
    qDebug() << "开局库加载完成，共" << m_book.size() << "个局面";
    return true;
}

bool OpeningBook::saveToFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "无法写入开局库文件:" << filename;
        return false;
    }

    QTextStream out(&file);
    out << "# Chinese Chess Opening Book\n";
    out << "# Format: zobrist_key from_row from_col to_row to_col weight winrate comment\n\n";

    for (auto it = m_book.begin(); it != m_book.end(); ++it) {
        quint64 key = it.key();
        const QList<BookEntry> &entries = it.value();

        for (const BookEntry &entry : entries) {
            out << key << " "
                << entry.move.fromRow << " " << entry.move.fromCol << " "
                << entry.move.toRow << " " << entry.move.toCol << " "
                << entry.weight << " " << entry.winRate;
            if (!entry.comment.isEmpty()) {
                out << " " << entry.comment;
            }
            out << "\n";
        }
    }

    file.close();
    qDebug() << "开局库保存完成";
    return true;
}

QList<BookEntry> OpeningBook::probe(const Position &position, quint64 zobristKey)
{
    if (!m_enabled) {
        return QList<BookEntry>();
    }

    if (m_book.contains(zobristKey)) {
        return m_book[zobristKey];
    }

    return QList<BookEntry>();
}

AIMove OpeningBook::selectMove(const Position &position, quint64 zobristKey)
{
    QList<BookEntry> entries = probe(position, zobristKey);
    if (entries.isEmpty()) {
        return AIMove(); // 不在开局库中
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
            qDebug() << "使用开局库走法:" << entry.move.fromRow << entry.move.fromCol
                     << "->" << entry.move.toRow << entry.move.toCol
                     << "权重:" << entry.weight << "胜率:" << entry.winRate << "%";
            return entry.move;
        }
    }

    return entries.first().move;
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

    // 注意：这里的zobrist key是占位符，实际使用时需要从真实局面计算
    // 这里只是示例，展示如何添加开局走法

    // 开局阶段的常见走法（使用0作为占位key，实际应用中会被替换）
    // 这些走法需要在实际游戏中通过Position计算真实的zobrist key

    // 示例：添加一些常见的开局原则
    // 1. 中炮开局
    // 2. 马局开局
    // 3. 飞相开局

    // 由于需要真实的局面才能计算zobrist key，这里暂时留空
    // 在实际使用时，应该通过学习大量棋谱来构建开局库

    qDebug() << "开局库初始化完成（待添加具体走法）";
}

int OpeningBook::getTotalMoves() const
{
    int total = 0;
    for (const QList<BookEntry> &entries : m_book) {
        total += entries.size();
    }
    return total;
}

void OpeningBook::addSymmetricMoves(quint64 key, const AIMove &move, int weight, int winRate)
{
    // 添加原始移动
    addMove(key, move, weight, winRate);

    // 可以添加镜像对称的移动（如果需要）
    // 中国象棋左右对称
    // AIMove mirrorMove(move.fromRow, 8 - move.fromCol, move.toRow, 8 - move.toCol);
    // addMove(mirrorKey, mirrorMove, weight, winRate);
}
