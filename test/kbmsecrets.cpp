// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#include "kbmsecrets.h"
#include <QCoreApplication>
#include <QSettings>
#include <QTemporaryDir>
#include <cstdio>
int main(int argc,char **argv) {
    QCoreApplication app(argc,argv); QTemporaryDir dir;
    const QByteArray plain("local-synthetic-token-12345");
    auto encrypted=KbmSecrets::protect(plain);
    if(encrypted==plain || encrypted.contains(plain) || KbmSecrets::unprotect(encrypted)!=plain) return 1;
    if(!KbmSecrets::unprotect(plain).isEmpty()) return 2;
    auto damaged=encrypted; damaged[damaged.size()/2]=damaged[damaged.size()/2]=='A'?'B':'A';
    if(!KbmSecrets::unprotect(damaged).isEmpty()) return 3;
    QSettings s(dir.filePath("secrets.ini"),QSettings::IniFormat);
    s.setValue("token",encrypted); s.sync();
    QSettings reload(dir.filePath("secrets.ini"),QSettings::IniFormat);
    if(KbmSecrets::unprotect(reload.value("token").toByteArray())!=plain) return 4;
    s.remove("token");s.sync();
    if(s.contains("token")) return 5;
    std::puts("PASS: DPAPI encrypt/decrypt, plaintext rejection, tamper rejection, disk roundtrip, deletion");
}
