// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#pragma once
#include "kbmengine.h"
#include <QPixmap>
#include <QWidget>
#include <functional>

class QPushButton;
const char *kbmActionName(kbm::Action action);

class KbmControlMap : public QWidget
{
public:
    explicit KbmControlMap(std::function<void(kbm::Action)> select, QWidget *parent = nullptr);
    void setAction(kbm::Action action);

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void mousePressEvent(QMouseEvent *) override;

private:
    void chooseGroup(int group);
    QRectF imageRect() const;
    QPointF anchor(int group) const;
    QPixmap controller;
    QList<QPushButton *> buttons;
    std::function<void(kbm::Action)> selectAction;
    int selectedGroup = -1;
};
