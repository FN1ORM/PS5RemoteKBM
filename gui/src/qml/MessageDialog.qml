import QtQuick
import QtQuick.Controls

BoundedDialog {
    id: dialog
    property alias text: message.text
    property bool restoreFocusOnClose: true
    property Item restoreFocusItem
    standardButtons: Dialog.Ok
    onOpened: message.forceActiveFocus(Qt.TabFocusReason)
    onClosed: if (restoreFocusOnClose && restoreFocusItem) restoreFocusItem.forceActiveFocus(Qt.TabFocusReason)
    contentItem: ScrollView {
        implicitWidth: 560
        implicitHeight: message.implicitHeight
        clip: true
        contentWidth: availableWidth
        contentHeight: message.implicitHeight
        Label {
            id: message
            width: dialog.availableWidth
            height: implicitHeight
            wrapMode: Text.Wrap
            textFormat: Text.PlainText
            font.pixelSize: 16
            Keys.onEscapePressed: dialog.reject()
            Keys.onReturnPressed: dialog.accept()
        }
    }
}
