// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQuickItem>
#include <QEventLoop>
#include <QTimer>
#include <QKeyEvent>
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
    qInstallMessageHandler([](QtMsgType, const QMessageLogContext &, const QString &message) {
        std::fprintf(stderr, "%s\n", qPrintable(message));
    });
    QTemporaryDir fixture;
    QFile backend(fixture.filePath("TestChiaki.qml"));
    check(backend.open(QIODevice::WriteOnly), "registration mock can be created");
    backend.write(R"(
pragma Singleton
import QtQml
import QtQuick.Window
QtObject {
    property QtObject settings: QtObject {
        property bool streamerMode: false
        property string psnAccountId: "AAAAAAAAAAA="
        property string currentProfile: ""
        property string psnAuthToken: ""
        property bool showStreamStats: false
        property int audioVideoDisabled: 0
        property int wifiDroppedNotif: 5
        property int audioVolume: 100
        property int videoPreset: 0
        property real sZoomFactor: 0
        property int displayTargetPrim: 0
        property int displayTargetTrc: 0
        property int displayTargetPeak: 1000
        property int displayTargetContrast: 1000
    }
    signal sessionError(string title, string text)
    signal sessionPinDialogRequested()
    signal sessionStopDialogRequested()
    property Window window
    property QtObject session: QtObject {
        property bool connected: true
        property bool viewOnly: false
        property bool muted: true
        property bool cantDisplay: false
        property real measuredBitrate: 5.0
        property real averagePacketLoss: 0.02
        property real initialRoundTripMs: 100
    }
    property bool autoConnect: false
    property bool discoveryEnabled: true
    property var hosts: [
        { display: true, mac: "fixture", name: "Test console", discovered: true, address: "192.0.2.1", registered: true, ps5: true, state: "ready" },
        { display: true, mac: "fixture", name: "Test console", manual: true, manualId: 4, address: "192.0.2.2", registered: true, ps5: true },
        { display: true, mac: "fixture", name: "Test console", manual: true, manualId: 7, address: "192.0.2.3", registered: true, ps5: true },
        { display: true, mac: "fixture", name: "Test console", duid: "fixture", registered: true, ps5: true },
        { display: false, mac: "hidden", name: "Hidden console", discovered: true, address: "192.0.2.4", ps5: true }
    ]
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
    QFile enums(fixture.filePath("TestWindowEnums.qml"));
    check(enums.open(QIODevice::WriteOnly), "stream enum fixture can be created");
    enums.write(R"(
pragma Singleton
import QtQml
QtObject {
    enum VideoMode { Normal, Zoom, Stretch }
    enum VideoPreset { Default, HighQuality, HighQualitySpatial, HighQualityAdvancedSpatial, Custom }
})");
    enums.close();
    qmlRegisterSingletonType(QUrl::fromLocalFile(enums.fileName()), "org.streetpea.chiaking", 1, 0, "ChiakiWindow");
    QQmlApplicationEngine engine;
    engine.loadData(R"(
import QtQuick
import QtQuick.Controls
import "." as UI
ApplicationWindow {
    width: 800; height: 600; visible: true
    UI.ConfirmDialog {
        objectName: "confirmation"
        title: "Remove saved IP?"
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
    auto header = confirmation->property("header").value<QQuickItem *>();
    check(header && !header->property("visible").toBool(), "a closed confirmation hides its title");
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
    engine.loadData(R"(
import QtQuick
import QtQuick.Controls
import "." as UI
ApplicationWindow {
    id: root
    width: 420
    height: 300
    visible: true
    property bool initialAsk: true
    UI.MainView { id: dashboard; anchors.fill: parent }
    UI.DisplaySettingsDialog { anchors.fill: parent }
    function verifyRoutes() {
        const card = dashboard.consoles[0];
        if (dashboard.consoles.length !== 1 || card.routes.length !== 4)
            return false;
        if (card.routes[0].index !== 0 || card.routes[1].index !== 2)
            return false;
        return dashboard.resolveRoute("saved:4") === 1 && dashboard.resolveRoute("psn:fixture") === 3;
    }
})", QUrl::fromLocalFile(QStringLiteral(KBM_QML_DIR "/DashboardTest.qml")));
    check(engine.rootObjects().size() == 3, "dashboard and display settings load");
    auto dashboardWindow = qobject_cast<QQuickWindow *>(engine.rootObjects().last());
    QVariant routes;
    QMetaObject::invokeMethod(dashboardWindow, "verifyRoutes", Q_RETURN_ARG(QVariant, routes));
    check(routes.toBool(), "one console card includes distinct local, saved IP and PSN routes");
    auto displayScroll = dashboardWindow->findChild<QQuickItem *>("displaySettingsScroll");
    auto displayForm = dashboardWindow->findChild<QQuickItem *>("displaySettingsForm");
    check(displayScroll && displayForm, "display form has a scroll viewport");
    for (auto size : {QSize(420, 300), QSize(1024, 600), QSize(480, 360)}) {
        dashboardWindow->resize(size);
        settle();
        check(displayForm->x() >= 0 && displayForm->x() + displayForm->width() <= displayScroll->width(),
              "display controls fit in the viewport after resizing");
    }
    engine.loadData(R"(
import QtQuick
import QtQuick.Controls
import QtQuick.Window
import org.streetpea.chiaking
import "." as UI
Window {
    id: root
    width: 480
    height: 340
    visible: true
    property int runtimeRendererBackend: 0
    property bool streamPanelOpen: false
    property bool hasVideo: true
    property bool keepVideo: true
    property bool loadingTransitionComplete: true
    property int droppedFrames: 1
    property int videoMode: 0
    property int videoPreset: 0
    property int grabs: 0
    signal menuRequested()
    function grabInput() { root.grabs += 1; }
    function releaseInput() { root.grabs -= 1; }
    function dismissStreamPanel() { streamPanelOpen = false; }
    function nativeInputCaptured() { return false; }
    function setStatsOverlayActive(active) {}
    function setOverlayInteractionActive(active) {}
    function requestOverlayUpdate() {}
    Component.onCompleted: Chiaki.window = root
    Loader {
        anchors.fill: parent
        active: Chiaki.window === root
        sourceComponent: UI.StreamView {}
    }
})", QUrl::fromLocalFile(QStringLiteral(KBM_QML_DIR "/StreamTest.qml")));
    check(engine.rootObjects().size() == 4, "stream view loads");
    auto streamWindow = qobject_cast<QQuickWindow *>(engine.rootObjects().last());
    settle();
    auto menuController = streamWindow->findChild<QObject *>("streamMenuController");
    auto menuWindow = streamWindow->findChild<QQuickWindow *>("separateStreamMenu");
    check(menuController && menuWindow, "both stream-menu presentations are available");
    for (int renderer : {0, 1}) {
        streamWindow->setProperty("runtimeRendererBackend", renderer);
        for (int cycle = 0; cycle < 3; ++cycle) {
            QMetaObject::invokeMethod(streamWindow, "menuRequested");
            settle();
            if (!menuController->property("open").toBool() || streamWindow->property("grabs").toInt() != 1)
                std::fprintf(stderr, "renderer %d, cycle %d, open %d, grabs %d\n", renderer, cycle,
                             menuController->property("open").toBool(), streamWindow->property("grabs").toInt());
            check(menuController->property("open").toBool() && streamWindow->property("grabs").toInt() == 1,
                  "stream menu opens and grabs input once");
            check(menuWindow->isVisible() == (renderer == 1), "stream menu uses the correct window for the renderer");
            if (renderer == 1) {
                QKeyEvent press(QEvent::KeyPress, Qt::Key_O, Qt::ControlModifier);
                QCoreApplication::sendEvent(menuWindow, &press);
            } else {
                QMetaObject::invokeMethod(streamWindow, "menuRequested");
            }
            settle();
            check(!menuController->property("open").toBool() && streamWindow->property("grabs").toInt() == 0,
                  "stream menu closes and releases input on every cycle");
        }
    }
    std::puts("PASS: responsive dialogs, registration retry, display settings, grouped routes and repeated stream-menu toggles");
}
