import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: gamePage
    color: "#f5e6d3"

    property string gameMode: "single"  // "single" 或 "two"

    signal backToMenu()

    // 音效管理器
    SoundManager {
        id: soundManager
        soundEnabled: true
        volume: 0.7
    }

    // 连接C++信号到音效播放
    Connections {
        target: chessBoardModel

        // 棋子移动信号
        function onPieceMoved(isCapture) {
            if (isCapture) {
                soundManager.playSound(SoundManager.SoundType.Capture)
            } else {
                soundManager.playSound(SoundManager.SoundType.Move)
            }
        }

        // 将军信号
        function onCheckDetected() {
            soundManager.playSound(SoundManager.SoundType.Check)
            // 显示将军提示
            checkNotification.text = chessBoardModel.gameStatus
            checkNotification.visible = true
            checkNotificationTimer.restart()
        }

        // 将死信号
        function onCheckmateDetected() {
            soundManager.playSound(SoundManager.SoundType.Checkmate)
        }
    }

    // 组件创建完成后设置游戏模式
    Component.onCompleted: {
        console.log("GamePage loaded with gameMode:", gameMode)

        // 检查是否有自动存档
        if (chessBoardModel.hasAutoSave) {
            // 显示对话框询问是否加载存档
            loadSaveDialog.open()
        } else {
            // 没有存档，开始新游戏
            startNewGameSession()
        }
    }

    // 开始新游戏会话的函数
    function startNewGameSession() {
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

    // 加载存档对话框
    Dialog {
        id: loadSaveDialog
        title: "发现存档"
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Yes | Dialog.No

        contentItem: Text {
            text: "检测到上次未完成的棋局，是否继续？"
            font.pixelSize: 16
            wrapMode: Text.WordWrap
        }

        onAccepted: {
            // 加载自动存档
            if (chessBoardModel.loadAutoSave()) {
                console.log("成功加载存档")

                // 根据存档恢复游戏模式
                if (gameMode === "single") {
                    chessBoardModel.aiEnabled = true
                }
            } else {
                console.log("加载存档失败，开始新游戏")
                startNewGameSession()
            }
        }

        onRejected: {
            // 开始新游戏
            startNewGameSession()
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
                text: i18n.tr("back_to_menu")
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
                text: i18n.tr("current_turn") + ": " + (chessBoard.isRedTurn ? i18n.tr("red_turn") : i18n.tr("black_turn"))
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
                        text: i18n.tr("ai_thinking")
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
                text: i18n.tr("undo")
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
                text: i18n.tr("redo")
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
                text: i18n.tr("restart")
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
                    text: i18n.tr("game_info")
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
                    text: i18n.tr("status") + ": " + chessBoardModel.gameStatus
                    font.pixelSize: 14
                    color: "#654321"
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                Text {
                    text: i18n.tr("move_count") + ": " + chessBoardModel.moveCount
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
                                text: i18n.tr("ai_opponent") + ":"
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
                                text: chessBoardModel.aiEnabled ? i18n.tr("enabled") : i18n.tr("disabled")
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
                                text: i18n.tr("difficulty") + ":"
                                font.pixelSize: 13
                                color: "#654321"
                            }

                            ComboBox {
                                id: difficultyCombo
                                Layout.fillWidth: true
                                model: [i18n.tr("easy"), i18n.tr("medium"), i18n.tr("hard"), i18n.tr("expert")]

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
                    text: i18n.tr("move_history")
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
                    text: i18n.tr("operations")
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
                        text: i18n.tr("hint")
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
                        text: i18n.tr("offer_draw")
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
                        text: i18n.tr("resign")
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
                        text: i18n.tr("save_game")
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
                        text: i18n.tr("load_game")
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
                text: i18n.tr("save_game_title")
                font.pixelSize: 24
                font.bold: true
                color: "#8b4513"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: i18n.tr("save_game_desc")
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
                    text: i18n.tr("close")
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
                text: i18n.tr("load_game_title")
                font.pixelSize: 24
                font.bold: true
                color: "#8b4513"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: i18n.tr("load_game_desc")
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
                text: i18n.tr("fen_error")
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
                    text: i18n.tr("cancel")
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
                    text: i18n.tr("load")
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

        function onDrawRequested() {
            // 显示和棋请求对话框
            drawRequestDialog.visible = true
        }

        function onDrawDeclined(message) {
            drawNotification.text = message
            drawNotification.visible = true
            drawNotificationTimer.restart()
        }
    }

    // 将军通知
    Rectangle {
        id: checkNotification
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: topBar.bottom
        anchors.topMargin: 20
        width: 400
        height: 80
        color: "#ff6b6b"
        radius: 15
        border.color: "#c92a2a"
        border.width: 3
        visible: false
        z: 150

        property alias text: checkNotificationText.text

        // 闪烁动画
        SequentialAnimation on opacity {
            running: checkNotification.visible
            loops: 3
            NumberAnimation { from: 1.0; to: 0.5; duration: 200 }
            NumberAnimation { from: 0.5; to: 1.0; duration: 200 }
        }

        // 震动效果
        SequentialAnimation on scale {
            running: checkNotification.visible
            NumberAnimation { from: 0.8; to: 1.1; duration: 150; easing.type: Easing.OutBack }
            NumberAnimation { from: 1.1; to: 1.0; duration: 100 }
        }

        Text {
            id: checkNotificationText
            anchors.centerIn: parent
            font.pixelSize: 24
            font.bold: true
            color: "white"
            text: "将军！"
        }

        Timer {
            id: checkNotificationTimer
            interval: 2500
            onTriggered: {
                checkNotification.visible = false
            }
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

    // 和棋请求对话框
    Rectangle {
        id: drawRequestDialog
        anchors.centerIn: parent
        width: 400
        height: 250
        color: "#f5f5f5"
        radius: 15
        border.color: "#8b4513"
        border.width: 3
        visible: false
        z: 200

        // 背景遮罩
        Rectangle {
            anchors.fill: parent
            anchors.margins: -1000
            color: "#80000000"
            z: -1
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 25
            spacing: 20

            Text {
                text: "对方提出和棋"
                font.pixelSize: 28
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
                text: (chessBoardModel.isRedTurn ? "黑方" : "红方") + "提出和棋请求\n是否接受？"
                font.pixelSize: 18
                color: "#654321"
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Item { Layout.fillHeight: true }

            RowLayout {
                Layout.fillWidth: true
                spacing: 15

                Button {
                    text: "拒绝"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50

                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 18
                        font.bold: true
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: parent.pressed ? "#c94040" : (parent.hovered ? "#e05555" : "#ff6666")
                        radius: 8
                        border.color: "#b83030"
                        border.width: 2
                    }

                    onClicked: {
                        drawRequestDialog.visible = false
                        chessBoardModel.declineDraw()
                    }
                }

                Button {
                    text: "接受"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50

                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 18
                        font.bold: true
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: parent.pressed ? "#4caf50" : (parent.hovered ? "#66bb6a" : "#81c784")
                        radius: 8
                        border.color: "#388e3c"
                        border.width: 2
                    }

                    onClicked: {
                        drawRequestDialog.visible = false
                        chessBoardModel.acceptDraw()
                    }
                }
            }
        }
    }

    // 游戏结束弹窗（胜利/失败）
    Rectangle {
        id: gameOverDialog
        anchors.centerIn: parent
        width: 500
        height: 400
        color: "#f5f5f5"
        radius: 20
        border.color: "#8b4513"
        border.width: 5
        visible: false
        z: 200
        scale: 0.3
        opacity: 0

        property string resultText: ""
        property bool isVictory: false

        // 出现动画
        ParallelAnimation {
            id: gameOverShowAnimation
            NumberAnimation {
                target: gameOverDialog
                property: "scale"
                from: 0.3
                to: 1.0
                duration: 600
                easing.type: Easing.OutBack
            }
            NumberAnimation {
                target: gameOverDialog
                property: "opacity"
                from: 0.0
                to: 1.0
                duration: 400
            }
        }

        // 背景遮罩
        Rectangle {
            anchors.fill: parent
            anchors.margins: -1000
            color: "#80000000"
            z: -1
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 30
            spacing: 20

            // 结果图标和文字
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 150

                // 胜利图标（王冠）
                Text {
                    visible: gameOverDialog.isVictory
                    anchors.centerIn: parent
                    text: "👑"
                    font.pixelSize: 100

                    // 旋转闪烁动画
                    SequentialAnimation on rotation {
                        running: gameOverDialog.visible && gameOverDialog.isVictory
                        loops: Animation.Infinite
                        NumberAnimation { from: -10; to: 10; duration: 500; easing.type: Easing.InOutQuad }
                        NumberAnimation { from: 10; to: -10; duration: 500; easing.type: Easing.InOutQuad }
                    }

                    SequentialAnimation on scale {
                        running: gameOverDialog.visible && gameOverDialog.isVictory
                        loops: Animation.Infinite
                        NumberAnimation { from: 1.0; to: 1.15; duration: 800; easing.type: Easing.InOutQuad }
                        NumberAnimation { from: 1.15; to: 1.0; duration: 800; easing.type: Easing.InOutQuad }
                    }
                }

                // 失败图标（哭脸）
                Text {
                    visible: !gameOverDialog.isVictory
                    anchors.centerIn: parent
                    text: "😢"
                    font.pixelSize: 100

                    // 摇晃动画
                    SequentialAnimation on x {
                        running: gameOverDialog.visible && !gameOverDialog.isVictory
                        loops: Animation.Infinite
                        NumberAnimation { from: parent.width / 2 - 50; to: parent.width / 2 + 50; duration: 100 }
                        NumberAnimation { from: parent.width / 2 + 50; to: parent.width / 2 - 50; duration: 100 }
                        NumberAnimation { from: parent.width / 2 - 50; to: parent.width / 2; duration: 100 }
                    }
                }
            }

            // 结果文字
            Text {
                text: gameOverDialog.resultText
                font.pixelSize: 32
                font.bold: true
                color: gameOverDialog.isVictory ? "#4caf50" : "#f44336"
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
            }

            // 分隔线
            Rectangle {
                Layout.fillWidth: true
                height: 2
                color: "#8b4513"
            }

            // 统计信息
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 10

                Text {
                    text: i18n.tr("total_moves") + ": " + chessBoardModel.moveCount
                    font.pixelSize: 18
                    color: "#654321"
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: chessBoardModel.gameStatus
                    font.pixelSize: 16
                    color: "#654321"
                    Layout.alignment: Qt.AlignHCenter
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }
            }

            Item { Layout.fillHeight: true }

            // 按钮组
            RowLayout {
                Layout.fillWidth: true
                spacing: 15

                Button {
                    text: i18n.tr("play_again")
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50

                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 18
                        font.bold: true
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: parent.pressed ? "#4caf50" : (parent.hovered ? "#66bb6a" : "#81c784")
                        radius: 8
                        border.color: "#388e3c"
                        border.width: 2
                    }

                    onClicked: {
                        gameOverDialog.visible = false
                        chessBoardModel.startNewGame()
                    }
                }

                Button {
                    text: i18n.tr("back_to_menu")
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50

                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 18
                        font.bold: true
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: parent.pressed ? "#654321" : (parent.hovered ? "#a0522d" : "#8b4513")
                        radius: 8
                        border.color: "#5d3a1a"
                        border.width: 2
                    }

                    onClicked: {
                        gameOverDialog.visible = false
                        gamePage.backToMenu()
                    }
                }
            }
        }
    }

    // 监听游戏结束信号
    Connections {
        target: chessBoardModel
        function onGameOver(result) {
            console.log("游戏结束:", result)

            // 设置结果文字
            gameOverDialog.resultText = result

            // 判断是否胜利（如果是红方且结果包含"红方胜"，或者结果包含"和棋"）
            if (result.indexOf("红方胜") >= 0) {
                gameOverDialog.isVictory = true
            } else if (result.indexOf("黑方胜") >= 0) {
                gameOverDialog.isVictory = false
            } else if (result.indexOf("和棋") >= 0) {
                gameOverDialog.isVictory = true  // 和棋也算不错的结果
            } else {
                gameOverDialog.isVictory = false
            }

            // 延迟显示弹窗，让玩家看清最后一步
            gameOverShowTimer.start()
        }
    }

    Timer {
        id: gameOverShowTimer
        interval: 800
        onTriggered: {
            gameOverDialog.visible = true
            gameOverShowAnimation.start()
        }
    }
}