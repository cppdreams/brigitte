import QtQuick 2.7

Item {
    readonly property color backgroundColor: '#1e1e1e'
    readonly property color fontColor: '#bdbdbd'
    readonly property color grayedFontColor: '#999999'

    // Have to check how to add fallback fonts:
    readonly property string monoFontFamily: "DejaVu Sans Mono"

    readonly property int largeFontSize: 11
    readonly property int mediumFontSize: 9
    // 'Courier New', monospace, 'Droid Sans Fallback'
//    readonly property string monoFontFamily: 'Droid Sans Mono'
//    readonly property string monoFontFamily: "Ubuntu Mono"
//    readonly property string monoFontFamily: "Bitstream Vera Sans Mono"
//    readonly property string monoFontFamily: "FreeMono"

//    Component.onCompleted: {
//        console.info(Qt.fontFamilies())
//    }
}
