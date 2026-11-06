import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

// Bound the entire popup, including its header/footer, to the current viewport.
Dialog {
    id: dialog
    parent: Overlay.overlay
    property real preferredWidth: 640
    width: Math.max(0, Math.min(preferredWidth, parent ? parent.width - 24 : preferredWidth))
    height: Math.max(0, Math.min(implicitHeight, parent ? parent.height - 24 : implicitHeight))
    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? Math.round((parent.height - height) / 2) : 0
    padding: 20
    font.pixelSize: 16
    modal: true
    Material.roundedScale: Material.MediumScale
    header: Label {
        text: dialog.title
        visible: text.length > 0
        padding: 20
        bottomPadding: 8
        font.pixelSize: 20
        font.bold: true
        wrapMode: Text.Wrap
        textFormat: Text.PlainText
    }
}
