// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#pragma once
#include "kbmengine.h"
#include <QSettings>
#include <QMap>
#include <Qt>
namespace KbmProfiles {
inline int bindingKey(int key, Qt::KeyboardModifiers modifiers) {
    if(!modifiers.testFlag(Qt::KeypadModifier))
        return key;
    // The keypad uses arrow key codes when Num Lock is off.
    switch(key) {
    case Qt::Key_Up: key=Qt::Key_8; break;
    case Qt::Key_Down: key=Qt::Key_2; break;
    case Qt::Key_Left: key=Qt::Key_4; break;
    case Qt::Key_Right: key=Qt::Key_6; break;
    default: break;
    }
    return key | int(Qt::KeypadModifier);
}

inline bool reservedKey(int key) {
    key &= ~int(Qt::KeypadModifier);
    return key==Qt::Key_F1 || key==Qt::Key_F2 || key==Qt::Key_F3 || key==Qt::Key_F4 || key==Qt::Key_Escape;
}

inline void load(QSettings &s, kbm::Config &c, QMap<int,int> &keys, QMap<int,int> &mouse) {
#define READ(field) c.field=s.value(#field,c.field).toDouble()
    READ(sensitivityX); READ(sensitivityY); READ(exponent); READ(deadzone); READ(antiDeadzone);
    READ(smoothingMs); READ(recenterMs); READ(maximum); READ(walkTilt);
#undef READ
    c.rollingWindow=s.value("rollingWindow",false).toBool();
    c.invertY=s.value("invertY",false).toBool(); c.sanitize();
    for(int a=0;a<kbm::Count;++a) {
        keys[a]=s.value(QString("key%1").arg(a),keys.value(a)).toInt();
        mouse[a]=std::max(0,std::min(5,s.value(QString("mouse%1").arg(a),mouse.value(a)).toInt()));
        if(reservedKey(keys[a])) keys[a]=0;
    }
}
inline void save(QSettings &s, kbm::Config &c, const QMap<int,int> &keys, const QMap<int,int> &mouse) {
    c.sanitize();
#define WRITE(field) s.setValue(#field,c.field)
    WRITE(sensitivityX); WRITE(sensitivityY); WRITE(exponent); WRITE(deadzone); WRITE(antiDeadzone);
    WRITE(smoothingMs); WRITE(recenterMs); WRITE(maximum); WRITE(walkTilt);
#undef WRITE
    s.setValue("invertY",c.invertY); s.setValue("rollingWindow",c.rollingWindow);
    for(int a=0;a<kbm::Count;++a) {s.setValue(QString("key%1").arg(a),keys.value(a));s.setValue(QString("mouse%1").arg(a),mouse.value(a));}
}
}
