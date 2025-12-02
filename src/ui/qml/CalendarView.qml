import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property date currentDate: new Date()
    property string viewMode: "month" // "month", "week", "day"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 導航列
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 56
            color: "#1976D2"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 8

                // 今天按鈕
                Button {
                    text: qsTr("今天")
                    flat: true
                    font.pixelSize: 14
                    Material.foreground: "white"
                    onClicked: {
                        currentDate = new Date()
                    }
                }

                // 上一頁
                ToolButton {
                    icon.source: "qrc:/icons/chevron_left.svg"
                    icon.color: "white"
                    onClicked: navigatePrevious()
                    
                    Text {
                        anchors.centerIn: parent
                        text: "<"
                        color: "white"
                        font.pixelSize: 18
                        font.bold: true
                    }
                }

                // 下一頁
                ToolButton {
                    icon.source: "qrc:/icons/chevron_right.svg"
                    icon.color: "white"
                    onClicked: navigateNext()
                    
                    Text {
                        anchors.centerIn: parent
                        text: ">"
                        color: "white"
                        font.pixelSize: 18
                        font.bold: true
                    }
                }

                // 當前日期/月份標題
                Label {
                    text: getHeaderText()
                    font.pixelSize: 20
                    font.weight: Font.Medium
                    color: "white"
                    Layout.leftMargin: 16
                }

                Item { Layout.fillWidth: true }

                // 視圖切換
                Row {
                    spacing: 4

                    Button {
                        text: qsTr("月")
                        flat: true
                        checkable: true
                        checked: viewMode === "month"
                        Material.foreground: "white"
                        onClicked: viewMode = "month"
                    }

                    Button {
                        text: qsTr("週")
                        flat: true
                        checkable: true
                        checked: viewMode === "week"
                        Material.foreground: "white"
                        onClicked: viewMode = "week"
                    }

                    Button {
                        text: qsTr("日")
                        flat: true
                        checkable: true
                        checked: viewMode === "day"
                        Material.foreground: "white"
                        onClicked: viewMode = "day"
                    }
                }
            }
        }

        // 行事曆內容
        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: {
                switch (viewMode) {
                    case "month": return monthViewComponent
                    case "week": return weekViewComponent
                    case "day": return dayViewComponent
                    default: return monthViewComponent
                }
            }
        }
    }

    // 月視圖
    Component {
        id: monthViewComponent

        GridLayout {
            columns: 7
            rows: 7
            rowSpacing: 1
            columnSpacing: 1

            // 星期標題
            Repeater {
                model: [qsTr("日"), qsTr("一"), qsTr("二"), qsTr("三"), qsTr("四"), qsTr("五"), qsTr("六")]
                
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 32
                    color: "#f5f5f5"
                    
                    Label {
                        anchors.centerIn: parent
                        text: modelData
                        font.weight: Font.Medium
                        color: index === 0 || index === 6 ? "#e91e63" : "#333333"
                    }
                }
            }

            // 日期格子
            Repeater {
                model: 42

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: isToday(index) ? "#e3f2fd" : "white"
                    border.color: "#e0e0e0"
                    border.width: 1

                    property date cellDate: getCellDate(index)
                    property bool isCurrentMonth: cellDate.getMonth() === currentDate.getMonth()

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 4
                        spacing: 2

                        Label {
                            text: cellDate.getDate()
                            font.pixelSize: 14
                            font.weight: isToday(index) ? Font.Bold : Font.Normal
                            color: {
                                if (!isCurrentMonth) return "#9e9e9e"
                                if (isToday(index)) return "#1976D2"
                                return "#333333"
                            }
                        }

                        // 事件列表區域
                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            
                            // 這裡將顯示該日期的事件
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            currentDate = cellDate
                        }
                        onDoubleClicked: {
                            currentDate = cellDate
                            viewMode = "day"
                        }
                    }
                }
            }
        }
    }

    // 週視圖
    Component {
        id: weekViewComponent

        Rectangle {
            color: "white"

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // 日期標題列
                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 48
                    spacing: 0

                    // 時間軸空白
                    Rectangle {
                        Layout.preferredWidth: 60
                        Layout.fillHeight: true
                        color: "#f5f5f5"
                    }

                    Repeater {
                        model: 7

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: isWeekDayToday(index) ? "#e3f2fd" : "#f5f5f5"
                            border.color: "#e0e0e0"
                            border.width: 1

                            Column {
                                anchors.centerIn: parent
                                spacing: 2

                                Label {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    text: getWeekDayName(index)
                                    font.pixelSize: 12
                                    color: "#666666"
                                }

                                Label {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    text: getWeekDayDate(index)
                                    font.pixelSize: 18
                                    font.weight: isWeekDayToday(index) ? Font.Bold : Font.Normal
                                    color: isWeekDayToday(index) ? "#1976D2" : "#333333"
                                }
                            }
                        }
                    }
                }

                // 時間格子
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    GridLayout {
                        columns: 8
                        rows: 24
                        rowSpacing: 0
                        columnSpacing: 0

                        Repeater {
                            model: 24 * 8

                            Rectangle {
                                property int row: Math.floor(index / 8)
                                property int col: index % 8

                                Layout.preferredWidth: col === 0 ? 60 : -1
                                Layout.fillWidth: col !== 0
                                Layout.preferredHeight: 48
                                color: col === 0 ? "#f5f5f5" : "white"
                                border.color: "#e0e0e0"
                                border.width: col === 0 ? 0 : 1

                                Label {
                                    visible: col === 0
                                    anchors.right: parent.right
                                    anchors.rightMargin: 8
                                    anchors.top: parent.top
                                    text: row.toString().padStart(2, '0') + ":00"
                                    font.pixelSize: 11
                                    color: "#666666"
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // 日視圖
    Component {
        id: dayViewComponent

        Rectangle {
            color: "white"

            RowLayout {
                anchors.fill: parent
                spacing: 0

                // 時間軸
                Rectangle {
                    Layout.preferredWidth: 60
                    Layout.fillHeight: true
                    color: "#f5f5f5"

                    Column {
                        anchors.fill: parent

                        Repeater {
                            model: 24

                            Rectangle {
                                width: parent.width
                                height: 60
                                color: "transparent"

                                Label {
                                    anchors.right: parent.right
                                    anchors.rightMargin: 8
                                    anchors.top: parent.top
                                    text: index.toString().padStart(2, '0') + ":00"
                                    font.pixelSize: 11
                                    color: "#666666"
                                }
                            }
                        }
                    }
                }

                // 事件區域
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Rectangle {
                        width: parent.width
                        height: 24 * 60
                        color: "white"

                        // 時間線
                        Repeater {
                            model: 24

                            Rectangle {
                                x: 0
                                y: index * 60
                                width: parent.width
                                height: 1
                                color: "#e0e0e0"
                            }
                        }

                        // 當前時間指示器
                        Rectangle {
                            visible: isTodayDate(currentDate)
                            x: 0
                            y: getCurrentTimeY()
                            width: parent.width
                            height: 2
                            color: "#f44336"

                            Rectangle {
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                width: 10
                                height: 10
                                radius: 5
                                color: "#f44336"
                            }
                        }

                        // 事件將在這裡顯示
                    }
                }
            }
        }
    }

    // 輔助函數
    function getHeaderText() {
        var months = [
            qsTr("一月"), qsTr("二月"), qsTr("三月"), qsTr("四月"),
            qsTr("五月"), qsTr("六月"), qsTr("七月"), qsTr("八月"),
            qsTr("九月"), qsTr("十月"), qsTr("十一月"), qsTr("十二月")
        ]
        
        if (viewMode === "day") {
            return currentDate.getFullYear() + "年" + 
                   months[currentDate.getMonth()] + 
                   currentDate.getDate() + "日"
        }
        
        return currentDate.getFullYear() + "年" + months[currentDate.getMonth()]
    }

    function navigatePrevious() {
        var newDate = new Date(currentDate)
        switch (viewMode) {
            case "month":
                newDate.setMonth(newDate.getMonth() - 1)
                break
            case "week":
                newDate.setDate(newDate.getDate() - 7)
                break
            case "day":
                newDate.setDate(newDate.getDate() - 1)
                break
        }
        currentDate = newDate
    }

    function navigateNext() {
        var newDate = new Date(currentDate)
        switch (viewMode) {
            case "month":
                newDate.setMonth(newDate.getMonth() + 1)
                break
            case "week":
                newDate.setDate(newDate.getDate() + 7)
                break
            case "day":
                newDate.setDate(newDate.getDate() + 1)
                break
        }
        currentDate = newDate
    }

    function getCellDate(index) {
        var firstDay = new Date(currentDate.getFullYear(), currentDate.getMonth(), 1)
        var startDay = firstDay.getDay()
        var cellDate = new Date(firstDay)
        cellDate.setDate(1 - startDay + index)
        return cellDate
    }

    function isToday(index) {
        var cellDate = getCellDate(index)
        var today = new Date()
        return cellDate.getDate() === today.getDate() &&
               cellDate.getMonth() === today.getMonth() &&
               cellDate.getFullYear() === today.getFullYear()
    }

    function isTodayDate(date) {
        var today = new Date()
        return date.getDate() === today.getDate() &&
               date.getMonth() === today.getMonth() &&
               date.getFullYear() === today.getFullYear()
    }

    function getWeekDayName(index) {
        var days = [qsTr("日"), qsTr("一"), qsTr("二"), qsTr("三"), qsTr("四"), qsTr("五"), qsTr("六")]
        return days[index]
    }

    function getWeekDayDate(index) {
        var weekStart = new Date(currentDate)
        weekStart.setDate(currentDate.getDate() - currentDate.getDay() + index)
        return weekStart.getDate()
    }

    function isWeekDayToday(index) {
        var weekStart = new Date(currentDate)
        weekStart.setDate(currentDate.getDate() - currentDate.getDay() + index)
        return isTodayDate(weekStart)
    }

    function getCurrentTimeY() {
        var now = new Date()
        return now.getHours() * 60 + now.getMinutes()
    }
}
