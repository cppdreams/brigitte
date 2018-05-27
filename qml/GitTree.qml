import QtQuick 2.7

Canvas {
    id: root

    antialiasing: true

    property bool horizontal: true
    property int rowHeight: 20

    readonly property int circleWidth: 8
    readonly property var colors: ["orange", "red", "purple", "yellow", "brown", "lightgreen"]
    readonly property int branchColWidth: 15

    Connections {
        target: cpp_commits
        onDataChanged: {
            if (roles[0] === cpp_commits.selectionRole){
                root.requestPaint();
            }
        }

        onLayoutChanged: {
            root.requestPaint();
        }

        onModelReset: {
            root.requestPaint();
        }
    }

    function getColor(column){
        return colors[column % colors.length];
    }

    function getX(index){
        return (index + 0.5) * root.branchColWidth;
    }

    function getY(index){
        return (index + 0.5) * root.rowHeight;
    }

    function pointVisible(coords){
        return coords[0] >= 0 && coords[0] <= width
                && coords[1] >= 0 && coords[1] <= height ;
    }

    function getCenter(commitIndex, branchIndex){
        return [getX(branchIndex), getY(commitIndex)];
    }

    function getCommitIndex(pixvalue){
        return parseInt(pixvalue / root.rowHeight, 10);
    }

    function getBranchIndex(pixvalue){
        return parseInt(pixvalue / root.branchColWidth, 10);
    }

    function getCommitAndBranchIndex(point){
        if (root.horizontal){
            return [getCommitIndex(width - point[0] + commits_view.contentY), getBranchIndex(point[1])];
        } else {
            return [getCommitIndex(point[1] + commits_view.contentY), getBranchIndex(point[0])];
        }
    }

    function shiftToCircleStart(point){
        return [point[0], point[1] + 0.5*root.circleWidth];
    }

    function getCommitPosRange(){
        if (root.horizontal){
            return width;
        } else {
            return height;
        }
    }

    function getBezierCurvePoints(point, nextPoint){
        var firstControlPoint, secondControlPoint, endPoint;

        var wayPointY = point[1] + root.rowHeight - 0.5*root.circleWidth;
        firstControlPoint  = [point[0], wayPointY];
        secondControlPoint = [nextPoint[0], point[1]];
        endPoint = [nextPoint[0], wayPointY];

        return [firstControlPoint, shiftToCircleStart(secondControlPoint), endPoint];
    }

    function drawCommit(i, ctx){
        var branchColumn = cpp_commits.getActiveBranchIndex(i);
        var pointCoords = getCenter(i, branchColumn);

        var parents = cpp_commits.getParents(i);
        for (var iparent = 0; iparent < parents.length; ++iparent){
            var targetBranchColumn = cpp_commits.getActiveBranchIndex(parents[iparent]);
            var nextCoords = getCenter(parents[iparent], targetBranchColumn);

            ctx.beginPath();
            var startPoint = shiftToCircleStart(pointCoords);
            ctx.moveTo(startPoint[0], startPoint[1]);
            if (targetBranchColumn === branchColumn){
                var targetPoint = shiftToCircleStart(getCenter(parents[iparent], targetBranchColumn));
                ctx.lineTo(targetPoint[0], targetPoint[1]);
            } else {
                var wayPointX = pointCoords[0] - root.rowHeight + 0.5*root.circleWidth;
                // Bezier curve to next row
                var bezierPoints = getBezierCurvePoints(pointCoords, nextCoords)
                ctx.bezierCurveTo(bezierPoints[0][0], bezierPoints[0][1],
                                  bezierPoints[1][0], bezierPoints[1][1],
                                  bezierPoints[2][0], bezierPoints[2][1]);
                // Line to next item, if required
                if (parents[iparent] !== i+1){
                    var shiftedNext = shiftToCircleStart(nextCoords);
                    ctx.lineTo(shiftedNext[0] , shiftedNext[1]);
                }
            }

            ctx.lineWidth = 2.;
            ctx.strokeStyle = root.getColor(targetBranchColumn);
            ctx.stroke();
        }

        ctx.beginPath();
        ctx.ellipse(pointCoords[0] - 0.5 * root.circleWidth,
                    pointCoords[1] - 0.5 * root.circleWidth,
                    root.circleWidth, root.circleWidth);
        ctx.fillStyle = root.getColor(branchColumn);
        // Uncomment for filled circles
        ctx.fill();

        // Selection
        if (cpp_commits.isSelected(i)){
            ctx.lineWidth = 2.;
            ctx.strokeStyle = root.getColor(branchColumn);
            ctx.strokeStyle = "white";
            ctx.stroke();
        }

        return pointCoords;
    }

    onPaint: {
        var ctx = getContext("2d");
        ctx.resetTransform();

        ctx.clearRect(0, 0, width, height);

        if (root.horizontal){
            ctx.translate(root.width, 0.);
            ctx.rotate(Math.PI / 2.);
        }

        ctx.translate(0., -commits_view.contentY);

        var istart = Math.max(root.getCommitIndex(commits_view.contentY) - 1, 0);
        var iend   = Math.min(Math.max(root.getCommitIndex(commits_view.contentY + getCommitPosRange()) + 1,
                                       0),
                              commits_view.count);
        for (var i = istart; i < iend; ++i){
            // Lines from invisible children (later in the DAG) will not be drawn in the main loop,
            // draw them now
            var children = cpp_commits.getChildren(i);
            for (var ichild = 0; ichild < children.length; ++ichild){
                var childColumn = cpp_commits.getActiveBranchIndex(children[ichild]);
                var childCoords = getCenter(children[ichild], childColumn);

                if (! pointVisible(childCoords)){
                    drawCommit(children[ichild], ctx);
                }
            }

            drawCommit(i, ctx);
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
            var icoords = getCommitAndBranchIndex([mouse.x, mouse.y]);
            if (cpp_commits.getActiveBranchIndex(icoords[0]) === icoords[1]){
                cpp_commits.setSelected(icoords[0], true);

                if (icoords[0] !== lastCommitSelected){
                    deselectLast();
                }
                lastCommitSelected = icoords[0];
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

        onClicked: {
            var icoords = getCommitAndBranchIndex([mouse.x, mouse.y]);

            if (cpp_commits.getActiveBranchIndex(icoords[0]) === icoords[1]){
                cpp_commits.filterOnBranch(icoords[0]);
            }
        }
    }
}
