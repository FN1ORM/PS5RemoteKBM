import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import org.streetpea.chiaking

import "controls" as C

DialogView {
    property var consoleIndex
    property var resolveIndex
    title: qsTr("Set console login PIN")
    buttonText: qsTr("Set")
    buttonEnabled: pin.acceptableInput
    onAccepted: {
        const index = resolveIndex ? resolveIndex() : consoleIndex;
        if (index >= 0) Chiaki.setConsolePin(index, pin.text.trim());
        stack.pop();
    }
    Item {
        GridLayout {
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                margins: 20
            }
            columns: width < 500 ? 1 : 2
            rowSpacing: 10
            columnSpacing: 20
                        Label {
                            Layout.alignment: Qt.AlignRight
                            text: qsTr("Console login PIN (4 digits)")
                        }

                        TextField {
                            id: pin
                            validator: RegularExpressionValidator { regularExpression: /[0-9]{4}/ }
                            echoMode: TextInput.Password
                            Layout.fillWidth: true
                        }
        }

    }
}