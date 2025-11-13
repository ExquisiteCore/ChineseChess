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
            captureSound.stop()
            captureSound.position = 0
            captureSound.play()
            break
        case SoundManager.SoundType.Check:
            checkSound.stop()
            checkSound.position = 0
            checkSound.play()
            break
        case SoundManager.SoundType.Checkmate:
            checkmateSound.stop()
            checkmateSound.position = 0
            checkmateSound.play()
            break
        case SoundManager.SoundType.Move:
        default:
            moveSound.stop()
            moveSound.position = 0
            moveSound.play()
            break
        }
    }

    Component.onCompleted: {
        console.log("=== SoundManager 已加载 ===")
    }

    // 吃子音效
    MediaPlayer {
        id: captureSound
        source: "file:resources/sounds/吃.wav"
        audioOutput: AudioOutput {
            id: captureOutput
            volume: root.volume
        }
        onErrorOccurred: function(error, errorString) {
            console.log("captureSound 错误:", error, errorString)
        }
        onPlaybackStateChanged: {
            if (playbackState === MediaPlayer.StoppedState && position > 0) {
                stop()
                position = 0
            }
        }
    }

    // 将军音效
    MediaPlayer {
        id: checkSound
        source: "file:resources/sounds/将军.wav"
        audioOutput: AudioOutput {
            volume: root.volume
        }
        onErrorOccurred: function(error, errorString) {
            console.log("checkSound 错误:", error, errorString)
        }
        onPlaybackStateChanged: {
            if (playbackState === MediaPlayer.StoppedState && position > 0) {
                stop()
                position = 0
            }
        }
    }

    // 绝杀音效
    MediaPlayer {
        id: checkmateSound
        source: "file:resources/sounds/绝杀.wav"
        audioOutput: AudioOutput {
            volume: root.volume
        }
        onErrorOccurred: function(error, errorString) {
            console.log("checkmateSound 错误:", error, errorString)
        }
        onPlaybackStateChanged: {
            if (playbackState === MediaPlayer.StoppedState && position > 0) {
                stop()
                position = 0
            }
        }
    }

    // 普通移动音效（使用落子音效）
    MediaPlayer {
        id: moveSound
        source: "file:resources/sounds/落子.wav"
        audioOutput: AudioOutput {
            volume: root.volume
        }
        onErrorOccurred: function(error, errorString) {
            console.log("moveSound 错误:", error, errorString)
        }
        onPlaybackStateChanged: {
            if (playbackState === MediaPlayer.StoppedState && position > 0) {
                stop()
                position = 0
            }
        }
    }
}

