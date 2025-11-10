import QtQuick
import QtQuick.Controls

Rectangle {
    id: chessBoard
    color: "#f5deb3"

    // 棋盘参数
    property int rows: 10        // 横线数量
    property int cols: 9         // 竖线数量
    property real gridSize: Math.min(width / (cols + 1), height / (rows + 1))  // 网格大小
    property real boardWidth: gridSize * (cols - 1)   // 棋盘实际宽度
    property real boardHeight: gridSize * (rows - 1)  // 棋盘实际高度
    property real offsetX: (width - boardWidth) / 2   // 水平偏移（居中）
    property real offsetY: (height - boardHeight) / 2 // 垂直偏移（居中）
    property real lineWidth: 2    // 线条宽度

    // 游戏状态（现在由 C++ 模型提供）
    property bool isRedTurn: chessBoardModel.isRedTurn
    property int liftedPieceIndex: chessBoardModel.liftedPieceIndex

    // 移除 QML 的 ListModel，改用 C++ 模型

    // 将逻辑坐标转换为屏幕坐标
    function gridToX(col) {
        return offsetX + col * gridSize
    }

    function gridToY(row) {
        return offsetY + row * gridSize
    }

    // 将屏幕坐标转换为逻辑坐标
    function screenToGridCol(x) {
        return Math.round((x - offsetX) / gridSize)
    }

    function screenToGridRow(y) {
        return Math.round((y - offsetY) / gridSize)
    }

    // 处理棋盘点击（包括空位）
    MouseArea {
        anchors.fill: parent
        z: -1  // 设置为负数，让棋子在上层
        propagateComposedEvents: true  // 允许事件传播到上层元素（棋子）
        onClicked: function(mouse) {
            var col = screenToGridCol(mouse.x)
            var row = screenToGridRow(mouse.y)
            console.log("棋盘被点击:", "位置:", row, col, "当前悬浮:", liftedPieceIndex)

            // 检查坐标是否有效
            if (col >= 0 && col < cols && row >= 0 && row < rows) {
                // 如果有棋子被提起，尝试移动到该位置
                if (liftedPieceIndex >= 0) {
                    chessBoardModel.movePieceToPosition(liftedPieceIndex, row, col)
                }
            }
            // 不设置 mouse.accepted，让事件继续传播
        }
    }

    // 绘制棋盘
    Canvas {
        id: boardCanvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            // 设置线条样式
            ctx.strokeStyle = "#654321"
            ctx.lineWidth = lineWidth
            ctx.lineCap = "round"
            ctx.lineJoin = "round"

            // 绘制横线
            for (var row = 0; row < rows; row++) {
                ctx.beginPath()
                ctx.moveTo(gridToX(0), gridToY(row))
                ctx.lineTo(gridToX(cols - 1), gridToY(row))
                ctx.stroke()
            }

            // 绘制竖线（中间断开，除了边线）
            for (var col = 0; col < cols; col++) {
                // 上半部分（红方）
                ctx.beginPath()
                ctx.moveTo(gridToX(col), gridToY(0))
                ctx.lineTo(gridToX(col), gridToY(4))
                ctx.stroke()

                // 下半部分（黑方）
                ctx.beginPath()
                ctx.moveTo(gridToX(col), gridToY(5))
                ctx.lineTo(gridToX(col), gridToY(9))
                ctx.stroke()
            }

            // 绘制九宫斜线（上方 - 红方）
            ctx.beginPath()
            ctx.moveTo(gridToX(3), gridToY(0))
            ctx.lineTo(gridToX(5), gridToY(2))
            ctx.stroke()

            ctx.beginPath()
            ctx.moveTo(gridToX(5), gridToY(0))
            ctx.lineTo(gridToX(3), gridToY(2))
            ctx.stroke()

            // 绘制九宫斜线（下方 - 黑方）
            ctx.beginPath()
            ctx.moveTo(gridToX(3), gridToY(7))
            ctx.lineTo(gridToX(5), gridToY(9))
            ctx.stroke()

            ctx.beginPath()
            ctx.moveTo(gridToX(5), gridToY(7))
            ctx.lineTo(gridToX(3), gridToY(9))
            ctx.stroke()

            // 绘制炮和兵的标记点
            drawMarker(ctx, 1, 2)  // 红方炮位
            drawMarker(ctx, 7, 2)
            drawMarker(ctx, 1, 7)  // 黑方炮位
            drawMarker(ctx, 7, 7)

            // 红方兵位
            drawMarker(ctx, 0, 3)
            drawMarker(ctx, 2, 3)
            drawMarker(ctx, 4, 3)
            drawMarker(ctx, 6, 3)
            drawMarker(ctx, 8, 3)

            // 黑方兵位
            drawMarker(ctx, 0, 6)
            drawMarker(ctx, 2, 6)
            drawMarker(ctx, 4, 6)
            drawMarker(ctx, 6, 6)
            drawMarker(ctx, 8, 6)
        }

        // 绘制标记点（炮和兵的位置）
        function drawMarker(ctx, col, row) {
            var x = gridToX(col)
            var y = gridToY(row)
            var markerSize = gridSize * 0.15
            var offset = gridSize * 0.08

            ctx.lineWidth = 1.5

            // 左上
            if (col > 0 && (row > 0 || (row === 0 && col >= 3 && col <= 5))) {
                ctx.beginPath()
                ctx.moveTo(x - offset, y - offset - markerSize)
                ctx.lineTo(x - offset, y - offset)
                ctx.lineTo(x - offset - markerSize, y - offset)
                ctx.stroke()
            }

            // 右上
            if (col < cols - 1 && (row > 0 || (row === 0 && col >= 3 && col <= 5))) {
                ctx.beginPath()
                ctx.moveTo(x + offset, y - offset - markerSize)
                ctx.lineTo(x + offset, y - offset)
                ctx.lineTo(x + offset + markerSize, y - offset)
                ctx.stroke()
            }

            // 左下
            if (col > 0 && (row < rows - 1 || (row === rows - 1 && col >= 3 && col <= 5))) {
                ctx.beginPath()
                ctx.moveTo(x - offset, y + offset + markerSize)
                ctx.lineTo(x - offset, y + offset)
                ctx.lineTo(x - offset - markerSize, y + offset)
                ctx.stroke()
            }

            // 右下
            if (col < cols - 1 && (row < rows - 1 || (row === rows - 1 && col >= 3 && col <= 5))) {
                ctx.beginPath()
                ctx.moveTo(x + offset, y + offset + markerSize)
                ctx.lineTo(x + offset, y + offset)
                ctx.lineTo(x + offset + markerSize, y + offset)
                ctx.stroke()
            }

            ctx.lineWidth = lineWidth
        }
    }

    // 楚河汉界文字
    Item {
        anchors.fill: parent

        // 楚河
        Text {
            x: gridToX(1)
            y: gridToY(4.5) - height / 2
            text: "楚河"
            font.pixelSize: gridSize * 0.6
            font.bold: true
            font.family: "KaiTi"
            color: "#8b4513"
            opacity: 0.6
        }

        // 汉界
        Text {
            x: gridToX(5.5)
            y: gridToY(4.5) - height / 2
            text: "汉界"
            font.pixelSize: gridSize * 0.6
            font.bold: true
            font.family: "KaiTi"
            color: "#8b4513"
            opacity: 0.6
        }
    }

    // 渲染所有棋子（使用 C++ 模型）
    Repeater {
        model: chessBoardModel
        delegate: ChessPiece {
            id: piece
            pieceType: model.pieceType
            isRed: model.isRed
            row: model.row
            col: model.col
            pieceSize: gridSize * 0.9

            // 根据索引控制悬浮状态
            lifted: liftedPieceIndex === index

            // 居中对齐到交叉点
            x: gridToX(model.col) - width / 2
            y: gridToY(model.row) - height / 2

            // 添加位置移动动画
            Behavior on x {
                NumberAnimation {
                    duration: 400
                    easing.type: Easing.OutCubic
                }
            }

            Behavior on y {
                NumberAnimation {
                    duration: 400
                    easing.type: Easing.OutCubic
                }
            }

            onClicked: {
                // 调用 C++ 的选择逻辑
                console.log("ChessBoard: 棋子点击事件接收, index:", index)
                chessBoardModel.selectPiece(index)
            }
        }
    }

    // 显示可走位置的标记
    Repeater {
        model: chessBoardModel.validMovePositions

        delegate: Rectangle {
            id: moveHint
            width: gridSize * 0.35
            height: gridSize * 0.35
            radius: width / 2
            color: "#80ff6b35"  // 半透明橙色
            border.color: "#ffad5a"
            border.width: 2

            // 定位到可走位置（QPoint中x是col，y是row）
            x: gridToX(modelData.x) - width / 2
            y: gridToY(modelData.y) - height / 2

            // 呼吸动画
            SequentialAnimation on opacity {
                running: true
                loops: Animation.Infinite
                NumberAnimation { from: 0.6; to: 1.0; duration: 800; easing.type: Easing.InOutQuad }
                NumberAnimation { from: 1.0; to: 0.6; duration: 800; easing.type: Easing.InOutQuad }
            }

            // 鼠标悬停效果
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor

                onEntered: {
                    moveHint.scale = 1.2
                }

                onExited: {
                    moveHint.scale = 1.0
                }

                onClicked: {
                    // 点击可走位置，尝试移动
                    if (liftedPieceIndex >= 0) {
                        console.log("点击可走位置:", modelData.y, modelData.x)
                        chessBoardModel.movePieceToPosition(liftedPieceIndex, modelData.y, modelData.x)
                    }
                }
            }

            Behavior on scale {
                NumberAnimation { duration: 150; easing.type: Easing.OutBack }
            }
        }
    }

    // 当尺寸改变时重绘
    onWidthChanged: boardCanvas.requestPaint()
    onHeightChanged: boardCanvas.requestPaint()
}
