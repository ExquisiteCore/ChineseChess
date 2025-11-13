import QtQuick
import QtMultimedia

Item {
    id: root

    // 音效开关
    property bool soundEnabled: true

    // 音量控制 (0.0 - 1.0)
    property real volume: 0.7

    // 音效类型枚举
    enum SoundType {
        Move,      // 普通移动
        Capture,   // 吃子
        Check,     // 将军
        Checkmate  // 绝杀
    }

    // 播放音效的函数
    function playSound(soundType) {
        if (!soundEnabled) return

        switch(soundType) {
        case SoundManager.SoundType.Capture:
            captureSound.play()
            break
        case SoundManager.SoundType.Check:
            checkSound.play()
            break
        case SoundManager.SoundType.Checkmate:
            checkmateSound.play()
            break
        case SoundManager.SoundType.Move:
        default:
            // 移动音效使用吃子音效的低音量版本
            moveSound.play()
            break
        }
    }

    // 吃子音效
    SoundEffect {
        id: captureSound
        source: "qrc:/sounds/resources/sounds/吃.wav"
        volume: root.volume
    }

    // 将军音效
    SoundEffect {
        id: checkSound
        source: "qrc:/sounds/resources/sounds/将军.wav"
        volume: root.volume
    }

    // 绝杀音效
    SoundEffect {
        id: checkmateSound
        source: "qrc:/sounds/resources/sounds/绝杀.wav"
        volume: root.volume
    }

    // 普通移动音效（复用吃子音效但音量更低）
    SoundEffect {
        id: moveSound
        source: "qrc:/sounds/resources/sounds/吃.wav"
        volume: root.volume * 0.5
    }
}
