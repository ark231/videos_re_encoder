#include "videoinfodialog.hpp"

#include "ui_videoinfodialog.h"

VideoInfoDialog::VideoInfoDialog(QWidget *parent) : QDialog(parent), ui_(new Ui::VideoInfoDialog) {
    ui_->setupUi(this);
}

VideoInfoDialog::~VideoInfoDialog() { delete ui_; }

concat::VideoInfo VideoInfoDialog::get_video_info(QWidget *parent, const QString &title, const QString &label,
                                                  const concat::VideoInfo &initial_info,
                                                  const concat::VideoInfo &input_info, bool *ok, Qt::WindowFlags flags,
                                                  Qt::InputMethodHints input_method_hints) {
    VideoInfoDialog dialog(parent);
    dialog.setWindowFlags(flags);
    dialog.setInputMethodHints(input_method_hints);
    dialog.setWindowTitle(title);
    dialog.ui_->label->setText(label);
    dialog.ui_->videoInfoWidget->set_infos(initial_info, input_info);
    concat::VideoInfo result;
    switch (dialog.exec()) {
        case QDialog::Accepted:
            *ok = true;
            result = dialog.ui_->videoInfoWidget->info();
            break;
        case QDialog::Rejected:
            *ok = false;
            break;
    }
    return result;
}