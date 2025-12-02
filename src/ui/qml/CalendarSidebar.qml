import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#f5f5f5"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 迷你日曆
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 200
            color: "white"
            radius: 8

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 4

                // 月份導航
                RowLayout {
                    Layout.fillWidth: true

                    ToolButton {
                        text: "<"
                        font.bold: true
                        onClicked: {
                            var d = new Date(miniCalendarDate)
                            d.setMonth(d.getMonth() - 1)
                            miniCalendarDate = d
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        text: miniCalendarDate.getFullYear() + "年" + (miniCalendarDate.getMonth() + 1) + "月"
                        horizontalAlignment: Text.AlignHCenter
                        font.weight: Font.Medium
                    }

                    ToolButton {
                        text: ">"
                        font.bold: true
                        onClicked: {
                            var d = new Date(miniCalendarDate)
                            d.setMonth(d.getMonth() + 1)
                            miniCalendarDate = d
                        }
                    }
                }

                // 星期標題
                Row {
                    Layout.fillWidth: true
                    spacing: 0

                    Repeater {
                        model: ["日", "一", "二", "三", "四", "五", "六"]

                        Label {
                            width: parent.width / 7
                            text: modelData
                            horizontalAlignment: Text.AlignHCenter
                            font.pixelSize: 10
                            color: index === 0 || index === 6 ? "#e91e63" : "#999999"
                        }
                    }
                }

                // 日期格子
                Grid {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    columns: 7
                    rows: 6

                    Repeater {
                        model: 42

                        Rectangle {
                            width: parent.width / 7
                            height: parent.height / 6
                            color: isToday(index) ? "#1976D2" : "transparent"
                            radius: width / 2

                            property date cellDate: getMiniCellDate(index)
                            property bool isCurrentMonth: cellDate.getMonth() === miniCalendarDate.getMonth()

                            Label {
                                anchors.centerIn: parent
                                text: cellDate.getDate()
                                font.pixelSize: 11
                                color: {
                                    if (isToday(index)) return "white"
                                    if (!isCurrentMonth) return "#cccccc"
                                    return "#333333"
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    // 點擊日期時的處理
                                }
                            }
                        }
                    }
                }
            }
        }

        // 分隔線
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 16
            color: "transparent"
        }

        // 行事曆列表標題
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 32
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8

                Label {
                    text: qsTr("行事曆")
                    font.weight: Font.Medium
                    font.pixelSize: 12
                    color: "#666666"
                }

                Item { Layout.fillWidth: true }

                ToolButton {
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                    
                    Label {
                        anchors.centerIn: parent
                        text: "+"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#666666"
                    }
                }
            }
        }

        // 行事曆列表
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: calendarList
                anchors.fill: parent
                model: calendarListModel
                spacing: 2

                delegate: Rectangle {
                    width: ListView.view.width
                    height: 36
                    color: mouseArea.containsMouse ? "#eeeeee" : "transparent"
                    radius: 4

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        spacing: 8

                        // 勾選框
                        CheckBox {
                            checked: model.enabled
                            onCheckedChanged: model.enabled = checked
                        }

                        // 顏色指示器
                        Rectangle {
                            Layout.preferredWidth: 12
                            Layout.preferredHeight: 12
                            radius: 3
                            color: model.color
                        }

                        // 行事曆名稱
                        Label {
                            Layout.fillWidth: true
                            text: model.name
                            elide: Text.ElideRight
                            font.pixelSize: 13
                        }

                        // 平台圖示
                        Label {
                            text: getPlatformIcon(model.platform)
                            font.pixelSize: 12
                            color: "#999999"
                        }
                    }

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.RightButton

                        onClicked: function(mouse) {
                            if (mouse.button === Qt.RightButton) {
                                contextMenu.popup()
                            }
                        }
                    }

                    Menu {
                        id: contextMenu

                        MenuItem {
                            text: qsTr("同步")
                        }

                        MenuItem {
                            text: qsTr("設定")
                        }

                        MenuSeparator {}

                        MenuItem {
                            text: qsTr("移除")
                        }
                    }
                }
            }
        }
    }

    // 資料模型
    property date miniCalendarDate: new Date()

    ListModel {
        id: calendarListModel

        ListElement {
            name: "我的行事曆"
            platform: "Google"
            color: "#4285F4"
            enabled: true
        }

        ListElement {
            name: "工作"
            platform: "Outlook"
            color: "#0078D4"
            enabled: true
        }

        ListElement {
            name: "家庭"
            platform: "Apple"
            color: "#FF9500"
            enabled: true
        }
    }

    // 輔助函數
    function getMiniCellDate(index) {
        var firstDay = new Date(miniCalendarDate.getFullYear(), miniCalendarDate.getMonth(), 1)
        var startDay = firstDay.getDay()
        var cellDate = new Date(firstDay)
        cellDate.setDate(1 - startDay + index)
        return cellDate
    }

    function isToday(index) {
        var cellDate = getMiniCellDate(index)
        var today = new Date()
        return cellDate.getDate() === today.getDate() &&
               cellDate.getMonth() === today.getMonth() &&
               cellDate.getFullYear() === today.getFullYear()
    }

    function getPlatformIcon(platform) {
        switch (platform) {
            case "Google": return "G"
            case "Apple": return "🍎"
            case "Outlook": return "O"
            default: return "?"
        }
    }
}
