#ifndef CHESSBOARDMODEL_H
#define CHESSBOARDMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QPoint>
#include "../core/Position.h"
#include "../core/ChessRules.h"

// 棋盘数据模型（适配层，连接 C++ 核心和 QML UI）
class ChessBoardModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool isRedTurn READ isRedTurn WRITE setIsRedTurn NOTIFY isRedTurnChanged)
    Q_PROPERTY(int liftedPieceIndex READ liftedPieceIndex WRITE setLiftedPieceIndex NOTIFY liftedPieceIndexChanged)
    Q_PROPERTY(QString fenString READ fenString NOTIFY fenStringChanged)
    Q_PROPERTY(QString gameStatus READ gameStatus NOTIFY gameStatusChanged)
    Q_PROPERTY(QList<QPoint> validMovePositions READ validMovePositions NOTIFY validMovePositionsChanged)

public:
    enum ChessPieceRoles {
        PieceTypeRole = Qt::UserRole + 1,
        IsRedRole,
        RowRole,
        ColRole
    };

    explicit ChessBoardModel(QObject *parent = nullptr);

    // QAbstractListModel 必需的方法
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // 属性访问器
    bool isRedTurn() const;
    void setIsRedTurn(bool turn);

    int liftedPieceIndex() const { return m_liftedPieceIndex; }
    void setLiftedPieceIndex(int index);

    QString fenString() const { return m_position.toFen(); }

    QString gameStatus() const { return m_gameStatus; }

    QList<QPoint> validMovePositions() const { return m_validMovePositions; }

    // 获取 Position 对象
    Position& position() { return m_position; }
    const Position& position() const { return m_position; }

    // 游戏逻辑方法
    Q_INVOKABLE bool canSelectPiece(int index) const;
    Q_INVOKABLE void selectPiece(int index);
    Q_INVOKABLE void resetBoard();
    Q_INVOKABLE bool loadFromFen(const QString &fen);
    Q_INVOKABLE void printDebugInfo();

    // 走棋相关
    Q_INVOKABLE bool movePiece(int fromIndex, int toIndex);
    Q_INVOKABLE bool movePieceToPosition(int fromIndex, int toRow, int toCol);
    Q_INVOKABLE QList<int> getValidMoves(int index) const;

signals:
    void isRedTurnChanged();
    void liftedPieceIndexChanged();
    void fenStringChanged();
    void boardChanged();
    void gameStatusChanged();
    void gameOver(const QString &result);  // 游戏结束信号
    void validMovePositionsChanged();       // 可走位置改变信号

private:
    void rebuildPiecesList();  // 从 Position 重建棋子列表
    void checkGameStatus();     // 检查游戏状态
    void updateValidMoves();    // 更新可走位置

    Position m_position;              // 核心局面对象
    QList<ChessPiece> m_piecesList;   // 用于 QML 显示的棋子列表
    int m_liftedPieceIndex;           // 当前悬浮的棋子索引
    QString m_gameStatus;             // 游戏状态文本
    QList<QPoint> m_validMovePositions; // 当前棋子的可走位置列表
};

#endif // CHESSBOARDMODEL_H
