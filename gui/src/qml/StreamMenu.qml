import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import org.streetpea.chiaking

Rectangle {
    id: menu
    signal dismissed()
    signal displayRequested()
    signal rendererRequested()

    function focusMenu() { done.forceActiveFocus(Qt.TabFocusReason); }

    color: "#202725"
    border.color: "#485851"
    radius: 10
    Material.theme: Material.Dark
    Material.accent: "#78dfb6"
    Material.primary: "#202725"
    Material.background: "#151918"
    Material.foreground: "#edf5f0"

    ColumnLayout {
        anchors { fill: parent; margins: 14 }
        spacing: 6

        RowLayout {
            Layout.fillWidth: true
            Label {
                Layout.fillWidth: true
                font.pixelSize: 19
                font.bold: true
                text: qsTr("Stream menu")
            }
            Button {
                id: done
                objectName: "streamMenuDone"
                text: qsTr("Done")
                onClicked: menu.dismissed()
            }
        }

        ScrollView {
            id: scroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: availableWidth
            contentHeight: body.implicitHeight
            Keys.onEscapePressed: menu.dismissed()
            Keys.onMenuPressed: menu.dismissed()

            ColumnLayout {
                id: body
                width: scroll.availableWidth
                spacing: 10

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    color: "#adbbb3"
                    text: qsTr("Ctrl+O opens or closes this menu. F1 captures the keyboard and mouse. Esc releases them.")
                }

                Flow {
                    Layout.fillWidth: true
                    spacing: 6

                    Button {
                        enabled: !!Chiaki.session && Chiaki.session.connected
                        text: Chiaki.session && Chiaki.session.viewOnly ? qsTr("Resume control") : qsTr("Watch only")
                        onClicked: { Chiaki.window.toggleViewOnly(); menu.dismissed(); }
                    }
                    Button { text: qsTr("Display"); onClicked: menu.displayRequested() }
                    Button {
                        text: qsTr("Renderer")
                        visible: Chiaki.window.videoPreset === ChiakiWindow.VideoPreset.Custom
                        onClicked: menu.rendererRequested()
                    }
                    Button { text: qsTr("Controls · F2"); onClicked: { menu.dismissed(); Chiaki.window.showNativeControls(); } }
                    Button { text: qsTr("Connection · F4"); onClicked: { menu.dismissed(); Chiaki.window.showConnectionTools(); } }
                    Button { text: qsTr("Disconnect"); onClicked: Chiaki.window.close() }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: width < 480 ? 1 : 2
                    columnSpacing: 16

                    Label { text: qsTr("Video fit") }
                    ComboBox {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        model: [qsTr("Fit"), qsTr("Zoom"), qsTr("Stretch")]
                        currentIndex: Chiaki.window.videoMode === ChiakiWindow.VideoMode.Zoom ? 1
                                    : Chiaki.window.videoMode === ChiakiWindow.VideoMode.Stretch ? 2 : 0
                        onActivated: index => Chiaki.window.videoMode = [ChiakiWindow.VideoMode.Normal,
                            ChiakiWindow.VideoMode.Zoom, ChiakiWindow.VideoMode.Stretch][index]
                    }

                    Label { text: qsTr("Zoom amount"); visible: zoom.active }
                    Loader {
                        id: zoom
                        Layout.fillWidth: true
                        active: Chiaki.window.videoMode === ChiakiWindow.VideoMode.Zoom
                        visible: active
                        sourceComponent: Slider {
                            from: -1
                            to: 4
                            stepSize: 0.01
                            value: Chiaki.window.ZoomFactor
                            onMoved: {
                                Chiaki.window.ZoomFactor = value;
                                Chiaki.settings.sZoomFactor = value;
                            }
                        }
                    }

                    Label { text: qsTr("Video processing") }
                    ComboBox {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        readonly property var presets: [ChiakiWindow.VideoPreset.Default, ChiakiWindow.VideoPreset.HighQuality,
                            ChiakiWindow.VideoPreset.HighQualitySpatial, ChiakiWindow.VideoPreset.HighQualityAdvancedSpatial,
                            ChiakiWindow.VideoPreset.Custom]
                        model: [qsTr("Default"), qsTr("High quality"), qsTr("High quality + spatial"),
                            qsTr("High quality + advanced spatial"), qsTr("Custom")]
                        currentIndex: presets.indexOf(Chiaki.window.videoPreset)
                        onActivated: index => {
                            Chiaki.window.videoPreset = presets[index];
                            Chiaki.settings.videoPreset = presets[index];
                        }
                    }

                    Label { text: qsTr("Volume") }
                    Slider {
                        Layout.fillWidth: true
                        from: 0
                        to: 128
                        stepSize: 1
                        value: Chiaki.settings.audioVolume
                        onMoved: Chiaki.settings.audioVolume = value
                    }
                }

                Switch {
                    text: qsTr("Microphone")
                    enabled: !!Chiaki.session && Chiaki.session.connected && !Chiaki.session.viewOnly
                    checked: !!Chiaki.session && !Chiaki.session.muted
                    onToggled: if (Chiaki.session) Chiaki.session.muted = !checked
                }

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Stream quality")
                    font.bold: true
                }

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    color: "#adbbb3"
                    text: {
                        if (!Chiaki.session || !Chiaki.session.connected)
                            return qsTr("Waiting for the console");
                        const stream = Chiaki.session;
                        const loss = isFinite(stream.averagePacketLoss) ? (stream.averagePacketLoss * 100).toFixed(1) + "%" : "—";
                        const bitrate = stream.measuredBitrate > 0 ? stream.measuredBitrate.toFixed(1) + " Mbps" : "—";
                        const rtt = stream.initialRoundTripMs >= 0 ? stream.initialRoundTripMs.toFixed(0) + " ms" : "—";
                        return qsTr("Received %1  ·  Packet loss %2  ·  Dropped frames %3  ·  Initial round trip %4")
                            .arg(bitrate).arg(loss).arg(Chiaki.window.droppedFrames).arg(rtt);
                    }
                }
                Switch {
                    text: qsTr("Show stream stats during play")
                    checked: Chiaki.settings.showStreamStats
                    onToggled: Chiaki.settings.showStreamStats = checked
                }
            }
        }
    }

    Keys.onEscapePressed: dismissed()
    Keys.onMenuPressed: dismissed()
}
