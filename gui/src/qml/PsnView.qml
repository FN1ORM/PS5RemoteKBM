import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

import org.streetpea.chiaking

Rectangle {
    id: view
    property bool allowClose: false
    property bool cancelling: false
    property bool textVisible: true
    property bool registOnly: false
    property list<Item> restoreFocusItems
    color: "black"


    function stop() {
        if (!allowClose)
            return;
        allowClose = false;
        cancelling = true;
        infoLabel.text = qsTr("Cancelling connection with console over PSN ...");
        Chiaki.psnCancel(false);
    }

    function grabInput(item) {
        Chiaki.window.grabInput();
        restoreFocusItems.push(Window.window.activeFocusItem);
        if (item)
            item.forceActiveFocus(Qt.TabFocusReason);
    }

    function releaseInput() {
        Chiaki.window.releaseInput();
        let item = restoreFocusItems.pop();
        if (item && item.visible)
            item.forceActiveFocus(Qt.TabFocusReason);
    }

    Keys.onEscapePressed: view.stop()

    Shortcut {
        sequence: "Ctrl+Q"
        onActivated: view.stop()
    }

    MouseArea {
        anchors.fill: parent
        enabled: view.allowClose
        acceptedButtons: Qt.RightButton
        onClicked: view.stop()
    }

    function finishFailure() {
        view.releaseInput();
        root.showMainView();
    }

    function showFailure(message) {
        if (cancelling) { root.showMainView(); return; }
        errorTitleLabel.text = qsTr("Connection could not be established");
        errorTextLabel.text = message + qsTr("\n\nYour saved registration is unchanged. If PSN traversal keeps failing, connect Tailscale on both devices and use the saved PS5 IP entry. Open F4 for diagnostics or F3 for the manual.");
        failTimer.stop();
        spinner.running = false;
    }

    ScrollView {
        id: progressScroll
        contentHeight: progressColumn.implicitHeight
        anchors { fill: parent; margins: 24 }
        contentWidth: availableWidth
        clip: true
        ColumnLayout {
            id: progressColumn
            height: implicitHeight
            width: progressScroll.availableWidth
            spacing: 20
            Label {
                id: infoLabel
                Layout.fillWidth: true
                visible: textVisible
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 18
                text: qsTr("Establishing connection with console over PSN ...")
            }
            BusyIndicator {
                id: spinner
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 48
                Layout.preferredHeight: 48
                visible: !errorTitleLabel.text
            }
            Label {
                id: closeMessageLabel
                Layout.fillWidth: true
                visible: textVisible && !cancelling
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("Press Escape or right-click to cancel the PSN attempt.")
            }
            Label {
                id: errorTitleLabel
                Layout.fillWidth: true
                font.pixelSize: 20
                font.bold: true
                wrapMode: Text.Wrap
                visible: text.length > 0
                onVisibleChanged: if (visible) { textVisible = false; view.allowClose = false; view.grabInput(errorTitleLabel); }
                Keys.onReturnPressed: view.finishFailure()
                Keys.onEscapePressed: view.finishFailure()
            }
            Label {
                id: errorTextLabel
                Layout.fillWidth: true
                font.pixelSize: 16
                wrapMode: Text.Wrap
                textFormat: Text.PlainText
            }
            Button {
                visible: errorTitleLabel.visible
                text: qsTr("Return to consoles")
                onClicked: view.finishFailure()
            }
        }
    }

    Timer {
        id: closeTimer
        interval: 1500
        running: true
        onTriggered: {
            view.allowClose = !errorTitleLabel.text
        }
    }

    Timer {
        id: failTimer
        interval: 2000
        running: false
        onTriggered: root.showMainView()
    }

    BoundedDialog {
        id: sessionPinDialog
        parent: Overlay.overlay
        title: qsTr("Console Login PIN")
        modal: true
        closePolicy: Popup.NoAutoClose
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAboutToShow: {
            standardButton(Dialog.Ok).enabled = Qt.binding(function() {
                return pinField.acceptableInput;
            });
            textVisible = false
            view.grabInput(pinField);
        }
        onClosed: {
            textVisible = true
            view.releaseInput()
        }
        onAccepted: Chiaki.enterPin(pinField.text)
        onRejected: Chiaki.stopSession(false)
        Material.roundedScale: Material.MediumScale

        TextField {
            id: pinField
            echoMode: Chiaki.settings.streamerMode ? TextInput.Password : TextInput.Normal
            implicitWidth: 200
            validator: RegularExpressionValidator { regularExpression: /[0-9]{4}/ }
            Keys.onReturnPressed: {
                if(sessionPinDialog.standardButton(Dialog.Ok).enabled)
                    sessionPinDialog.standardButton(Dialog.Ok).clicked()
            }
        }
    }

    Connections {
        target: Chiaki

        function onConnectStateChanged() 
        {
            switch(Chiaki.connectState)
            {
                case Chiaki.PsnConnectState.LinkingConsole:
                    infoLabel.text = registOnly ? qsTr("Registering PlayStation console with chiaki-ng ...") : qsTr("Linking chiaki-ng with PlayStation console ...")
                    view.allowClose = false
                    break
                case Chiaki.PsnConnectState.RegisteringConsole:
                    view.registOnly = true;
                    break
                case Chiaki.PsnConnectState.RegistrationFinished:
                    infoLabel.text = qsTr("Successfully registered console")
                    failTimer.restart()
                    break
                case Chiaki.PsnConnectState.DataConnectionStart:
                    infoLabel.text = qsTr("Console Linked ... Establishing data connection with console over PSN ...")
                    view.allowClose = true
                    break
                case Chiaki.PsnConnectState.DataConnectionFinished:
                    view.allowClose = false
                    break
                case Chiaki.PsnConnectState.ConnectFailed:
                    if(!cancelling)
                        infoLabel.text = qsTr("Connection over PSN failed closing ...")
                    view.showFailure(infoLabel.text)
                    break
                case Chiaki.PsnConnectState.ConnectFailedStart:
                    if(!cancelling)
                        infoLabel.text = qsTr("PSN couldn't establish connection with PlayStation. Please try again ...")
                    view.showFailure(infoLabel.text)
                    break
                case Chiaki.PsnConnectState.ConnectFailedConsoleUnreachable:
                    if(!cancelling)
                        infoLabel.text = qsTr("Couldn't contact PlayStation over established connection, likely unsupported network type")
                    view.showFailure(infoLabel.text)
                    break
                case Chiaki.PsnConnectState.WaitingForInternet:
                    infoLabel.text = qsTr("Establishing Internet Connection to PSN...")
                    break
            }
        }

        function onSessionChanged() 
        {
            if (!Chiaki.session) {
                if (!errorTitleLabel.text && !failTimer.running)
                    root.showMainView();
            }
        }

        function onSessionError(title, text)
        {
            errorTitleLabel.text = title;
            errorTextLabel.text = text;
            closeTimer.start();
        }

        function onSessionPinDialogRequested() 
        {
            if (sessionPinDialog.opened)
                return;
            sessionPinDialog.open();
        }
    }
}
