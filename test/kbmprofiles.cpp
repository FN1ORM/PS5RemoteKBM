// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#include "kbmprofiles.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <cstdio>
int main(int argc,char **argv) {
    QCoreApplication app(argc,argv); QTemporaryDir dir; kbm::Config a,b;
    a.sensitivityX=2.3;a.sensitivityY=.7;a.exponent=1.8;a.invertY=true;
    a.deadzone=.15;a.antiDeadzone=.21;a.maximum=.8;a.smoothingMs=24;a.recenterMs=90;a.walkTilt=.3;
    QMap<int,int> keys{{kbm::Cross,Qt::Key_X},{kbm::PS,Qt::Key_P}},mouse{{kbm::L2,2},{kbm::R2,1}};
    { QSettings s(dir.filePath("controls.ini"),QSettings::IniFormat);s.beginGroup("profiles/A");KbmProfiles::save(s,a,keys,mouse);s.sync(); }
    QMap<int,int> loadedKeys,loadedMouse;
    { QSettings s(dir.filePath("controls.ini"),QSettings::IniFormat);s.beginGroup("profiles/A");KbmProfiles::load(s,b,loadedKeys,loadedMouse); }
    if(b.sensitivityX!=2.3 || b.sensitivityY!=.7 || b.exponent!=1.8 || !b.invertY || b.deadzone!=.15
        || b.antiDeadzone!=.21 || b.maximum!=.8 || b.smoothingMs!=24 || b.recenterMs!=90 || b.walkTilt!=.3
        || loadedKeys[kbm::Cross]!=Qt::Key_X || loadedMouse[kbm::R2]!=1 || loadedMouse[kbm::L2]!=2) return 1;
    QSettings s(dir.filePath("controls.ini"),QSettings::IniFormat);s.beginGroup("profiles/B");
    s.setValue("sensitivityX",-10);s.setValue("maximum",8);s.setValue("key0",int(Qt::Key_Escape));s.setValue("mouse0",999);
    KbmProfiles::load(s,b,loadedKeys,loadedMouse);
    if(b.sensitivityX!=.01 || b.maximum!=1 || loadedKeys[0]!=0 || loadedMouse[0]!=5) return 2;
    s.endGroup();s.beginGroup("profiles/A"); KbmProfiles::load(s,b,loadedKeys,loadedMouse);
    if(b.sensitivityX!=2.3 || loadedKeys[kbm::Cross]!=Qt::Key_X) return 3;
    std::puts("PASS: all mouse settings/bindings roundtrip, isolated profiles, invalid config and reserved keys");
}
