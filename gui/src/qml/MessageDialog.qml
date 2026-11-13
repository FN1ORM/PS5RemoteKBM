import QtQuick
import QtQuick.Controls

BoundedDialog {
    id: dialog
    property alias text: message.text
    property bool restoreFocusOnClose: true
    property Item restoreFocusItem
    standardButtons: Dialog.Ok
    preferredHeight: measure.implicitHeight + (dialog.title.length ? headingMeasure.implicitHeight + 28 : 0)
                     + (dialog.footer ? 70 : 0) + topPadding + bottomPadding

    Text {
        id: headingMeasure
        visible: false
        text: dialog.title
        textFormat: Text.PlainText
        font.pixelSize: 20
        font.bold: true
        width: Math.max(0, Math.min(dialog.preferredWidth, dialog.parent ? dialog.parent.width - 24 : dialog.preferredWidth) - 40)
        wrapMode: Text.Wrap
    }
    onOpened: message.forceActiveFocus(Qt.TabFocusReason)
    onClosed: if (restoreFocusOnClose && restoreFocusItem) restoreFocusItem.forceActiveFocus(Qt.TabFocusReason)
    Text {
        id: measure
        visible: false
        text: message.text
        textFormat: Text.PlainText
        font: message.font
        width: Math.max(0, Math.min(dialog.preferredWidth, dialog.parent ? dialog.parent.width - 24 : dialog.preferredWidth)
                        - dialog.leftPadding - dialog.rightPadding)
        wrapMode: Text.Wrap
    }
    contentItem: ScrollView {
        implicitWidth: 560
        implicitHeight: measure.implicitHeight
        clip: true
        contentWidth: availableWidth
        contentHeight: measure.implicitHeight
        Label {
            id: message
            width: Math.max(0, dialog.width - dialog.leftPadding - dialog.rightPadding)
            wrapMode: Text.Wrap
            textFormat: Text.PlainText
            font.pixelSize: 16
            Keys.onEscapePressed: dialog.reject()
            Keys.onReturnPressed: dialog.accept()
        }
    }
}
