import QtQuick
import org.kde.kirigami as Kirigami

Rectangle {
    id: root

    readonly property var colorPalettes: [
        ["#6366F1", "#EC4899"],
        ["#06B6D4", "#3B82F6"],
        ["#F59E0B", "#EF4444"],
        ["#22C55E", "#06B6D4"],
        ["#A855F7", "#EC4899"],
        ["#38BDF8", "#818CF8"]
    ]

    property bool showIcon: true
    property real iconOpacity: 0.55
    property int seed: 0

    color: "transparent"
    clip: true

    Rectangle {
        id: baseGradient
        anchors.centerIn: parent
        width: parent.width * 1.9
        height: parent.height * 1.9
        gradient: Gradient {
            GradientStop {
                id: stopA
                position: 0.0
                color: root.colorPalettes[0][0]
            }
            GradientStop {
                id: stopC
                position: 0.5
                color: Qt.lighter(root.colorPalettes[0][0], 1.35)
            }
            GradientStop {
                id: stopB
                position: 1.0
                color: root.colorPalettes[0][1]
            }
        }
        NumberAnimation {
            target: baseGradient
            properties: "rotation"
            from: 0
            to: 360
            duration: 18000
            easing.type: Easing.Linear
            loops: Animation.Infinite
        }
    }

    function paletteIndexForSeed(s) {
        var n = root.colorPalettes.length
        return ((s % n) + n) % n
    }

    property int paletteIndex: 0

    onSeedChanged: {
        if (root.colorPalettes.length === 0)
            return
        root.paletteIndex = root.paletteIndexForSeed(root.seed)
        stopA.color = root.colorPalettes[root.paletteIndex][0]
        stopC.color = Qt.lighter(root.colorPalettes[root.paletteIndex][0], 1.35)
        stopB.color = root.colorPalettes[root.paletteIndex][1]
        paletteTimer.restart()
    }

    Timer {
        id: paletteTimer
        interval: 5000
        repeat: true
        triggeredOnStart: false
        running: true
        onTriggered: {
            root.paletteIndex = (root.paletteIndex + 1) % root.colorPalettes.length
            animA.to = root.colorPalettes[root.paletteIndex][0]
            animC.to = Qt.lighter(root.colorPalettes[root.paletteIndex][0], 1.35)
            animB.to = root.colorPalettes[root.paletteIndex][1]
            animA.restart()
            animC.restart()
            animB.restart()
        }
    }

    Timer {
        interval: 300 + (((root.seed % 5) + 5) % 5) * 900
        triggeredOnStart: false
        running: true
        onTriggered: {
            root.paletteIndex = root.paletteIndexForSeed(root.seed)
            stopA.color = root.colorPalettes[root.paletteIndex][0]
            stopC.color = Qt.lighter(root.colorPalettes[root.paletteIndex][0], 1.35)
            stopB.color = root.colorPalettes[root.paletteIndex][1]
            paletteTimer.restart()
        }
    }

    ColorAnimation {
        id: animA
        target: stopA
        duration: 3500
        easing.type: Easing.InOutSine
    }

    ColorAnimation {
        id: animC
        target: stopC
        duration: 3500
        easing.type: Easing.InOutSine
    }

    ColorAnimation {
        id: animB
        target: stopB
        duration: 3500
        easing.type: Easing.InOutSine
    }

    property real timeSec: 0
    Timer {
        interval: 33
        repeat: true
        running: true
        onTriggered: root.timeSec += 0.033
    }

    Image {
        id: orb1
        width: root.width * 1.2
        height: root.width * 1.2
        x: root.width * (0.40 + 0.35 * Math.sin(2 * Math.PI * root.timeSec / 9 + root.seed))
        y: root.height * (0.45 + 0.35 * Math.sin(2 * Math.PI * root.timeSec / 12 + 1.7 * root.seed))
        source: "glow.svg"
        smooth: true
        opacity: 0.45
    }

    Image {
        id: orb2
        width: root.width * 0.9
        height: root.width * 0.9
        x: root.width * (0.40 + 0.35 * Math.sin(2 * Math.PI * root.timeSec / 13 + 2.1 * root.seed + 3))
        y: root.height * (0.40 + 0.35 * Math.sin(2 * Math.PI * root.timeSec / 10 + 0.6 * root.seed + 1))
        source: "glow.svg"
        smooth: true
        opacity: 0.35
    }

    Kirigami.Icon {
        anchors.centerIn: parent
        width: Math.min(root.width, root.height) * 0.3
        height: width
        source: "audio-x-generic"
        color: "white"
        opacity: root.iconOpacity
        visible: root.showIcon
    }
}
