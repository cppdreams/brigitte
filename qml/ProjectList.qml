import QtQuick 2.7

Item {
    id: root

    ListView {
        anchors { fill: parent; margins: 5 }

        model: cpp_projects
        delegate: project_delegate
    }

    Component {
        id: project_delegate

        MouseArea {
            id: mouse_area

            width:  parent.width
            height: 40

            hoverEnabled: true

            Rectangle {
                id: background
                anchors { fill: parent }
                color: mouse_area.containsMouse ? Qt.lighter(settings.backgroundColor)
                                                : 'transparent'
            }

            Row {
                anchors { fill: parent; leftMargin: 5; rightMargin: 5 }
                spacing: 5

                Text {
                    text:  projectname
                    width: 100
                    height: parent.height
                    verticalAlignment: Text.AlignVCenter
                    color: settings.fontColor
                }

                Text {
                    text:  path
                    width: parent.width - 100 - 15
                    height: parent.height
                    verticalAlignment: Text.AlignVCenter
                    color: settings.fontColor
                }
            }

            onPressed: {
                cpp_allcommits.loadFrom(path, max_commits);
                // Temporary hack!
                root.visible = false;
            }
        }
    }
}
