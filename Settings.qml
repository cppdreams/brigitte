import QtQuick 2.7

Item {
    readonly property color backgroundColor: '#1e1e1e'
    readonly property color fontColor: '#bdbdbd'

    // Have to check how to add fallback fonts:
    readonly property string monoFontFamily: "DejaVu Sans Mono"
    // 'Courier New', monospace, 'Droid Sans Fallback'
//    readonly property string monoFontFamily: 'Droid Sans Mono'
//    readonly property string monoFontFamily: "Ubuntu Mono"
//    readonly property string monoFontFamily: "Bitstream Vera Sans Mono"
//    readonly property string monoFontFamily: "FreeMono"

//    Component.onCompleted: {
//        console.info(Qt.fontFamilies())
//    }
}
