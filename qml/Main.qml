import QtQuick
import QtQuick.Controls

Window {
    width: 1024
    height: 768
    visible: true
    title: qsTr("中国象棋")

    color: "#f5e6d3"

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: menuPage
    }

    Component {
        id: menuPage
        MenuPage {
            onStartSinglePlayer: {
                stackView.push(gamePageComponent, { "gameMode": "single" })
            }
            onStartTwoPlayer: {
                stackView.push(gamePageComponent, { "gameMode": "two" })
            }
            onShowSettings: {
                // 后续实现设置页面
                console.log("显示设置")
            }
            onExitGame: {
                Qt.quit()
            }
        }
    }

    Component {
        id: gamePageComponent
        GamePage {
            property string gameMode: "two"

            onBackToMenu: {
                stackView.pop()
            }
        }
    }
}
