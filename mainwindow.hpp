#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAudioOutput>
#include <QDir>
#include <QMainWindow>
#include <QMap>
#include <QMediaPlayer>
#include <QSettings>
#include <QTemporaryDir>
#include <QUrl>
#include <chrono>
#include <optional>
#include <toml.hpp>
#include <tuple>

#include "processwidget.hpp"
#include "videoinfo.hpp"
#include "videoinfowidget.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

   private:
    QUrl read_video_dir_cache_();
    void write_video_dir_cache_(QUrl);
    void update_effective_period_of_cache_();

    void open_video_();
    void select_output_dir_();
    void save_result_();
    void select_savefile_name_plugin_();
    void edit_default_video_info_();

   private:
    Ui::MainWindow *ui_;
    ProcessWidget *process_ = nullptr;  // deleted on close
    QSettings *settings_ = nullptr;
    toml::value presets_;
    concat::VideoInfo source_video_info_;
    std::chrono::duration<int, std::milli> source_length_;
    static constexpr auto NO_PLUGIN = "do not use any plugins";
#ifdef _WIN32
    static constexpr auto PYTHON = "py";
#else
    static constexpr auto PYTHON = "python";
#endif
    QUrl source_path_ = QUrl{};
    QUrl output_path_ = QUrl{};

    QDir savefile_name_plugins_dir_();
    QStringList search_savefile_name_plugins_();
    QStringList savefile_name_plugins_();
    int savefile_name_plugin_index_();

    QString current_preset_;
    concat::VideoInfo cache_;
    void change_preset_(QString name);
    void select_default_preset_();

    // steps for opening file
    void create_savefile_name_();
    void register_savefile_name_();
    void start_opening_();
    void probe_for_video_info_();
    void register_video_info_();
    // end steps

    // steps for creating and saving result
    void start_saving_();
    void re_encode_videos_();
    void cleanup_after_saving_();
    // end steps
};
#endif  // MAINWINDOW_H
