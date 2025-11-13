import QtQuick
import Qt.labs.settings

Settings {
    id: appSettings

    // 音效设置
    property real soundVolume: 0.7
    property bool soundEnabled: true

    // 语言设置
    property string language: "zh_CN"  // zh_CN 或 en_US

    // 其他设置可以在这里添加
    // property int aiDifficulty: 1  // 0=Easy, 1=Medium, 2=Hard, 3=Expert
}
