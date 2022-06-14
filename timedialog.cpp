#include "timedialog.hpp"

#include "ui_timedialog.h"

TimeDialog::TimeDialog(QWidget *parent) : QDialog(parent), ui_(new Ui::TimeDialog) { ui_->setupUi(this); }

TimeDialog::~TimeDialog() { delete ui_; }

QTime TimeDialog::get_time(QWidget *parent, const QString &title, const QString &label, const QTime time, bool *ok,
                           Qt::WindowFlags flags, Qt::InputMethodHints input_method_hints) {
    TimeDialog dialog(parent);
    dialog.setWindowFlags(flags);
    dialog.setInputMethodHints(input_method_hints);
    dialog.setWindowTitle(title);
    dialog.ui_->label->setText(label);
    dialog.ui_->timeEdit->setTime(time);
    QTime result;
    switch (dialog.exec()) {
        case QDialog::Accepted:
            *ok = true;
            result = dialog.ui_->timeEdit->time();
            break;
        case QDialog::Rejected:
            *ok = false;
            break;
    }
    return result;
}