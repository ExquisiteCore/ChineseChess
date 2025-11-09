#include "Position.h"
#include <QDebug>

Position::Position()
    : m_currentTurn(PieceColor::Red)
    , m_halfMoveClock(0)
    , m_fullMoveNumber(1)
{
    m_board.initializeStartPosition();
}

Position::Position(const Board &board)
    : m_board(board)
    , m_currentTurn(PieceColor::Red)
    , m_halfMoveClock(0)
    , m_fullMoveNumber(1)
{
}

void Position::switchTurn()
{
    m_currentTurn = (m_currentTurn == PieceColor::Red) ? PieceColor::Black : PieceColor::Red;
}

QString Position::toFen() const
{
    QString fen;

    // 1. 棋盘布局
    for (int row = 0; row < Board::ROWS; ++row) {
        int emptyCount = 0;

        for (int col = 0; col < Board::COLS; ++col) {
            const ChessPiece *piece = m_board.pieceAt(row, col);

            if (piece && piece->isValid()) {
                // 如果有空格计数，先添加数字
                if (emptyCount > 0) {
                    fen += QString::number(emptyCount);
                    emptyCount = 0;
                }
                // 添加棋子字符
                fen += piece->fenChar();
            } else {
                ++emptyCount;
            }
        }

        // 行末的空格
        if (emptyCount > 0) {
            fen += QString::number(emptyCount);
        }

        // 行分隔符（最后一行不加）
        if (row < Board::ROWS - 1) {
            fen += "/";
        }
    }

    // 2. 当前回合
    fen += (m_currentTurn == PieceColor::Red) ? " w" : " b";

    // 3-4. 保留字段（兼容国际象棋FEN）
    fen += " - -";

    // 5. 半回合计数
    fen += " " + QString::number(m_halfMoveClock);

    // 6. 全回合计数
    fen += " " + QString::number(m_fullMoveNumber);

    return fen;
}

bool Position::fromFen(const QString &fen)
{
    QStringList parts = fen.split(' ', Qt::SkipEmptyParts);
    if (parts.isEmpty())
        return false;

    // 1. 解析棋盘布局
    m_board.clear();
    QStringList rows = parts[0].split('/');

    if (rows.size() != Board::ROWS) {
        qDebug() << "FEN 错误: 行数不正确，应为" << Board::ROWS << "行";
        return false;
    }

    for (int row = 0; row < rows.size(); ++row) {
        const QString &rowStr = rows[row];
        int col = 0;

        for (const QChar &ch : rowStr) {
            if (ch.isDigit()) {
                // 数字表示空格数量
                int emptyCount = ch.digitValue();
                col += emptyCount;
            } else {
                // 棋子字符
                if (col >= Board::COLS) {
                    qDebug() << "FEN 错误: 第" << row << "行列数超出";
                    return false;
                }
                ChessPiece piece = ChessPiece::fromFenChar(ch, row, col);
                if (piece.isValid()) {
                    m_board.setPiece(row, col, piece);
                }
                ++col;
            }
        }

        if (col != Board::COLS) {
            qDebug() << "FEN 错误: 第" << row << "行列数不正确";
            return false;
        }
    }

    // 2. 当前回合
    if (parts.size() > 1) {
        m_currentTurn = (parts[1] == "w") ? PieceColor::Red : PieceColor::Black;
    }

    // 5. 半回合计数
    if (parts.size() > 4) {
        m_halfMoveClock = parts[4].toInt();
    }

    // 6. 全回合计数
    if (parts.size() > 5) {
        m_fullMoveNumber = parts[5].toInt();
    }

    return true;
}

QString Position::toBoardFen() const
{
    QString fen;

    for (int row = 0; row < Board::ROWS; ++row) {
        int emptyCount = 0;

        for (int col = 0; col < Board::COLS; ++col) {
            const ChessPiece *piece = m_board.pieceAt(row, col);

            if (piece && piece->isValid()) {
                if (emptyCount > 0) {
                    fen += QString::number(emptyCount);
                    emptyCount = 0;
                }
                fen += piece->fenChar();
            } else {
                ++emptyCount;
            }
        }

        if (emptyCount > 0) {
            fen += QString::number(emptyCount);
        }

        if (row < Board::ROWS - 1) {
            fen += "/";
        }
    }

    return fen;
}

void Position::print() const
{
    qDebug() << "=== 局面信息 ===";
    qDebug() << "当前回合:" << (m_currentTurn == PieceColor::Red ? "红方" : "黑方");
    qDebug() << "半回合计数:" << m_halfMoveClock;
    qDebug() << "全回合计数:" << m_fullMoveNumber;
    qDebug() << "FEN:" << toFen();
    m_board.print();
}
