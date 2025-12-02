import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "white"

    signal search(string query)

    property alias focus: searchField.focus

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        // 搜索圖示
        Rectangle {
            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            radius: 18
            color: searchField.activeFocus ? "#e3f2fd" : "transparent"

            Label {
                anchors.centerIn: parent
                text: "🔍"
                font.pixelSize: 18
            }
        }

        // 搜索輸入框
        TextField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: qsTr("搜索事件、任務...")
            font.pixelSize: 14
            background: Rectangle {
                color: "transparent"
            }

            onAccepted: {
                if (text.trim().length > 0) {
                    root.search(text.trim())
                }
            }
        }

        // 清除按鈕
        ToolButton {
            visible: searchField.text.length > 0
            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            
            Label {
                anchors.centerIn: parent
                text: "✕"
                font.pixelSize: 14
                color: "#666666"
            }

            onClicked: {
                searchField.text = ""
                searchField.focus = true
            }
        }

        // 篩選按鈕
        ToolButton {
            Layout.preferredWidth: 36
            Layout.preferredHeight: 36

            Label {
                anchors.centerIn: parent
                text: "⚙"
                font.pixelSize: 18
                color: "#666666"
            }

            onClicked: filterMenu.open()

            Menu {
                id: filterMenu
                y: parent.height

                MenuItem {
                    text: qsTr("所有平台")
                    checkable: true
                    checked: true
                }

                MenuSeparator {}

                MenuItem {
                    text: "Google Calendar"
                    checkable: true
                    checked: true
                }

                MenuItem {
                    text: "Apple Calendar"
                    checkable: true
                    checked: true
                }

                MenuItem {
                    text: "Microsoft Outlook"
                    checkable: true
                    checked: true
                }

                MenuSeparator {}

                MenuItem {
                    text: qsTr("日期範圍...")
                }
            }
        }
    }

    // 底部分隔線
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: "#e0e0e0"
    }
}
