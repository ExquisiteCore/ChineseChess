import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: gamePage
    color: "#f5e6d3"

    property string gameMode: "single"  // "single" 或 "two"

    signal backToMenu()

    // 组件创建完成后设置游戏模式
    Component.onCompleted: {
        console.log("GamePage loaded with gameMode:", gameMode)

        // 重置游戏为初始状态
        chessBoardModel.startNewGame()

        // 设置游戏模式
        chessBoardModel.isTwoPlayerMode = (gameMode === "two")
        console.log("isTwoPlayerMode set to:", chessBoardModel.isTwoPlayerMode)

        if (gameMode === "single") {
            chessBoardModel.aiEnabled = true
            console.log("AI enabled for single player mode")
        } else {
            console.log("Two player mode - AI disabled")
        }
    }

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

            // AI思考指示器
            Rectangle {
                visible: chessBoardModel.aiThinking
                Layout.preferredWidth: 100
                Layout.preferredHeight: 30
                color: "#ff9800"
                radius: 5

                RowLayout {
                    anchors.centerIn: parent
                    spacing: 5

                    Text {
                        text: "AI思考中"
                        font.pixelSize: 12
                        color: "white"
                    }

                    // 简单的动画指示器
                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: "white"

                        SequentialAnimation on opacity {
                            running: chessBoardModel.aiThinking
                            loops: Animation.Infinite
                            NumberAnimation { from: 1.0; to: 0.3; duration: 500 }
                            NumberAnimation { from: 0.3; to: 1.0; duration: 500 }
                        }
                    }
                }
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
    RowLayout {
        anchors.top: topBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 20
        spacing: 20

        // 左侧弹性空间（较小）
        Item {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
        }

        // 棋盘容器
        Rectangle {
            id: boardContainer
            Layout.preferredWidth: Math.min(parent.width * 0.6, parent.height * 0.9)
            Layout.preferredHeight: Layout.preferredWidth * 1.1
            Layout.alignment: Qt.AlignVCenter
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
            Layout.preferredWidth: 250
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignVCenter
            color: "#ffe4b5"
            radius: 10
            border.color: "#8b4513"
            border.width: 2

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10

                // 标题
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

                // 游戏状态
                Text {
                    text: "状态: " + chessBoardModel.gameStatus
                    font.pixelSize: 14
                    color: "#654321"
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                Text {
                    text: "步数: " + chessBoardModel.moveCount
                    font.pixelSize: 14
                    color: "#654321"
                }

                // AI控制区域（仅在人机对战模式显示）
                Rectangle {
                    visible: !chessBoardModel.isTwoPlayerMode
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    color: "#fff8dc"
                    radius: 5
                    border.color: "#8b4513"
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 8

                        // AI启用开关
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10

                            Text {
                                text: "AI对手:"
                                font.pixelSize: 14
                                font.bold: true
                                color: "#654321"
                            }

                            Switch {
                                id: aiSwitch
                                checked: chessBoardModel.aiEnabled
                                onCheckedChanged: {
                                    chessBoardModel.aiEnabled = checked
                                }
                            }

                            Text {
                                text: chessBoardModel.aiEnabled ? "已启用" : "已禁用"
                                font.pixelSize: 12
                                color: chessBoardModel.aiEnabled ? "#4caf50" : "#999999"
                            }
                        }

                        // AI难度选择
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10
                            enabled: chessBoardModel.aiEnabled

                            Text {
                                text: "难度:"
                                font.pixelSize: 13
                                color: "#654321"
                            }

                            ComboBox {
                                id: difficultyCombo
                                Layout.fillWidth: true
                                model: ["简单", "中等", "困难", "专家"]

                                // 使用Component.onCompleted设置初始值，避免绑定循环
                                Component.onCompleted: {
                                    currentIndex = chessBoardModel.aiDifficulty - 1
                                }

                                // 使用onActivated只在用户手动选择时触发
                                onActivated: function(index) {
                                    chessBoardModel.aiDifficulty = index + 1
                                }

                                delegate: ItemDelegate {
                                    width: difficultyCombo.width
                                    contentItem: Text {
                                        text: modelData
                                        color: "#654321"
                                        font.pixelSize: 12
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    highlighted: difficultyCombo.highlightedIndex === index
                                }

                                contentItem: Text {
                                    leftPadding: 10
                                    text: difficultyCombo.displayText
                                    font.pixelSize: 12
                                    color: "#654321"
                                    verticalAlignment: Text.AlignVCenter
                                }

                                background: Rectangle {
                                    implicitWidth: 120
                                    implicitHeight: 30
                                    border.color: "#8b4513"
                                    border.width: 1
                                    radius: 3
                                    color: parent.pressed ? "#e0e0e0" : "#ffffff"
                                }
                            }
                        }
                    }
                }

                // 走子历史标题
                Text {
                    text: "走子历史"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#8b4513"
                    Layout.topMargin: 10
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#8b4513"
                }

                // 走子历史列表
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.preferredHeight: 200
                    color: "white"
                    radius: 5
                    border.color: "#8b4513"
                    border.width: 1

                    ListView {
                        id: historyListView
                        anchors.fill: parent
                        anchors.margins: 5
                        clip: true
                        model: chessBoardModel.moveHistory

                        delegate: Text {
                            text: modelData
                            font.pixelSize: 13
                            color: "#654321"
                            padding: 3
                        }

                        ScrollBar.vertical: ScrollBar {
                            policy: ScrollBar.AsNeeded
                        }

                        // 自动滚动到底部
                        onCountChanged: {
                            historyListView.positionViewAtEnd()
                        }
                    }
                }

                // 操作按钮组
                Text {
                    text: "操作"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#8b4513"
                    Layout.topMargin: 10
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#8b4513"
                }

                // 第一行按钮：提示、求和、认输
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 5

                    Button {
                        text: "提示"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 35

                        contentItem: Text {
                            text: parent.text
                            font.pixelSize: 14
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            color: parent.pressed ? "#4a90e2" : (parent.hovered ? "#5ca3f5" : "#6db3ff")
                            radius: 5
                            border.color: "#3a7bc8"
                            border.width: 1
                        }

                        onClicked: {
                            chessBoardModel.showHint()
                        }
                    }

                    Button {
                        text: "求和"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 35

                        contentItem: Text {
                            text: parent.text
                            font.pixelSize: 14
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            color: parent.pressed ? "#e8a03d" : (parent.hovered ? "#f5b854" : "#ffc966")
                            radius: 5
                            border.color: "#d4901f"
                            border.width: 1
                        }

                        onClicked: {
                            chessBoardModel.offerDraw()
                        }
                    }

                    Button {
                        text: "认输"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 35

                        contentItem: Text {
                            text: parent.text
                            font.pixelSize: 14
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            color: parent.pressed ? "#c94040" : (parent.hovered ? "#e05555" : "#ff6666")
                            radius: 5
                            border.color: "#b83030"
                            border.width: 1
                        }

                        onClicked: {
                            chessBoardModel.resign()
                        }
                    }
                }

                // 第二行按钮：保存、加载
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 5

                    Button {
                        text: "保存局面"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 35

                        contentItem: Text {
                            text: parent.text
                            font.pixelSize: 14
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            color: parent.pressed ? "#50a060" : (parent.hovered ? "#60b870" : "#70d080")
                            radius: 5
                            border.color: "#408850"
                            border.width: 1
                        }

                        onClicked: {
                            saveDialog.visible = true
                        }
                    }

                    Button {
                        text: "加载局面"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 35

                        contentItem: Text {
                            text: parent.text
                            font.pixelSize: 14
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            color: parent.pressed ? "#9070c0" : (parent.hovered ? "#a080d0" : "#b090e0")
                            radius: 5
                            border.color: "#7858a0"
                            border.width: 1
                        }

                        onClicked: {
                            loadDialog.visible = true
                        }
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }

        // 右侧弹性空间（较小）
        Item {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
        }
    }

    // 保存局面对话框
    Rectangle {
        id: saveDialog
        anchors.centerIn: parent
        width: 500
        height: 300
        color: "#f5f5f5"
        radius: 10
        border.color: "#8b4513"
        border.width: 3
        visible: false
        z: 100

        // 当对话框显示时，更新FEN字符串
        onVisibleChanged: {
            if (visible) {
                saveTextEdit.text = chessBoardModel.saveGame()
                saveTextEdit.selectAll()  // 自动全选，方便复制
            }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15

            Text {
                text: "保存局面"
                font.pixelSize: 24
                font.bold: true
                color: "#8b4513"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "复制下面的 FEN 字符串保存："
                font.pixelSize: 14
                color: "#654321"
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "white"
                border.color: "#8b4513"
                border.width: 1
                radius: 5

                TextEdit {
                    id: saveTextEdit
                    anchors.fill: parent
                    anchors.margins: 10
                    font.pixelSize: 14
                    color: "#654321"
                    wrapMode: TextEdit.Wrap
                    selectByMouse: true
                    readOnly: true
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Button {
                    text: "关闭"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40

                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 16
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: parent.pressed ? "#654321" : (parent.hovered ? "#a0522d" : "#8b4513")
                        radius: 5
                        border.color: "#654321"
                        border.width: 1
                    }

                    onClicked: {
                        saveDialog.visible = false
                    }
                }
            }
        }
    }

    // 加载局面对话框
    Rectangle {
        id: loadDialog
        anchors.centerIn: parent
        width: 500
        height: 350
        color: "#f5f5f5"
        radius: 10
        border.color: "#8b4513"
        border.width: 3
        visible: false
        z: 100

        // 当对话框显示时，清空文本并聚焦
        onVisibleChanged: {
            if (visible) {
                loadTextEdit.text = ""
                loadTextEdit.forceActiveFocus()
                loadErrorText.visible = false
            }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15

            Text {
                text: "加载局面"
                font.pixelSize: 24
                font.bold: true
                color: "#8b4513"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "粘贴 FEN 字符串："
                font.pixelSize: 14
                color: "#654321"
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "white"
                border.color: "#8b4513"
                border.width: 1
                radius: 5

                TextEdit {
                    id: loadTextEdit
                    anchors.fill: parent
                    anchors.margins: 10
                    font.pixelSize: 14
                    color: "#654321"
                    wrapMode: TextEdit.Wrap
                    selectByMouse: true
                }
            }

            // 错误提示
            Text {
                id: loadErrorText
                text: "FEN 字符串格式错误，请检查后重试"
                font.pixelSize: 14
                color: "#ff0000"
                visible: false
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Button {
                    text: "取消"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40

                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 16
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: parent.pressed ? "#999999" : (parent.hovered ? "#aaaaaa" : "#bbbbbb")
                        radius: 5
                        border.color: "#888888"
                        border.width: 1
                    }

                    onClicked: {
                        loadDialog.visible = false
                        loadTextEdit.text = ""
                    }
                }

                Button {
                    text: "加载"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40

                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 16
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: parent.pressed ? "#50a060" : (parent.hovered ? "#60b870" : "#70d080")
                        radius: 5
                        border.color: "#408850"
                        border.width: 1
                    }

                    onClicked: {
                        if (chessBoardModel.loadGame(loadTextEdit.text)) {
                            console.log("局面加载成功")
                            loadDialog.visible = false
                            loadTextEdit.text = ""
                            loadErrorText.visible = false
                        } else {
                            console.log("局面加载失败")
                            loadErrorText.visible = true
                        }
                    }
                }
            }
        }
    }

    // 求和提示对话框
    Connections {
        target: chessBoardModel
        function onDrawOffered(message) {
            drawNotification.text = message
            drawNotification.visible = true
            drawNotificationTimer.restart()
        }
    }

    // 求和通知
    Rectangle {
        id: drawNotification
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: topBar.bottom
        anchors.topMargin: 20
        width: 300
        height: 50
        color: "#ffc966"
        radius: 10
        border.color: "#d4901f"
        border.width: 2
        visible: false
        z: 100

        property alias text: notificationText.text

        Text {
            id: notificationText
            anchors.centerIn: parent
            font.pixelSize: 16
            font.bold: true
            color: "#654321"
        }

        Timer {
            id: drawNotificationTimer
            interval: 3000
            onTriggered: {
                drawNotification.visible = false
            }
        }
    }
}