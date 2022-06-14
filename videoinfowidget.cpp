#include "videoinfowidget.hpp"

#include "ui_videoinfowidget.h"
#include "util_macros.hpp"
VideoInfoWidget::VideoInfoWidget(QWidget *parent)
    : QWidget(parent),
      ui_(new Ui::VideoInfoWidget),
      cache_({QSize{0, 0}, 0.0, true, "", "", {}}),
      input_info_({concat::ValueRange<QSize>{}, concat::ValueRange<double>{}, true, {""}, {""}, {}}) {
    ui_->setupUi(this);
    connect(ui_->pushButton_add, &QPushButton::clicked, this, &VideoInfoWidget::add_argument_slot_output_);
    connect(ui_->pushButton_remove, &QPushButton::clicked, this,
            &VideoInfoWidget::remove_current_argument_slot_output_);
    connect(ui_->pushButton_add_input_args, &QPushButton::clicked, this, &VideoInfoWidget::add_argument_slot_input_);
    connect(ui_->pushButton_remove_input_args, &QPushButton::clicked, this,
            &VideoInfoWidget::remove_current_argument_slot_input_);
    connect(ui_->comboBox_resolution, &QComboBox::currentTextChanged, this, &VideoInfoWidget::update_input_resolution_);
    connect(ui_->comboBox_framerate, &QComboBox::currentTextChanged, this, &VideoInfoWidget::update_input_framerate_);
    connect(ui_->comboBox_audio_codec, &QComboBox::currentTextChanged, this,
            &VideoInfoWidget::update_input_audio_codec_);
    connect(ui_->comboBox_video_codec, &QComboBox::currentTextChanged, this,
            &VideoInfoWidget::update_input_video_codec_);
    connect(ui_->lineEdit_audio_codec, &QLineEdit::textEdited,
            [this](const QString &new_string) { this->cache_.audio_codec = new_string; });
    connect(ui_->lineEdit_video_codec, &QLineEdit::textEdited,
            [this](const QString &new_string) { this->cache_.video_codec = new_string; });
}

VideoInfoWidget::~VideoInfoWidget() { delete ui_; }

void VideoInfoWidget::add_argument_slot_impl_(QListWidget *widget, QPushButton *button) {
    auto item = new QListWidgetItem("arg", widget);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
    button->setEnabled(true);
}
void VideoInfoWidget::add_argument_slot_input_() {
    add_argument_slot_impl_(ui_->listWidget_input_args, ui_->pushButton_remove_input_args);
}
void VideoInfoWidget::add_argument_slot_output_() {
    add_argument_slot_impl_(ui_->listWidget_args, ui_->pushButton_remove);
}
void VideoInfoWidget::remove_current_argument_slot_impl_(QListWidget *widget, QPushButton *button) {
    int id_to_remove;
    if (widget->currentRow() == -1) {
        id_to_remove = widget->count() - 1;
    } else {
        id_to_remove = widget->currentRow();
    }
    delete widget->takeItem(id_to_remove);
    if (widget->count() == 0) {
        button->setEnabled(false);
    }
}
void VideoInfoWidget::remove_current_argument_slot_input_() {
    remove_current_argument_slot_impl_(ui_->listWidget_input_args, ui_->pushButton_remove_input_args);
}
void VideoInfoWidget::remove_current_argument_slot_output_() {
    remove_current_argument_slot_impl_(ui_->listWidget_args, ui_->pushButton_remove);
}
void VideoInfoWidget::toggle_input_is_enabled_(QString text, QVector<QWidget *> widgets) {
    bool new_enabled;
    if (text == tr("custom")) {
        new_enabled = true;
    } else {
        new_enabled = false;
    }
    for (auto widget : widgets) {
        widget->setEnabled(new_enabled);
    }
}
void VideoInfoWidget::update_input_resolution_(QString text) {
    toggle_input_is_enabled_(text, {ui_->spinBox_width, ui_->spinBox_height});
    QSize new_size{ui_->spinBox_width->value(), ui_->spinBox_height->value()};
    VIDEO_RE_ENCODER_TRY_VARIANT {
        if (text == tr("same as highest")) {
            new_size = std::get<concat::ValueRange<QSize>>(input_info_.resolution).highest;
        } else if (text == tr("same as lowest")) {
            new_size = std::get<concat::ValueRange<QSize>>(input_info_.resolution).lowest;
        } else if (text == tr("1920x1080")) {
            new_size = {1920, 1080};
        } else if (text == tr("3840x2160")) {
            new_size = {3840, 2160};
        } else if (text == tr("custom")) {
            new_size = std::get<QSize>(cache_.resolution);
        }
    }
    VIDEO_RE_ENCODER_CATCH_VARIANT_2(input_info_.resolution, cache_.resolution)
    ui_->spinBox_width->setValue(new_size.width());
    ui_->spinBox_height->setValue(new_size.height());
}
void VideoInfoWidget::update_input_framerate_(QString text) {
    toggle_input_is_enabled_(text, {ui_->doubleSpinBox_framerate});
    double new_value = ui_->doubleSpinBox_framerate->value();
    VIDEO_RE_ENCODER_TRY_VARIANT {
        if (text == tr("same as highest")) {
            new_value = std::get<concat::ValueRange<double>>(input_info_.framerate).highest;
        } else if (text == tr("same as lowest")) {
            new_value = std::get<concat::ValueRange<double>>(input_info_.framerate).lowest;
        } else if (text == tr("60fps")) {
            new_value = 60;
        } else if (text == tr("30fps")) {
            new_value = 30;
        } else if (text == tr("custom")) {
            new_value = std::get<double>(cache_.framerate);
        }
    }
    VIDEO_RE_ENCODER_CATCH_VARIANT_2(input_info_.framerate, cache_.framerate)
    ui_->doubleSpinBox_framerate->setValue(new_value);
}
void VideoInfoWidget::update_input_audio_codec_(QString text) {
    toggle_input_is_enabled_(text, {ui_->lineEdit_audio_codec});
    if (text == tr("same as input")) {
        ui_->stackedWidget_audio_codec->setCurrentWidget(ui_->page_combobox_audio_codec);
    } else {
        ui_->stackedWidget_audio_codec->setCurrentWidget(ui_->page_lineedit_audio_codec);
        if (text == tr("custom")) {
            VIDEO_RE_ENCODER_TRY_VARIANT { ui_->lineEdit_audio_codec->setText(std::get<QString>(cache_.audio_codec)); }
            VIDEO_RE_ENCODER_CATCH_VARIANT(cache_.audio_codec)
        }
    }
}
void VideoInfoWidget::update_input_video_codec_(QString text) {
    toggle_input_is_enabled_(text, {ui_->lineEdit_video_codec});
    if (text == tr("same as input")) {
        ui_->stackedWidget_video_codec->setCurrentWidget(ui_->page_combobox_video_codec);
    } else {
        ui_->stackedWidget_video_codec->setCurrentWidget(ui_->page_lineedit_video_codec);
        QString new_value = ui_->lineEdit_video_codec->text();
        VIDEO_RE_ENCODER_TRY_VARIANT {
            if (text == tr("h264")) {
                new_value = "h264";  // not translated as this will be directly used
            } else if (text == tr("hevc")) {
                new_value = "hevc";
            } else if (text == tr("custom")) {
                new_value = std::get<QString>(cache_.video_codec);
            }
        }
        VIDEO_RE_ENCODER_CATCH_VARIANT(cache_.video_codec)
        ui_->lineEdit_video_codec->setText(new_value);
    }
}
void VideoInfoWidget::set_infos(const concat::VideoInfo &initial_values, const concat::VideoInfo &input_info) {
    input_info_ = input_info;
    QString initial_text;
    VIDEO_RE_ENCODER_TRY_VARIANT {
        if (std::holds_alternative<concat::SameAsHighest<QSize>>(initial_values.resolution)) {
            initial_text = tr("same as highest");
        } else if (std::holds_alternative<concat::SameAsLowest<QSize>>(initial_values.resolution)) {
            initial_text = tr("same as lowest");
        } else if (std::holds_alternative<QSize>(initial_values.resolution)) {
            initial_text = tr("custom");  // プリセットはとりあえずカスタム扱い
            cache_.resolution = initial_values.resolution;
        }
    }
    VIDEO_RE_ENCODER_CATCH_VARIANT(initial_values.resolution)
    ui_->comboBox_resolution->setCurrentText(initial_text);
    initial_text = "";
    VIDEO_RE_ENCODER_TRY_VARIANT {
        if (std::holds_alternative<concat::SameAsHighest<double>>(initial_values.framerate)) {
            initial_text = tr("same as highest");
        } else if (std::holds_alternative<concat::SameAsLowest<double>>(initial_values.framerate)) {
            initial_text = tr("same as lowest");
        } else if (std::holds_alternative<double>(initial_values.framerate)) {
            initial_text = tr("custom");  // プリセットはとりあえずカスタム扱い
            cache_.framerate = initial_values.framerate;
        }
    }
    VIDEO_RE_ENCODER_CATCH_VARIANT(initial_values.framerate)
    ui_->comboBox_framerate->setCurrentText(initial_text);
    ui_->radioButton_vfr->setChecked(initial_values.is_vfr);
    initial_text = "";
    VIDEO_RE_ENCODER_TRY_VARIANT {
        if (std::holds_alternative<concat::SameAsInput<QString>>(initial_values.audio_codec)) {
            initial_text = tr("same as input");
        } else {
            initial_text = tr("custom");
            cache_.audio_codec = initial_values.audio_codec;
        }
    }
    VIDEO_RE_ENCODER_CATCH_VARIANT(initial_values.audio_codec)
    ui_->comboBox_audio_codec->setCurrentText(initial_text);
    VIDEO_RE_ENCODER_TRY_VARIANT {
        for (const auto &item : std::get<QSet<QString>>(input_info.audio_codec)) {
            ui_->comboBox_input_audio_codec->addItem(item);
        }
    }
    VIDEO_RE_ENCODER_CATCH_VARIANT(input_info.audio_codec)
    initial_text = "";
    VIDEO_RE_ENCODER_TRY_VARIANT {
        if (std::holds_alternative<concat::SameAsInput<QString>>(initial_values.video_codec)) {
            initial_text = tr("same as input");
        } else {
            initial_text = tr("custom");
            cache_.video_codec = initial_values.video_codec;
        }
    }
    VIDEO_RE_ENCODER_CATCH_VARIANT(initial_values.video_codec)
    ui_->comboBox_video_codec->setCurrentText(initial_text);
    VIDEO_RE_ENCODER_TRY_VARIANT {
        for (const auto &item : std::get<QSet<QString>>(input_info.video_codec)) {
            ui_->comboBox_input_video_codec->addItem(item);
        }
    }
    VIDEO_RE_ENCODER_CATCH_VARIANT(input_info.video_codec)
    update_everything_();

    ui_->listWidget_args->clear();
    for (const auto &arg : initial_values.encoding_args) {
        add_argument_slot_output_();
        ui_->listWidget_args->item(ui_->listWidget_args->count() - 1)->setText(arg);
    }
    ui_->listWidget_input_args->clear();
    for (const auto &arg : initial_values.input_file_args) {
        add_argument_slot_input_();
        ui_->listWidget_input_args->item(ui_->listWidget_input_args->count() - 1)->setText(arg);
    }
}
concat::VideoInfo VideoInfoWidget::info() const {
    concat::VideoInfo result{};
    auto resolution = ui_->comboBox_resolution->currentText();
    if (resolution == tr("same as highest")) {
        result.resolution = concat::SameAsHighest<QSize>{{ui_->spinBox_width->value(), ui_->spinBox_height->value()}};
    } else if (resolution == tr("same as lowest")) {
        result.resolution = concat::SameAsLowest<QSize>{{ui_->spinBox_width->value(), ui_->spinBox_height->value()}};
    } else {
        result.resolution = QSize{ui_->spinBox_width->value(), ui_->spinBox_height->value()};
    }
    auto framerate = ui_->comboBox_framerate->currentText();
    if (framerate == tr("same as highest")) {
        result.framerate = concat::SameAsHighest<double>{ui_->doubleSpinBox_framerate->value()};
    } else if (framerate == tr("same as lowest")) {
        result.framerate = concat::SameAsLowest<double>{ui_->doubleSpinBox_framerate->value()};
    } else {
        result.framerate = ui_->doubleSpinBox_framerate->value();
    }
    result.is_vfr = ui_->radioButton_vfr->isChecked();
    auto audio_codec = ui_->comboBox_audio_codec->currentText();
    if (audio_codec == tr("same as input")) {
        result.audio_codec = concat::SameAsInput<QString>{ui_->comboBox_input_audio_codec->currentText()};
    } else {
        result.audio_codec = ui_->lineEdit_audio_codec->text();
    }
    auto video_codec = ui_->comboBox_video_codec->currentText();
    if (video_codec == tr("same as input")) {
        result.video_codec = concat::SameAsInput<QString>{ui_->comboBox_input_video_codec->currentText()};
    } else {
        result.video_codec = ui_->lineEdit_video_codec->text();
    }
    for (auto i = 0; i < ui_->listWidget_args->count(); i++) {
        result.encoding_args += ui_->listWidget_args->item(i)->text();
    }
    for (auto i = 0; i < ui_->listWidget_input_args->count(); i++) {
        result.input_file_args += ui_->listWidget_input_args->item(i)->text();
    }
    return result;
}
void VideoInfoWidget::update_everything_() {
    update_input_resolution_(ui_->comboBox_resolution->currentText());
    update_input_framerate_(ui_->comboBox_framerate->currentText());
    update_input_audio_codec_(ui_->comboBox_audio_codec->currentText());
    update_input_video_codec_(ui_->comboBox_video_codec->currentText());
}