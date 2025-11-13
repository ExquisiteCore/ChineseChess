import QtQuick
import QtQuick.Controls

Window {
    width: 1024
    height: 768
    visible: true
    title: i18n.tr("game_title")

    color: "#f5e6d3"

    // 全局翻译管理器
    TranslationManager {
        id: i18n
    }

    // 全局设置管理器
    AppSettings {
        id: appSettings
    }

    // 全局音效管理器
    SoundManager {
        id: globalSoundManager
        soundEnabled: appSettings.soundEnabled
        volume: appSettings.soundVolume
    }

    // 组件完成后加载保存的语言设置
    Component.onCompleted: {
        i18n.setLanguage(appSettings.language)
    }

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
                stackView.push(settingsPageComponent)
            }
            onExitGame: {
                Qt.quit()
            }
        }
    }

    Component {
        id: settingsPageComponent
        SettingsPage {
            onBackToMenu: {
                stackView.pop()
            }
            onSettingsChanged: function(volume, soundEnabled, language) {
                // 更新全局音效设置
                globalSoundManager.volume = volume
                globalSoundManager.soundEnabled = soundEnabled

                // 更新翻译
                i18n.setLanguage(language)

                console.log("设置已应用: 音量=" + volume + ", 音效启用=" + soundEnabled + ", 语言=" + language)
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
