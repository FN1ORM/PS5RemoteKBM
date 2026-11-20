// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#include "kbmcontrolmap.h"
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <array>

namespace {
using namespace kbm;
struct Group {
    const char *label;
    QPointF target; // Normalized coordinates in the controller illustration.
    QList<Action> actions;
};
const std::array<Group, 12> groups {{
    {"L2  ·  Trigger", {.19, .019}, {L2}},
    {"L1  ·  Bumper", {.20, .047}, {L1}},
    {"Create", {.26, .13}, {Share}},
    {"D-pad", {.18, .29}, {DUp, DRight, DDown, DLeft}},
    {"Left stick", {.335, .50}, {Up, Right, Down, Left, L3, Walk}},
    {"PS button", {.50, .50}, {PS}},
    {"R2  ·  Trigger", {.81, .019}, {R2}},
    {"R1  ·  Bumper", {.80, .047}, {R1}},
    {"Options", {.74, .13}, {Options}},
    {"Touchpad click", {.59, .22}, {Touchpad}},
    {"Face buttons", {.825, .29}, {Triangle, Circle, Cross, Square}},
    {"Right stick", {.666, .50}, {RightStickUp, RightStickRight, RightStickDown, RightStickLeft, R3}}
}};
constexpr int rows = groups.size() / 2;
}

const char *kbmActionName(kbm::Action action)
{
    static constexpr const char *names[] = {
        "Cross", "Circle", "Square", "Triangle", "D-pad left", "D-pad right", "D-pad up", "D-pad down",
        "L1", "R1", "L3", "R3", "Options", "Create / Share", "Touchpad click", "PS button",
        "L2", "R2", "Move up", "Move down", "Move left", "Move right", "Walk",
        "Right stick up", "Right stick down", "Right stick left", "Right stick right"
    };
    static_assert(std::size(names) == kbm::Count);
    return names[action];
}

KbmControlMap::KbmControlMap(std::function<void(kbm::Action)> select, QWidget *parent)
    : QWidget(parent), controller(":/icons/keystream-controller.png"), selectAction(std::move(select))
{
    setMinimumSize(430, 248);
    for (int i = 0; i < int(groups.size()); ++i) {
        auto button = new QPushButton(QString::fromUtf8(groups[i].label), this);
        button->setObjectName("mapCallout");
        button->setCheckable(true);
        button->setAutoExclusive(true);
        button->setAutoDefault(false);
        button->setCursor(Qt::PointingHandCursor);
        buttons.append(button);
        connect(button, &QPushButton::clicked, this, [this, i] { chooseGroup(i); });
    }
}

void KbmControlMap::setAction(kbm::Action action)
{
    for (int i = 0; i < int(groups.size()); ++i) {
        if (groups[i].actions.contains(action)) {
            selectedGroup = i;
            buttons[i]->setChecked(true);
            update();
            return;
        }
    }
}

void KbmControlMap::chooseGroup(int index)
{
    const auto &actions = groups[index].actions;
    if (actions.size() == 1) {
        selectAction(actions.first());
        return;
    }
    QMenu menu(this);
    for (auto action : actions) {
        auto item = menu.addAction(QString::fromUtf8(kbmActionName(action)));
        connect(item, &QAction::triggered, this, [this, action] { selectAction(action); });
    }
    auto button = buttons[index];
    menu.exec(button->mapToGlobal(QPoint(0, button->height())));
    if (selectedGroup >= 0)
        buttons[selectedGroup]->setChecked(true);
}

QRectF KbmControlMap::imageRect() const
{
    const qreal available = width() - 2 * (buttons.first()->width() + 28);
    const qreal w = std::min({available, height() * 1.20, 540.0});
    const QSizeF size(w, w * 2 / 3);
    return QRectF(QPointF((width() - size.width()) / 2, (height() - size.height()) / 2), size);
}

QPointF KbmControlMap::anchor(int group) const
{
    const QRectF image = imageRect();
    const auto point = groups[group].target;
    return image.topLeft() + QPointF(image.width() * point.x(), image.height() * point.y());
}

void KbmControlMap::resizeEvent(QResizeEvent *)
{
    const int rail = std::clamp(width() / 6, 108, 156);
    const int step = std::min(51, (height() - 20) / rows);
    const int h = std::min(34, step - 4);
    const int top = (height() - (rows - 1) * step - h) / 2;
    for (int i = 0; i < buttons.size(); ++i) {
        buttons[i]->setGeometry(i < rows ? 12 : width() - 12 - rail, top + (i % rows) * step, rail, h);
    }
}

void KbmControlMap::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QRadialGradient light(rect().center(), width() * .53);
    light.setColorAt(0, QColor("#223b35"));
    light.setColorAt(1, QColor("#101917"));
    p.fillRect(rect(), light);
    auto line = [&](int i, bool highlight) {
        const auto box = buttons[i]->geometry();
        const QPointF start(i < rows ? box.right() : box.left(), box.center().y());
        const QPointF end = anchor(i);
        const qreal elbow = start.x() + (end.x() - start.x()) * .36;
        QPainterPath path(start);
        path.lineTo(elbow, start.y());
        path.lineTo(end);
        p.setPen(QPen(highlight ? QColor("#8ce4c6") : QColor("#526e66"), highlight ? 1.7 : 1));
        p.drawPath(path);
    };
    for (int i = 0; i < buttons.size(); ++i) line(i, false);
    p.drawPixmap(imageRect(), controller, QRectF(controller.rect()));
    if (selectedGroup >= 0) {
        line(selectedGroup, true);
        p.setBrush(QColor("#192722"));
        p.drawEllipse(anchor(selectedGroup), 4.5, 4.5);
    }
}

void KbmControlMap::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && imageRect().contains(event->position())) {
        int nearest = -1;
        qreal radius = std::clamp(imageRect().width() * .095, 12.0, 40.0);
        for (int i = 0; i < buttons.size(); ++i) {
            const auto distance = QLineF(event->position(), anchor(i)).length();
            if (distance < radius) { nearest = i; radius = distance; }
        }
        if (nearest >= 0) {
            chooseGroup(nearest);
            return;
        }
    }
    QWidget::mousePressEvent(event);
}
