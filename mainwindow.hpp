#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAudioOutput>
#include <QDir>
#include <QList>
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
class QListWidgetItem;

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

   private:
    enum class VideoDataRole {
        source_video_info = Qt::UserRole,  // concat::VideoInfo
        output_video_info,                 // concat::VideoInfo
        output_path,                       // QUrl
        length,  // QTime(一日を超えると表示がバグるだろうがまあいいだろう)
        preset,  // QString
    };
    Ui::MainWindow *ui_;
    ProcessWidget *process_ = nullptr;  // deleted on close
    QSettings *settings_ = nullptr;
    toml::value presets_;
    QList<QUrl> current_unregistered_input_paths_;
    int encoding_loop_current_index_ = 0;
    static constexpr auto NO_PLUGIN = "do not use any plugins";
#ifdef _WIN32
    static constexpr auto PYTHON = "py";
#else
    static constexpr auto PYTHON = "python";
#endif

    QDir savefile_name_plugins_dir_();
    QStringList search_savefile_name_plugins_();
    QStringList savefile_name_plugins_();
    int savefile_name_plugin_index_();

    void update_output_infos_();
    void update_total_length_();

    void register_user_video_info_(concat::VideoInfo new_value);
    void register_user_output_path_();

    void sort_files_();

    void change_preset_(QString name);
    void select_default_preset_();

    void register_output_path_();

    // steps for opening file
    void create_savefile_name_();
    void register_savefile_name_();
    void start_opening_();
    void probe_for_video_info_();
    void register_video_info_();
    // end steps

    // steps for creating and saving result
    void start_saving_();
    void re_encode_video_();
    void check_loop_state_();
    void cleanup_after_saving_();
    // end steps
};
#endif  // MAINWINDOW_H
