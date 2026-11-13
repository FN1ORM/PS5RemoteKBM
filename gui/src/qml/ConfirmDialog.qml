import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

FocusScope {
    id: dialog
    anchors.fill: parent
    z: 100
    visible: false

    property string title
    property string text
    property var callback
    property var rejectCallback
    property Item restoreFocusItem

    function open(returnFocusTo = null) {
        if (!visible)
            restoreFocusItem = returnFocusTo || (Window.window ? Window.window.activeFocusItem : null);
        visible = true;
        noButton.forceActiveFocus(Qt.TabFocusReason);
    }

    function close() {
        const previous = restoreFocusItem;
        visible = false;
        restoreFocusItem = null;
        callback = null;
        rejectCallback = null;
        if (previous && previous.visible)
            previous.forceActiveFocus(Qt.TabFocusReason);
    }

    function accept() {
        if (!visible)
            return;
        const action = callback;
        close();
        if (action) action();
    }

    function reject() {
        if (!visible)
            return;
        const action = rejectCallback;
        close();
        if (action) action();
    }

    Keys.onEscapePressed: reject()
    Rectangle {
        anchors.fill: parent
        color: "#99000000"
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
            onWheel: wheel => wheel.accepted = true
        }
    }

    Rectangle {
        id: panel
        objectName: "confirmationPanel"
        anchors.centerIn: parent
        width: Math.max(0, Math.min(560, dialog.width - 24))
        height: Math.max(0, Math.min(dialog.height - 24, body.implicitHeight + buttons.implicitHeight + 44))
        radius: 12
        color: "#202725"
        border.color: "#485851"

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            ScrollView {
                id: scroller
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                contentWidth: availableWidth
                contentHeight: body.implicitHeight
                activeFocusOnTab: false

                ColumnLayout {
                    id: body
                    width: scroller.availableWidth
                    spacing: 8

                    Label {
                        objectName: "confirmationTitle"
                        Layout.fillWidth: true
                        text: dialog.title
                        font.bold: true
                        font.pixelSize: 20
                        wrapMode: Text.Wrap
                        textFormat: Text.PlainText
                    }

                    Label {
                        Layout.fillWidth: true
                        text: dialog.text
                        wrapMode: Text.Wrap
                        textFormat: Text.PlainText
                    }
                }
            }

            RowLayout {
                id: buttons
                Layout.alignment: Qt.AlignRight
                Button {
                    id: noButton
                    objectName: "confirmationNo"
                    text: qsTr("No")
                    onClicked: dialog.reject()
                    Keys.onReturnPressed: dialog.reject()
                    KeyNavigation.left: yesButton
                    KeyNavigation.backtab: yesButton
                    KeyNavigation.tab: yesButton
                }
                Button {
                    id: yesButton
                    text: qsTr("Yes")
                    onClicked: dialog.accept()
                    Keys.onReturnPressed: dialog.accept()
                    KeyNavigation.right: noButton
                    KeyNavigation.tab: noButton
                    KeyNavigation.backtab: noButton
                }
            }
        }
    }
}
