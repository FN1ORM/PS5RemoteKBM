// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#pragma once
#include <QDialog>
#include <QFont>
inline void styleKbmDialog(QDialog *dialog) {
    dialog->setFont(QFont("Segoe UI",10));
    dialog->setStyleSheet(
        "QDialog, QWidget { background: #101c2d; color: #e5edf7; }"
        "QLabel { background: transparent; }"
        "QPushButton { background: #20364f; border: 1px solid #365571; border-radius: 6px; padding: 7px 12px; }"
        "QPushButton:hover { background: #2b4866; }"
        "QPushButton:focus { border-color: #6bc4ff; }"
        "QPushButton:disabled { color: #7f91a7; background: #18283a; }"
        "QComboBox, QDoubleSpinBox, QLineEdit, QPlainTextEdit { background: #0b1422; border: 1px solid #365571; padding: 5px; selection-background-color: #24618c; }"
        "QScrollArea { border: 0; }"
        "QScrollBar:vertical { background: #101c2d; width: 12px; }"
        "QScrollBar::handle:vertical { background: #426383; min-height: 24px; border-radius: 5px; }"
    );
}
