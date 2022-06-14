#include "mainwindow.hpp"

#include <Section.h>
#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include <QApplication>
#include <QDataStream>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QMetaEnum>
#include <QPair>
#include <QPushButton>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStringList>
#include <QStyle>
#include <QTextStream>
#include <QTime>
#include <QUrl>
#include <QVBoxLayout>
#include <QVector>
#include <chrono>
#include <ciso646>
#include <cuchar>
#include <filesystem>
#include <functional>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <timedialog.hpp>

#include "./ui_mainwindow.h"
#include "processwidget.hpp"
#include "util_macros.hpp"
#include "videoinfodialog.hpp"
#include "videoinfowidget.hpp"

namespace {
class Tracer {
    const char *funcname_;

   public:
    Tracer(const char *funcname) : funcname_(funcname) { qDebug() << "enter" << funcname_; }
    ~Tracer() { qDebug() << "exit" << funcname_; }
};
#ifndef NDEBUG
#    define TRACE Tracer tracer(__FUNCTION__);
#else
#    define TRACE
#endif
concat::VideoInfo retrieve_input_info(const concat::VideoInfo &info) {
    TRACE
    auto result = concat::VideoInfo::create_input_info();
    if (std::holds_alternative<QSize>(info.resolution)) {
        auto resolution = std::get<QSize>(info.resolution);
        result.resolution = concat::ValueRange<QSize>{resolution, resolution};
    }
    if (std::holds_alternative<double>(info.framerate)) {
        auto framerate = std::get<double>(info.framerate);
        result.framerate = concat::ValueRange<double>{framerate, framerate};
    }
    if (std::holds_alternative<QString>(info.audio_codec)) {
        result.audio_codec = QSet<QString>{std::get<QString>(info.audio_codec)};
    }
    if (std::holds_alternative<QString>(info.video_codec)) {
        result.video_codec = QSet<QString>{std::get<QString>(info.video_codec)};
    }
    // encoding_argsはinitial_valueで指定すべき
    return result;
}
constexpr auto INITIAL_ANIMATION_DURATION = 200;
}  // namespace
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui_(new Ui::MainWindow) {
    TRACE
    ui_->setupUi(this);
    ui_->pushButton_diropen->setIcon(this->style()->standardIcon(QStyle::SP_DirOpenIcon));
#ifndef NDEBUG
    this->setWindowTitle(QStringLiteral("%1 (debug build)").arg(this->windowTitle()));
#endif
    connect(ui_->actionopen, &QAction::triggered, this, &MainWindow::open_video_);
    connect(ui_->pushButton_diropen, &QPushButton::pressed, this, &MainWindow::select_output_dir_);
    connect(ui_->pushButton_save, &QPushButton::pressed, this, &MainWindow::save_result_);
    connect(ui_->actionsavefile_name_generator, &QAction::triggered, this, &MainWindow::select_savefile_name_plugin_);
    connect(ui_->actioneffective_period_of_cache, &QAction::triggered, this,
            &MainWindow::update_effective_period_of_cache_);
    connect(ui_->actiondefault_video_info, &QAction::triggered, this, &MainWindow::edit_default_video_info_);
    connect(ui_->comboBox_preset, &QComboBox::currentTextChanged, this, &MainWindow::change_preset_);
    connect(ui_->actiondefault_preset, &QAction::triggered, this, &MainWindow::select_default_preset_);
    QDir settings_dir(QApplication::applicationDirPath() + "/settings");
    if (QDir().mkpath(settings_dir.absolutePath())) {  // QDir::mkpath() returns true even when path already exists
        settings_ = new QSettings(settings_dir.filePath("settings.ini"), QSettings::IniFormat);
        auto preset_path = settings_dir.filePath("presets.toml");
        if (QFile::exists(preset_path)) {
            try {
                presets_ = toml::parse(preset_path.toStdString());
            } catch (std::runtime_error &e) {
                QMessageBox::warning(nullptr, tr("warning"),
                                     tr("failed to load preset file (%1) info: \n%2").arg(preset_path).arg(e.what()));
            }
            for (const auto &[key, value] : presets_.as_table()) {
                if (key == "VERSION") {
                    continue;
                }
                ui_->comboBox_preset->addItem(QString::fromStdString(key));
            }
        }
    }
    current_preset_ = ui_->comboBox_preset->currentText();
    cache_ = settings_->value("default_video_info").value<concat::VideoInfo>();
    source_video_info_ = concat::VideoInfo::create_input_info();
    auto default_preset =
        settings_->value("default_preset").isNull() ? "custom" : settings_->value("default_preset").toString();
    ui_->comboBox_preset->setCurrentText(default_preset == "custom" ? tr("custom") : default_preset);
    change_preset_(ui_->comboBox_preset->currentText());
}

MainWindow::~MainWindow() {
    TRACE
    delete ui_;
    if (settings_ != nullptr) {
        settings_->deleteLater();
    }
}
QUrl MainWindow::read_video_dir_cache_() {
    TRACE
    settings_->beginGroup("video_dir_cache");
    auto last_saved = settings_->value("last_saved").toDateTime().toMSecsSinceEpoch();
    auto effective_period = settings_->value("effective_period").toTime().msecsSinceStartOfDay();
    auto dir = settings_->value("dir").toUrl();
    settings_->endGroup();
    if (QDateTime::currentDateTime().toMSecsSinceEpoch() - last_saved < effective_period) {
        return dir;
    } else {
        return QUrl();
    }
}
void MainWindow::write_video_dir_cache_(QUrl dir) {
    TRACE
    settings_->beginGroup("video_dir_cache");
    settings_->setValue("last_saved", QDateTime::currentDateTime());
    // effective period is written in update_effective_period_of_cache()
    settings_->setValue("dir", dir);
    settings_->endGroup();
}
void MainWindow::update_effective_period_of_cache_() {
    TRACE
    bool confirmed = false;
    auto period = TimeDialog::get_time(nullptr, tr("effective period of cache"), tr("enter effective period of cache"),
                                       settings_->value("video_dir_cache/effective_period").toTime(), &confirmed);
    if (confirmed && period.isValid()) {
        settings_->setValue("video_dir_cache/effective_period", period);
    }
}

void MainWindow::edit_default_video_info_() {
    TRACE
    bool confirmed = false;
    auto default_video_info = settings_->value("default_video_info").value<concat::VideoInfo>();
    auto default_input_video_info = retrieve_input_info(default_video_info);
    auto video_info = VideoInfoDialog::get_video_info(
        nullptr, tr("effective period of cache"), tr("enter effective period of cache"),
        settings_->value("default_video_info").value<concat::VideoInfo>(), default_input_video_info, &confirmed);
    if (confirmed) {
        settings_->setValue("default_video_info", QVariant::fromValue(video_info));
    }
}

void MainWindow::open_video_() {
    TRACE
    auto videodirs = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation);
    auto videodir = read_video_dir_cache_();
    if (videodir.isEmpty() && not videodirs.isEmpty()) {
        videodir = QUrl::fromLocalFile(videodirs[0]);
    }
    auto filename =
        QFileDialog::getOpenFileName(this, tr("open video file"), videodir.toLocalFile(), tr("Videos (*.mp4)"));
    auto filepath = QUrl::fromLocalFile(filename);
    if (not filepath.isValid()) {
        QMessageBox::warning(nullptr, tr("warning"), tr("no file was selected"));
        return;
    }
    source_path_ = filepath;
    QDir filedir{filepath.toLocalFile()};
    filedir.cdUp();
    write_video_dir_cache_(QUrl::fromLocalFile(filedir.path()));
    ui_->label_source_path->setText(source_path_.toLocalFile());
    ui_->pushButton_save->setEnabled(true);
    process_ = new ProcessWidget(true);
    process_->setAttribute(Qt::WA_DeleteOnClose, true);
    create_savefile_name_();
}

void MainWindow::select_output_dir_() {
    auto output_dir = QFileDialog::getExistingDirectory(this, tr("result directory"), ui_->lineEdit_output_dir->text());
    if (not QDir{output_dir}.exists(output_dir)) {
        QMessageBox::warning(nullptr, tr("no existing dir"), tr("no existing directory was selected"));
        return;
    }
    ui_->lineEdit_output_dir->setText(output_dir);
}
namespace impl_ {
constexpr Qt::ConnectionType ONESHOT_AUTO_CONNECTION =
    static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::SingleShotConnection);
class OnTrue {
    std::function<void(void)> func_;

   public:
    OnTrue(std::function<void(void)> func) : func_(func) { TRACE }
    void operator()(bool arg) {
        TRACE
        if (arg) {
            this->func_();
        }
    }
};
}  // namespace impl_
void MainWindow::create_savefile_name_() {
    TRACE
    QString filename = source_path_.fileName();
    if (settings_->contains("savefile_name_plugin") && settings_->value("savefile_name_plugin") != NO_PLUGIN) {
        process_->start(
            PYTHON,
            {savefile_name_plugins_dir_().absoluteFilePath(settings_->value("savefile_name_plugin").toString()),
             filename},
            false);
        connect(process_, &ProcessWidget::finished, this, impl_::OnTrue([=] { this->register_savefile_name_(); }),
                impl_::ONESHOT_AUTO_CONNECTION);
    } else {
        register_savefile_name_();
    }
}
void MainWindow::register_savefile_name_() {
    TRACE
    QString default_savefile_name = source_path_.fileName();
    if (settings_->contains("savefile_name_plugin") && settings_->value("savefile_name_plugin") != NO_PLUGIN) {
        default_savefile_name = process_->get_stdout();
    }
    QDir source_dir{source_path_.toLocalFile()};
    source_dir.cdUp();
    ui_->lineEdit_output_dir->setText(source_dir.absolutePath());
    ui_->lineEdit_output_filename->setText(default_savefile_name);
    probe_for_video_info_();
}
void MainWindow::probe_for_video_info_() {
    TRACE
    QStringList ffprobe_arguments{"-hide_banner", "-show_streams", "-show_format", "-of", "json", "-v", "quiet"};
    QString filename = source_path_.toLocalFile();
    process_->start("ffprobe", ffprobe_arguments + QStringList{filename}, true);
    connect(process_, &ProcessWidget::finished, this, impl_::OnTrue([=] { this->register_video_info_(); }),
            impl_::ONESHOT_AUTO_CONNECTION);
}
void MainWindow::register_video_info_() {
    TRACE
    QString filepath = source_path_.toLocalFile();
    QRegularExpression fraction_pattern(R"((\d+)/(\d+))");
    QJsonParseError err;
    auto prove_result = QJsonDocument::fromJson(process_->get_stdout().toUtf8(), &err);
    if (prove_result.isNull()) {
        QMessageBox::critical(this, tr("ffprobe parse error"),
                              tr("failed to parse result of ffprobe\nerror message:%1").arg(err.errorString()));
        return;
    }
    auto duration_str = prove_result.object()["format"].toObject()["duration"].toString();
    bool ok;
    double duration = duration_str.toDouble(&ok);
    if (not ok) {
        QMessageBox::critical(this, tr("ffprobe parse error"), tr("failed to parse duration [%1]").arg(duration_str));
        return;
    }
    source_length_ = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(duration));
    auto info = concat::VideoInfo::create_input_info();
    bool video_found = false, audio_found = false;
    for (auto stream_value : prove_result.object()["streams"].toArray()) {
        auto stream = stream_value.toObject();
        if (stream["codec_type"] == "video") {
            video_found = true;
            info.video_codec = stream["codec_name"].toString();
            info.resolution = QSize(stream["width"].toInt(), stream["height"].toInt());
            auto match = fraction_pattern.match(stream["r_frame_rate"].toString());
            bool ok1, ok2;
            info.framerate = static_cast<double>(match.captured(1).toInt(&ok1)) / match.captured(2).toInt(&ok2);
            if (not(ok1 && ok2)) {
                QMessageBox::critical(this, tr("ffprobe parse error"),
                                      tr("failed to parse frame rate [%1]").arg(stream["r_frame_rate"].toString()));
                return;
            }
            match = fraction_pattern.match(stream["avg_frame_rate"].toString());
            double avg_framerate = static_cast<double>(match.captured(1).toInt(&ok1)) / match.captured(2).toInt(&ok2);
            if (not(ok1 && ok2)) {
                QMessageBox::critical(this, tr("ffprobe parse error"),
                                      tr("failed to parse frame rate [%1]").arg(stream["r_frame_rate"].toString()));
                return;
            }
            info.is_vfr = std::get<double>(info.framerate) != avg_framerate;
        } else if (stream["codec_type"] == "audio") {
            audio_found = true;
            info.audio_codec = stream["codec_name"].toString();
        }
    }
    if (not video_found) {
        QMessageBox::critical(this, tr("ffprobe parse error"), tr("video stream was not found"));
        return;
    }
    if (not audio_found) {
        QMessageBox::critical(this, tr("ffprobe parse error"), tr("audio stream was not found"));
        return;
    }
    source_video_info_ = info;
    if (current_preset_ == tr("custom")) {
        cache_ = ui_->videoInfoWidget->info();
    }
    change_preset_(ui_->comboBox_preset->currentText());
}
namespace impl_ {
int decode_ffmpeg(QStringView, QStringView new_stderr) {
    TRACE
    QRegularExpression time_pattern(R"(time=(?<hours>\d\d):(?<minutes>\d\d):(?<seconds>\d\d).(?<centiseconds>\d\d))");
    auto match = time_pattern.match(new_stderr);
    if (not match.hasMatch()) {
        return -1;
    }
    using std::chrono::duration_cast;
    using std::chrono::hours;
    using std::chrono::minutes;
    using std::chrono::seconds;
    using centiseconds = std::chrono::duration<int, std::centi>;
    using std::chrono::milliseconds;
    return duration_cast<milliseconds>(
               hours(match.captured("hours").toInt()) + minutes(match.captured("minutes").toInt()) +
               seconds(match.captured("seconds").toInt()) + centiseconds(match.captured("centiseconds").toInt()))
        .count();
}
}  // namespace impl_
void MainWindow::re_encode_videos_() {
    TRACE
    bool resolution_changed = false, audio_codec_changed = false, video_codec_changed = false;
    auto output_video_info = ui_->videoInfoWidget->info();
    output_video_info.resolve_reference();
    VIDEO_RE_ENCODER_TRY_VARIANT {
        if (std::get<QSize>(output_video_info.resolution) != std::get<QSize>(source_video_info_.resolution)) {
            resolution_changed = true;
        }
    }
    VIDEO_RE_ENCODER_CATCH_VARIANT_2(output_video_info.resolution, source_video_info_.resolution);
    VIDEO_RE_ENCODER_TRY_VARIANT {
        if (std::get<QString>(output_video_info.audio_codec) != std::get<QString>(source_video_info_.audio_codec)) {
            audio_codec_changed = true;
        }
    }
    VIDEO_RE_ENCODER_CATCH_VARIANT_2(output_video_info.audio_codec, source_video_info_.audio_codec);
    VIDEO_RE_ENCODER_TRY_VARIANT {
        if (std::get<QString>(output_video_info.video_codec) != std::get<QString>(source_video_info_.video_codec)) {
            video_codec_changed = true;
        }
    }
    VIDEO_RE_ENCODER_CATCH_VARIANT_2(output_video_info.video_codec, source_video_info_.video_codec);
    QStringList arguments;
    VIDEO_RE_ENCODER_TRY_VARIANT {
        // clang-format off
        arguments << output_video_info.input_file_args
                  << "-i" << source_path_.toLocalFile()
                  << "-c:a" << (audio_codec_changed? std::get<QString>(output_video_info.audio_codec) : "copy")
                  << "-c:v" << (video_codec_changed? std::get<QString>(output_video_info.video_codec) : "copy");
        // clang-format on
    }
    VIDEO_RE_ENCODER_CATCH_VARIANT_2(output_video_info.audio_codec, output_video_info.video_codec);
    VIDEO_RE_ENCODER_TRY_VARIANT {
        if (resolution_changed) {
            auto resolution = std::get<QSize>(output_video_info.resolution);
            arguments << "-s" << QStringLiteral("%1x%2").arg(resolution.width()).arg(resolution.height());
        }
    }
    VIDEO_RE_ENCODER_CATCH_VARIANT(output_video_info.resolution);
    arguments += output_video_info.encoding_args;
    arguments << output_path_.toLocalFile();
    using VT = ProcessWidget::ProgressParams::ValueType;
    auto format = [](VT value) {
        return QTime::fromMSecsSinceStartOfDay(value).toString(tr("hh'h'mm'm'ss's'zzz'ms'"));
    };
    process_->start("ffmpeg", arguments, true,
                    {0, source_length_.count(), impl_::decode_ffmpeg, [format](VT, VT current, VT total) {
                         return QStringLiteral("%1/%2").arg(format(current)).arg(format(total));
                     }});
    connect(process_, &ProcessWidget::finished, this, &MainWindow::cleanup_after_saving_,
            impl_::ONESHOT_AUTO_CONNECTION);
}
void MainWindow::cleanup_after_saving_() { TRACE }
void MainWindow::start_saving_() {
    TRACE
    process_ = new ProcessWidget(false, this, Qt::Window | Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint);
    process_->setWindowModality(Qt::WindowModal);
    process_->setAttribute(Qt::WA_DeleteOnClose, true);
    process_->show();
    output_path_ =
        QUrl::fromLocalFile(QDir{ui_->lineEdit_output_dir->text()}.filePath(ui_->lineEdit_output_filename->text()));
    if (QFile{output_path_.toLocalFile()}.exists()) {
        QMessageBox::warning(nullptr, tr("existing file"),
                             tr("file '%1' already exists. This software currently doesn't support overwriting file.")
                                 .arg(output_path_.toLocalFile()));
        process_->close();
        return;
    }

    re_encode_videos_();
}
void MainWindow::save_result_() {
    TRACE
    start_saving_();
}

int MainWindow::savefile_name_plugin_index_() {
    TRACE
    int result = 0;
    if (settings_->contains("savefile_name_plugin")) {
        int raw_idx = savefile_name_plugins_().indexOf(settings_->value("savefile_name_plugin"));
        result = qMax(raw_idx, 0);
    }

    return result;
}
QDir MainWindow::savefile_name_plugins_dir_() {
    TRACE
    return QDir(QApplication::applicationDirPath() + "/plugins/savefile_name");
}
QStringList MainWindow::search_savefile_name_plugins_() {
    TRACE
    return savefile_name_plugins_dir_().entryList({"*.py"}, QDir::Files);
}

QStringList MainWindow::savefile_name_plugins_() {
    TRACE
    return QStringList{NO_PLUGIN} + search_savefile_name_plugins_();
}

void MainWindow::select_savefile_name_plugin_() {
    TRACE
    QString plugin = NO_PLUGIN;
    bool confirmed;
    plugin = QInputDialog::getItem(this, tr("select plugin"), tr("Select savefile name plugin."),
                                   savefile_name_plugins_(), savefile_name_plugin_index_(), false, &confirmed);
    if (confirmed && settings_ != nullptr) {
        settings_->setValue("savefile_name_plugin", plugin);
    }
}

void MainWindow::select_default_preset_() {
    QStringList presets(tr("custom"));
    for (const auto &[key, value] : presets_.as_table()) {
        if (key == "VERSION") {
            continue;
        }
        presets << QString::fromStdString(key);
    }
    bool confirmed = false;
    auto default_preset = QInputDialog::getItem(nullptr, tr("default preset"), tr("select default preset"), presets, 0,
                                                false, &confirmed);
    if (confirmed) {
        if (default_preset == tr("custom")) {
            default_preset = "custom";
        }
        settings_->setValue("default_preset", default_preset);
    }
}

void MainWindow::change_preset_(QString name) {
    if (name == tr("custom")) {
        ui_->videoInfoWidget->setEnabled(true);
        ui_->videoInfoWidget->set_infos(cache_, retrieve_input_info(source_video_info_));
    } else {
        ui_->videoInfoWidget->setEnabled(false);
        if (current_preset_ == tr("custom")) {
            cache_ = ui_->videoInfoWidget->info();
        }
        try {
            ui_->videoInfoWidget->set_infos(
                concat::VideoInfo::from_toml(presets_["VERSION"].as_integer(), presets_[name.toStdString()]),
                retrieve_input_info(source_video_info_));
        } catch (std::exception &e) {
            QMessageBox::warning(nullptr, tr("warning"),
                                 tr("failed to load preset '%1' info: \n%2").arg(name).arg(e.what()));
        }
    }
    current_preset_ = name;
}