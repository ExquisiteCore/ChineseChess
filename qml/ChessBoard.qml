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

    // 棋子数据模型
    ListModel {
        id: piecesModel

        Component.onCompleted: {
            // 黑方（上方）
            // 第0行：车 马 象 士 将 士 象 马 车
            append({ pieceType: "车", isRed: false, row: 0, col: 0 })
            append({ pieceType: "马", isRed: false, row: 0, col: 1 })
            append({ pieceType: "象", isRed: false, row: 0, col: 2 })
            append({ pieceType: "士", isRed: false, row: 0, col: 3 })
            append({ pieceType: "将", isRed: false, row: 0, col: 4 })
            append({ pieceType: "士", isRed: false, row: 0, col: 5 })
            append({ pieceType: "象", isRed: false, row: 0, col: 6 })
            append({ pieceType: "马", isRed: false, row: 0, col: 7 })
            append({ pieceType: "车", isRed: false, row: 0, col: 8 })

            // 第2行：炮
            append({ pieceType: "炮", isRed: false, row: 2, col: 1 })
            append({ pieceType: "炮", isRed: false, row: 2, col: 7 })

            // 第3行：卒
            append({ pieceType: "卒", isRed: false, row: 3, col: 0 })
            append({ pieceType: "卒", isRed: false, row: 3, col: 2 })
            append({ pieceType: "卒", isRed: false, row: 3, col: 4 })
            append({ pieceType: "卒", isRed: false, row: 3, col: 6 })
            append({ pieceType: "卒", isRed: false, row: 3, col: 8 })

            // 红方（下方）
            // 第6行：兵
            append({ pieceType: "兵", isRed: true, row: 6, col: 0 })
            append({ pieceType: "兵", isRed: true, row: 6, col: 2 })
            append({ pieceType: "兵", isRed: true, row: 6, col: 4 })
            append({ pieceType: "兵", isRed: true, row: 6, col: 6 })
            append({ pieceType: "兵", isRed: true, row: 6, col: 8 })

            // 第7行：炮
            append({ pieceType: "炮", isRed: true, row: 7, col: 1 })
            append({ pieceType: "炮", isRed: true, row: 7, col: 7 })

            // 第9行：車 馬 相 仕 帥 仕 相 馬 車
            append({ pieceType: "車", isRed: true, row: 9, col: 0 })
            append({ pieceType: "馬", isRed: true, row: 9, col: 1 })
            append({ pieceType: "相", isRed: true, row: 9, col: 2 })
            append({ pieceType: "仕", isRed: true, row: 9, col: 3 })
            append({ pieceType: "帥", isRed: true, row: 9, col: 4 })
            append({ pieceType: "仕", isRed: true, row: 9, col: 5 })
            append({ pieceType: "相", isRed: true, row: 9, col: 6 })
            append({ pieceType: "馬", isRed: true, row: 9, col: 7 })
            append({ pieceType: "車", isRed: true, row: 9, col: 8 })
        }
    }

    // 将逻辑坐标转换为屏幕坐标
    function gridToX(col) {
        return offsetX + col * gridSize
    }

    function gridToY(row) {
        return offsetY + row * gridSize
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

    // 渲染所有棋子
    Repeater {
        model: piecesModel
        delegate: ChessPiece {
            pieceType: model.pieceType
            isRed: model.isRed
            row: model.row
            col: model.col
            pieceSize: gridSize * 0.9

            // 居中对齐到交叉点
            x: gridToX(model.col) - width / 2
            y: gridToY(model.row) - height / 2

            onClicked: {
                console.log("点击了棋子:", model.pieceType, "位置:", model.row, model.col)
            }
        }
    }

    // 当尺寸改变时重绘
    onWidthChanged: boardCanvas.requestPaint()
    onHeightChanged: boardCanvas.requestPaint()
}
