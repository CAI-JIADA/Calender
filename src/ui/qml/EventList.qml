import QtQuick
import QtQuick.Controls

ListView {
    id: root

    property var events: []

    model: events
    spacing: 4

    delegate: Rectangle {
        width: ListView.view.width
        height: 64
        color: mouseArea.containsMouse ? "#f5f5f5" : "white"
        radius: 8

        RowLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

            // 顏色條
            Rectangle {
                Layout.preferredWidth: 4
                Layout.fillHeight: true
                radius: 2
                color: modelData.color || "#1976D2"
            }

            // 時間
            Column {
                Layout.preferredWidth: 60
                spacing: 2

                Label {
                    text: formatTime(modelData.startTime)
                    font.pixelSize: 13
                    font.weight: Font.Medium
                }

                Label {
                    text: formatTime(modelData.endTime)
                    font.pixelSize: 11
                    color: "#666666"
                }
            }

            // 事件資訊
            Column {
                Layout.fillWidth: true
                spacing: 4

                Label {
                    width: parent.width
                    text: modelData.title
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    elide: Text.ElideRight
                }

                Row {
                    spacing: 8

                    // 地點
                    Row {
                        spacing: 4
                        visible: modelData.location && modelData.location.length > 0

                        Label {
                            text: "📍"
                            font.pixelSize: 11
                        }

                        Label {
                            text: modelData.location
                            font.pixelSize: 11
                            color: "#666666"
                        }
                    }

                    // 平台標誌
                    Rectangle {
                        width: 16
                        height: 16
                        radius: 4
                        color: getPlatformColor(modelData.platform)

                        Label {
                            anchors.centerIn: parent
                            text: getPlatformIcon(modelData.platform)
                            font.pixelSize: 9
                            color: "white"
                        }
                    }
                }
            }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                // 選擇事件
                window.selectedEvent = modelData
            }
        }
    }

    // 空狀態
    Label {
        anchors.centerIn: parent
        visible: events.length === 0
        text: qsTr("沒有事件")
        font.pixelSize: 14
        color: "#999999"
    }

    // 輔助函數
    function formatTime(dateTime) {
        if (!dateTime) return "--:--"
        return dateTime.toLocaleTimeString(Qt.locale(), { hour: '2-digit', minute: '2-digit' })
    }

    function getPlatformIcon(platform) {
        switch (platform) {
            case "Google": return "G"
            case "Apple": return "A"
            case "Outlook": return "O"
            default: return "?"
        }
    }

    function getPlatformColor(platform) {
        switch (platform) {
            case "Google": return "#4285F4"
            case "Apple": return "#FF9500"
            case "Outlook": return "#0078D4"
            default: return "#9e9e9e"
        }
    }
}
