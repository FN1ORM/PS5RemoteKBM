// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#include "kbminput.h"
#include "kbmprofiles.h"
#include "kbmcontrolmap.h"
#include "kbmwidgets.h"
#include "qmlmainwindow.h"
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
#include <QHeaderView>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QTreeWidget>
#include <QScreen>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace {
class StickPreview : public QWidget {
public:
    kbm::Report report;
    bool captured=false;
    explicit StickPreview(QWidget *p):QWidget(p) { setMinimumSize(280,160); setFocusPolicy(Qt::StrongFocus); }
    void paintEvent(QPaintEvent*) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing); p.fillRect(rect(),QColor(25,29,37));
        const bool touchDown = report.buttons & CHIAKI_CONTROLLER_BUTTON_TOUCHPAD;
        p.setPen(touchDown ? QColor("#83edbf") : QColor("#d0dad5"));
        p.drawText(QRect(0, 8, width(), 22), Qt::AlignCenter,
                   touchDown ? "Touchpad click  ·  Pressed" : "Touchpad click  ·  Released");
        for(int n=0;n<2;++n) {
            QPointF center(width()*(n?.72:.28),93); p.setPen(QColor(140,150,170)); p.drawEllipse(center,42,42);
            const double x=(n?report.rx:report.lx)/32767.0, y=(n?report.ry:report.ly)/32767.0;
            p.setBrush(QColor(0,170,255)); p.drawEllipse(center+QPointF(x*38,y*38),5,5); p.setBrush(Qt::NoBrush);
            p.drawText(QRectF(center.x()-85,139,170,20),Qt::AlignCenter,n?"Right stick":"Left stick");
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
        Qt::Key_P,0,0,Qt::Key_W,Qt::Key_S,Qt::Key_A,Qt::Key_D,Qt::Key_Control,
        KbmProfiles::bindingKey(Qt::Key_8,Qt::KeypadModifier),KbmProfiles::bindingKey(Qt::Key_2,Qt::KeypadModifier),
        KbmProfiles::bindingKey(Qt::Key_4,Qt::KeypadModifier),KbmProfiles::bindingKey(Qt::Key_6,Qt::KeypadModifier)};
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
    if(wasCaptured) qInfo("Native input released; all controls neutral");
    if(preview) { auto p=static_cast<StickPreview*>(preview); p->captured=false; p->report={}; p->update(); }
    if(status) status->setText("Released. Choose Test capture to try your bindings locally.");
}
bool KbmInput::capture(bool test) {
    release();
    const auto *streamWindow = qobject_cast<QmlMainWindow *>(window);
    if (!test && (!session || session->IsViewOnly() || !streamWindow || streamWindow->isInputGrabbed()
                  || !window->isActive() || window->visibility() == QWindow::Minimized))
        return false;
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
    if (binding >= 0 && ((event->type() == QEvent::FocusOut && object->objectName() == "keyBinding")
            || (event->type() == QEvent::WindowDeactivate && object == dialog))) {
        binding = -1;
        if (rebuildBindings) rebuildBindings();
    }
    if(testing && object==preview && (event->type()==QEvent::FocusOut || event->type()==QEvent::Hide)) release();
    if(event->type()==QEvent::KeyPress || event->type()==QEvent::KeyRelease) {
        auto e=static_cast<QKeyEvent*>(event); bool down=event->type()==QEvent::KeyPress;
        if(binding>=0 && down && !e->isAutoRepeat()) {
            const int key=KbmProfiles::bindingKey(e->key(),e->modifiers());
            if (!KbmProfiles::reservedKey(key)) {
                keys[binding] = key;
                save();
            }
            binding = -1;
            if (rebuildBindings) rebuildBindings();
            return true;
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
        if (object == window && e->key() == Qt::Key_O && e->modifiers() == Qt::ControlModifier) {
            if (down)
                return false; // The main window handles the stream shortcut before game input.
            return true;
        }
        if(captured && down && e->key()==Qt::Key_Escape) {release(); return true;}
        if(object==window && down && !e->isAutoRepeat() && e->key()==Qt::Key_F1) {if(captured) release(); else capture(false); return true;}
        if(captured) {
            if(e->key()==Qt::Key_Alt || e->key()==Qt::Key_Meta) {release(); return false;}
            if(!e->isAutoRepeat()) {
                const int key=KbmProfiles::bindingKey(e->key(),e->modifiers());
                // Keep older non-keypad bindings usable on keypad keys unless
                // the keypad key has a more specific assignment.
                const int matched=std::find(keys.cbegin(),keys.cend(),key)!=keys.cend()?key:e->key();
                // Use the scan code where Windows provides it so the key-up
                // releases the same physical input across a Num Lock change.
                const int64_t physical=e->nativeScanCode()?e->nativeScanCode():key;
                for(auto it=keys.cbegin();it!=keys.cend();++it)
                    if(!down || it.value()==matched)
                        engine.key(-1-physical*kbm::Count-it.key(),kbm::Action(it.key()),down);
            }
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
    buildControls(0, kbm::Touchpad);
}

void KbmInput::buildControls(int initialPage, int initialAction)
{
    dialog = new QDialog;
    styleKbmDialog(dialog);
    dialog->setWindowTitle("KeyStream — Control mapping");
    dialog->setStyleSheet(dialog->styleSheet() + QStringLiteral(
        "QTabWidget::pane { border: 1px solid #3b4f47; background: #101917; }"
        "QTabBar::tab { background: #202725; color: #b8c9c1; padding: 8px 16px; }"
        "QTabBar::tab:selected { color: #8de2c1; border-bottom: 2px solid #78dfb6; }"
        "QPushButton#mapCallout { background: #1b2824; color: #d1dfd8; border: 1px solid #42584d; padding: 2px; }"
        "QPushButton#mapCallout:checked { color: #a0f1d2; background: #223d32; border-color: #83deba; }"
        "QPushButton#mapCallout:hover, QPushButton#mapCallout:focus { border-color: #b1f5dc; }"
        "QTreeWidget { background: #17201d; alternate-background-color: #1b2622; border: 0; }"
        "QTreeWidget::item { padding: 5px; } QTreeWidget::item:selected { background: #315644; color: white; }"
        "QHeaderView::section { background: #25342e; color: #c5d6cc; padding: 6px; border: 0; }"
        "QPushButton#keyBinding { border-color: #75bc9d; color: #b9f2d8; }"));
    dialog->resize(900, 650);
    if (window->screen())
        dialog->resize(dialog->size().boundedTo(window->screen()->availableGeometry().size() - QSize(40, 60)));
    auto layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(9);
    auto title = new QLabel("Control mapping");
    auto titleFont = title->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);
    auto row = new QHBoxLayout;
    auto profiles = new QComboBox;
    auto s = controlsSettings();
    s.beginGroup("profiles");
    auto list = s.childGroups();
    if (!list.contains(profile)) list.prepend(profile);
    profiles->addItems(list);
    profiles->setCurrentText(profile);
    row->addWidget(new QLabel("Profile"));
    row->addWidget(profiles, 1);
    auto add = new QPushButton("Save as…");
    auto reset = new QPushButton("Reset defaults");
    row->addWidget(add);
    row->addWidget(reset);
    layout->addLayout(row);

    auto tabs = new QTabWidget;
    layout->addWidget(tabs, 1);
    auto actions = new QComboBox;
    for (int a = 0; a < kbm::Count; ++a)
        actions->addItem(kbmActionName(kbm::Action(a)));
    auto picture = new KbmControlMap([actions](kbm::Action a) { actions->setCurrentIndex(a); });
    tabs->addTab(picture, "Picture");
    auto table = new QTreeWidget;
    table->setHeaderLabels({"Control", "Keyboard", "Mouse button"});
    table->setRootIsDecorated(false);
    table->setAlternatingRowColors(true);
    table->setUniformRowHeights(true);
    table->header()->setSectionResizeMode(QHeaderView::Stretch);
    for (int a = 0; a < kbm::Count; ++a)
        new QTreeWidgetItem(table, {kbmActionName(kbm::Action(a))});
    tabs->addTab(table, "List");

    auto scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    auto body = new QWidget;
    auto form = new QFormLayout(body);
    form->setRowWrapPolicy(QFormLayout::WrapLongRows);
    scroll->setWidget(body);
    tabs->addTab(scroll, "Mouse tuning");
    auto &c=engine.config;
    auto numeric=[&](const char *label,double &value,double lo,double hi,double step) {
        auto spin=new QDoubleSpinBox; spin->setRange(lo,hi); spin->setDecimals(3); spin->setSingleStep(step); spin->setValue(value);
        double *address=&value; connect(spin,&QDoubleSpinBox::valueChanged,this,[this,address](double v){release(); *address=v; save();}); form->addRow(label,spin);
    };
    numeric("X sensitivity",c.sensitivityX,.01,20,.1); numeric("Y sensitivity",c.sensitivityY,.01,20,.1);
    numeric("Response exponent (1 = linear)",c.exponent,.2,4,.1); numeric("Deadzone",c.deadzone,0,.95,.01);
    numeric("Anti-deadzone",c.antiDeadzone,0,1,.01); numeric("Maximum stick output",c.maximum,.05,1,.05);
    numeric("Smoothing window (ms)",c.smoothingMs,0,200,1); numeric("Recenter ramp (ms)",c.recenterMs,0,500,1);
    numeric("Walking tilt",c.walkTilt,.05,1,.05);
    auto invert=new QCheckBox; invert->setChecked(c.invertY); connect(invert,&QCheckBox::toggled,this,[this](bool v){release();engine.config.invertY=v;save();}); form->addRow("Invert Y",invert);
    auto rolling=new QCheckBox;rolling->setChecked(c.rollingWindow);
    connect(rolling,&QCheckBox::toggled,this,[this](bool v){release();engine.config.rollingWindow=v;save();});
    form->addRow("Rolling mouse window (less stop drift)",rolling);
    auto precise=new QPushButton("Precise mouse preset (8 ms, no recenter tail)");form->addRow(precise);
    auto smooth=new QPushButton("Smooth stick preset (16 ms, 35 ms tail)");form->addRow(smooth);
    auto testPage = new QWidget;
    auto testLayout = new QVBoxLayout(testPage);
    preview = new StickPreview(testPage);
    testLayout->addWidget(preview);
    status = new QLabel("Choose Test capture, then try a binding. Esc releases input.");
    status->setWordWrap(true);
    testLayout->addWidget(status);
    auto test = new QPushButton("Test capture");
    testLayout->addWidget(test, 0, Qt::AlignLeft);
    testLayout->addStretch();
    connect(test, &QPushButton::clicked, this, [this] { capture(true); });
    tabs->addTab(testPage, "Input test");

    auto editor = new QWidget;
    auto grid = new QGridLayout(editor);
    grid->setContentsMargins(0, 4, 0, 0);
    grid->addWidget(new QLabel("Control"), 0, 0);
    grid->addWidget(new QLabel("Keyboard"), 0, 1);
    grid->addWidget(new QLabel("Mouse button"), 0, 3);
    auto key = new QPushButton;
    key->setObjectName("keyBinding");
    key->setToolTip("Click, then press a key. Esc cancels.");
    auto clear = new QPushButton("Clear");
    clear->setAccessibleName("Clear keyboard binding");
    auto mouse = new QComboBox;
    mouse->addItems({"Unbound", "Left", "Right", "Middle", "Side 1", "Side 2"});
    grid->addWidget(actions, 1, 0);
    grid->addWidget(key, 1, 1);
    grid->addWidget(clear, 1, 2);
    grid->addWidget(mouse, 1, 3);
    grid->setColumnStretch(0, 3);
    grid->setColumnStretch(1, 2);
    grid->setColumnStretch(3, 2);
    layout->addWidget(editor);
    auto footer = new QHBoxLayout;
    auto hint = new QLabel("Select a control, then click its keyboard or mouse binding.");
    hint->setWordWrap(true);
    footer->addWidget(hint, 1);
    auto close = new QPushButton("Close");
    footer->addWidget(close);
    layout->addLayout(footer);
    connect(close, &QPushButton::clicked, dialog, &QDialog::reject);

    rebuildBindings = [this, actions, table, picture, key, mouse, clear] {
        const int selected = actions->currentIndex();
        const QSignalBlocker blockTable(table), blockMouse(mouse);
        for (int a = 0; a < kbm::Count; ++a) {
            auto item = table->topLevelItem(a);
            item->setText(1, keys.value(a) ? QKeySequence(keys.value(a)).toString(QKeySequence::NativeText) : "Unbound");
            item->setText(2, mouse->itemText(mouseButtons.value(a)));
        }
        table->setCurrentItem(table->topLevelItem(selected));
        picture->setAction(kbm::Action(selected));
        key->setText(binding == selected ? "Press a key…" : table->topLevelItem(selected)->text(1));
        clear->setEnabled(keys.value(selected) != 0);
        mouse->setCurrentIndex(mouseButtons.value(selected));
    };
    connect(actions, &QComboBox::currentIndexChanged, dialog, [this] { binding = -1; rebuildBindings(); });
    connect(table, &QTreeWidget::currentItemChanged, dialog, [table, actions](QTreeWidgetItem *item) {
        if (item) actions->setCurrentIndex(table->indexOfTopLevelItem(item));
    });
    connect(key, &QPushButton::clicked, dialog, [this, actions] {
        binding = actions->currentIndex();
        rebuildBindings();
    });
    connect(clear, &QPushButton::clicked, dialog, [this, actions] {
        binding = -1;
        keys[actions->currentIndex()] = 0;
        save();
        rebuildBindings();
    });
    connect(mouse, &QComboBox::currentIndexChanged, dialog, [this, actions](int index) {
        binding = -1;
        mouseButtons[actions->currentIndex()] = index;
        save();
        rebuildBindings();
    });
    connect(tabs, &QTabWidget::currentChanged, dialog, [this, editor, hint](int index) {
        release();
        binding = -1;
        editor->setVisible(index < 2);
        if (index < 2)
            hint->setText("Select a control, then click its keyboard or mouse binding.");
        else if (index == 2)
            hint->setText("Mouse tuning applies to the active input profile.");
        else
            hint->setText("Hold a mapped input to see its state. Esc releases capture.");
        rebuildBindings();
    });
    actions->setCurrentIndex(initialAction);
    rebuildBindings();
    tabs->setCurrentIndex(initialPage);
    auto recreate = [this, tabs, actions] {
        const int page = tabs->currentIndex(), action = actions->currentIndex();
        release();
        binding = -1;
        rebuildBindings = {};
        auto old = dialog;
        dialog = nullptr;
        preview = nullptr;
        status = nullptr;
        old->hide();
        old->deleteLater();
        buildControls(page, action);
    };
    connect(precise,&QPushButton::clicked,this,[this,recreate]{release();auto &c=engine.config;c.rollingWindow=true;c.smoothingMs=8;c.recenterMs=0;c.exponent=1;c.deadzone=0;c.antiDeadzone=0;save();recreate();});
    connect(smooth,&QPushButton::clicked,this,[this,recreate]{release();engine.config.rollingWindow=false;engine.config.smoothingMs=16;engine.config.recenterMs=35;save();recreate();});
    connect(profiles,&QComboBox::currentTextChanged,this,[this,recreate](const QString &name){auto s=controlsSettings();s.setValue("active",name);load();recreate();});
    connect(add,&QPushButton::clicked,this,[this,recreate]{bool ok=false;auto name=QInputDialog::getText(dialog,"Save profile","Name (letters, numbers, spaces, hyphen):",QLineEdit::Normal,"",&ok).trimmed();
        if(!ok || name.isEmpty()) return; for(auto ch:name) if(!ch.isLetterOrNumber() && ch!=' ' && ch!='-' && ch!='_') return;
        profile=name.left(64);save();recreate();});
    connect(reset,&QPushButton::clicked,this,[this,recreate]{defaults();save();recreate();});
    connect(dialog,&QDialog::finished,this,[this]{release();binding=-1;if(rebuildBindings)rebuildBindings();if(local_test_mode)qApp->quit();});
    dialog->show(); dialog->raise(); dialog->activateWindow();
}
int KbmInput::runLocalTest() { local_test_mode=true; qApp->setQuitOnLastWindowClosed(false); showControls(); return qApp->exec(); }
