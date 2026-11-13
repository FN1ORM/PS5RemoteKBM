import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

// Bound the entire popup, including its header/footer, to the current viewport.
Dialog {
    id: dialog
    popupType: Popup.Item
    parent: Overlay.overlay
    property real preferredWidth: 640
    property bool showHeading: false
    onAboutToShow: showHeading = true
    onAboutToHide: showHeading = false
    width: Math.max(0, Math.min(preferredWidth, parent ? parent.width - 24 : preferredWidth))
    property real preferredHeight: implicitHeight
    height: Math.max(0, Math.min(preferredHeight, parent ? parent.height - 24 : preferredHeight))
    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? Math.round((parent.height - height) / 2) : 0
    padding: 20
    font.pixelSize: 16
    modal: true
    Material.roundedScale: Material.MediumScale
    background: Rectangle {
        color: "#202725"
        radius: 12
        border.color: "#485851"
    }
    header: Label {
        text: dialog.title
        visible: dialog.showHeading && text.length > 0
        color: "#edf5f0"
        padding: 20
        bottomPadding: 8
        font.pixelSize: 20
        font.bold: true
        wrapMode: Text.Wrap
        textFormat: Text.PlainText
    }
}
