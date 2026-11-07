// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#include "kbminput.h"
#include "kbmprofiles.h"
#include "kbmwidgets.h"
#include "streamsession.h"
#include <QApplication>
#include <QWindow>
#include <QDialog>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QScrollArea>
#include <QKeyEvent>
#include <QKeySequence>
#include <QSettings>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QDebug>
#include <QScreen>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace {
const char *names[kbm::Count]={"Cross","Circle","Square","Triangle","D-pad left","D-pad right",
    "D-pad up","D-pad down","L1","R1","L3","R3","Options","Share","Touchpad click","PS",
    "L2","R2","Move up","Move down","Move left","Move right","Walk"};
class StickPreview : public QWidget {
public:
    kbm::Report report;
    bool captured=false;
    explicit StickPreview(QWidget *p):QWidget(p) { setMinimumSize(280,160); setFocusPolicy(Qt::StrongFocus); }
    void paintEvent(QPaintEvent*) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing); p.fillRect(rect(),QColor(25,29,37));
        p.setPen(Qt::white); p.drawText(12,22,captured?"CAPTURED — Esc releases":"Released — click Test capture below");
        for(int n=0;n<2;++n) {
            QPointF center(width()*(n?.72:.28),93); p.setPen(QColor(140,150,170)); p.drawEllipse(center,42,42);
            const double x=(n?report.rx:report.lx)/32767.0, y=(n?report.ry:report.ly)/32767.0;
            p.setBrush(QColor(0,170,255)); p.drawEllipse(center+QPointF(x*38,y*38),5,5); p.setBrush(Qt::NoBrush);
            p.drawText(QRectF(center.x()-85,139,170,20),Qt::AlignCenter,n?"Right stick / mouse":"Left stick / WASD");
        }
    }
};
QSettings controlsSettings() { return QSettings("PS5RemoteKBM","Controls"); }
}

KbmInput::KbmInput(QWindow *w):QObject(w),window(w) {
    clock.start(); load();
    qApp->installEventFilter(this); qApp->installNativeEventFilter(this);
    connect(&timer,&QTimer::timeout,this,[this]{tick();}); timer.setTimerType(Qt::PreciseTimer); timer.start(100);
    connect(qApp,&QGuiApplication::applicationStateChanged,this,[this](Qt::ApplicationState s){if(s!=Qt::ApplicationActive) release();});
    connect(qApp,&QCoreApplication::aboutToQuit,this,[this]{release();});
}
KbmInput::~KbmInput() { release(); qApp->removeNativeEventFilter(this); qApp->removeEventFilter(this); delete dialog; }
void KbmInput::defaults() {
    engine.config=kbm::Config{}; keys.clear(); mouseButtons.clear();
    const int bindings[kbm::Count]={Qt::Key_Space,Qt::Key_C,Qt::Key_E,Qt::Key_R,Qt::Key_Left,Qt::Key_Right,
        Qt::Key_Up,Qt::Key_Down,Qt::Key_Q,Qt::Key_F,Qt::Key_Shift,Qt::Key_V,Qt::Key_Return,0,Qt::Key_Tab,
        Qt::Key_P,0,0,Qt::Key_W,Qt::Key_S,Qt::Key_A,Qt::Key_D,Qt::Key_Control};
    for(int a=0;a<kbm::Count;++a) if(bindings[a]) keys.insert(a,bindings[a]);
    mouseButtons.insert(kbm::R2,1); mouseButtons.insert(kbm::L2,2);
}
void KbmInput::load() {
    defaults(); auto s=controlsSettings(); profile=s.value("active","Default").toString(); s.beginGroup("profiles/"+profile);
    KbmProfiles::load(s,engine.config,keys,mouseButtons);
}
void KbmInput::save() {
    engine.config.sanitize(); auto s=controlsSettings(); s.setValue("active",profile); s.beginGroup("profiles/"+profile);
    KbmProfiles::save(s,engine.config,keys,mouseButtons);
    s.sync();
    if(s.status()!=QSettings::NoError) QMessageBox::warning(dialog,"Controls","Could not save the input profile.");
}
void KbmInput::setSession(StreamSession *next) {
    release(); session=next;
    if(next) connect(next,&StreamSession::SessionQuit,this,[this]{release();});
}
void KbmInput::publish() {
    if(!session || testing) return;
    ChiakiControllerState state; chiaki_controller_state_set_idle(&state);
    state.buttons=report.buttons; state.l2_state=report.l2; state.r2_state=report.r2;
    state.left_x=report.lx; state.left_y=report.ly; state.right_x=report.rx; state.right_y=report.ry;
    session->SetNativeControllerState(state);
}
void KbmInput::release() {
    const bool wasCaptured=captured; engine.reset(); report={}; publish(); captured=false; testing=false;
    timer.setInterval(100);
#ifdef Q_OS_WIN
    if(wasCaptured) {
        ClipCursor(nullptr);
        RAWINPUTDEVICE device{1,2,RIDEV_REMOVE,nullptr}; RegisterRawInputDevices(&device,1,sizeof(device));
    }
#endif
    if(wasCaptured) { window->setCursor(Qt::ArrowCursor); if(preview) preview->unsetCursor(); }
    if(wasCaptured) window->setTitle("PS5 Remote KBM — released (F1 capture, F2 controls)");
    if(wasCaptured) qInfo("Native input released; all controls neutral");
    if(preview) { auto p=static_cast<StickPreview*>(preview); p->captured=false; p->report={}; p->update(); }
    if(status) status->setText("Released. F1 captures in the stream; Esc always releases. F2 opens controls.");
}
bool KbmInput::capture(bool test) {
    release();
    if(!test && (!session || session->IsViewOnly() || !window->isActive() || window->visibility()==QWindow::Minimized)) return false;
#ifdef Q_OS_WIN
    HWND target=reinterpret_cast<HWND>(test?preview->winId():window->winId());
    RAWINPUTDEVICE device{1,2,0,target};
    if(!RegisterRawInputDevices(&device,1,sizeof(device))) {
        qWarning("Raw mouse registration failed (%lu)",GetLastError()); return false;
    }
    RECT bounds; GetClientRect(target,&bounds); MapWindowPoints(target,nullptr,reinterpret_cast<POINT*>(&bounds),2);
    if(!ClipCursor(&bounds)) { device.dwFlags=RIDEV_REMOVE; device.hwndTarget=nullptr; RegisterRawInputDevices(&device,1,sizeof(device)); return false; }
#else
    return false;
#endif
    testing=test; captured=true; engine.reset(); timer.setInterval(8);
    window->setTitle("PS5 Remote KBM — MOUSE CAPTURED (Esc releases)");
    if(test) { preview->setFocus(); preview->setCursor(Qt::BlankCursor); }
    else window->setCursor(Qt::BlankCursor);
    if(status) status->setText(test?"Test capture — no console reports sent. Esc releases.":"CAPTURED — Esc releases");
    qInfo("Native input captured (%s)",test?"local test":"stream");
    return true;
}
void KbmInput::tick() {
    if(captured) {
#ifdef Q_OS_WIN
        HWND target=reinterpret_cast<HWND>(testing?preview->winId():window->winId());
        if(GetAncestor(target,GA_ROOT)!=GetForegroundWindow() || IsIconic(GetAncestor(target,GA_ROOT))) {release(); return;}
#endif
        report=engine.tick(clock.nsecsElapsed()/1e9); publish();
    }
    if(preview) { auto p=static_cast<StickPreview*>(preview); p->report=report; p->captured=captured; p->update(); }
    if(status && captured) status->setText(QString("%1 | LX %2 LY %3 RX %4 RY %5 | L2 %6 R2 %7 | buttons 0x%8 | Esc releases")
        .arg(testing?"LOCAL TEST":"STREAM").arg(report.lx).arg(report.ly).arg(report.rx).arg(report.ry).arg(report.l2).arg(report.r2).arg(report.buttons,0,16));
}
bool KbmInput::nativeEventFilter(const QByteArray&,void *message,qintptr*) {
#ifdef Q_OS_WIN
    auto msg=static_cast<MSG*>(message);
    if(!captured || msg->message!=WM_INPUT) return false;
    UINT size=0;
    if(GetRawInputData(reinterpret_cast<HRAWINPUT>(msg->lParam),RID_INPUT,nullptr,&size,sizeof(RAWINPUTHEADER))!=0 || size>4096) return false;
    QByteArray data(int(size),Qt::Uninitialized);
    if(GetRawInputData(reinterpret_cast<HRAWINPUT>(msg->lParam),RID_INPUT,data.data(),&size,sizeof(RAWINPUTHEADER))!=size) return false;
    auto raw=reinterpret_cast<const RAWINPUT*>(data.constData());
    if(raw->header.dwType!=RIM_TYPEMOUSE) return false;
    const auto &m=raw->data.mouse;
    if(!(m.usFlags&MOUSE_MOVE_ABSOLUTE)) engine.mouse(m.lLastX,m.lLastY,clock.nsecsElapsed()/1e9);
    const USHORT downs[]={RI_MOUSE_LEFT_BUTTON_DOWN,RI_MOUSE_RIGHT_BUTTON_DOWN,RI_MOUSE_MIDDLE_BUTTON_DOWN,RI_MOUSE_BUTTON_4_DOWN,RI_MOUSE_BUTTON_5_DOWN};
    const USHORT ups[]={RI_MOUSE_LEFT_BUTTON_UP,RI_MOUSE_RIGHT_BUTTON_UP,RI_MOUSE_MIDDLE_BUTTON_UP,RI_MOUSE_BUTTON_4_UP,RI_MOUSE_BUTTON_5_UP};
    for(int b=0;b<5;++b) for(auto it=mouseButtons.cbegin();it!=mouseButtons.cend();++it) if(it.value()==b+1) {
        if(m.usButtonFlags&downs[b]) engine.key(10000+b*kbm::Count+it.key(),kbm::Action(it.key()),true);
        if(m.usButtonFlags&ups[b]) engine.key(10000+b*kbm::Count+it.key(),kbm::Action(it.key()),false);
    }
    tick();
#endif
    // Let DefWindowProc consume foreground WM_INPUT for OS resource cleanup.
    return false;
}
bool KbmInput::eventFilter(QObject *object,QEvent *event) {
    if(object==window && (event->type()==QEvent::FocusOut || event->type()==QEvent::WindowDeactivate
        || event->type()==QEvent::Hide || event->type()==QEvent::Move || event->type()==QEvent::Resize || event->type()==QEvent::WindowStateChange)) release();
    if(testing && object==preview && (event->type()==QEvent::FocusOut || event->type()==QEvent::Hide)) release();
    if(event->type()==QEvent::KeyPress || event->type()==QEvent::KeyRelease) {
        auto e=static_cast<QKeyEvent*>(event); bool down=event->type()==QEvent::KeyPress;
        if(binding>=0 && down && !e->isAutoRepeat()) {
            if(e->key()!=Qt::Key_Escape && e->key()!=Qt::Key_F1 && e->key()!=Qt::Key_F2 && e->key()!=Qt::Key_F3 && e->key()!=Qt::Key_F4) keys[binding]=e->key();
            binding=-1; if(rebuildBindings) rebuildBindings(); save(); return true;
        }
        if(object==window && down && !e->isAutoRepeat() && e->key()==Qt::Key_F2) {showControls(); return true;}
        if (object == window && session && e->key() == Qt::Key_F5 && e->modifiers() == Qt::AltModifier) {
            if (down && !e->isAutoRepeat())
                QMetaObject::invokeMethod(window, "toggleViewOnly", Qt::QueuedConnection);
            return true;
        }
        if(object==window && down && !e->isAutoRepeat() && (e->key()==Qt::Key_F3 || e->key()==Qt::Key_F4)) {
            release(); QMetaObject::invokeMethod(window,e->key()==Qt::Key_F3?"showUserManual":"showConnectionTools",Qt::QueuedConnection); return true;
        }
        if(captured && down && e->key()==Qt::Key_Escape) {release(); return true;}
        if(object==window && down && !e->isAutoRepeat() && e->key()==Qt::Key_F1) {if(captured) release(); else capture(false); return true;}
        if(captured) {
            if(e->key()==Qt::Key_Alt || e->key()==Qt::Key_Meta) {release(); return false;}
            if(!e->isAutoRepeat()) for(auto it=keys.cbegin();it!=keys.cend();++it) if(it.value()==e->key())
                engine.key(e->key()*kbm::Count+it.key(),kbm::Action(it.key()),down);
            tick(); return true;
        }
    }
    if(captured && (event->type()==QEvent::MouseButtonPress || event->type()==QEvent::MouseButtonRelease
        || event->type()==QEvent::MouseButtonDblClick || event->type()==QEvent::MouseMove || event->type()==QEvent::Wheel)) return true;
    return false;
}
void KbmInput::showControls() {
    release();
    if(dialog) {dialog->show(); dialog->raise(); dialog->activateWindow(); return;}
    dialog=new QDialog; styleKbmDialog(dialog); dialog->setWindowTitle("PS5 Remote KBM — Controls and local input test"); dialog->resize(620,760);
    if(window->screen()) dialog->resize(dialog->size().boundedTo(window->screen()->availableGeometry().size()-QSize(40,60)));
    auto layout=new QVBoxLayout(dialog);
    auto help=new QLabel("F1 captures during streaming. Esc, Alt+Tab, focus loss and disconnect release all input.\nF2 opens this panel. Mouse aiming emulates a stick; each game limits camera speed."); help->setWordWrap(true); layout->addWidget(help);
    auto row=new QHBoxLayout; auto profiles=new QComboBox; auto s=controlsSettings(); s.beginGroup("profiles");
    auto list=s.childGroups(); if(!list.contains(profile)) list.prepend(profile); profiles->addItems(list); profiles->setCurrentText(profile);
    row->addWidget(new QLabel("Profile")); row->addWidget(profiles,1); auto add=new QPushButton("Save as…"); auto reset=new QPushButton("Reset defaults"); row->addWidget(add); row->addWidget(reset); layout->addLayout(row);
    auto scroll=new QScrollArea; scroll->setWidgetResizable(true); auto body=new QWidget; auto form=new QFormLayout(body); form->setRowWrapPolicy(QFormLayout::WrapLongRows); scroll->setWidget(body); layout->addWidget(scroll,1);
    auto &c=engine.config;
    auto numeric=[&](const char *label,double &value,double lo,double hi,double step) {
        auto spin=new QDoubleSpinBox; spin->setRange(lo,hi); spin->setDecimals(3); spin->setSingleStep(step); spin->setValue(value);
        double *address=&value; connect(spin,&QDoubleSpinBox::valueChanged,this,[this,address](double v){release(); *address=v; save();}); form->addRow(label,spin);
    };
    numeric("X sensitivity",c.sensitivityX,.01,20,.1); numeric("Y sensitivity",c.sensitivityY,.01,20,.1);
    numeric("Response exponent (1 = linear)",c.exponent,.2,4,.1); numeric("Deadzone",c.deadzone,0,.95,.01);
    numeric("Anti-deadzone",c.antiDeadzone,0,1,.01); numeric("Maximum stick output",c.maximum,.05,1,.05);
    numeric("Smoothing window (ms)",c.smoothingMs,0,200,1); numeric("Recenter ramp (ms)",c.recenterMs,0,500,1);
    numeric("Walking tilt (hold Ctrl)",c.walkTilt,.05,1,.05);
    auto invert=new QCheckBox; invert->setChecked(c.invertY); connect(invert,&QCheckBox::toggled,this,[this](bool v){release();engine.config.invertY=v;save();}); form->addRow("Invert Y",invert);
    auto rolling=new QCheckBox;rolling->setChecked(c.rollingWindow);
    connect(rolling,&QCheckBox::toggled,this,[this](bool v){release();engine.config.rollingWindow=v;save();});
    form->addRow("Rolling mouse window (less stop drift)",rolling);
    auto precise=new QPushButton("Precise mouse preset (8 ms, no recenter tail)");form->addRow(precise);
    auto smooth=new QPushButton("Smooth stick preset (16 ms, 35 ms tail)");form->addRow(smooth);
    QList<QPushButton*> keyLabels;
    for(int a=0;a<kbm::Count;++a) {
        auto line=new QHBoxLayout; auto button=new QPushButton; keyLabels.append(button); line->addWidget(button,1);
        auto clear=new QPushButton("Clear"); line->addWidget(clear); auto mouse=new QComboBox; mouse->addItems({"No mouse","Left","Right","Middle","Side 1","Side 2"});
        mouse->setCurrentIndex(std::max(0,std::min(5,mouseButtons.value(a)))); line->addWidget(mouse);
        connect(button,&QPushButton::clicked,this,[this,a,button]{binding=a;button->setText("Press one key… Esc cancels");});
        connect(clear,&QPushButton::clicked,this,[this,a]{keys[a]=0;save();if(rebuildBindings)rebuildBindings();});
        connect(mouse,&QComboBox::currentIndexChanged,this,[this,a](int n){release();mouseButtons[a]=n;save();}); form->addRow(names[a],line);
    }
    rebuildBindings=[this,keyLabels]{for(int a=0;a<kbm::Count;++a) keyLabels[a]->setText(keys.value(a)?QKeySequence(keys.value(a)).toString():"Unbound");}; rebuildBindings();
    preview=new StickPreview(dialog); layout->addWidget(preview);
    status=new QLabel("Released. Test capture sends no console input."); status->setWordWrap(true); layout->addWidget(status);
    auto test=new QPushButton("Test capture (Esc releases)"); layout->addWidget(test); connect(test,&QPushButton::clicked,this,[this]{capture(true);});
    auto close=new QPushButton("Close"); layout->addWidget(close); connect(close,&QPushButton::clicked,dialog,&QDialog::reject);
    auto recreate=[this]{release();binding=-1;rebuildBindings={};auto old=dialog;dialog=nullptr;preview=nullptr;status=nullptr;old->hide();old->deleteLater();showControls();};
    connect(precise,&QPushButton::clicked,this,[this,recreate]{release();auto &c=engine.config;c.rollingWindow=true;c.smoothingMs=8;c.recenterMs=0;c.exponent=1;c.deadzone=0;c.antiDeadzone=0;save();recreate();});
    connect(smooth,&QPushButton::clicked,this,[this,recreate]{release();engine.config.rollingWindow=false;engine.config.smoothingMs=16;engine.config.recenterMs=35;save();recreate();});
    connect(profiles,&QComboBox::currentTextChanged,this,[this,recreate](const QString &name){auto s=controlsSettings();s.setValue("active",name);load();recreate();});
    connect(add,&QPushButton::clicked,this,[this,recreate]{bool ok=false;auto name=QInputDialog::getText(dialog,"Save profile","Name (letters, numbers, spaces, hyphen):",QLineEdit::Normal,"",&ok).trimmed();
        if(!ok || name.isEmpty()) return; for(auto ch:name) if(!ch.isLetterOrNumber() && ch!=' ' && ch!='-' && ch!='_') return;
        profile=name.left(64);save();recreate();});
    connect(reset,&QPushButton::clicked,this,[this,recreate]{defaults();save();recreate();});
    connect(dialog,&QDialog::finished,this,[this]{release();binding=-1;if(local_test_mode)qApp->quit();});
    dialog->show(); dialog->raise(); dialog->activateWindow();
}
int KbmInput::runLocalTest() { local_test_mode=true; qApp->setQuitOnLastWindowClosed(false); showControls(); return qApp->exec(); }
QString KbmInput::statusText() const {
    if(!captured) return "Released — F1 capture • F2 controls";
    return QString("Captured — Esc release | L %1,%2  R %3,%4").arg(report.lx).arg(report.ly).arg(report.rx).arg(report.ry);
}
