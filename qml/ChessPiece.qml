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

    width: pieceSize
    height: pieceSize

    // 棋子圆盘
    Rectangle {
        id: pieceCircle
        anchors.centerIn: parent
        width: pieceSize
        height: pieceSize
        radius: pieceSize / 2

        // 渐变背景
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: selected ? "#ffe4b5" : (isRed ? "#f5deb3" : "#fffacd")
            }
            GradientStop {
                position: 1.0
                color: selected ? "#ffd700" : (isRed ? "#f0e68c" : "#f5f5dc")
            }
        }

        // 边框
        border.color: isRed ? "#8b0000" : "#000000"
        border.width: 3

        // 阴影效果
        Rectangle {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: 2
            width: parent.width
            height: parent.height
            radius: parent.radius
            color: "#40000000"
            z: -1
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
        color: isRed ? "#8b0000" : "#000000"
        style: Text.Raised
        styleColor: isRed ? "#ffd700" : "#d3d3d3"
    }

    // 鼠标交互
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor

        onClicked: {
            chessPiece.clicked()
        }
    }

    // 悬停效果
    scale: mouseArea.containsMouse ? 1.1 : 1.0
    Behavior on scale {
        NumberAnimation { duration: 150; easing.type: Easing.OutCubic }
    }

    // 点击信号
    signal clicked()
}
