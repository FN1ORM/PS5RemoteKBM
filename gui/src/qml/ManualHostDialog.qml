import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import org.streetpea.chiaking

import "controls" as C

DialogView {
    title: qsTr("Add Manual Console")
    buttonText: qsTr("Add")
    buttonEnabled: hostField.text.trim() && (consoleCombo.model[consoleCombo.currentIndex].index != -1)
    onAccepted: {
        Chiaki.addManualHost(consoleCombo.model[consoleCombo.currentIndex].index, hostField.text.trim());
        close();
    }

    Item {
        ScrollView {
            id: formScroll
            contentHeight: formGrid.implicitHeight + 40
            anchors.fill: parent
            clip: true
            contentWidth: availableWidth
        GridLayout {
            id: formGrid
            height: implicitHeight
            width: Math.max(0, Math.min(720, formScroll.availableWidth - 32))
            anchors {
                top: parent.top
                horizontalCenter: parent.horizontalCenter
                topMargin: 20
            }
            columns: width < 500 ? 1 : 2
            rowSpacing: 20
            columnSpacing: 20

            Label {
                Layout.alignment: formGrid.columns === 1 ? Qt.AlignLeft : Qt.AlignRight
                text: qsTr("Host:")
            }

            C.TextField {
                id: hostField
                echoMode: Chiaki.settings.streamerMode ? TextInput.Password : TextInput.Normal
                Layout.fillWidth: true
                Layout.preferredWidth: 400
                firstInFocusChain: true
            }

            Label {
                Layout.alignment: formGrid.columns === 1 ? Qt.AlignLeft : Qt.AlignRight
                text: qsTr("Registered Consoles:")
            }

            C.ComboBox {
                id: consoleCombo
                Layout.fillWidth: true
                Layout.preferredWidth: 400
                lastInFocusChain: true
                textRole: "name"
                model: {
                    let m = [];
                    if(Chiaki.settings.registeredHosts.length > 0)
                    {
                        m.push({
                            name: qsTr("Select an Option"),
                            index: -1,
                        });
                    }
                    let i = 0;
                    for (; i < Chiaki.settings.registeredHosts.length; ++i) {
                        let host = Chiaki.settings.registeredHosts[i];
                        m.push({
                            name: "%1 (%2)".arg(Chiaki.settings.streamerMode ? "hidden" : host.mac).arg(host.name),
                            index: i,
                        });
                    }
                    m.push({
                        name: qsTr("Register on first Connection"),
                        index: i,
                    });
                    return m;
                }
            }
        }
        }
    }
}
