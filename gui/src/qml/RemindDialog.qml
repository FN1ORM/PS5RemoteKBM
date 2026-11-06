import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import "controls" as C

import org.streetpea.chiaking

MessageDialog {
    id: dialog
    restoreFocusOnClose: false
    property var remotePlay
    property var callback
    property bool newDialogOpen: false
    onAboutToShow: newDialogOpen = false
    onAccepted: {
        newDialogOpen = true;
        restoreFocus();
        if (callback) callback();
    }
    onRejected: {
        if(dialog.remotePlay)
            Chiaki.settings.remotePlayAsk = false;
        else
            Chiaki.settings.addSteamShortcutAsk = false;
    }
    onClosed: {
        if(newDialogOpen)
            return;
        restoreFocus();
        if(!remotePlay && Chiaki.settings.remotePlayAsk)
        {
            if(!Chiaki.settings.psnRefreshToken || !Chiaki.settings.psnAuthToken || !Chiaki.settings.psnAuthTokenExpiry || !Chiaki.settings.psnAccountId)
                root.showRemindDialog(qsTr("Remote Play via PSN"), qsTr("Would you like to sign in to PSN?\nThis enables account-based connection attempts. Console registration and compatible network traversal are still required.") + "\n\n" + qsTr("(Note: If you select no now and want to do this later, go to the Config section of the settings.)"), true, () => root.showPSNTokenDialog(false));
            else
                Chiaki.settings.remotePlayAsk = false;
        }
    }

    function restoreFocus() {
        if (restoreFocusItem)
            restoreFocusItem.forceActiveFocus(Qt.TabFocusReason);
    }

    standardButtons: Dialog.NoButton
    footer: Flow {
        padding: 12
        spacing: 8
        Button { text: qsTr("Yes"); onClicked: dialog.accept() }
        Button { text: qsTr("No"); onClicked: dialog.reject() }
        Button { text: qsTr("Remind me later"); onClicked: dialog.close() }
    }
}
