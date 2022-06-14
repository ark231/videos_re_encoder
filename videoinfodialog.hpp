#ifndef VIDEOINFODIALOG_HPP
#define VIDEOINFODIALOG_HPP

#include <QDialog>

#include "videoinfo.hpp"

namespace Ui {
class VideoInfoDialog;
}

class VideoInfoDialog : public QDialog {
    Q_OBJECT

   public:
    explicit VideoInfoDialog(QWidget *parent = nullptr);
    ~VideoInfoDialog();

    static concat::VideoInfo get_video_info(QWidget *parent, const QString &title, const QString &label,
                                            const concat::VideoInfo &initial_info, const concat::VideoInfo &input_info,
                                            bool *ok = nullptr, Qt::WindowFlags flags = Qt::WindowFlags(),
                                            Qt::InputMethodHints input_method_hints = Qt::ImhNone);

   private:
    Ui::VideoInfoDialog *ui_;
};

#endif  // VIDEOINFODIALOG_HPP
