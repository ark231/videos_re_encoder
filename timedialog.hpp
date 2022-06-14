#ifndef TIMEDIALOG_HPP
#define TIMEDIALOG_HPP

#include <QDialog>
#include <QTime>

namespace Ui {
class TimeDialog;
}

class TimeDialog : public QDialog {
    Q_OBJECT

   public:
    explicit TimeDialog(QWidget *parent = nullptr);
    ~TimeDialog();

    static QTime get_time(QWidget *parent, const QString &title, const QString &label, const QTime time,
                          bool *ok = nullptr, Qt::WindowFlags flags = Qt::WindowFlags(),
                          Qt::InputMethodHints input_method_hints = Qt::ImhNone);

   private:
    Ui::TimeDialog *ui_;
};

#endif  // TIMEDIALOG_HPP
