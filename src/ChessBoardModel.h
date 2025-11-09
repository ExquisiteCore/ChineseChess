#ifndef CHESSBOARDMODEL_H
#define CHESSBOARDMODEL_H

#include <QAbstractListModel>
#include <QString>

// 棋子数据结构
struct ChessPieceData {
    QString type;       // 棋子类型：帥/將/車/馬等
    bool isRed;         // 是否为红方
    int row;            // 行位置 (0-9)
    int col;            // 列位置 (0-8)
};

// 棋盘数据模型
class ChessBoardModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool isRedTurn READ isRedTurn WRITE setIsRedTurn NOTIFY isRedTurnChanged)
    Q_PROPERTY(int liftedPieceIndex READ liftedPieceIndex WRITE setLiftedPieceIndex NOTIFY liftedPieceIndexChanged)

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
    bool isRedTurn() const { return m_isRedTurn; }
    void setIsRedTurn(bool turn);

    int liftedPieceIndex() const { return m_liftedPieceIndex; }
    void setLiftedPieceIndex(int index);

    // 游戏逻辑方法
    Q_INVOKABLE bool canSelectPiece(int index) const;
    Q_INVOKABLE void selectPiece(int index);

signals:
    void isRedTurnChanged();
    void liftedPieceIndexChanged();

private:
    void initializeBoard();  // 初始化棋盘

    QList<ChessPieceData> m_pieces;   // 所有棋子数据
    bool m_isRedTurn;                 // 当前回合
    int m_liftedPieceIndex;           // 当前悬浮的棋子索引
};

#endif // CHESSBOARDMODEL_H
