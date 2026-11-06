// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQuickItem>
#include <QEventLoop>
#include <QTimer>
#include <cstdio>
#include <cstdlib>
static void settle() { QEventLoop loop;QTimer::singleShot(80,&loop,&QEventLoop::quit);loop.exec(); }
static void check(bool ok,const char *name) {if(!ok){std::fprintf(stderr,"FAIL: %s\n",name);std::exit(1);}}
int main(int argc,char **argv) {
    qputenv("QT_QUICK_CONTROLS_STYLE","Material");
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
    QGuiApplication app(argc,argv); QQmlApplicationEngine engine;
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
    std::puts("PASS: wrapped long popup scrolls and follows live viewport resizing; short popup shrinks");
}
