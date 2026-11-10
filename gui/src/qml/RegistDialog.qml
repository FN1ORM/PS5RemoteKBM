import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import org.streetpea.chiaking

import "controls" as C

DialogView {
    id: registration
    property bool ps5: true
    property alias host: hostField.text
    property bool showingResult: false
    property bool registering: false
    property bool registered: false
    property bool searchTimedOut: false
    property string registrationLog: ""
    title: qsTr("Register Console")
    buttonText: showingResult ? (registering ? qsTr("Registering…") : registered ? qsTr("Done") : qsTr("Try again")) : qsTr("Register")
    backEnabled: !registering
    buttonEnabled: showingResult ? !registering : hostField.text.trim() && pin.acceptableInput && cpin.acceptableInput
        && (ps4_7.checked ? onlineId.text.trim() : accountId.text.trim())
    StackView.onActivated: {
        if(Chiaki.settings.psnAccountId)
            accountId.text = Chiaki.settings.psnAccountId
    }
    onAccepted: {
        if (!buttonEnabled)
            return
        if (showingResult) {
            if (registered) {
                close()
            } else {
                showingResult = false
                pin.forceActiveFocus(Qt.TabFocusReason)
            }
            return
        }

        const address = hostField.text.trim()
        const psnId = ps4_7.checked ? onlineId.text.trim() : accountId.text.trim()
        registrationLog = ""
        searchTimedOut = false
        registered = false
        showingResult = true
        registering = true
        resultScroll.contentItem.contentY = 0
        const started = Chiaki.registerHost(address, psnId, pin.text.trim(), cpin.text.trim(),
                                           address === "255.255.255.255", consoleButtons.checkedButton.target,
                                           function(msg, ok, done) {
            if (!done) {
                registrationLog += msg + "\n"
                if (msg.includes("Regist timed out waiting for search response"))
                    searchTimedOut = true
                return
            }
            registered = ok
            registering = false
        })
        if (started) {
            pin.clear()
            resultScroll.forceActiveFocus(Qt.TabFocusReason)
        } else {
            registering = false
            showingResult = false
        }
    }

    Item {
        ScrollView {
            id: formScroll
            visible: !registration.showingResult
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
            rowSpacing: 10
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
                text: qsTr("PSN Online-ID:")
                visible: onlineId.visible
            }

            C.TextField {
                id: onlineId
                echoMode: Chiaki.settings.streamerMode ? TextInput.Password : TextInput.Normal
                visible: ps4_7.checked
                placeholderText: qsTr("username, case-sensitive")
                Layout.fillWidth: true
                Layout.preferredWidth: 400
            }

            Label {
                Layout.alignment: formGrid.columns === 1 ? Qt.AlignLeft : Qt.AlignRight
                text: qsTr("PSN Account-ID:")
                visible: accountId.visible
            }

            C.TextField {
                id: accountId
                visible: !ps4_7.checked
                echoMode: Chiaki.settings.streamerMode ? TextInput.Password : TextInput.Normal
                placeholderText: qsTr("base64")
                Layout.fillWidth: true
                Layout.preferredWidth: 400 - loginButton.width - 10
                KeyNavigation.priority: {
                    if(readOnly)
                        KeyNavigation.BeforeItem
                    else
                        KeyNavigation.AfterItem
                }
                KeyNavigation.up: hostField
                KeyNavigation.right: loginButton
                KeyNavigation.down: pin

                C.Button {
                    id: loginButton
                    anchors {
                        left: parent.right
                        verticalCenter: parent.verticalCenter
                        leftMargin: 10
                    }
                    topPadding: 18
                    bottomPadding: 18
                    text: qsTr("PSN Login")
                    onClicked: stack.push(psnLoginDialogComponent, {login: true, callback: (id) => accountId.text = id})
                    visible: !Chiaki.settings.psnAccountId
                    Material.roundedScale: Material.SmallScale
                    KeyNavigation.priority: KeyNavigation.BeforeItem
                    KeyNavigation.up: hostField
                    KeyNavigation.left: accountId
                    KeyNavigation.right: lookupButton
                    KeyNavigation.down: pin
                }
                C.Button {
                    id: lookupButton
                    anchors {
                        left: loginButton.right
                        verticalCenter: parent.verticalCenter
                        leftMargin: 10
                    }
                    topPadding: 18
                    bottomPadding: 18
                    text: qsTr("Public Lookup")
                    onClicked: stack.push(psnLoginDialogComponent, {login: false, callback: (id) => accountId.text = id})
                    visible: !Chiaki.settings.psnAccountId
                    Material.roundedScale: Material.SmallScale
                    KeyNavigation.up: hostField
                    KeyNavigation.priority: KeyNavigation.BeforeItem
                    KeyNavigation.left: loginButton
                    KeyNavigation.right: lookupButton
                    KeyNavigation.down: pin
                }
            }

            Label {
                Layout.alignment: formGrid.columns === 1 ? Qt.AlignLeft : Qt.AlignRight
                text: qsTr("Remote Play PIN:")
            }

            C.TextField {
                id: pin
                objectName: "remotePlayPin"
                validator: RegularExpressionValidator { regularExpression: /[0-9]{8}/ }
                Layout.fillWidth: true
                Layout.preferredWidth: 400
                echoMode: TextInput.Password
                KeyNavigation.priority: {
                    if(readOnly)
                        KeyNavigation.BeforeItem
                    else
                        KeyNavigation.AfterItem
                }
                KeyNavigation.up: accountId
            }

            Label {
                Layout.alignment: formGrid.columns === 1 ? Qt.AlignLeft : Qt.AlignRight
                text: qsTr("Console Pin [Optional]")
            }

            C.TextField {
                id: cpin
                echoMode: TextInput.Password
                validator: RegularExpressionValidator { regularExpression: /^$|[0-9]{4}/ }
                Layout.fillWidth: true
                Layout.preferredWidth: 400
            }

            Label {
                Layout.alignment: formGrid.columns === 1 ? Qt.AlignLeft : Qt.AlignRight
                text: qsTr("Console:")
            }

            ColumnLayout {
                spacing: 0

                C.RadioButton {
                    id: ps4_7
                    property int target: 800
                    text: qsTr("PS4 Firmware < 7.0")
                }

                C.RadioButton {
                    id: ps4_75
                    property int target: 900
                    text: qsTr("PS4 Firmware >= 7.0, < 8.0")
                }

                C.RadioButton {
                    id: ps4_8
                    property int target: 1000
                    text: qsTr("PS4 Firmware >= 8.0")
                    checked: !ps5
                }

                C.RadioButton {
                    id: ps5_0
                    property int target: 1000100
                    lastInFocusChain: true
                    text: qsTr("PS5")
                    checked: ps5
                }
            }
        }
        }

        ButtonGroup {
            id: consoleButtons
            buttons: [ps4_7, ps4_75, ps4_8, ps5_0]
        }

        ScrollView {
            id: resultScroll
            objectName: "registrationResult"
            visible: registration.showingResult
            anchors.fill: parent
            anchors.bottomMargin: 56
            clip: true
            contentWidth: availableWidth
            contentHeight: resultColumn.implicitHeight + 48
            Keys.onReturnPressed: if (!registration.registering) registration.accepted()

            ColumnLayout {
                id: resultColumn
                x: (resultScroll.availableWidth - width) / 2
                y: 24
                width: Math.max(0, Math.min(720, resultScroll.availableWidth - 32))
                spacing: 14

                Label {
                    Layout.fillWidth: true
                    font.pixelSize: 22
                    font.bold: true
                    wrapMode: Text.Wrap
                    text: registration.registering ? qsTr("Looking for the console…")
                        : registration.registered ? qsTr("Console registered") : qsTr("Registration failed")
                }

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    text: registration.registering ? qsTr("Keep the Pair Device screen open on the console.")
                        : registration.registered ? qsTr("Select Done to return to the app.")
                        : registration.searchTimedOut
                            ? qsTr("The PS5 did not answer the pairing search. Check its current IP address and that the phone is connected to the same home network. Then open Pair Device again and use the fresh PIN.")
                            : qsTr("Check the details below and use a fresh pairing PIN when you retry.")
                }

                Label {
                    Layout.fillWidth: true
                    visible: text.length > 0
                    wrapMode: Text.Wrap
                    textFormat: Text.PlainText
                    font.pixelSize: 16
                    text: registration.registrationLog
                }
            }
        }

        Component {
            id: psnLoginDialogComponent
            PSNLoginDialog { }
        }
    }
}
