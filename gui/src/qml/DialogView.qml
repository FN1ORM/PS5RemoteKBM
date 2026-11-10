import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

Item {
    id: dialog
    property alias header: headerLabel.text
    property alias title: titleLabel.text
    property alias buttonText: okButton.text
    property alias buttonEnabled: okButton.enabled
    property alias buttonVisible: okButton.visible
    property bool backEnabled: true
    property Item restoreFocusItem
    default property Item mainItem: null

    signal accepted()
    signal rejected()

    function close() {
        root.closeDialog();
    }

    Keys.onEscapePressed: if (backEnabled) close()

    Keys.onMenuPressed: {
        if (okButton.enabled)
            okButton.clicked()
    }

    StackView.onDeactivating: {
        restoreFocusItem = Window.window.activeFocusItem;
    }

    StackView.onActivated: {
        if (!restoreFocusItem) {
            let item = mainItem ? mainItem.nextItemInFocusChain() : null;
            if (item)
                item.forceActiveFocus(Qt.TabFocusReason);
        } else {
            restoreFocusItem.forceActiveFocus(Qt.TabFocusReason);
            restoreFocusItem = null;
        }
    }

    onMainItemChanged: {
        if (mainItem) {
            mainItem.parent = contentItem;
            mainItem.anchors.fill = contentItem;
        }
    }

    ToolBar {
        id: toolBar
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: headerLabel.visible ? 88 : 64
        RowLayout {
            anchors { fill: parent; margins: 8 }
            spacing: 12
            Button {
                text: "Back"
                Layout.preferredWidth: 64
                enabled: dialog.backEnabled
                flat: true
                focusPolicy: Qt.NoFocus
                onClicked: { dialog.rejected(); dialog.close(); }
            }
            ColumnLayout {
                Layout.fillWidth: true
                Label {
                    id: titleLabel
                    Layout.fillWidth: true
                    font.pixelSize: 20
                    font.bold: true
                    wrapMode: Text.Wrap
                }
                Label {
                    id: headerLabel
                    Layout.fillWidth: true
                    visible: text.length > 0
                    font.pixelSize: 12
                    wrapMode: Text.Wrap
                }
            }
            Button {
                id: okButton
                font.pixelSize: 16
                padding: 12
                focusPolicy: Qt.NoFocus
                onClicked: dialog.accepted()
            }
        }
    }

    Item {
        id: contentItem
        anchors {
            top: toolBar.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
    }
}
