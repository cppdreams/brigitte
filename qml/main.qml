import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 2.2

Window {
    id: root

    visible: true
    width: 1200
    height: 800
    title: qsTr("briGITte")

    readonly property int rowHeight: 20

    Settings {
        id: settings
    }

    Rectangle {
        id: background

        anchors { fill: parent }
        color: settings.backgroundColor
    }

    Column {
        anchors.fill: parent

        //        Item {
        //            id: topWrapper

        //            width: parent.width
        //            height: 300

        //            ScrollBar {
        //                id: hbar
        //                anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        //                active: hovered || pressed
        //                hoverEnabled: true
        //                orientation: Qt.Horizontal

        //                // commits_view.contentY is in [0, commits_view.contentHeight - commits_view.height]
        //                // position is in [0, 1. - size]
        //                // Do linear interpolation between the two
        //                position: (commits_view.contentHeight - commits_view.height - commits_view.contentY)
        //                          / (commits_view.contentHeight - commits_view.height) * (1. - size)
        //                size: Math.max(canvas.width / commits_view.contentHeight, 0.02)

        //                onPositionChanged: {
        //                    // position also changes if user scrolls commits_view, only set commits_view position
        //                    // if the user used this scrollbar
        //                    if (pressed){
        //                        commits_view.contentY = commits_view.contentHeight - commits_view.height
        //                                - position / (1. - size) * (commits_view.contentHeight - commits_view.height);
        //                    }
        //                }
        //            }

        Row {
            width:  parent.width
            height: parent.height - 30

            GitTree {
                id: canvas

                width: 300
                height: parent.height
                horizontal: false
                rowHeight: root.rowHeight

                Column {
                    id: buttonscol

                    anchors { right: parent.right; bottom: parent.bottom; bottomMargin: 15 }
                    width: 100
                    height: 45
                    spacing: 5

                    Button {
                        text: "Branch/Merge"
                        width: parent.width
                        height: 20
                        onClicked: {
                            cpp_commits.filterOnBranching();
                            searchfield.clear();
                        }
                    }
                    Button {
                        text: "All"
                        width: parent.width
                        height: 20
                        onClicked: {
                            cpp_commits.resetFilter();
                            searchfield.clear();
                        }
                    }                    
                }
            }

            ListView {
                id: commits_view

                width: parent.width - 300
                height: parent.height
                clip: true
                focus: true
                // No phone-style overshooting
                boundsBehavior: Flickable.StopAtBounds

                ScrollBar.vertical: ScrollBar {}

                onContentYChanged: {
                    canvas.requestPaint();
                }

                model:    cpp_commits
                delegate: commit

                Keys.onPressed: {
                    if (event.key === Qt.Key_F5) {
                        cpp_allcommits.refresh();
                        event.accepted = true;
                    }
                }
            }
        }

        Component {
            id: commit

            Rectangle {
                color: selected ? Qt.lighter(settings.backgroundColor) : 'transparent'

                width: parent.width
                height: root.rowHeight

                MouseArea {
                    id: delegate_area
                    anchors { fill: parent }
                    hoverEnabled: true

                    onContainsMouseChanged: {
                        cpp_commits.setSelected(index, containsMouse);
                    }

                    Row {
                        anchors { fill: parent; leftMargin: 5; rightMargin: 5 }

                        spacing: 5

                        Text {
                            text: message.split('\n')[0]
                            font.pointSize: settings.mediumFontSize
                            width: parent.width - 370 - 15
                            height: parent.height
                            leftPadding: 2
                            clip: true
                            color: settings.fontColor
                            verticalAlignment: Text.AlignVCenter
                        }

                        Text {
                            text: author
                            width: 180
                            height: parent.height
                            clip: true
                            color: settings.fontColor
                            verticalAlignment: Text.AlignVCenter
                            font.pointSize: settings.mediumFontSize
                        }

                        Text {
                            text: time.toLocaleString(Qt.locale)
                            width: 130
                            height: parent.height
                            color: settings.fontColor
                            verticalAlignment: Text.AlignVCenter
                            font.pointSize: settings.mediumFontSize
                        }

                        // TextInput to allow selecting
                        TextInput {
                            text: shortsha
                            width: 60
                            height: parent.height

                            readOnly: true
                            selectByMouse: true

                            color: settings.fontColor
                            verticalAlignment: Text.AlignVCenter
                            font.family: settings.monoFontFamily
                            font.pointSize: settings.mediumFontSize
                        }
                    }
                }
            }
        }

        Rectangle {
            id: statusbar
            width: parent.width
            height: 30
            color: 'black'

            Row {
                anchors { fill: parent; leftMargin: 5; rightMargin: 5 }
                spacing: 5

                TextField {
                    id: searchfield

                    function clear(){
                        text = "";
                    }

                    placeholderText: "<Search>"
                    width: 295
                    height: parent.height - 4
                    anchors { verticalCenter: parent.verticalCenter }

                    onTextChanged: {
                        cpp_commits.search(text);
                    }
                }

                Text {
                    text: commits_view.count + " commits"

                    height: parent.height
                    font.pointSize: settings.mediumFontSize
                    color: settings.fontColor
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }

    ProjectList {
        anchors.fill: parent
    }
}
