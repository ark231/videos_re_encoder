#ifndef PROCESSWIDGET_HPP
#define PROCESSWIDGET_HPP

#include <QProcess>
#include <QString>
#include <QStringLiteral>
#include <QThread>
#include <QWidget>
#include <chrono>
#include <functional>
#include <list>
#include <numeric>
#include <optional>

namespace Ui {
class ProcessWidget;
}

class QTextEdit;

class ProcessWidget : public QWidget {
    Q_OBJECT

   public:
    explicit ProcessWidget(bool close_on_final = false, QWidget *parent = nullptr,
                           Qt::WindowFlags flags = Qt::WindowFlags());
    ~ProcessWidget();
    class ProgressParams {
       public:
        using Clock = std::chrono::high_resolution_clock;
        using TimePoint = Clock::time_point;
        // using Duration = std::chrono::nanoseconds;
        using Duration = std::chrono::duration<double, std::nano>;
        using ValueType = int;

       private:
        bool is_active_;
        /// @brief calculate(retrieve) progress from input
        /// @retval <0 error
        std::function<ValueType(QStringView, QStringView)> calc_progress_;
        std::function<QString(ValueType, ValueType, ValueType)> format_progress_;
        static constexpr int MAX_NUM_SAMPLES = 20;
        using Speed = double;  // ValueType/Duration
        std::list<Speed> samples_;
        TimePoint previous_time_;
        ValueType previous_value_;
        bool estimation_is_initialized_ = false;

       public:
        ValueType min;
        ValueType max;
        ProgressParams(
            ValueType min = 0, ValueType max = 100,
            std::optional<decltype(calc_progress_)> calc_progress = std::nullopt,
            decltype(format_progress_) format_progress =
                [](ValueType a0, ValueType a1, ValueType a2) {
                    return QStringLiteral("%1/%2").arg(a1 - a0).arg(a2 - a0);
                })
            : min(min), max(max), format_progress_(format_progress) {
            is_active_ = calc_progress.has_value();
            if (is_active_) {
                calc_progress_ = calc_progress.value();
            }
        }
        bool is_active() { return is_active_; }
        ValueType calc_progress(QStringView stdout_text, QStringView stderr_text) {
            Q_ASSERT(this->is_active());
            return calc_progress_(stdout_text, stderr_text);
        }
        QString format_progress(ValueType current) {
            Q_ASSERT(this->is_active());
            return format_progress_(this->min, current, this->max);
        }
        std::optional<Duration> estimate_remaining(int new_value, TimePoint now) {
            if (not estimation_is_initialized_) {
                previous_value_ = new_value;
                previous_time_ = now;
                estimation_is_initialized_ = true;
                return std::nullopt;
            }
            if (samples_.size() >= MAX_NUM_SAMPLES) {
                samples_.pop_front();
            }
            using std::chrono::duration_cast;
            samples_.push_back(static_cast<double>(new_value - previous_value_) /
                               duration_cast<Duration>(now - previous_time_).count());
            previous_value_ = new_value;
            previous_time_ = now;
            Speed average_speed = std::accumulate(samples_.begin(), samples_.end(), Speed(0)) / samples_.size();
            return Duration((max - new_value) / average_speed);
        }
    };
    /**
     * @brief start process with arguments
     *
     * @param command
     * @param arguments
     * @param is_final if this is true, close button is enabled when program finishes.
     * @param progress_params parameters for progress bar
     */
    void start(const QString &command, const QStringList &arguments, bool is_final = true,
               ProgressParams progress_params = ProgressParams());
    /**
     * @brief if QProcess::waitForStarted() returned false, show error message
     *
     * @param timeout_msec
     */
    bool wait_for_started_with_check(int timeout_msec = -1);
    /**
     * @brief if QProcess::waitForFinished() returned false, show error message
     *
     * @param timeout_msec
     */
    bool wait_for_finished_with_check(int timeout_msec = -1);
    /**
     * @brief Get the content of stdout textarea. This function does not block.
     * @warning If this function is called while process is running, returned value will be incomplete.
     *
     * @param index index of command whose command will be returned. negative value means latest command
     * @return QString content of stdout textarea
     */
    QString get_stdout(int index = -1);
    /**
     * @brief Get the content of stderr textarea. This function does not block.
     * @warning If this function is called while process is running, returned value will be incomplete.
     *
     * @param index index of command whose command will be returned. negative value means latest command
     * @return QString content of stderr textarea
     */
    QString get_stderr(int index = -1);
    void clear_stdout(int index = -1);
    void clear_stderr(int index = -1);
    QString program();
    QStringList arguments();

   signals:
    void finished(bool is_success);

   private:
    Ui::ProcessWidget *ui_;
    QThread thread_;
    QProcess *process_ = nullptr;
    int current_stdout_tab_idx_ = -1;
    int current_stderr_tab_idx_ = -1;
    ProgressParams current_progress_params_;
    bool close_on_final_;
   signals:
    void start_process(const QString &command, const QStringList &arguments, QIODeviceBase::OpenMode);
    void sigkill();
   private slots:
    void update_label_on_start_();
    void update_label_on_finish_(int exit_code, QProcess::ExitStatus exit_status);
    void update_stdout_();
    void update_stderr_();
    void kill_process_();
    void enable_closing_();
    void disable_closing_();
    void do_close_();
    void show_error_(QProcess::ProcessError error);

   private:
    QTextEdit *stdout_textedit_of_(int idx);
    QTextEdit *stderr_textedit_of_(int idx);
    void update_progress_(QStringView stdout_text, QStringView stderr_text);
};

#endif  // PROCESSWIDGET_HPP
