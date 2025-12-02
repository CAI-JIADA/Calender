import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 1200
    height: 800
    visible: true
    title: qsTr("行事曆整合工具")

    // 屬性
    property var selectedEvent: null
    property date currentDate: new Date()
    property string viewMode: "month" // "month", "week", "day"

    // 顏色主題
    readonly property color primaryColor: "#1976D2"
    readonly property color secondaryColor: "#424242"
    readonly property color backgroundColor: "#FAFAFA"
    readonly property color surfaceColor: "#FFFFFF"

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // 側邊欄 - 行事曆列表
        Rectangle {
            Layout.preferredWidth: 250
            Layout.fillHeight: true
            color: "#f5f5f5"

            CalendarSidebar {
                id: sidebar
                anchors.fill: parent
                anchors.margins: 8
            }
        }

        // 分隔線
        Rectangle {
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            color: "#e0e0e0"
        }

        // 主內容區
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // 搜索列
            SearchBar {
                id: searchBar
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                onSearch: function(query) {
                    // 呼叫搜索引擎
                    console.log("Searching:", query)
                }
            }

            // 分隔線
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#e0e0e0"
            }

            // 行事曆視圖
            CalendarView {
                id: calendarView
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentDate: window.currentDate
                viewMode: window.viewMode
            }
        }

        // 分隔線
        Rectangle {
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            color: "#e0e0e0"
            visible: selectedEvent !== null
        }

        // 右側欄 - 事件詳情
        EventDetailPanel {
            id: eventDetail
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            visible: selectedEvent !== null
            event: selectedEvent
        }
    }

    // 底部狀態列
    footer: ToolBar {
        height: 32
        
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16

            Label {
                id: statusLabel
                text: qsTr("就緒")
                color: "#666666"
            }

            Item { Layout.fillWidth: true }

            // 同步狀態
            Row {
                spacing: 8
                visible: typeof syncManager !== 'undefined'

                BusyIndicator {
                    width: 16
                    height: 16
                    running: typeof syncManager !== 'undefined' && syncManager.isSyncing
                }

                Label {
                    text: {
                        if (typeof syncManager === 'undefined') return ""
                        if (syncManager.isSyncing) return qsTr("正在同步...")
                        if (syncManager.lastSyncTime)
                            return qsTr("上次同步: ") + Qt.formatDateTime(syncManager.lastSyncTime, "HH:mm")
                        return ""
                    }
                    color: "#666666"
                    font.pixelSize: 12
                }
            }
        }
    }

    // 快捷鍵
    Shortcut {
        sequence: "Ctrl+T"
        onActivated: {
            currentDate = new Date()
            viewMode = "day"
        }
    }

    Shortcut {
        sequence: "Ctrl+M"
        onActivated: viewMode = "month"
    }

    Shortcut {
        sequence: "Ctrl+W"
        onActivated: viewMode = "week"
    }

    Shortcut {
        sequence: "Ctrl+D"
        onActivated: viewMode = "day"
    }

    Shortcut {
        sequence: "Ctrl+F"
        onActivated: searchBar.focus = true
    }
}
