// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#include "jsonrequester.h"
#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QEventLoop>
#include <QTimer>
#include <cstdio>
int main(int argc,char **argv) {
    QCoreApplication app(argc,argv); QTcpServer server;
    if(!server.listen(QHostAddress::LocalHost,0)) return 1;
    QObject::connect(&server,&QTcpServer::newConnection,&server,[&]{
        auto socket=server.nextPendingConnection();
        QObject::connect(socket,&QTcpSocket::readyRead,socket,[socket]{socket->readAll();socket->write("HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");socket->disconnectFromHost();});
        QObject::connect(socket,&QTcpSocket::disconnected,socket,&QObject::deleteLater);
    });
    JsonRequester requester; QEventLoop loop; bool received=false,safe=false;
    QObject::connect(&requester,&JsonRequester::requestError,&loop,[&](const QString &url,const QString &error,QNetworkReply::NetworkError){
        received=true;safe=!url.contains("SYNTHETIC_SECRET") && !error.contains("SYNTHETIC_SECRET")
            && error.contains("403") && url.contains("redacted");loop.quit();
    });
    QTimer::singleShot(3000,&loop,&QEventLoop::quit);
    requester.makeGetRequest(QString("http://127.0.0.1:%1/SYNTHETIC_SECRET?code=SYNTHETIC_SECRET").arg(server.serverPort()),"Basic synthetic");
    loop.exec();
    if(!received || !safe) return 2;
    std::puts("PASS: loopback HTTP failure is actionable; token-bearing URL absent from error signals");
    return 0;
}
