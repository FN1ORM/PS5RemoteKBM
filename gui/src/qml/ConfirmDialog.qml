import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import "controls" as C

MessageDialog {
    id: dialog
    restoreFocusOnClose: false
    property var callback
    property var rejectCallback
    property bool newDialogOpen: false
    onAboutToShow: newDialogOpen = false
    onAccepted: {
        newDialogOpen = true;
        restoreFocus();
        if (callback) callback();
    }
    onClosed: if(!newDialogOpen) { restoreFocus() }

    onRejected: {
        if(rejectCallback)
        {
            newDialogOpen = true;
            restoreFocus();
            rejectCallback();
        }
    }

    function restoreFocus() {
        if (restoreFocusItem)
            restoreFocusItem.forceActiveFocus(Qt.TabFocusReason);
    }

    standardButtons: Dialog.Yes | Dialog.No
}
