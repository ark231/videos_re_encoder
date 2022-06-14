#ifndef VIDEO_CONCATENATER_VIDEOINFO
#define VIDEO_CONCATENATER_VIDEOINFO

#include <QMetaType>
#include <QSet>
#include <QSize>
#include <QString>
#include <QVector>
#include <toml.hpp>
#include <variant>
namespace concat {
template <class T>
struct SameAsHighest {
    T value;
};
template <class T>
struct SameAsLowest {
    T value;
};
template <class T>
struct SameAsInput {
    T value;
};
template <class T>
struct ValueRange {
    T highest;
    T lowest;
};
template <class T>
using RangedVariant = std::variant<SameAsHighest<T>, SameAsLowest<T>, T, ValueRange<T>>;
template <class T>
using SelectableVariant = std::variant<SameAsInput<T>, T, QSet<T>>;
struct VideoInfo {
    static constexpr int VERSION = 1;
    RangedVariant<QSize> resolution;
    RangedVariant<double> framerate;
    bool is_vfr;
    SelectableVariant<QString> audio_codec;
    SelectableVariant<QString> video_codec;
    QVector<QString> encoding_args;
    QVector<QString> input_file_args;

    static VideoInfo from_toml(int version, toml::value& toml_value);
    static VideoInfo create_input_info() {
        return {ValueRange<QSize>{}, ValueRange<double>{}, true, QSet<QString>{}, QSet<QString>{}};
    }
    void resolve_reference();
};
}  // namespace concat
Q_DECLARE_METATYPE(concat::SameAsHighest<QSize>);
Q_DECLARE_METATYPE(concat::SameAsHighest<double>);
Q_DECLARE_METATYPE(concat::SameAsLowest<QSize>);
Q_DECLARE_METATYPE(concat::SameAsLowest<double>);
Q_DECLARE_METATYPE(concat::ValueRange<QSize>);
Q_DECLARE_METATYPE(concat::ValueRange<double>);
Q_DECLARE_METATYPE(concat::RangedVariant<QSize>);
Q_DECLARE_METATYPE(concat::RangedVariant<double>);
Q_DECLARE_METATYPE(concat::SelectableVariant<QString>);
Q_DECLARE_METATYPE(concat::VideoInfo);

#endif