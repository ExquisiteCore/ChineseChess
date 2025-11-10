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

    property string currentGameMode: "single"  // 保存当前游戏模式

    Component {
        id: menuPage
        MenuPage {
            onStartSinglePlayer: {
                console.log("Starting single player mode")
                currentGameMode = "single"
                stackView.push(gamePageComponent)
            }
            onStartTwoPlayer: {
                console.log("Starting two player mode")
                currentGameMode = "two"
                stackView.push(gamePageComponent)
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
            // 从Window的属性读取游戏模式
            gameMode: currentGameMode

            onBackToMenu: {
                stackView.pop()
            }
        }
    }
}
