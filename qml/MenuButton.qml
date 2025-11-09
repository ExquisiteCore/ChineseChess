import QtQuick
import QtQuick.Controls

Button {
    id: menuButton

    property color buttonColor: "#8b4513"
    property color hoverColor: "#a0522d"
    property color pressedColor: "#654321"

    contentItem: Text {
        text: menuButton.text
        font.pixelSize: 24
        font.bold: true
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        color: menuButton.pressed ? pressedColor :
               (menuButton.hovered ? hoverColor : buttonColor)
        radius: 10
        border.color: "#ffd700"
        border.width: 2

        Behavior on color {
            ColorAnimation { duration: 150 }
        }

        // 添加光泽效果
        Rectangle {
            anchors.fill: parent
            anchors.margins: 2
            radius: 8
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#40ffffff" }
                GradientStop { position: 0.5; color: "#00ffffff" }
            }
        }
    }

    // 鼠标悬停光标
    hoverEnabled: true

    // 点击动画
    scale: pressed ? 0.95 : 1.0
    Behavior on scale {
        NumberAnimation { duration: 100 }
    }
}
