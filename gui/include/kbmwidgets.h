// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#pragma once
#include <QDialog>
#include <QFont>
inline void styleKbmDialog(QDialog *dialog) {
    dialog->setFont(QFont("Segoe UI",10));
    dialog->setStyleSheet(
        "QDialog, QWidget { background: #202725; color: #edf5f0; }"
        "QLabel { background: transparent; }"
        "QPushButton { background: #293531; border: 1px solid #485851; border-radius: 6px; padding: 7px 12px; }"
        "QPushButton:hover { background: #374940; }"
        "QPushButton:focus { border-color: #78dfb6; }"
        "QPushButton:disabled { color: #9fafa6; background: #202725; }"
        "QComboBox, QDoubleSpinBox, QLineEdit, QPlainTextEdit { background: #151918; border: 1px solid #485851; padding: 5px; selection-background-color: #356650; }"
        "QScrollArea { border: 0; }"
        "QScrollBar:vertical { background: #202725; width: 12px; }"
        "QScrollBar::handle:vertical { background: #5b7568; min-height: 24px; border-radius: 5px; }"
    );
}
