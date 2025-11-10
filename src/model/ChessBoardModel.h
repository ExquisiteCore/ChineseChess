#ifndef CHESSBOARDMODEL_H
#define CHESSBOARDMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QPoint>
#include <QTimer>
#include <QFutureWatcher>
#include "../core/Position.h"
#include "../core/ChessRules.h"
#include "../core/GameController.h"
#include "../ai/ChessAI.h"

// 棋盘数据模型（适配层，连接 C++ 核心和 QML UI）
class ChessBoardModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool isRedTurn READ isRedTurn WRITE setIsRedTurn NOTIFY isRedTurnChanged)
    Q_PROPERTY(int liftedPieceIndex READ liftedPieceIndex WRITE setLiftedPieceIndex NOTIFY liftedPieceIndexChanged)
    Q_PROPERTY(QString fenString READ fenString NOTIFY fenStringChanged)
    Q_PROPERTY(QString gameStatus READ gameStatus NOTIFY gameStatusChanged)
    Q_PROPERTY(QList<QPoint> validMovePositions READ validMovePositions NOTIFY validMovePositionsChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
    Q_PROPERTY(int moveCount READ moveCount NOTIFY moveCountChanged)
    Q_PROPERTY(QStringList moveHistory READ moveHistory NOTIFY moveHistoryChanged)
    Q_PROPERTY(bool aiEnabled READ aiEnabled WRITE setAiEnabled NOTIFY aiEnabledChanged)
    Q_PROPERTY(bool aiThinking READ aiThinking NOTIFY aiThinkingChanged)
    Q_PROPERTY(int aiDifficulty READ aiDifficulty WRITE setAiDifficulty NOTIFY aiDifficultyChanged)

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

    bool canUndo() const;
    bool canRedo() const;
    int moveCount() const;
    QStringList moveHistory() const;

    bool aiEnabled() const { return m_aiEnabled; }
    void setAiEnabled(bool enabled);

    bool aiThinking() const { return m_aiThinking; }

    int aiDifficulty() const { return static_cast<int>(m_ai.getDifficulty()); }
    void setAiDifficulty(int difficulty);

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

    // 游戏控制
    Q_INVOKABLE void undoMove();
    Q_INVOKABLE void redoMove();
    Q_INVOKABLE void startNewGame();
    Q_INVOKABLE QString exportGameHistory() const;

    // 保存/加载功能
    Q_INVOKABLE QString saveGame() const;         // 返回FEN字符串用于保存
    Q_INVOKABLE bool loadGame(const QString &fen); // 从FEN字符串加载

    // 游戏操作
    Q_INVOKABLE void showHint();      // 显示提示（高亮当前可走位置）
    Q_INVOKABLE void offerDraw();     // 提出和棋
    Q_INVOKABLE void resign();        // 认输

signals:
    void isRedTurnChanged();
    void liftedPieceIndexChanged();
    void fenStringChanged();
    void boardChanged();
    void gameStatusChanged();
    void gameOver(const QString &result);  // 游戏结束信号
    void validMovePositionsChanged();       // 可走位置改变信号
    void canUndoChanged();                  // 悔棋可用性改变
    void canRedoChanged();                  // 重做可用性改变
    void moveCountChanged();                 // 步数改变
    void moveHistoryChanged();               // 历史记录改变
    void drawOffered(const QString &message);  // 提出和棋
    void hintShown();                        // 显示提示
    void aiEnabledChanged();                 // AI启用状态改变
    void aiThinkingChanged();                // AI思考状态改变
    void aiDifficultyChanged();              // AI难度改变

private:
    void rebuildPiecesList();  // 从 Position 重建棋子列表
    void checkGameStatus();     // 检查游戏状态
    void updateValidMoves();    // 更新可走位置
    void triggerAIMove();       // 触发AI走棋
    void executeAIMove();       // 执行AI走棋（在定时器中调用）
    void onAIFinished();        // AI思考完成的槽函数

    // 辅助方法：执行移动并更新模型
    void updateModelAfterMove(int fromRow, int fromCol, int toRow, int toCol,
                              const ChessPiece *movedPiece, const ChessPiece *targetPiece);

    Position m_position;              // 核心局面对象
    QList<ChessPiece> m_piecesList;   // 用于 QML 显示的棋子列表
    int m_liftedPieceIndex;           // 当前悬浮的棋子索引
    QString m_gameStatus;             // 游戏状态文本
    QList<QPoint> m_validMovePositions; // 当前棋子的可走位置列表
    GameController m_gameController;   // 游戏控制器
    ChessAI m_ai;                      // AI引擎
    bool m_aiEnabled;                  // AI是否启用
    bool m_aiThinking;                 // AI是否正在思考
    QTimer *m_aiTimer;                 // AI延迟定时器（避免AI瞬间走棋）
    QFutureWatcher<AIMove> *m_aiWatcher; // AI异步任务监视器
};

#endif // CHESSBOARDMODEL_H
