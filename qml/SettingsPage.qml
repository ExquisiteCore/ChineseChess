import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: settingsPage
    color: "#f5e6d3"

    signal backToMenu()
    signal settingsChanged(real volume, bool soundEnabled, string language)

    // 临时设置（应用前）
    property real tempVolume: appSettings.soundVolume
    property bool tempSoundEnabled: appSettings.soundEnabled
    property string tempLanguage: appSettings.language

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.6
        spacing: 30

        // 标题
        Text {
            text: i18n.tr("settings_title")
            font.pixelSize: 48
            font.bold: true
            color: "#8b4513"
            Layout.alignment: Qt.AlignHCenter
        }

        // 设置面板
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 400
            color: "white"
            radius: 10
            border.color: "#ddd"
            border.width: 2

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 30
                spacing: 25

                // 音效启用开关
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 20

                    Text {
                        text: i18n.tr("sound_enabled")
                        font.pixelSize: 20
                        color: "#333"
                        Layout.preferredWidth: 150
                    }

                    Switch {
                        id: soundEnabledSwitch
                        checked: tempSoundEnabled
                        onCheckedChanged: {
                            tempSoundEnabled = checked
                        }
                    }

                    Item { Layout.fillWidth: true }
                }

                // 音量控制
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 20
                    enabled: tempSoundEnabled

                    Text {
                        text: i18n.tr("sound_volume")
                        font.pixelSize: 20
                        color: tempSoundEnabled ? "#333" : "#999"
                        Layout.preferredWidth: 150
                    }

                    Slider {
                        id: volumeSlider
                        Layout.fillWidth: true
                        from: 0
                        to: 1
                        value: tempVolume
                        onValueChanged: {
                            tempVolume = value
                        }

                        background: Rectangle {
                            x: volumeSlider.leftPadding
                            y: volumeSlider.topPadding + volumeSlider.availableHeight / 2 - height / 2
                            implicitWidth: 200
                            implicitHeight: 4
                            width: volumeSlider.availableWidth
                            height: implicitHeight
                            radius: 2
                            color: "#e0e0e0"

                            Rectangle {
                                width: volumeSlider.visualPosition * parent.width
                                height: parent.height
                                color: tempSoundEnabled ? "#ff9800" : "#ccc"
                                radius: 2
                            }
                        }

                        handle: Rectangle {
                            x: volumeSlider.leftPadding + volumeSlider.visualPosition * (volumeSlider.availableWidth - width)
                            y: volumeSlider.topPadding + volumeSlider.availableHeight / 2 - height / 2
                            implicitWidth: 20
                            implicitHeight: 20
                            radius: 10
                            color: volumeSlider.pressed ? "#f57c00" : "#ff9800"
                            border.color: tempSoundEnabled ? "#e65100" : "#999"
                            border.width: 2
                        }
                    }

                    Text {
                        text: Math.round(tempVolume * 100) + "%"
                        font.pixelSize: 18
                        color: tempSoundEnabled ? "#666" : "#999"
                        Layout.preferredWidth: 60
                    }
                }

                // 分隔线
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#ddd"
                }

                // 语言选择
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 20

                    Text {
                        text: i18n.tr("language")
                        font.pixelSize: 20
                        color: "#333"
                        Layout.preferredWidth: 150
                    }

                    ComboBox {
                        id: languageComboBox
                        Layout.fillWidth: true
                        model: [
                            { text: i18n.tr("chinese"), value: "zh_CN" },
                            { text: i18n.tr("english"), value: "en_US" }
                        ]
                        textRole: "text"

                        Component.onCompleted: {
                            currentIndex = tempLanguage === "zh_CN" ? 0 : 1
                        }

                        onActivated: {
                            tempLanguage = model[currentIndex].value
                        }

                        delegate: ItemDelegate {
                            width: languageComboBox.width
                            contentItem: Text {
                                text: modelData.text
                                color: "#333"
                                font.pixelSize: 16
                                verticalAlignment: Text.AlignVCenter
                            }
                            highlighted: languageComboBox.highlightedIndex === index
                        }

                        contentItem: Text {
                            leftPadding: 10
                            rightPadding: languageComboBox.indicator.width + languageComboBox.spacing
                            text: languageComboBox.displayText
                            font.pixelSize: 18
                            color: "#333"
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            implicitWidth: 120
                            implicitHeight: 40
                            border.color: languageComboBox.pressed ? "#ff9800" : "#ccc"
                            border.width: 2
                            radius: 5
                        }

                        popup: Popup {
                            y: languageComboBox.height - 1
                            width: languageComboBox.width
                            implicitHeight: contentItem.implicitHeight
                            padding: 1

                            contentItem: ListView {
                                clip: true
                                implicitHeight: contentHeight
                                model: languageComboBox.popup.visible ? languageComboBox.delegateModel : null
                                currentIndex: languageComboBox.highlightedIndex

                                ScrollIndicator.vertical: ScrollIndicator { }
                            }

                            background: Rectangle {
                                border.color: "#ccc"
                                radius: 2
                            }
                        }
                    }

                    Item { Layout.fillWidth: true }
                }

                Item { Layout.fillHeight: true }
            }
        }

        // 按钮行
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 20

            Button {
                text: i18n.tr("apply")
                Layout.preferredWidth: 150
                Layout.preferredHeight: 50

                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 20
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    color: parent.pressed ? "#388e3c" : (parent.hovered ? "#4caf50" : "#43a047")
                    radius: 8
                    border.color: "#2e7d32"
                    border.width: 2
                }

                onClicked: {
                    // 保存设置
                    appSettings.soundVolume = tempVolume
                    appSettings.soundEnabled = tempSoundEnabled
                    appSettings.language = tempLanguage

                    // 发射信号通知设置已更改
                    settingsChanged(tempVolume, tempSoundEnabled, tempLanguage)

                    // 返回菜单
                    backToMenu()
                }
            }

            Button {
                text: i18n.tr("cancel")
                Layout.preferredWidth: 150
                Layout.preferredHeight: 50

                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 20
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    color: parent.pressed ? "#c62828" : (parent.hovered ? "#e53935" : "#d32f2f")
                    radius: 8
                    border.color: "#b71c1c"
                    border.width: 2
                }

                onClicked: {
                    // 重置临时设置
                    tempVolume = appSettings.soundVolume
                    tempSoundEnabled = appSettings.soundEnabled
                    tempLanguage = appSettings.language

                    // 返回菜单
                    backToMenu()
                }
            }
        }
    }
}
