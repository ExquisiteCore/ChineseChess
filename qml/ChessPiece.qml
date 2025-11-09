import QtQuick

Item {
    id: chessPiece

    // 棋子属性
    property string pieceType: ""     // 棋子类型: 帥/將/仕/士/相/象/馬/車/炮/兵/卒
    property bool isRed: true         // 是否为红方
    property int row: 0               // 当前行
    property int col: 0               // 当前列
    property bool selected: false     // 是否被选中
    property real pieceSize: 40       // 棋子大小
    property bool lifted: false       // 是否被提起（悬浮状态）

    width: pieceSize
    height: pieceSize

    // 悬浮时提高层级，确保在其他棋子上方
    z: lifted ? 100 : 0

    // 棋子圆盘
    Rectangle {
        id: pieceCircle
        anchors.centerIn: parent
        width: pieceSize
        height: pieceSize
        radius: pieceSize / 2

        // 渐变背景（暖色调）
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: selected ? "#fff5e6" : "#ffefd5"  // 暖米色（木瓜色）
            }
            GradientStop {
                position: 1.0
                color: selected ? "#ffcc80" : "#ffd699"  // 暖橙黄色
            }
        }

        // 边框（暖棕色）
        border.color: "#d2691e"  // 巧克力橙色
        border.width: 3

        // 阴影效果（悬浮时增强）
        Rectangle {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: lifted ? 8 : 2  // 悬浮时阴影向下偏移更多
            width: parent.width * (lifted ? 1.2 : 1.0)    // 悬浮时阴影更大
            height: parent.height * (lifted ? 1.2 : 1.0)
            radius: width / 2
            color: lifted ? "#60000000" : "#40000000"     // 悬浮时阴影更深
            z: -1

            // 阴影大小和颜色的平滑过渡
            Behavior on width {
                NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
            }
            Behavior on height {
                NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
            }
            Behavior on color {
                ColorAnimation { duration: 300 }
            }
            Behavior on anchors.verticalCenterOffset {
                NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
            }
        }

        // 选中时的光晕效果
        Rectangle {
            visible: selected
            anchors.centerIn: parent
            width: parent.width + 8
            height: parent.height + 8
            radius: (parent.width + 8) / 2
            color: "transparent"
            border.color: "#ff4500"
            border.width: 3
            z: -2

            SequentialAnimation on opacity {
                running: selected
                loops: Animation.Infinite
                NumberAnimation { from: 1.0; to: 0.3; duration: 600 }
                NumberAnimation { from: 0.3; to: 1.0; duration: 600 }
            }
        }
    }

    // 棋子文字
    Text {
        anchors.centerIn: parent
        text: pieceType
        font.pixelSize: pieceSize * 0.55
        font.bold: true
        font.family: "KaiTi"
        color: isRed ? "#c62828" : "#1a1a1a"  // 红方用深红色，黑方用深灰色
        style: Text.Outline
        styleColor: isRed ? "#ffccbc" : "#d7ccc8"  // 暖色调描边
    }

    // 鼠标交互
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor

        onClicked: function(mouse) {
            mouse.accepted = true  // 阻止事件传播到父级（棋盘）
            chessPiece.clicked()  // 只发射信号，不修改 lifted 状态
        }
    }

    // 悬停效果（仅在未悬浮时生效）
    scale: lifted ? 1.15 : (mouseArea.containsMouse ? 1.1 : 1.0)
    Behavior on scale {
        NumberAnimation { duration: 150; easing.type: Easing.OutCubic }
    }

    // 悬浮动画：Y轴偏移
    property real liftOffset: lifted ? -pieceSize * 0.3 : 0
    transform: Translate {
        y: liftOffset + floatOffset
    }

    Behavior on liftOffset {
        NumberAnimation { duration: 300; easing.type: Easing.OutBack }
    }

    // 悬浮时的轻微上下浮动动画
    property real floatOffset: 0

    SequentialAnimation on floatOffset {
        running: lifted
        loops: Animation.Infinite
        NumberAnimation {
            from: 0
            to: -3
            duration: 800
            easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            from: -3
            to: 0
            duration: 800
            easing.type: Easing.InOutQuad
        }
    }

    // 点击信号
    signal clicked()
}
