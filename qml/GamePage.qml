import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: gamePage
    color: "#f5e6d3"

    signal backToMenu()

    // 顶部工具栏
    Rectangle {
        id: topBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 60
        color: "#8b4513"

        RowLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 10

            Button {
                text: "返回菜单"
                Layout.preferredWidth: 120
                Layout.preferredHeight: 40

                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 16
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    color: parent.pressed ? "#654321" : (parent.hovered ? "#a0522d" : "#6b3410")
                    radius: 5
                    border.color: "#ffd700"
                    border.width: 1
                }

                onClicked: gamePage.backToMenu()
            }

            Item { Layout.fillWidth: true }

            Text {
                text: "当前回合: " + (chessBoard.isRedTurn ? "红方" : "黑方")
                font.pixelSize: 20
                font.bold: true
                color: "white"
                Layout.alignment: Qt.AlignVCenter
            }

            Item { Layout.fillWidth: true }

            Button {
                text: "悔棋"
                Layout.preferredWidth: 100
                Layout.preferredHeight: 40
                enabled: chessBoardModel.canUndo

                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 16
                    color: parent.enabled ? "white" : "#aaaaaa"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    color: parent.enabled ? (parent.pressed ? "#654321" : (parent.hovered ? "#a0522d" : "#6b3410")) : "#555555"
                    radius: 5
                    border.color: "#ffd700"
                    border.width: 1
                }

                onClicked: {
                    chessBoardModel.undoMove()
                }
            }

            Button {
                text: "重做"
                Layout.preferredWidth: 100
                Layout.preferredHeight: 40
                enabled: chessBoardModel.canRedo

                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 16
                    color: parent.enabled ? "white" : "#aaaaaa"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    color: parent.enabled ? (parent.pressed ? "#654321" : (parent.hovered ? "#a0522d" : "#6b3410")) : "#555555"
                    radius: 5
                    border.color: "#ffd700"
                    border.width: 1
                }

                onClicked: {
                    chessBoardModel.redoMove()
                }
            }

            Button {
                text: "重新开始"
                Layout.preferredWidth: 100
                Layout.preferredHeight: 40

                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 16
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    color: parent.pressed ? "#654321" : (parent.hovered ? "#a0522d" : "#6b3410")
                    radius: 5
                    border.color: "#ffd700"
                    border.width: 1
                }

                onClicked: {
                    chessBoardModel.startNewGame()
                }
            }
        }
    }

    // 游戏区域
    Item {
        anchors.top: topBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        // 棋盘容器
        Rectangle {
            id: boardContainer
            anchors.centerIn: parent
            width: Math.min(parent.width * 0.7, parent.height * 0.9)
            height: width * 1.1
            color: "#daa520"
            radius: 10
            border.color: "#8b4513"
            border.width: 3

            // 棋盘
            ChessBoard {
                id: chessBoard
                anchors.fill: parent
                anchors.margins: 20
            }
        }

        // 右侧信息面板
        Rectangle {
            anchors.left: boardContainer.right
            anchors.top: boardContainer.top
            anchors.right: parent.right
            anchors.bottom: boardContainer.bottom
            anchors.leftMargin: 20
            anchors.rightMargin: 20
            color: "#ffe4b5"
            radius: 10
            border.color: "#8b4513"
            border.width: 2

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 15

                Text {
                    text: "游戏信息"
                    font.pixelSize: 24
                    font.bold: true
                    color: "#8b4513"
                    Layout.alignment: Qt.AlignHCenter
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 2
                    color: "#8b4513"
                }

                Text {
                    text: "游戏状态: " + chessBoardModel.gameStatus
                    font.pixelSize: 16
                    color: "#654321"
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                Text {
                    text: "已走步数: " + chessBoardModel.moveCount
                    font.pixelSize: 16
                    color: "#654321"
                }

                Text {
                    text: "FEN: " + chessBoardModel.fenString
                    font.pixelSize: 12
                    color: "#654321"
                    wrapMode: Text.WrapAnywhere
                    Layout.fillWidth: true
                }

                Item { Layout.fillHeight: true }
            }
        }
    }
}
