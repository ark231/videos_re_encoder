#ifndef VIDEOINFOWIDGET_HPP
#define VIDEOINFOWIDGET_HPP

#include <QWidget>
#include <memory>

#include "videoinfo.hpp"

namespace Ui {
class VideoInfoWidget;
}
class QListWidget;
class QPushButton;

class VideoInfoWidget : public QWidget {
    Q_OBJECT

   public:
    explicit VideoInfoWidget(QWidget *parent = nullptr);
    ~VideoInfoWidget();
    void set_infos(const concat::VideoInfo &initial_values, const concat::VideoInfo &input_info);
    concat::VideoInfo info() const;

   private:
    Ui::VideoInfoWidget *ui_;
    concat::VideoInfo input_info_;
    concat::VideoInfo cache_;
    void add_argument_slot_impl_(QListWidget *widget, QPushButton *button);
    void add_argument_slot_input_();
    void add_argument_slot_output_();
    void remove_current_argument_slot_impl_(QListWidget *widget, QPushButton *button);
    void remove_current_argument_slot_input_();
    void remove_current_argument_slot_output_();
    void toggle_input_is_enabled_(QString text, QVector<QWidget *> widgets);
    void update_input_resolution_(QString text);
    void update_input_framerate_(QString text);
    void update_input_audio_codec_(QString text);
    void update_input_video_codec_(QString text);
    void update_everything_();
};

#endif  // VIDEOINFOWIDGET_HPP
