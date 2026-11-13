import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import org.streetpea.chiaking

Pane {
    id: consolePane
    padding: 0
    background: Rectangle { color: "#151918" }
    property var selectedRoutes: ({})
    objectName: "consoleDashboard"
    readonly property var consoles: groupHosts(Chiaki.hosts)

    function routeKey(host) {
        if (host.discovered) return "local:" + host.mac + ":" + host.address;
        if (host.manual) return "saved:" + host.manualId;
        return "psn:" + host.duid;
    }

    function groupHosts(hosts) {
        const groups = new Map();
        for (let index = 0; index < hosts.length; ++index) {
            const host = hosts[index];
            if (!host.display)
                continue;
            const key = host.mac ? "mac:" + host.mac : routeKey(host);
            if (!groups.has(key))
                groups.set(key, { key: key, name: host.name, ps5: host.ps5, routes: [] });
            const group = groups.get(key);
            group.routes.push({ key: routeKey(host), index: index, host: host });
        }
        return Array.from(groups.values()).map(group => {
            const priority = route => route.host.discovered ? 0 : route.host.manual ? 1 : 2;
            group.routes.sort((a, b) => priority(a) - priority(b) || b.index - a.index);
            return group;
        });
    }

    function selectRoute(consoleKey, key) {
        const next = Object.assign({}, selectedRoutes);
        next[consoleKey] = key;
        selectedRoutes = next;
    }

    function resolveRoute(key) {
        const hosts = Chiaki.hosts;
        for (let index = 0; index < hosts.length; ++index) {
            if (hosts[index].display && routeKey(hosts[index]) === key)
                return index;
        }
        return -1;
    }

    function routeLabel(host) {
        if (host.discovered) return qsTr("Local network");
        if (host.manual) return qsTr("Saved IP / VPN");
        return qsTr("PSN");
    }

    StackView.onActivated: {
        if (hostsView.count > 0)
            hostsView.forceActiveFocus(Qt.TabFocusReason);
        else
            forceActiveFocus(Qt.TabFocusReason);
        if(!Chiaki.autoConnect && !root.initialAsk && !Chiaki.window.directStream)
        {
            root.initialAsk = true;
            if(Chiaki.settings.addSteamShortcutAsk && (typeof Chiaki.createSteamShortcut === "function"))
                root.showRemindDialog(qsTr("Official Steam artwork + controller layout"), qsTr("Would you like to either create a new non-Steam game for chiaki-ng\nor update an existing non-Steam game with the official artwork and controller layout?") + "\n\n" + qsTr("(Note: If you select no now and want to do this later, click the button or press R3 from the main menu.)"), false, () => root.showSteamShortcutDialog(true));
            else if(Chiaki.settings.remotePlayAsk)
            {
                if(!Chiaki.settings.psnRefreshToken || !Chiaki.settings.psnAuthToken || !Chiaki.settings.psnAuthTokenExpiry || !Chiaki.settings.psnAccountId)
                    root.showRemindDialog(qsTr("Remote Play via PSN"), qsTr("Would you like to sign in to PSN?\nThis allows attempts at:\n- Authorized automatic console registration\n- Remote play across networks\nConsole authorization and compatible NAT/network access are required.") + "\n\n" + qsTr("You can sign in later from the Config section of Settings. If PSN cannot connect, use manual IP registration through a routed VPN."), true, () => root.showPSNTokenDialog(false));
                else
                    Chiaki.settings.remotePlayAsk = false;
            }
        }
    }

    Keys.onMenuPressed: root.showSettingsDialog()
    Keys.onPressed: event => {
        if (event.key === Qt.Key_PageDown && Chiaki.settings.psnAuthToken) {
            Chiaki.refreshPsnToken();
            event.accepted = true;
        } else if (event.key === Qt.Key_F1 && typeof Chiaki.createSteamShortcut === "function") {
            root.showSteamShortcutDialog(false);
            event.accepted = true;
        }
    }
    Keys.onEscapePressed: root.showConfirmDialog(qsTr("Quit KeyStream?"),
        qsTr("Close the app?"), () => Qt.quit())

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: Math.max(64, header.implicitHeight + 22)
            color: "#202725"

            RowLayout {
                id: header
                anchors { fill: parent; leftMargin: 20; rightMargin: 20 }
                spacing: 10
                Image {
                    Layout.preferredWidth: 42
                    Layout.preferredHeight: 42
                    source: "qrc:/icons/keystream.svg"
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Label {
                        text: "KeyStream"
                        color: "#edf5f0"
                        font.pixelSize: 23
                        font.bold: true
                    }
                    Label {
                        Layout.fillWidth: true
                        color: "#adbbb3"
                        font.pixelSize: 12
                        elide: Text.ElideRight
                        textFormat: Text.PlainText
                        text: qsTr("Profile · %1").arg(Chiaki.settings.streamerMode ? qsTr("Hidden") :
                            Chiaki.settings.currentProfile || qsTr("Default"))
                    }
                }
                Button {
                    text: qsTr("Add console")
                    Material.background: "#78dfb6"
                    Material.foreground: "#14221b"
                    onClicked: root.showManualHostDialog()
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            spacing: 3
            ToolButton { text: qsTr("Consoles"); checked: true }
            ToolButton { text: qsTr("Controls"); onClicked: Chiaki.window.showNativeControls() }
            ToolButton { text: qsTr("Connection"); onClicked: Chiaki.window.showConnectionTools() }
            ToolButton { text: qsTr("Settings"); onClicked: root.showSettingsDialog() }
            Item { Layout.fillWidth: true }
            ToolButton { text: qsTr("Help"); onClicked: Chiaki.window.showUserManual() }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 1
            color: "#36423d"
        }

        ListView {
            id: hostsView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            topMargin: 16
            bottomMargin: 16
            spacing: 12
            model: consolePane.consoles
            keyNavigationWraps: true
            activeFocusOnTab: true
            Keys.onReturnPressed: if (currentItem) currentItem.connectToConsole()
            ScrollBar.vertical: ScrollBar {}

            delegate: FocusScope {
                id: card
                required property var modelData
                x: (hostsView.width - width) / 2
                width: Math.max(0, Math.min(800, hostsView.width - 40))
                height: body.implicitHeight + 32
                readonly property var routes: modelData.routes
                readonly property int selectedIndex: Math.max(0, routes.findIndex(route =>
                    route.key === consolePane.selectedRoutes[modelData.key]))
                readonly property var selectedRoute: routes[selectedIndex]
                readonly property var selectedHost: selectedRoute.host
                function connectToConsole() { connectButton.clicked(); }

                Rectangle {
                    anchors.fill: parent
                    radius: 10
                    color: "#202725"
                    border.color: card.activeFocus || (hostsView.activeFocus && card.ListView.isCurrentItem)
                                  ? "#78dfb6" : "#36423d"
                }

                ColumnLayout {
                    id: body
                    anchors { fill: parent; margins: 16 }
                    spacing: 6

                    RowLayout {
                        Layout.fillWidth: true
                        Label {
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                            font.pixelSize: 20
                            font.bold: true
                            textFormat: Text.PlainText
                            text: Chiaki.settings.streamerMode ? qsTr("Console") : card.modelData.name
                        }
                        Label {
                            font.pixelSize: 13
                            color: "#adbbb3"
                            text: card.modelData.ps5 === undefined ? "" : card.modelData.ps5 ? "PS5" : "PS4"
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                        font.pixelSize: 13
                        color: "#adbbb3"
                        textFormat: Text.PlainText
                        text: {
                            const host = card.selectedHost;
                            const state = host.discovered && host.state === "ready" ? qsTr("Online on local network") :
                                host.discovered && host.state === "standby" ? qsTr("In Rest Mode") :
                                qsTr("Reachability unverified");
                            return (host.registered ? qsTr("Registered") : qsTr("Pairing required")) + " · " + state;
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        ComboBox {
                            Layout.fillWidth: true
                            Layout.minimumWidth: 0
                            Accessible.name: qsTr("Connection method")
                            model: card.routes.map(route => {
                                const host = route.host;
                                return consolePane.routeLabel(host) + (host.address && !Chiaki.settings.streamerMode ?
                                    " · " + host.address : "");
                            })
                            currentIndex: card.selectedIndex
                            onActivated: index => consolePane.selectRoute(card.modelData.key, card.routes[index].key)
                        }
                        Button {
                            id: connectButton
                            text: card.selectedHost.registered ? qsTr("Connect") : qsTr("Register")
                            Material.background: "#78dfb6"
                            Material.foreground: "#14221b"
                            onClicked: {
                                const index = consolePane.resolveRoute(card.selectedRoute.key);
                                if (index >= 0)
                                    Chiaki.connectToHost(index, card.selectedHost.discovered ? card.selectedHost.name : "");
                            }
                        }
                    }

                    Flow {
                        Layout.fillWidth: true
                        spacing: 4
                        ToolButton {
                            text: qsTr("Wake up")
                            visible: card.selectedHost.registered && (card.selectedHost.manual ||
                                     (card.selectedHost.discovered && card.selectedHost.state === "standby"))
                            onClicked: {
                                const index = consolePane.resolveRoute(card.selectedRoute.key);
                                if (index >= 0) Chiaki.wakeUpHost(index, card.selectedHost.discovered ? card.selectedHost.name : "");
                            }
                        }
                        ToolButton {
                            text: qsTr("Login PIN")
                            visible: card.selectedHost.registered
                            onClicked: {
                                const key = card.selectedRoute.key;
                                const index = consolePane.resolveRoute(key);
                                if (index >= 0) root.showConsolePinDialog(index, () => consolePane.resolveRoute(key));
                            }
                        }
                        ToolButton {
                            text: card.selectedHost.manual ? qsTr("Remove saved IP") : qsTr("Hide")
                            visible: card.selectedHost.manual || (card.selectedHost.discovered && !card.selectedHost.registered)
                            onClicked: {
                                const key = card.selectedRoute.key;
                                const host = card.selectedHost;
                                if (host.manual) {
                                    root.showConfirmDialog(qsTr("Remove saved IP?"),
                                        qsTr("This removes the address from KeyStream. Your console pairing is kept."), () => {
                                            const index = consolePane.resolveRoute(key);
                                            if (index >= 0) Chiaki.deleteHost(index);
                                        });
                                } else {
                                    root.showConfirmDialog(qsTr("Hide this console?"),
                                        qsTr("You can show it again from Settings → Consoles."),
                                        () => Chiaki.hideHost(host.mac, host.name));
                                }
                            }
                        }
                    }
                }
            }
        }

        Label {
            Layout.fillWidth: true
            Layout.bottomMargin: 12
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            visible: hostsView.count === 0
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            color: "#adbbb3"
            text: qsTr("No consoles to display. Add an address or turn on local discovery.")
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: Math.max(48, footer.implicitHeight + 4)
            color: "#202725"
            RowLayout {
                id: footer
                anchors { fill: parent; leftMargin: 16; rightMargin: 16 }
                Switch {
                    text: qsTr("Local discovery")
                    checked: Chiaki.discoveryEnabled
                    onToggled: Chiaki.discoveryEnabled = checked
                }
                Item { Layout.fillWidth: true }
                ToolButton {
                    text: qsTr("Refresh PSN")
                    visible: !!Chiaki.settings.psnAuthToken
                    onClicked: Chiaki.refreshPsnToken()
                }
            }
        }
    }
}
