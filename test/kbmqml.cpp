// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQuickItem>
#include <QEventLoop>
#include <QTimer>
#include <QFile>
#include <QTemporaryDir>
#include <QtQml/qqml.h>
#include <cstdio>
#include <cstdlib>
static void settle() { QEventLoop loop;QTimer::singleShot(80,&loop,&QEventLoop::quit);loop.exec(); }
static void check(bool ok,const char *name) {if(!ok){std::fprintf(stderr,"FAIL: %s\n",name);std::exit(1);}}
int main(int argc,char **argv) {
    qputenv("QT_QUICK_CONTROLS_STYLE","Material");
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
    QGuiApplication app(argc,argv);
    QTemporaryDir fixture;
    QFile backend(fixture.filePath("TestChiaki.qml"));
    check(backend.open(QIODevice::WriteOnly), "registration mock can be created");
    backend.write(R"(
pragma Singleton
import QtQml
QtObject {
    property QtObject settings: QtObject {
        property bool streamerMode: false
        property string psnAccountId: "AAAAAAAAAAA="
    }
    property var pending
    function registerHost(host, account, pin, consolePin, broadcast, target, callback) {
        pending = callback
        return true
    }
    function complete(success) {
        if (!success)
            pending("[E] Regist timed out waiting for search response", true, false)
        pending("", success, true)
    }
})");
    backend.close();
    qmlRegisterSingletonType(QUrl::fromLocalFile(backend.fileName()), "org.streetpea.chiaking", 1, 0, "Chiaki");
    QQmlApplicationEngine engine;
    engine.loadData(R"(
import QtQuick
import QtQuick.Controls
import "." as UI
ApplicationWindow {
    width: 800; height: 600; visible: true
    UI.ConfirmDialog {
        objectName: "confirmation"
        property int acceptedCount: 0
        property int rejectedCount: 0
        callback: function() { acceptedCount++ }
        rejectCallback: function() { rejectedCount++ }
        text: "Confirm without losing callback behavior"
    }
    UI.MessageDialog {
        objectName: "message"
        title: "A long connection failure title that must wrap within a small window"
        text: ("A long diagnostic explanation with a URL https://example.invalid/very/long/path/" + "x".repeat(120) + "\n").repeat(24)
        Component.onCompleted: open()
    }
})", QUrl::fromLocalFile(QStringLiteral(KBM_QML_DIR "/LayoutTest.qml")));
    check(!engine.rootObjects().isEmpty(),"QML component loads");
    auto window=qobject_cast<QQuickWindow*>(engine.rootObjects().first());
    auto popup=window->findChild<QObject*>("message");check(popup,"message exists");
    for(auto size:{QSize(1280,720),QSize(800,600),QSize(480,360),QSize(1024,768)}) {
        window->resize(size);settle();
        double w=popup->property("width").toDouble(), h=popup->property("height").toDouble();
        double x=popup->property("x").toDouble(), y=popup->property("y").toDouble();
        check(w>0 && h>0 && x>=0 && y>=0 && x+w<=size.width() && y+h<=size.height(),"open popup stays within resized window");
        auto content=popup->property("contentItem").value<QObject*>();
        check(content && content->property("contentHeight").toDouble()>content->property("height").toDouble(),"long message has scrollable overflow");
    }
    popup->setProperty("text","Short message");settle();
    check(popup->property("height").toDouble()<400,"short message shrinks after long message");
    QMetaObject::invokeMethod(popup,"close");settle();
    auto confirmation=window->findChild<QObject*>("confirmation");check(confirmation,"confirmation loads");
    QMetaObject::invokeMethod(confirmation,"open");settle();
    QMetaObject::invokeMethod(confirmation,"accept");settle();
    check(confirmation->property("acceptedCount").toInt()==1,"yes callback fires once");
    QMetaObject::invokeMethod(confirmation,"open");settle();
    check(!confirmation->property("newDialogOpen").toBool(),"reused confirmation resets state");
    QMetaObject::invokeMethod(confirmation,"reject");settle();
    check(confirmation->property("rejectedCount").toInt()==1,"no callback fires once");
    check(!confirmation->property("restoreFocusOnClose").toBool(),"confirmation controls focus transfer to next view");
    engine.loadData(R"(
import QtQuick
import QtQuick.Controls
import QtQuick.Window
import org.streetpea.chiaking
import "." as UI
Window {
    id: root
    width: 480
    height: 360
    visible: true
    property bool closedRegistration: false
    function closeDialog() { closedRegistration = true }
    function failRegistration() { Chiaki.complete(false) }
    function finishRegistration() { Chiaki.complete(true) }
    StackView {
        anchors.fill: parent
        initialItem: UI.RegistDialog {
            objectName: "registration"
            host: "192.0.2.1"
        }
    }
})", QUrl::fromLocalFile(QStringLiteral(KBM_QML_DIR "/RegistrationTest.qml")));
    check(engine.rootObjects().size() == 2, "registration loads in a plain Quick window");
    auto registrationWindow = engine.rootObjects().last();
    auto registration = registrationWindow->findChild<QObject*>("registration");
    check(registration, "registration page exists");
    auto pin = registration->findChild<QQuickItem*>("remotePlayPin");
    auto result = registration->findChild<QQuickItem*>("registrationResult");
    check(pin && result, "registration form and result exist");
    pin->setProperty("text", "12345678");
    check(registration->property("buttonEnabled").toBool(), "valid form permits registration");
    QMetaObject::invokeMethod(registration, "accepted");
    settle();
    check(result->isVisible() && result->width() > 0 && result->height() > 0 && !pin->isVisible(),
          "result has its own area and hides the PIN form");
    check(!registration->property("buttonEnabled").toBool(), "registration cannot be started twice");
    QMetaObject::invokeMethod(registrationWindow, "failRegistration");
    check(registration->property("searchTimedOut").toBool(), "search timeout is recognized");
    check(registration->property("buttonText").toString() == "Try again", "timeout offers retry");
    QMetaObject::invokeMethod(registration, "accepted");
    check(!result->isVisible() && pin->isVisible() && pin->property("text").toString().isEmpty(),
          "retry returns to form with the expired PIN cleared");
    pin->setProperty("text", "87654321");
    QMetaObject::invokeMethod(registration, "accepted");
    QMetaObject::invokeMethod(registrationWindow, "finishRegistration");
    QMetaObject::invokeMethod(registration, "accepted");
    check(registrationWindow->property("closedRegistration").toBool(), "success can return to the app");
    std::puts("PASS: responsive dialogs and registration failure/retry/success in a plain Quick window");
}
