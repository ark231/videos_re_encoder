#include "videoinfo.hpp"
namespace concat {
void VideoInfo::resolve_reference() {
    if (std::holds_alternative<SameAsHighest<QSize>>(this->resolution)) {
        this->resolution = std::get<SameAsHighest<QSize>>(this->resolution).value;
    } else if (std::holds_alternative<SameAsLowest<QSize>>(this->resolution)) {
        this->resolution = std::get<SameAsLowest<QSize>>(resolution).value;
    }
    if (std::holds_alternative<SameAsHighest<double>>(this->framerate)) {
        this->framerate = std::get<SameAsHighest<double>>(this->framerate).value;
    } else if (std::holds_alternative<SameAsLowest<double>>(this->framerate)) {
        this->framerate = std::get<SameAsLowest<double>>(this->framerate).value;
    }
    if (std::holds_alternative<SameAsInput<QString>>(this->audio_codec)) {
        this->audio_codec = std::get<SameAsInput<QString>>(this->audio_codec).value;
    }
    if (std::holds_alternative<SameAsInput<QString>>(this->video_codec)) {
        this->video_codec = std::get<SameAsInput<QString>>(this->video_codec).value;
    }
}
VideoInfo VideoInfo::from_toml(int version, toml::value& toml_value) {
    VideoInfo result;
    if (toml_value["resolution"].is_string()) {
        auto resolution = toml_value["resolution"].as_string();
        if (resolution == "same as highest") {
            result.resolution = SameAsHighest<QSize>();
        } else if (resolution == "same as lowest") {
            result.resolution = SameAsLowest<QSize>();
        }
    } else {
        auto resolution = toml_value["resolution"];
        result.resolution = QSize(resolution["width"].as_integer(), resolution["height"].as_integer());
    }
    if (toml_value["framerate"].is_string()) {
        auto framerate = toml_value["framerate"].as_string();
        if (framerate == "same as highest") {
            result.framerate = SameAsHighest<double>();
        } else if (framerate == "same as lowest") {
            result.framerate = SameAsLowest<double>();
        }
    } else {
        result.framerate = toml_value["framerate"].as_floating();
    }
    result.is_vfr = toml_value["is_vfr"].as_boolean();
    auto audio_codec = toml_value["audio_codec"].as_string();
    if (audio_codec == "same as input") {
        result.audio_codec = SameAsInput<QString>();
    } else {
        result.audio_codec = QString::fromStdString(audio_codec);
    }
    auto video_codec = toml_value["video_codec"].as_string();
    if (video_codec == "same as input") {
        result.video_codec = SameAsInput<QString>();
    } else {
        result.video_codec = QString::fromStdString(video_codec);
    }
    auto encoding_args = toml::find<std::vector<std::string>>(toml_value, "encoding_args");
    for (const auto& encoding_arg : encoding_args) {
        result.encoding_args.push_back(QString::fromStdString(encoding_arg));
    }
    if (version >= 1) {
        auto input_file_args = toml::find<std::vector<std::string>>(toml_value, "input_file_args");
        for (const auto& input_file_arg : input_file_args) {
            result.input_file_args.push_back(QString::fromStdString(input_file_arg));
        }
    }
    return result;
}
}  // namespace concat