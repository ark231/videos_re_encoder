#include <QApplication>
#include <QLocale>
#include <QTranslator>

#if defined(_WIN32) && !defined(NDEBUG)
#    define NOMINMAX
#    include <windows.h>
#endif

#include "mainwindow.hpp"
#include "videoinfo_stream.hpp"

int main(int argc, char *argv[]) {
#if defined(_WIN32) && !defined(NDEBUG)
    // https://qiita.com/comocc/items/4604bea440018dfb5bd1
    FILE *fp;
    if (not AttachConsole(ATTACH_PARENT_PROCESS)) {
        AllocConsole();
    }
    freopen_s(&fp, "CONOUT$", "w", stdout); /* 標準出力(stdout)を新しいコンソールに向ける */
    freopen_s(&fp, "CONOUT$", "w", stderr); /* 標準エラー出力(stderr)を新しいコンソールに向ける */
#endif
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();  // NOLINT(readability-identifier-naming)
    for (const QString &locale : uiLanguages) {
        const QString baseName = "video_cutter_" + QLocale(locale).name();  // NOLINT(readability-identifier-naming)
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    qRegisterMetaType<concat::VideoInfo>("concat::VideoInfo");

    MainWindow w;
    w.show();
    return a.exec();
}
