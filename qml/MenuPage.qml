import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: menuPage
    color: "#f5e6d3"

    signal startSinglePlayer()
    signal startTwoPlayer()
    signal showSettings()
    signal exitGame()

    // 背景装饰
    Image {
        anchors.fill: parent
        source: "qrc:/images/menu_bg.png"
        fillMode: Image.PreserveAspectCrop
        opacity: 0.3
        // 如果没有图片，使用渐变背景
        visible: false
    }

    // 渐变背景
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#d4a574" }
            GradientStop { position: 0.5; color: "#f5e6d3" }
            GradientStop { position: 1.0; color: "#c9a67a" }
        }
        opacity: 0.5
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 30

        // 游戏标题
        Item {
            Layout.alignment: Qt.AlignHCenter
            width: titleText.width
            height: titleText.height

            // 阴影层
            Text {
                text: "中国象棋"
                font.pixelSize: 72
                font.bold: true
                font.family: "SimHei"
                color: "#40000000"
                anchors.centerIn: parent
                anchors.horizontalCenterOffset: 3
                anchors.verticalCenterOffset: 3
            }

            // 主标题
            Text {
                id: titleText
                text: "中国象棋"
                font.pixelSize: 72
                font.bold: true
                font.family: "SimHei"
                color: "#8b0000"

                style: Text.Outline
                styleColor: "#ffd700"
            }
        }

        // 副标题
        Text {
            text: "Chinese Chess"
            font.pixelSize: 24
            font.family: "Arial"
            color: "#654321"
            Layout.alignment: Qt.AlignHCenter
            opacity: 0.8
        }

        // 间隔
        Item { height: 20 }

        // 菜单按钮列
        ColumnLayout {
            spacing: 15
            Layout.alignment: Qt.AlignHCenter

            MenuButton {
                text: "单人对战"
                Layout.preferredWidth: 250
                Layout.preferredHeight: 60
                onClicked: menuPage.startSinglePlayer()
            }

            MenuButton {
                text: "双人对战"
                Layout.preferredWidth: 250
                Layout.preferredHeight: 60
                onClicked: menuPage.startTwoPlayer()
            }

            MenuButton {
                text: "设置"
                Layout.preferredWidth: 250
                Layout.preferredHeight: 60
                onClicked: menuPage.showSettings()
            }

            MenuButton {
                text: "退出游戏"
                Layout.preferredWidth: 250
                Layout.preferredHeight: 60
                buttonColor: "#b22222"
                hoverColor: "#dc143c"
                onClicked: menuPage.exitGame()
            }
        }
    }

    // 版本信息
    Text {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 20
        text: "Version 0.1"
        font.pixelSize: 14
        color: "#8b7355"
        opacity: 0.7
    }
}
