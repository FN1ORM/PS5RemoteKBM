import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import org.streetpea.chiaking

Item {
    id: root
    property list<Item> restoreFocusItems
    property bool initialAsk: false
    Shortcut { sequence: "F2"; onActivated: Chiaki.window.showNativeControls() }
    Shortcut { sequence: "F3"; onActivated: Chiaki.window.showUserManual() }
    Shortcut { sequence: "F4"; onActivated: Chiaki.window.showConnectionTools() }
    readonly property bool separateStreamPanels: Chiaki.window.runtimeRendererBackend === 1
    property string streamPanel: ""
    readonly property bool streamPanelOpen: streamPanel !== ""
    Material.theme: Material.Dark
    Material.accent: "#78dfb6"
    Material.primary: "#202725"
    Material.background: "#151918"
    Material.foreground: "#edf5f0"

    function controllerButton(name) {
        let type = "deck";
        for (let i = 0; i < Chiaki.controllers.length; ++i) {
            if (Chiaki.controllers[i].playStation) {
                type = "ps";
                break;
            }
        }
        return "image://svg/button-%1#%2".arg(type).arg(name);
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

    function autoRegister(auto, host, ps5) {
        if(auto)
            Chiaki.autoRegister()
        else
            showRegistDialog(host, ps5);
    }

    function openStreamPanel(panel) {
        const first = !streamPanelOpen;
        streamPanel = panel;
        if (first)
            grabInput(null);
        if (separateStreamPanels)
            streamSettingsWindow.requestActivate();
        Chiaki.window.requestOverlayUpdate();
    }

    function dismissStreamPanel() {
        if (!streamPanelOpen)
            return;
        streamPanel = "";
        releaseInput();
        Chiaki.window.requestOverlayUpdate();
        Chiaki.window.requestActivate();
    }

    function openDisplaySettings() { openStreamPanel("display"); }
    function openPlaceboSettings() { openStreamPanel("renderer"); }

    function closeDialog() {
        if (streamPanel === "color")
            openPlaceboSettings();
        else if (streamPanelOpen)
            dismissStreamPanel();
        else if (stack.depth > 1)
            stack.pop();
    }

    function showMainView() {
        dismissStreamPanel();
        if (confirmDialog.visible) confirmDialog.close();
        if (stack.depth > 1)
            stack.pop(stack.get(0));
        else
            stack.replace(stack.get(0), mainViewComponent);
    }

    function showStreamView() {
        if (confirmDialog.visible) confirmDialog.close();
        if (infoDialog.visible) infoDialog.close();
        if (remindDialog.visible) remindDialog.close();
        stack.replace(stack.get(0), streamViewComponent, {}, StackView.Immediate);
    }

    function showPsnView() {
        if (confirmDialog.visible) confirmDialog.close();
        stack.replace(stack.get(0), psnViewComponent, {}, StackView.Immediate)
    }

    function showManualHostDialog() {
        stack.push(manualHostDialogComponent);
    }

    function showConfirmDialog(title, text, callback, rejectCallback = null, returnFocusTo = null) {
        confirmDialog.title = title;
        confirmDialog.text = text;
        confirmDialog.callback = callback;
        confirmDialog.rejectCallback = rejectCallback;
        confirmDialog.open(returnFocusTo);
    }

    function showInfoDialog(title, text) {
        infoDialog.title = title;
        infoDialog.text = text;
        infoDialog.restoreFocusItem = Window.window.activeFocusItem;
        infoDialog.open();
    }

    function showRendererFallbackDialog(reason) {
        showInfoDialog(qsTr("Renderer Fallback"),
                       qsTr("Vulkan renderer is unavailable. KeyStream switched to OpenGL.\n\nReason: %1").arg(reason));
    }

    function showRemindDialog(title, text, remotePlay, callback) {
        remindDialog.title = title;
        remindDialog.text = text;
        remindDialog.remotePlay = remotePlay;
        remindDialog.callback = callback;
        remindDialog.restoreFocusItem = Window.window.activeFocusItem;
        remindDialog.open();
    }


    function showRegistDialog(host, ps5) {
        stack.push(registDialogComponent, {host: host, ps5: ps5});
    }

    function showSettingsDialog(tab = 0) {
        stack.push(settingsDialogComponent, { initialTab: tab });
    }

    function showDisplaySettingsDialog() {
        stack.push(displaySettingsDialogComponent);
    }

    function showPlaceboSettingsDialog() {
        stack.push(placeboSettingsDialogComponent);
    }

    function showPlaceboColorMappingDialog() {
        if (streamPanelOpen)
            openStreamPanel("color");
        else
            stack.push(placeboColorMappingDialogComponent);
    }

    function showProfileDialog() {
        stack.push(profileDialogComponent)
    }

    function showConsolePinDialog(consoleIndex, resolveIndex = null) {
        stack.push(consolePinDialogComponent, {consoleIndex: consoleIndex, resolveIndex: resolveIndex});
    }

    function showSteamShortcutDialog(fromReminder) {
        stack.push(steamShortcutDialogComponent, {fromReminder: fromReminder});
    }

    function showPSNTokenDialog(psnurl, expired) {
        stack.push(psnTokenDialogComponent, {psnurl: psnurl, expired: expired});
    }

    function showControllerMappingDialog() {
        stack.push(controllerMappingDialogComponent)
    }

    Component.onCompleted: {
        if (Chiaki.session)
            stack.replace(stack.get(0), streamViewComponent, {}, StackView.Immediate);
        else if (Chiaki.autoConnect)
            stack.replace(stack.get(0), autoConnectViewComponent, {}, StackView.Immediate);
    }

    Pane {
        anchors.fill: parent
        visible: !Chiaki.window.hasVideo && !Chiaki.window.keepVideo
    }

    StackView {
        id: stack
        anchors.fill: parent
        hoverEnabled: false
        initialItem: mainViewComponent
        font.pixelSize: 16

        replaceEnter: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 0.01
                to: 1.0
                duration: 200
            }
        }

        replaceExit: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 1.0
                to: 0.0
                duration: 200
            }
        }
    }

    function streamPanelComponent() {
        switch (streamPanel) {
        case "display": return displaySettingsDialogComponent;
        case "renderer": return placeboSettingsDialogComponent;
        case "color": return placeboColorMappingDialogComponent;
        default: return null;
        }
    }

    Component {
        id: streamPanelContent
        Rectangle {
            color: "#151918"
            border.color: "#485851"
            clip: true
            // Prevent clicks on the panel background from reaching the stream.
            MouseArea { anchors.fill: parent; acceptedButtons: Qt.AllButtons }
            Loader {
                anchors.fill: parent
                anchors.margins: 1
                sourceComponent: root.streamPanelComponent()
                onLoaded: {
                    const first = item.mainItem ? item.mainItem.nextItemInFocusChain() : null;
                    if (first) first.forceActiveFocus(Qt.TabFocusReason);
                }
            }
        }
    }

    Loader {
        anchors.centerIn: parent
        z: 30
        width: Math.max(0, Math.min(1160, root.width - 32))
        height: Math.max(0, Math.min(700, root.height - 32))
        active: root.streamPanelOpen && !root.separateStreamPanels
        sourceComponent: streamPanelContent
    }

    Window {
        id: streamSettingsWindow
        transientParent: Chiaki.window
        visible: root.streamPanelOpen && root.separateStreamPanels
        flags: Qt.Tool | Qt.FramelessWindowHint
        x: Chiaki.window.x + Math.round((root.width - width) / 2)
        y: Chiaki.window.y + Math.round((root.height - height) / 2)
        width: Math.max(1, Math.min(1160, root.width - 32))
        height: Math.max(1, Math.min(700, root.height - 32))
        Material.theme: Material.Dark
        Material.accent: "#78dfb6"
        Material.foreground: "#edf5f0"
        Material.background: "#151918"
        Material.primary: "#202725"
        onClosing: root.dismissStreamPanel()
        Shortcut {
            sequence: "Ctrl+O"
            enabled: streamSettingsWindow.visible
            autoRepeat: false
            onActivated: root.dismissStreamPanel()
        }
        Shortcut {
            sequence: StandardKey.Cancel
            enabled: streamSettingsWindow.visible
            onActivated: root.closeDialog()
        }
        Loader {
            anchors.fill: parent
            active: streamSettingsWindow.visible
            sourceComponent: streamPanelContent
        }
    }

    MessageDialog {
        id: infoDialog
    }

    ConfirmDialog {
        id: confirmDialog
    }

    RemindDialog {
        id: remindDialog
    }

    Connections {
        target: Chiaki

        function onSessionChanged() {
            if (Chiaki.session)
                root.showStreamView();
        }

        function onShowPsnView() {
            root.showPsnView();
        }

        function onPsnCredsExpired() {
            Chiaki.settings.psnRefreshToken = ""
            Chiaki.settings.psnAuthToken = ""
            Chiaki.settings.psnAuthTokenExpiry = ""
            Chiaki.settings.psnAccountId = ""
            root.showPSNTokenDialog(true);
        }

        function onError(title, text) {
            root.showInfoDialog(title, text);
        }

        function onRegistDialogRequested(host, ps5, duid) {
            if(!duid)
                showRegistDialog(host, ps5);
            else
            {
                if(ps5)
                    root.showConfirmDialog(qsTr("Registration Type"), qsTr("Try PSN automatic registration? This can be attempted across networks. If it fails, use manual IP + PIN through the VPN subnet route. Console authorization and network support are required."), () => root.autoRegister(true, host, ps5), () => root.autoRegister(false, host, ps5))
                else
                    root.showConfirmDialog(qsTr("Registration Type"), qsTr("Would you like to use automatic registration (must be main PS4 console registered to your PSN)?"), () => root.autoRegister(true, host, ps5), () => root.autoRegister(false, host, ps5))
            }
        }

        function onWakeupStartInitiated() {
            stack.replace(stack.get(0), autoConnectViewComponent, {}, StackView.Immediate);
        }
    }

    Component {
        id: mainViewComponent
        MainView { }
    }

    Component {
        id: streamViewComponent
        StreamView { }
    }

    Component {
        id: autoConnectViewComponent
        AutoConnectView { }
    }

    Component {
        id: psnViewComponent
        PsnView {}
    }

    Component {
        id: manualHostDialogComponent
        ManualHostDialog { }
    }

    Component {
        id: settingsDialogComponent
        SettingsDialog { }
    }

    Component {
        id: displaySettingsDialogComponent
        DisplaySettingsDialog { }
    }

    Component {
        id: placeboSettingsDialogComponent
        PlaceboSettingsDialog { }
    }

    Component {
        id: placeboColorMappingDialogComponent
        PlaceboColorMappingDialog { }
    }

    Component {
        id: profileDialogComponent
        ProfileDialog { }
    }

    Component {
        id: consolePinDialogComponent
        ConsolePinDialog { }
    }

    Component {
        id: steamShortcutDialogComponent
        SteamShortcutDialog { }
    }

    Component {
        id: psnTokenDialogComponent
        PSNTokenDialog { }
    }

    Component {
        id: registDialogComponent
        RegistDialog { }
    }

    Component {
        id: controllerMappingDialogComponent
        ControllerMappingDialog { }
    }
}
