import QtQuick 2.7
import QtQuick.Window 2.2

Window {
    id: root

    visible: true
    width: 1200
    height: 800
    title: qsTr("gitlook")

    readonly property int rowHeight: 20
    readonly property int branchColWidth: 15

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

        Canvas {
            id: canvas
            width:  parent.width
            height: 300
            antialiasing: true

            readonly property int circleWidth: 8
            readonly property var colors: ["orange", "red", "purple", "yellow", "brown", "lightgreen"]

            Connections {
                target: cpp_commits
                onDataChanged: {
                    if (roles[0] === cpp_commits.selectionRole){
                        canvas.requestPaint();
                    }
                }
            }

            function getColor(column){
                return colors[column % colors.length];
            }

            function getY(index){
                return (index + 0.5) * root.branchColWidth;
            }

            function getX(index){
                return width + commits_view.contentY - (index + 0.5) * root.rowHeight;
            }

            function getCenter(commitIndex, branchIndex){
                return [getX(commitIndex), getY(branchIndex)];
            }

            function pointVisible(coords){
                return coords[0] >= 0 && coords[0] <= width
                        && coords[1] >= 0 && coords[1] <= height;
            }

            function getCommitIndex(xvalue){
                return parseInt((width + commits_view.contentY - xvalue) / root.rowHeight, 10);
            }

            function getBranchIndex(yvalue){
                return parseInt(yvalue / root.branchColWidth, 10)
            }

            function drawCommit(i, ctx){
                var branchColumn = cpp_commits.getActiveBranchIndex(i);
                var pointCoords = getCenter(i, branchColumn);

                var parents = cpp_commits.getParents(i);
                for (var iparent = 0; iparent < parents.length; ++iparent){
                    var targetBranchColumn = cpp_commits.getActiveBranchIndex(parents[iparent]);
                    var nextCoords = getCenter(parents[iparent], targetBranchColumn);

                    ctx.beginPath();
                    ctx.moveTo(pointCoords[0] - 0.5*canvas.circleWidth, pointCoords[1]);
                    if (targetBranchColumn === branchColumn){
                        ctx.lineTo(getX(parents[iparent]) - 0.5*canvas.circleWidth, getY(targetBranchColumn));
                    } else {
                        var wayPointX = pointCoords[0] - root.rowHeight + 0.5*canvas.circleWidth;
                        // Bezier curve to next row
                        ctx.bezierCurveTo(wayPointX, pointCoords[1],
                                          pointCoords[0] - 0.5*canvas.circleWidth, nextCoords[1],
                                          wayPointX, nextCoords[1]);
                        // Line to next item, if required
                        if (parents[iparent] !== i+1){
                            ctx.lineTo(nextCoords[0] - 0.5*canvas.circleWidth, nextCoords[1]);
                        }
                    }

                    ctx.lineWidth = 2.;
                    ctx.strokeStyle = canvas.getColor(targetBranchColumn);
                    ctx.stroke();
                }

                ctx.beginPath();
                ctx.ellipse(pointCoords[0] - 0.5 * canvas.circleWidth,
                            pointCoords[1] - 0.5 * canvas.circleWidth,
                            canvas.circleWidth, canvas.circleWidth);
                ctx.fillStyle = canvas.getColor(branchColumn);
                // Uncomment for filled circles
                ctx.fill();

                // Selection
                if (cpp_commits.isSelected(i)){
                    ctx.lineWidth = 2.;
                    ctx.strokeStyle = canvas.getColor(branchColumn);
                    ctx.strokeStyle = "white";
                    ctx.stroke();
                }

                return pointCoords;
            }

            onPaint: {
                var ctx = getContext("2d");

                ctx.clearRect(0, 0, width, height);

                var istart = Math.max(getCommitIndex(width), 0);
                for (var i = istart; i < commits_view.count; ++i){
                    // Lines from invisible children (later in the DAG) will not be drawn in the main loop,
                    // draw them now
                    var children = cpp_commits.getChildren(i);
                    for (var ichild = 0; ichild < children.length; ++ichild){
                        var childColumn = cpp_commits.getActiveBranchIndex(children[ichild]);
                        var childCoords = getCenter(children[ichild], childColumn);

                        //                        console.info(childCoords);
                        if (! pointVisible(childCoords)){
                            drawCommit(children[ichild], ctx);
                        }
                    }

                    var pointCoords = drawCommit(i, ctx);

                    //                    break;

                    if (pointCoords[0] < 0){
                        break;
                    }
                }
            }

            MouseArea {
                id: canvasCapture

                anchors.fill: parent
                hoverEnabled: true

                property var lastCommitSelected;

                function deselectLast(){
                    if (lastCommitSelected >= 0){
                        cpp_commits.setSelected(lastCommitSelected, false);
                    }
                }

                onPositionChanged: {
                    var icommit = canvas.getCommitIndex(mouse.x);
                    if (cpp_commits.getActiveBranchIndex(icommit) === canvas.getBranchIndex(mouse.y)){
                        cpp_commits.setSelected(icommit, true);

                        if (icommit !== lastCommitSelected){
                            deselectLast();
                        }
                        lastCommitSelected = icommit;
                    } else {
                        deselectLast();
                        lastCommitSelected = -1;
                    }
                }

                onContainsMouseChanged: {
                    if (! containsMouse){
                        deselectLast();
                    }
                }
            }
        }

        //        Row {
        //            width:  parent.width
        //            height: parent.height - 300

        //            Canvas {
        //                id: canvas

        //                width: 300
        //                height: parent.height
        //                antialiasing: true

        //                function getY(row){
        //                    return (row + 0.5) * root.rowHeight - commits_view.contentY;
        //                }

        //                function getX(column){
        //                    return (column + 0.5) * root.branchColWidth;
        //                }

        //                function getCenter(row, column){
        //                    return [getX(column), getY(row)];
        //                }

        //                function pointVisible(coords){
        //                    return coords[0] >= 0 && coords[1] <= height;
        //                }

        //                onPaint: {
        //                    var ctx = getContext("2d");

        //                    ctx.clearRect(0, 0, canvas.width, canvas.height);

        //                    var istart = Math.max(parseInt((commits_view.contentY) / root.rowHeight - 0.5, 10), 0);
        //                    for (var i = istart; i < commits_view.count; ++i){
        //                        var branchColumn = cpp_commits.getActiveBranchIndex(i);
        //                        var pointCoords = getCenter(i, branchColumn);

        //                        var parents = cpp_commits.getParents(i);
        //                        for (var iparent = 0; iparent < parents.length; ++iparent){
        //                            var targetBranchColumn = cpp_commits.getActiveBranchIndex(parents[iparent]);
        //                            var nextCoords = getCenter(parents[iparent], targetBranchColumn);

        //                            ctx.beginPath();
        //                            ctx.moveTo(pointCoords[0], pointCoords[1] + 0.5*canvas.circleWidth);
        //                            if (targetBranchColumn === branchColumn){
        //                                ctx.lineTo(canvas.getX(targetBranchColumn), canvas.getY(parents[iparent]) - 0.5*canvas.circleWidth);
        //                            } else {
        //                                var wayPointY = pointCoords[1] + root.rowHeight - 0.5*canvas.circleWidth;
        //                                // Bezier curve to next row
        //                                ctx.bezierCurveTo(pointCoords[0], wayPointY,
        //                                                  nextCoords[0], pointCoords[1] + 0.5*canvas.circleWidth,
        //                                                  nextCoords[0], wayPointY);
        //                                // Line to next item, if required
        //                                if (parents[iparent] !== i+1){
        //                                    ctx.lineTo(nextCoords[0], nextCoords[1] - 0.5*canvas.circleWidth);
        //                                }
        //                            }

        //                            ctx.lineWidth = 2.;
        //                            ctx.strokeStyle = canvas.getColor(targetBranchColumn);
        //                            ctx.stroke();
        //                        }

        //                        ctx.beginPath();
        //                        ctx.ellipse(pointCoords[0] - 0.5 * canvas.circleWidth,
        //                                    pointCoords[1] - 0.5 * canvas.circleWidth,
        //                                    canvas.circleWidth, canvas.circleWidth);
        //                        ctx.fillStyle = canvas.getColor(branchColumn);
        //                        // Uncomment for filled circles
        //                        ctx.fill();

        //                        //                    ctx.lineWidth = 2.;
        //                        //                    ctx.strokeStyle = canvas.getColor(branchColumn);
        //                        //                    ctx.strokeStyle = "white";
        //                        //                    ctx.stroke();

        //                        // Stop as soon as we have a commit that is no longer visible
        //                        if (! pointVisible(pointCoords)){
        //                            break;
        //                        }
        //                    }
        //                }
        //            }

        ListView {
            id: commits_view

            width: parent.width
            height: parent.height

            onContentYChanged: {
                canvas.requestPaint();
            }

            model:    cpp_commits
            delegate: commit
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
                        anchors.fill: parent

                        Text {
                            text: message.split('\n')[0]
                            width: parent.width - 75
                            height: parent.height
                            clip: true
                            color: settings.fontColor
                            verticalAlignment: Text.AlignVCenter
                        }

                        Text {
                            text: shortsha
                            width: 75
                            height: parent.height
                            color: settings.fontColor
                            verticalAlignment: Text.AlignVCenter
                            font.family: settings.monoFontFamily
                        }
                    }
                }
            }
        }
    }
}
