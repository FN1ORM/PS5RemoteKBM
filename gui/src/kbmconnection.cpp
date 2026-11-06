// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#include "kbmconnection.h"
#include "settings.h"
#include "kbmwidgets.h"
#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QProcess>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPointer>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QMessageBox>
#include <QScreen>
#include <QUrl>
#include <QWindow>
#include <QScrollArea>
#include <memory>

void openKbmManual() {
    QDir dir(QCoreApplication::applicationDirPath());
    // Portable layout first; source build layout second. Never send credentials to a website.
    for(int depth=0; depth<4; ++depth) {
        const QString path=dir.filePath("docs/kbm/user-manual.html");
        if(QFileInfo::exists(path)) { QDesktopServices::openUrl(QUrl::fromLocalFile(path)); return; }
        dir.cdUp();
    }
    QMessageBox::information(nullptr,"User manual","The offline manual is missing. Restore docs/kbm/user-manual.html from the app package.");
}

void showKbmConnectionTools(QWindow *owner, Settings *settings) {
    auto dialog=new QDialog;
    styleKbmDialog(dialog);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle("PS5 Remote KBM — Connection and quality");
    dialog->setModal(true);
    dialog->resize(680,620);
    if(owner->screen()) dialog->resize(dialog->size().boundedTo(owner->screen()->availableGeometry().size()-QSize(40,60)));
    QObject::connect(owner,&QObject::destroyed,dialog,&QObject::deleteLater);
    auto outer=new QVBoxLayout(dialog);
    auto scroll=new QScrollArea;scroll->setWidgetResizable(true);outer->addWidget(scroll);
    auto body=new QWidget;scroll->setWidget(body);auto layout=new QVBoxLayout(body);
    auto explain=new QLabel("Registration saves permission to control the console. A network path is still needed.\n"
        "Saved PS5 IP: keep the Android subnet router online on the PS5 Wi-Fi.\n"
        "Registered PSN entry: can work without the phone if PSN traversal succeeds.\n"
        "Rest Mode ends gameplay; network wake settings allow a later reconnection.");
    explain->setWordWrap(true); layout->addWidget(explain);
    auto presets=new QComboBox;
    presets->addItems({"Stable connection — 720p / 30 fps / H.264 / 5 Mbps", "Responsive — 720p / 60 fps / H.264 / 8 Mbps", "Clearer picture — 1080p / 60 fps / H.264 / 15 Mbps"});
    layout->addWidget(presets);
    auto apply=new QPushButton("Apply quality for the next connection");layout->addWidget(apply);
    auto note=new QLabel("Both Local PS5 and Remote PS5 profiles will change: a VPN to a private PS5 IP can select Local. "
        "Lower bandwidth can help congestion; 30 fps is less responsive. No setting removes distance, Wi-Fi interference or ISP loss.");
    note->setWordWrap(true);layout->addWidget(note);
    QPointer<Settings> safeSettings(settings);
    QObject::connect(apply,&QPushButton::clicked,dialog,[safeSettings,presets,note] {
        if(!safeSettings) return;
        const int index=presets->currentIndex();
        auto resolution=index==2?CHIAKI_VIDEO_RESOLUTION_PRESET_1080p:CHIAKI_VIDEO_RESOLUTION_PRESET_720p;
        auto fps=index==0?CHIAKI_VIDEO_FPS_PRESET_30:CHIAKI_VIDEO_FPS_PRESET_60;
        unsigned bitrate=index==0?5000:index==1?8000:15000;
        safeSettings->SetResolutionLocalPS5(resolution);safeSettings->SetResolutionRemotePS5(resolution);
        safeSettings->SetFPSLocalPS5(fps);safeSettings->SetFPSRemotePS5(fps);
        safeSettings->SetBitrateLocalPS5(bitrate);safeSettings->SetBitrateRemotePS5(bitrate);
        safeSettings->SetCodecLocalPS5(CHIAKI_CODEC_H264);safeSettings->SetCodecRemotePS5(CHIAKI_CODEC_H264);
        safeSettings->Sync();
        note->setText("Saved for BOTH PS5 profiles. Disconnect and reconnect to use it. Your registration and PSN sign-in are unchanged.");
    });
    auto peers=new QComboBox;peers->setPlaceholderText("Load Tailscale peers, then choose the Android subnet router");layout->addWidget(peers);
    auto refresh=new QPushButton("Load Tailscale peers (read only)");layout->addWidget(refresh);
    auto ping=new QPushButton("Check selected peer: direct or relay, and round-trip time");ping->setEnabled(false);layout->addWidget(ping);
    auto output=new QPlainTextEdit;output->setMinimumHeight(140);output->setReadOnly(true);output->setPlainText("These tests do not change VPN settings. Peer pings measure the laptop-to-phone path, not PS5 video loss or input latency. "
        "During gameplay, enable Show Stream Stats in Settings for stream packet loss, bitrate and dropped frames.\n\n"
        "A direct Tailscale path is usually preferable. Initial replies via DERP are normal while a direct path is negotiated. Persistent DERP may limit streaming.");layout->addWidget(output,1);
    QString executable=QDir(qEnvironmentVariable("ProgramFiles","C:/Program Files")).filePath("Tailscale/tailscale.exe");
    if(!QFileInfo::exists(executable)) executable.clear();
    auto process=new QProcess(dialog); auto timeout=new QTimer(dialog);timeout->setSingleShot(true);
    auto statusMode=std::make_shared<bool>(true);
    QObject::connect(timeout,&QTimer::timeout,dialog,[process,output]{process->kill();output->appendPlainText("Diagnostic time limit reached; the app remains usable.");});
    QObject::connect(process,&QProcess::errorOccurred,dialog,[refresh,ping,peers,output,timeout](QProcess::ProcessError error){
        if(error==QProcess::FailedToStart) { timeout->stop();refresh->setEnabled(true);ping->setEnabled(peers->count()>0);output->setPlainText("Could not start the installed Tailscale CLI."); }
    });
    QObject::connect(process,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),dialog,[=](int code,QProcess::ExitStatus) {
        timeout->stop();refresh->setEnabled(true);
        auto bytes=process->readAllStandardOutput();
        if(*statusMode) {
            peers->clear();auto doc=QJsonDocument::fromJson(bytes);auto obj=doc.object();
            if(code || obj.value("BackendState").toString()!="Running") output->setPlainText("Tailscale is not connected or status could not be read. Connect it in the Tailscale app first.");
            else {
                const auto list=obj.value("Peer").toObject();
                for(const auto &value:list) {
                    const auto peer=value.toObject();const auto ips=peer.value("TailscaleIPs").toArray();if(ips.isEmpty())continue;
                    auto ip=ips.first().toString();if(ip.contains(':'))continue;
                    peers->addItem(QString("%1 · %2 · %3").arg(peer.value("HostName").toString(),peer.value("OS").toString(),peer.value("Online").toBool()?"online":"offline"),ip);
                    if(peer.value("OS").toString()=="android")peers->setCurrentIndex(peers->count()-1);
                }
                output->setPlainText("Choose the phone that advertises the PS5 route, then run the check. This list alone does not prove that the PS5 is reachable.");
            }
        } else {
            output->setPlainText(QString::fromUtf8(bytes));
            if(code)output->appendPlainText("The check did not complete successfully. Confirm the phone is online and connected to this tailnet.");
            output->appendPlainText("\nDirect peer ping is not a PS5 stream benchmark. A successful phone ping can coexist with a missing subnet route or sleeping PS5. See F3 for the full troubleshooting steps.");
        }
        ping->setEnabled(peers->count()>0);
    });
    auto run=[=](bool status) {
        if(executable.isEmpty()) {output->setPlainText("Tailscale CLI was not found in Program Files/Tailscale. Use the CLI commands in the manual if installed elsewhere.");return;}
        *statusMode=status;refresh->setEnabled(false);ping->setEnabled(false);output->setPlainText("Checking…");
        process->start(executable,status?QStringList{"status","--json"}:QStringList{"ping","--c=5","--until-direct=false","--timeout=3s",peers->currentData().toString()});
        timeout->start(25000);
    };
    QObject::connect(refresh,&QPushButton::clicked,dialog,[=]{run(true);});
    QObject::connect(ping,&QPushButton::clicked,dialog,[=]{run(false);});
    auto help=new QPushButton("Open full user manual (F3)");layout->addWidget(help);QObject::connect(help,&QPushButton::clicked,dialog,[]{openKbmManual();});
    auto close=new QPushButton("Close");layout->addWidget(close);QObject::connect(close,&QPushButton::clicked,dialog,&QDialog::close);
    QObject::connect(dialog,&QDialog::finished,dialog,[process]{if(process->state()!=QProcess::NotRunning)process->kill();});
    dialog->show();dialog->windowHandle()->setTransientParent(owner);
}
