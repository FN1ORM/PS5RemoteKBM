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
    title: qsTr("Register console")
    header: qsTr("Pair KeyStream with a PlayStation user")
    property bool detailsVisible: false
    buttonText: showingResult ? (registering ? qsTr("Registering…") : registered ? qsTr("Done") : qsTr("Try again")) : qsTr("Register")
    backEnabled: !registering
    buttonEnabled: showingResult ? !registering : hostField.text.trim() && pin.acceptableInput && cpin.acceptableInput
        && (ps4_7.checked ? onlineId.text.trim() : accountId.text.trim())
    StackView.onActivated: {
        if (!accountId.text && Chiaki.settings.psnAccountId)
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
        detailsVisible = false
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
                text: qsTr("1. Console type")
            }

            ColumnLayout {
                Layout.fillWidth: true
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
                    text: qsTr("PS5")
                    checked: ps5
                }
            }

            Label {
                Layout.alignment: formGrid.columns === 1 ? Qt.AlignLeft : Qt.AlignRight
                text: qsTr("Console address")
            }

            C.TextField {
                id: hostField
                echoMode: Chiaki.settings.streamerMode ? TextInput.Password : TextInput.Normal
                Layout.fillWidth: true
                Layout.preferredWidth: 400
            }

            Label {
                Layout.alignment: formGrid.columns === 1 ? Qt.AlignLeft : Qt.AlignRight
                text: qsTr("2. PSN Online ID")
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
                text: qsTr("2. PSN Account ID")
                visible: accountId.visible
            }

            C.TextField {
                id: accountId
                visible: !ps4_7.checked
                echoMode: Chiaki.settings.streamerMode ? TextInput.Password : TextInput.Normal
                placeholderText: qsTr("base64")
                Layout.fillWidth: true
                Layout.preferredWidth: 400
                KeyNavigation.priority: {
                    if(readOnly)
                        KeyNavigation.BeforeItem
                    else
                        KeyNavigation.AfterItem
                }
                KeyNavigation.up: hostField
                KeyNavigation.right: loginButton
                KeyNavigation.down: pin

            }

            Flow {
                Layout.columnSpan: formGrid.columns
                Layout.fillWidth: true
                visible: accountId.visible
                spacing: 6
                C.Button {
                    id: loginButton
                    text: qsTr("PSN Login")
                    onClicked: stack.push(psnLoginDialogComponent, {login: true, callback: (id) => accountId.text = id})
                }
                C.Button {
                    text: qsTr("Public Lookup")
                    onClicked: stack.push(psnLoginDialogComponent, {login: false, callback: (id) => accountId.text = id})
                }
            }

            Label {
                Layout.columnSpan: formGrid.columns
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                color: "#adbbb3"
                text: qsTr("On the console, sign in as the user you want to pair. Open Settings → System → Remote Play → Pair Device for a fresh code.")
            }

            Label {
                Layout.alignment: formGrid.columns === 1 ? Qt.AlignLeft : Qt.AlignRight
                text: qsTr("3. Remote Play PIN")
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
                text: qsTr("Console login PIN (optional)")
            }

            C.TextField {
                id: cpin
                echoMode: TextInput.Password
                validator: RegularExpressionValidator { regularExpression: /^$|[0-9]{4}/ }
                Layout.fillWidth: true
                Layout.preferredWidth: 400
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

                Button {
                    visible: registration.registrationLog.length > 0
                    text: registration.detailsVisible ? qsTr("Hide details") : qsTr("Details")
                    onClicked: registration.detailsVisible = !registration.detailsVisible
                }

                Label {
                    Layout.fillWidth: true
                    visible: registration.detailsVisible
                    wrapMode: Text.Wrap
                    textFormat: Text.PlainText
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
