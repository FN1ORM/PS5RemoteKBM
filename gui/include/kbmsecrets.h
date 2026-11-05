// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#pragma once
#include <QByteArray>
#include <QDebug>
#ifdef Q_OS_WIN
#include <windows.h>
#include <wincrypt.h>
#endif
namespace KbmSecrets {
inline QByteArray protect(const QByteArray &plain) {
    if(plain.isEmpty()) return {};
#ifdef Q_OS_WIN
    DATA_BLOB in{DWORD(plain.size()),reinterpret_cast<BYTE*>(const_cast<char*>(plain.constData()))},out{};
    if(!CryptProtectData(&in,L"PS5 Remote KBM",nullptr,nullptr,nullptr,CRYPTPROTECT_UI_FORBIDDEN,&out))
        qFatal("Windows could not protect credentials (%lu); refusing plaintext storage",GetLastError());
    QByteArray result("DPAPI1:"); result.append(QByteArray(reinterpret_cast<char*>(out.pbData),out.cbData).toBase64());
    SecureZeroMemory(out.pbData,out.cbData); LocalFree(out.pbData); return result;
#else
    return plain;
#endif
}
inline QByteArray unprotect(const QByteArray &stored) {
    if(stored.isEmpty()) return {};
#ifdef Q_OS_WIN
    if(!stored.startsWith("DPAPI1:")) { qWarning("Unprotected imported credentials ignored; sign in and register again"); return {}; }
    QByteArray blob=QByteArray::fromBase64(stored.mid(7));
    DATA_BLOB in{DWORD(blob.size()),reinterpret_cast<BYTE*>(blob.data())},out{};
    if(!CryptUnprotectData(&in,nullptr,nullptr,nullptr,nullptr,CRYPTPROTECT_UI_FORBIDDEN,&out)) {
        qWarning("Windows could not unlock stored credentials; sign in and register again"); return {};
    }
    QByteArray result(reinterpret_cast<char*>(out.pbData),out.cbData); SecureZeroMemory(out.pbData,out.cbData); LocalFree(out.pbData); return result;
#else
    return stored;
#endif
}
}
