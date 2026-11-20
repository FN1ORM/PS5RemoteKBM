// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#pragma once
#include "kbmengine.h"
#include <QObject>
#include <QAbstractNativeEventFilter>
#include <QPointer>
#include <QElapsedTimer>
#include <QTimer>
#include <QMap>
#include <functional>
class QWindow;
class QDialog;
class QWidget;
class QLabel;
class StreamSession;
class KbmInput : public QObject, public QAbstractNativeEventFilter {
    QWindow *window;
    QPointer<StreamSession> session;
    QDialog *dialog=nullptr;
    QWidget *preview=nullptr;
    QLabel *status=nullptr;
    kbm::Engine engine;
    kbm::Report report;
    QMap<int,int> keys, mouseButtons;
    QString profile="Default";
    QTimer timer;
    QElapsedTimer clock;
    bool captured=false, testing=false, local_test_mode=false;
    int binding=-1;
    std::function<void()> rebuildBindings;
    void buildControls(int initialPage, int initialAction);
    void load();
    void save();
    void defaults();
    void tick();
    void publish();
    bool capture(bool test);
public:
    explicit KbmInput(QWindow *window);
    ~KbmInput() override;
    void setSession(StreamSession *next);
    void release();
    void showControls();
    int runLocalTest();
    bool isCaptured() const { return captured; }
    bool eventFilter(QObject *object,QEvent *event) override;
    bool nativeEventFilter(const QByteArray &type,void *message,qintptr *result) override;
};
