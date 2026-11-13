import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Window

import org.streetpea.chiaking

import "controls" as C

Item {
    id: view

    readonly property var hostWindow: view.Window.window
    property bool sessionError: false
    property bool sessionLoading: true
    property list<Item> restoreFocusItems
    readonly property bool useSeparateMenuWindow: Chiaki.window.runtimeRendererBackend === 1
    readonly property int streamMenuHeight: Math.max(1, Math.min(370, height - 24))
    readonly property bool streamStatsVisible: Chiaki.settings.showStreamStats && !!Chiaki.session && !menuController.open && !root.streamPanelOpen && !sessionLoading && !sessionError && !(Chiaki.settings.audioVideoDisabled & 0x02)
    property int separateMenuX: 0
    property int separateMenuY: 0
    property int separateMenuWidth: 0
    property int separateStatsX: 0
    property int separateStatsY: 0
    property int separateStatsWidth: 0
    property int separateStatsHeight: 0
    property bool sessionStopDialogActive: false
    property bool sessionPinDialogActive: false

    function grabInput(item) {
        Chiaki.window.grabInput();
        restoreFocusItems.push(hostWindow ? hostWindow.activeFocusItem : null);
        if (item)
            item.forceActiveFocus(Qt.TabFocusReason);
    }

    function releaseInput() {
        if (!restoreFocusItems.length)
            return;
        Chiaki.window.releaseInput();
        let item = restoreFocusItems.pop();
        if (item && item.visible)
            item.forceActiveFocus(Qt.TabFocusReason);
    }

    function updateSeparateMenuGeometry() {
        const topLeft = Qt.point(Chiaki.window.x, Chiaki.window.y);
        if (useSeparateMenuWindow) {
            separateMenuX = Math.round(topLeft.x + 12);
            separateMenuY = Math.round(topLeft.y + view.height - streamMenuHeight - 12);
            separateMenuWidth = Math.max(1, Math.round(view.width - 24));
            separateStatsX = Math.round(topLeft.x);
            separateStatsY = Math.round(topLeft.y);
            separateStatsWidth = Math.round(view.width);
            separateStatsHeight = Math.round(view.height);
        }
    }

    function updateOverlayInteractionActive() {
        Chiaki.window.setOverlayInteractionActive(
            menuController.open ||
            root.streamPanelOpen ||
            sessionStopDialogActive ||
            separateSessionStopWindow.visible ||
            sessionPinDialogActive ||
            separateSessionPinWindow.visible
        );
    }

    StackView.onActivating: {
        Chiaki.window.keepVideo = true;
        sessionError = false;
        errorTitleLabel.text = "";
        errorTextLabel.text = "";
        sessionLoading = !(Chiaki.window.loadingTransitionComplete || (Chiaki.settings.audioVideoDisabled & 0x02));
    }
    StackView.onDeactivated: {
        menuController.close(false);
        while (restoreFocusItems.length)
            releaseInput();
        Chiaki.window.keepVideo = false;
    }

    Component.onCompleted: {
        updateSeparateMenuGeometry();
        updateOverlayInteractionActive();
        Chiaki.window.setStatsOverlayActive(streamStatsVisible);
    }
    onStreamStatsVisibleChanged: {
        if (Chiaki.window)
            Chiaki.window.setStatsOverlayActive(streamStatsVisible);
    }
    onWidthChanged: updateSeparateMenuGeometry()
    onHeightChanged: updateSeparateMenuGeometry()
    onUseSeparateMenuWindowChanged: updateSeparateMenuGeometry()
    onStreamMenuHeightChanged: updateSeparateMenuGeometry()

    Connections {
        target: Chiaki.window
        function onXChanged() { view.updateSeparateMenuGeometry() }
        function onYChanged() { view.updateSeparateMenuGeometry() }
        function onWidthChanged() { view.updateSeparateMenuGeometry() }
        function onHeightChanged() { view.updateSeparateMenuGeometry() }
        function onVisibilityChanged() { view.updateSeparateMenuGeometry() }
    }

    QtObject {
        id: menuController
        objectName: "streamMenuController"
        property bool open: false

        function toggle() {
            if (root.streamPanelOpen) {
                root.dismissStreamPanel();
                return;
            }
            if (open) {
                close();
                return;
            }
            view.updateSeparateMenuGeometry();
            view.grabInput(null);
            open = true;
            view.updateOverlayInteractionActive();
        }

        function close(restoreActivation = true) {
            if (!open)
                return;
            open = false;
            view.releaseInput();
            view.updateOverlayInteractionActive();
            if (restoreActivation && useSeparateMenuWindow)
                Chiaki.window.requestActivate();
        }
    }

    Connections {
        target: root
        function onStreamPanelOpenChanged() { view.updateOverlayInteractionActive(); }
    }

    Rectangle {
        id: loadingView
        anchors.fill: parent
        color: "black"
        opacity: sessionError || sessionLoading || (Chiaki.settings.audioVideoDisabled & 0x02) ? 1.0 : 0.0
        visible: opacity

        Behavior on opacity { NumberAnimation { duration: 250 } }

        Item {
            anchors {
                top: parent.verticalCenter
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            BusyIndicator {
                id: spinner
                anchors.centerIn: parent
                width: 70
                height: width
                visible: sessionLoading
                running: sessionLoading
            }

            Label {
                anchors {
                    top: spinner.bottom
                    horizontalCenter: spinner.horizontalCenter
                    topMargin: 30
                }
                width: Math.min(640, view.width - 48)
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
                text: Chiaki.session && Chiaki.session.connected ? qsTr("Waiting for the first video frame…") : qsTr("Connecting to the console…")
                visible: sessionLoading
            }

            Label {
                id: audioVideoDisabledTitleLabel
                anchors {
                    bottom: spinner.top
                    horizontalCenter: spinner.horizontalCenter
                }
                text: (Chiaki.settings.audioVideoDisabled & 0x01) ? qsTr("Audio and Video Disabled") : qsTr("Video Disabled")
                font.pixelSize: 24
                visible: !sessionLoading && !sessionError && (Chiaki.settings.audioVideoDisabled & 0x02)
            }

            Label {
                id: audioVideoDisabledTextLabel
                anchors {
                    top: audioVideoDisabledTitleLabel.bottom
                    horizontalCenter: audioVideoDisabledTitleLabel.horizontalCenter
                    topMargin: 10
                }
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 20
                text: (Chiaki.settings.audioVideoDisabled & 0x01) ? qsTr("You have disabled audio and video in your settings.\nTo re-enable change Audio/Video to Audio and Video Enabled in the General tab of the settings.") : qsTr("You have disabled video in your settings.\nTo re-enable change Audio/Video to Audio and Video Enabled in the General tab of the settings.")
                visible: !sessionLoading && !sessionError && (Chiaki.settings.audioVideoDisabled & 0x02)
            }


        }
    }

    ScrollView {
        id: streamFailure
        anchors.centerIn: parent
        width: Math.max(0, Math.min(620, view.width - 40))
        height: Math.max(0, Math.min(view.height - 40, failureBody.implicitHeight + 24))
        visible: view.sessionError
        clip: true
        contentWidth: availableWidth
        contentHeight: failureBody.implicitHeight
        property bool detailsVisible: false

        ColumnLayout {
            id: failureBody
            width: streamFailure.availableWidth
            spacing: 12

            Label {
                id: errorTitleLabel
                Layout.fillWidth: true
                font.pixelSize: 22
                font.bold: true
                wrapMode: Text.Wrap
                textFormat: Text.PlainText
            }
            Label {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                text: qsTr("Check that the console is awake and the selected route still reaches it. If you use a saved IP, verify the console address and the VPN subnet route before retrying.")
            }
            Flow {
                Layout.fillWidth: true
                spacing: 8
                Button {
                    id: returnToConsoles
                    text: qsTr("Back to consoles")
                    onClicked: root.showMainView()
                }
                Button {
                    text: streamFailure.detailsVisible ? qsTr("Hide details") : qsTr("Details")
                    onClicked: streamFailure.detailsVisible = !streamFailure.detailsVisible
                }
                Button {
                    text: qsTr("Connection tools")
                    onClicked: Chiaki.window.showConnectionTools()
                }
            }
            Label {
                id: errorTextLabel
                Layout.fillWidth: true
                visible: streamFailure.detailsVisible
                wrapMode: Text.Wrap
                textFormat: Text.PlainText
                color: "#adbbb3"
            }
        }
        Keys.onEscapePressed: root.showMainView()
    }

    ColumnLayout {
        id: cantDisplayMessage
        anchors.centerIn: parent
        opacity: Chiaki.window.hasVideo && Chiaki.session && Chiaki.session.cantDisplay ? 1.0 : 0.0
        visible: opacity
        spacing: 30

        Behavior on opacity { NumberAnimation { duration: 250 } }

        onVisibleChanged: {
            if (visible) {
                menuController.close();
                view.grabInput(goToHomeButton);
            } else {
                view.releaseInput();
            }
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("The screen contains content that can't be displayed using Remote Play.")
        }

        Button {
            id: goToHomeButton
            enabled: Chiaki.session && !Chiaki.session.viewOnly
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: 60
            text: qsTr("Go to Home Screen")
            Material.background: activeFocus ? parent.Material.accent : undefined
            Material.roundedScale: Material.SmallScale
            onClicked: Chiaki.sessionGoHome()
            Keys.onReturnPressed: clicked()
            Keys.onEscapePressed: clicked()
        }
    }

    RoundButton {
        anchors {
            right: parent.right
            top: parent.top
            margins: 40
        }
        icon.source: "qrc:/icons/discover-off-24px.svg"
        icon.width: 50
        icon.height: 50
        padding: 20
        checked: true
        opacity: networkIndicatorTimer.running ? 0.7 : 0.0
        visible: opacity
        Material.background: Material.accent

        Behavior on opacity { NumberAnimation { duration: 400 } }

        Timer {
            id: networkIndicatorTimer
            running: Chiaki.session?.averagePacketLoss > (Chiaki.settings.wifiDroppedNotif * 0.01)
            interval: 400
        }
    }

    Component {
        id: streamStatsContent
        Item {
            id: streamStatsContentRoot
            anchors.fill: parent
            ColumnLayout {
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    rightMargin: 5
                }

                Label {
                    Layout.alignment: Qt.AlignRight
                    text: "Mbps"
                    font.pixelSize: 18
                    visible: Chiaki.session ? true : false

                    Label {
                        anchors {
                            right: parent.left
                            baseline: parent.baseline
                            rightMargin: 5
                        }
                        text: parent.visible ? Chiaki.session.measuredBitrate.toFixed(1) : ""
                        color: Material.accent
                        font.bold: true
                        font.pixelSize: 28
                    }
                }

                Label {
                    Layout.alignment: Qt.AlignRight
                    text: qsTr("queue depth avg")
                    font.pixelSize: 15
                    opacity: Chiaki.session ? 1 : 0
                    visible: opacity > 0

                    Behavior on opacity { NumberAnimation { duration: 250 } }

                    Label {
                        anchors {
                            right: parent.left
                            baseline: parent.baseline
                            rightMargin: 5
                        }
                        text: parent.visible ? Chiaki.window.queueDepthAverage.toFixed(1) : ""
                        font.bold: true
                        color: "#90caf9"
                        font.pixelSize: 18
                    }
                }

                Label {
                    Layout.alignment: Qt.AlignRight
                    text: qsTr("pending frame age")
                    font.pixelSize: 15
                    opacity: Chiaki.session ? 1 : 0
                    visible: opacity > 0

                    Behavior on opacity { NumberAnimation { duration: 250 } }

                    Label {
                        anchors {
                            right: parent.left
                            baseline: parent.baseline
                            rightMargin: 5
                        }
                        text: parent.visible ? qsTr("%1 ms").arg((Chiaki.window.pendingFrameAge * 1000.0).toFixed(0)) : ""
                        font.bold: true
                        color: "#90caf9"
                        font.pixelSize: 18
                    }
                }

                Label {
                    Layout.alignment: Qt.AlignRight
                    id: statsPacketLossLabel
                    text: qsTr("packet loss")
                    font.pixelSize: 15
                    opacity: Chiaki.session ? 1 : 0
                    visible: opacity > 0

                    Behavior on opacity { NumberAnimation { duration: 250 } }

                    Label {
                        anchors {
                            right: parent.left
                            baseline: parent.baseline
                            rightMargin: 5
                        }
                        text: parent.visible ? "%1<font size=\"1\">%</font>".arg((((Chiaki.session && isFinite(Chiaki.session.averagePacketLoss)) ? Chiaki.session.averagePacketLoss : 0) * 100).toFixed(1)) : ""
                        font.bold: true
                        color: "#ef9a9a"
                        font.pixelSize: 18
                    }
                }

                Label {
                    Layout.alignment: Qt.AlignRight
                    text: qsTr("dropped frames")
                    font.pixelSize: 15
                    opacity: Chiaki.session ? 1 : 0
                    visible: opacity > 0

                    Behavior on opacity { NumberAnimation { duration: 250 } }

                    Label {
                        id: statsDroppedFramesLabel
                        anchors {
                            right: parent.left
                            baseline: parent.baseline
                            rightMargin: 5
                        }
                        text: parent.visible ? Chiaki.window.droppedFrames : ""
                        color: "#ef9a9a"
                        font.bold: true
                        font.pixelSize: 18
                    }
                }

                Label {
                    Layout.alignment: Qt.AlignRight
                    text: qsTr("lost frames")
                    font.pixelSize: 15
                    opacity: Chiaki.session ? 1 : 0
                    visible: opacity > 0

                    Behavior on opacity { NumberAnimation { duration: 250 } }

                    Label {
                        anchors {
                            right: parent.left
                            baseline: parent.baseline
                            rightMargin: 5
                        }
                        text: parent.visible ? ((Chiaki.session && isFinite(Chiaki.session.framesLost)) ? Chiaki.session.framesLost : 0) : ""
                        color: "#ef9a9a"
                        font.bold: true
                        font.pixelSize: 18
                    }
                }
            }
        }
    }

    Rectangle {
        id: streamToolbar
        anchors { top: parent.top; right: parent.right; margins: 12 }
        z: 5
        visible: !!Chiaki.session && Chiaki.session.connected && !menuController.open && !root.streamPanelOpen
                 && !sessionPinDialogActive && !sessionStopDialogActive
                 && !separateSessionStopWindow.visible && !separateSessionPinWindow.visible
        width: Math.min(parent.width - 24, toolbarRow.implicitWidth + 16)
        height: toolbarRow.implicitHeight + 8
        radius: 8
        color: "#ee202725"
        border.color: "#485851"

        RowLayout {
            id: toolbarRow
            anchors { fill: parent; leftMargin: 8; rightMargin: 8 }
            spacing: 8

            Label {
                Layout.fillWidth: true
                elide: Text.ElideRight
                font.bold: true
                color: "#edf5f0"
                text: Chiaki.session && Chiaki.session.viewOnly ? qsTr("Watch only") : qsTr("Remote play")
            }

            Label {
                id: captureState
                property bool captured: false
                visible: view.width >= 410
                font.pixelSize: 12
                color: "#adbbb3"
                text: captured ? qsTr("Keyboard captured · Esc releases") : qsTr("Keyboard released")
                Timer {
                    interval: 250
                    running: streamToolbar.visible
                    triggeredOnStart: true
                    repeat: true
                    onTriggered: captureState.captured = Chiaki.window.nativeInputCaptured()
                }
            }

            ToolButton {
                text: Chiaki.session && Chiaki.session.viewOnly ? qsTr("Take control") : qsTr("Watch")
                Accessible.name: Chiaki.session && Chiaki.session.viewOnly ? qsTr("Resume remote play") : qsTr("Switch to Watch only")
                onClicked: Chiaki.window.toggleViewOnly()
            }

            ToolButton {
                text: qsTr("Menu")
                Accessible.name: qsTr("Stream menu, Ctrl+O")
                onClicked: menuController.toggle()
            }
        }
    }

    Component {
        id: menuContent
        StreamMenu {
            onDismissed: menuController.close()
            onDisplayRequested: {
                menuController.close(false);
                root.openDisplaySettings();
            }
            onRendererRequested: {
                menuController.close(false);
                root.openPlaceboSettings();
            }
        }
    }

    Loader {
        anchors { bottom: parent.bottom; left: parent.left; right: parent.right; margins: 12 }
        z: 10
        height: streamMenuHeight
        active: !useSeparateMenuWindow && menuController.open
        sourceComponent: menuContent
        onLoaded: item.focusMenu()
    }

    Window {
        id: separateMenuWindow
        objectName: "separateStreamMenu"
        transientParent: Chiaki.window
        flags: Qt.Tool | Qt.FramelessWindowHint
        color: "#202725"
        x: separateMenuX
        y: separateMenuY
        width: separateMenuWidth
        height: streamMenuHeight
        visible: useSeparateMenuWindow && menuController.open
        onClosing: menuController.close()
        onVisibleChanged: if (visible) requestActivate()

        Shortcut {
            sequence: "Ctrl+O"
            enabled: separateMenuWindow.visible
            autoRepeat: false
            onActivated: menuController.close()
        }

        Loader {
            anchors.fill: parent
            active: separateMenuWindow.visible
            sourceComponent: menuContent
            onLoaded: item.focusMenu()
        }
    }

    BoundedDialog {
        id: sessionStopDialog
        property int closeAction: 0
        parent: Overlay.overlay
        modal: true
        padding: 30
        onAboutToShow: {
            closeAction = 0;
            sessionStopDialogActive = true;
            view.updateOverlayInteractionActive();
        }
        onClosed: {
            view.releaseInput();
            sessionStopDialogActive = false;
            view.updateOverlayInteractionActive();
            if (closeAction)
                Chiaki.stopSession(closeAction == 1);
        }

        ColumnLayout {
            Label {
                Layout.alignment: Qt.AlignCenter
                text: qsTr("Disconnect Session")
                font.bold: true
                font.pixelSize: 18
            }

            Label {
                Layout.topMargin: 10
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                Layout.alignment: Qt.AlignCenter
                text: qsTr("Rest Mode ends this stream. Choose Rest, or disconnect and leave the PS5 awake.")
                font.pixelSize: 16
            }

            RowLayout {
                Layout.topMargin: 30
                Layout.alignment: Qt.AlignCenter
                spacing: 12

                Button {
                    id: sleepButton
                    Layout.preferredWidth: 150
                    Layout.minimumHeight: 48
                    Layout.maximumHeight: 48
                    text: qsTr("Rest PS5")
                    font.pixelSize: 18
                    Material.roundedScale: Material.SmallScale
                    Material.background: activeFocus ? parent.Material.accent : undefined
                    KeyNavigation.right: noButton
                    Keys.onReturnPressed: clicked()
                    Keys.onEscapePressed: sessionStopDialog.close()
                    onVisibleChanged: if (visible) view.grabInput(sleepButton)
                    onClicked: {
                        sessionStopDialog.closeAction = 1;
                        sessionStopDialog.close();
                    }
                }

                Button {
                    id: noButton
                    Layout.preferredWidth: 150
                    Layout.minimumHeight: 48
                    Layout.maximumHeight: 48
                    text: qsTr("Leave awake")
                    font.pixelSize: 18
                    Material.roundedScale: Material.SmallScale
                    Material.background: activeFocus ? parent.Material.accent : undefined
                    KeyNavigation.left: sleepButton
                    Keys.onReturnPressed: clicked()
                    Keys.onEscapePressed: sessionStopDialog.close()
                    onClicked: {
                        sessionStopDialog.closeAction = 2;
                        sessionStopDialog.close();
                    }
                }
            }
        }
    }

    Window {
        id: separateSessionStopWindow
        property int closeAction: 0
        readonly property int dialogWidth: Math.max(1, Math.min(640, view.width - 24))
        readonly property int dialogHeight: Math.max(1, Math.min(260, view.height - 24))
        visible: false
        flags: Qt.Dialog | Qt.FramelessWindowHint
        color: "transparent"
        transientParent: Chiaki.window
        modality: Qt.ApplicationModal
        x: Chiaki.window.x + Math.round((view.width - width) / 2)
        y: Chiaki.window.y + Math.round((view.height - height) / 2)
        width: dialogWidth
        height: dialogHeight

        onVisibleChanged: {
            if (visible) {
                closeAction = 0;
                requestActivate();
                view.grabInput(separateSleepButton);
            } else {
                view.releaseInput();
                if (closeAction)
                    Chiaki.stopSession(closeAction === 1);
            }
            view.updateOverlayInteractionActive();
        }

        Rectangle {
            anchors.fill: parent
            radius: 12
            color: Material.background
            border.color: "#666666"
            border.width: 1
        }

        ColumnLayout {
            anchors { fill: parent; margins: 20 }

            Label {
                Layout.alignment: Qt.AlignCenter
                text: qsTr("Disconnect Session")
                font.bold: true
                font.pixelSize: 18
            }

            Label {
                Layout.topMargin: 10
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                Layout.alignment: Qt.AlignCenter
                text: qsTr("Rest Mode ends this stream. Choose Rest, or disconnect and leave the PS5 awake.")
                font.pixelSize: 16
            }

            RowLayout {
                Layout.topMargin: 30
                Layout.alignment: Qt.AlignCenter
                spacing: 12

                Button {
                    id: separateSleepButton
                    Layout.preferredWidth: 150
                    Layout.minimumHeight: 48
                    Layout.maximumHeight: 48
                    text: qsTr("Rest PS5")
                    font.pixelSize: 18
                    Material.roundedScale: Material.SmallScale
                    Material.background: activeFocus ? parent.Material.accent : undefined
                    KeyNavigation.right: separateNoButton
                    Keys.onReturnPressed: clicked()
                    Keys.onEscapePressed: separateSessionStopWindow.visible = false
                    onClicked: {
                        separateSessionStopWindow.closeAction = 1;
                        separateSessionStopWindow.visible = false;
                    }
                }

                Button {
                    id: separateNoButton
                    Layout.preferredWidth: 150
                    Layout.minimumHeight: 48
                    Layout.maximumHeight: 48
                    text: qsTr("Leave awake")
                    font.pixelSize: 18
                    Material.roundedScale: Material.SmallScale
                    Material.background: activeFocus ? parent.Material.accent : undefined
                    KeyNavigation.left: separateSleepButton
                    Keys.onReturnPressed: clicked()
                    Keys.onEscapePressed: separateSessionStopWindow.visible = false
                    onClicked: {
                        separateSessionStopWindow.closeAction = 2;
                        separateSessionStopWindow.visible = false;
                    }
                }
            }
        }
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
            view.grabInput(pinField);
            sessionPinDialogActive = true;
            view.updateOverlayInteractionActive();
        }
        onClosed: {
            view.releaseInput();
            sessionPinDialogActive = false;
            view.updateOverlayInteractionActive();
        }
        onAccepted: Chiaki.enterPin(pinField.text)
        onRejected: Chiaki.stopSession(false)
        Material.roundedScale: Material.MediumScale

        TextField {
            id: pinField
            echoMode: TextInput.Password
            implicitWidth: 200
            validator: RegularExpressionValidator { regularExpression: /[0-9]{4}/ }
            Keys.onReturnPressed: {
                if(sessionPinDialog.standardButton(Dialog.Ok).enabled)
                    sessionPinDialog.standardButton(Dialog.Ok).clicked()
            }
        }
    }

    Window {
        id: separateSessionPinWindow
        readonly property int dialogWidth: Math.max(1, Math.min(360, view.width - 24))
        readonly property int dialogHeight: Math.max(1, Math.min(220, view.height - 24))
        visible: false
        flags: Qt.Dialog | Qt.FramelessWindowHint
        color: "transparent"
        transientParent: Chiaki.window
        modality: Qt.ApplicationModal
        x: Chiaki.window.x + Math.round((view.width - width) / 2)
        y: Chiaki.window.y + Math.round((view.height - height) / 2)
        width: dialogWidth
        height: dialogHeight

        onVisibleChanged: {
            if (visible) {
                separatePinField.text = "";
                requestActivate();
                view.grabInput(separatePinField);
            } else {
                view.releaseInput();
            }
            view.updateOverlayInteractionActive();
        }

        Rectangle {
            anchors.fill: parent
            radius: 12
            color: Material.background
            border.color: "#666666"
            border.width: 1
        }

        ColumnLayout {
            anchors {
                fill: parent
                margins: 24
            }

            Label {
                Layout.alignment: Qt.AlignCenter
                text: qsTr("Console Login PIN")
                font.bold: true
                font.pixelSize: 24
            }

            TextField {
                id: separatePinField
                Layout.topMargin: 20
                Layout.fillWidth: true
                echoMode: TextInput.Password
                validator: RegularExpressionValidator { regularExpression: /[0-9]{4}/ }
                Keys.onReturnPressed: if (acceptableInput) separatePinOkButton.clicked()
                Keys.onEscapePressed: separateSessionPinWindow.visible = false
            }

            RowLayout {
                Layout.topMargin: 20
                Layout.alignment: Qt.AlignRight
                spacing: 16

                Button {
                    text: qsTr("Cancel")
                    Keys.onReturnPressed: clicked()
                    onClicked: {
                        separateSessionPinWindow.visible = false;
                        Chiaki.stopSession(false);
                    }
                }

                Button {
                    id: separatePinOkButton
                    text: qsTr("OK")
                    enabled: separatePinField.acceptableInput
                    Keys.onReturnPressed: clicked()
                    onClicked: {
                        const pin = separatePinField.text;
                        separateSessionPinWindow.visible = false;
                        Chiaki.enterPin(pin);
                    }
                }
            }
        }
    }

    Connections {
        target: Chiaki

        function onSessionChanged() {
            if (!Chiaki.session) {
                if (!view.sessionError)
                    root.showMainView();
            } else {
                sessionError = false;
                errorTitleLabel.text = "";
                errorTextLabel.text = "";
                sessionLoading = !(Chiaki.window.loadingTransitionComplete || (Chiaki.settings.audioVideoDisabled & 0x02));
            }
        }

        function onSessionError(title, text) {
            menuController.close(false);
            root.dismissStreamPanel();
            if (sessionPinDialog.opened) sessionPinDialog.close();
            if (separateSessionPinWindow.visible) separateSessionPinWindow.visible = false;
            errorTitleLabel.text = title;
            errorTextLabel.text = text;
            streamFailure.detailsVisible = false;
            sessionLoading = false;
            if (!sessionError) {
                sessionError = true;
                view.grabInput(returnToConsoles);
            }
            Chiaki.window.requestOverlayUpdate();
        }

        function onSessionPinDialogRequested() {
            if (sessionPinDialog.opened || separateSessionPinWindow.visible)
                return;
            menuController.close(false);
            root.dismissStreamPanel();
            if (useSeparateMenuWindow)
                separateSessionPinWindow.visible = true;
            else
                sessionPinDialog.open();
            Chiaki.window.requestOverlayUpdate();
        }

        function onSessionStopDialogRequested() {
            if (sessionStopDialog.opened || separateSessionStopWindow.visible)
                return;
            menuController.close(false);
            root.dismissStreamPanel();
            if (useSeparateMenuWindow)
                separateSessionStopWindow.visible = true;
            else
                sessionStopDialog.open();
            Chiaki.window.requestOverlayUpdate();
        }
    }

    Connections {
        target: Chiaki.window

        function onLoadingTransitionCompleteChanged() {
            if (Chiaki.window.loadingTransitionComplete) {
                sessionLoading = false;
                Chiaki.window.noteLoadingTransitionComplete();
                Chiaki.window.requestOverlayUpdate();
            } else if (Chiaki.session) {
                sessionLoading = !(Chiaki.settings.audioVideoDisabled & 0x02);
            }
        }

        function onMenuRequested() {
            if (!Chiaki.session || sessionError || sessionPinDialog.opened || sessionStopDialog.opened || separateSessionPinWindow.visible || separateSessionStopWindow.visible)
                return;
            menuController.toggle();
            Chiaki.window.requestOverlayUpdate();
        }
    }
    Connections {
        target: Chiaki

        function onSessionChanged() {
            if (!Chiaki.session)
                menuController.close();
        }
    }
    Connections {
        target: Chiaki.session

        function onConnectedChanged() {
            if (Chiaki.settings.audioVideoDisabled & 0x02)
                sessionLoading = false;
        }
    }
}
