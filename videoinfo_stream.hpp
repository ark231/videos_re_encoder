#ifndef VIDEO_CONCATENATER_VIDEOINFO_STREAM
#define VIDEO_CONCATENATER_VIDEOINFO_STREAM
#include <QDebug>

#include "videoinfo.hpp"
namespace concat {
inline namespace operators {
template <class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
template <class T>
QDataStream& operator<<(QDataStream& stream, const SameAsHighest<T>& value) {
    stream << value.value;
    return stream;
}
template <class T>
QDataStream& operator>>(QDataStream& stream, SameAsHighest<T>& value) {
    stream >> value.value;
    return stream;
}
template <class T>
QDataStream& operator<<(QDataStream& stream, const SameAsLowest<T>& value) {
    stream << value.value;
    return stream;
}
template <class T>
QDataStream& operator>>(QDataStream& stream, SameAsLowest<T>& value) {
    stream >> value.value;
    return stream;
}
template <class T>
QDataStream& operator<<(QDataStream& stream, const ValueRange<T>& value) {
    stream << value.highest << value.lowest;
    return stream;
}
template <class T>
QDataStream& operator>>(QDataStream& stream, ValueRange<T>& value) {
    stream >> value.highest >> value.lowest;
    return stream;
}
template <class ValueType, class VariantType, std::size_t idx = 0>
std::size_t index_of_impl() {
    if constexpr (idx < std::variant_size_v<VariantType>) {
        if (typeid(remove_cvref_t<ValueType>) == typeid(std::variant_alternative_t<idx, VariantType>)) {
            return idx;
        } else {
            return index_of_impl<ValueType, VariantType, idx + 1>();
        }
    } else {
        return std::variant_npos;
    }
}
template <class ValueType, class VariantType>
std::size_t index_of() {
    return index_of_impl<ValueType, VariantType>();
}
template <class T>
QDataStream& operator<<(QDataStream& stream, const RangedVariant<T>& value) {
    stream << value.index();
    auto stream_ref = &stream;
    std::visit([stream_ref](const auto& contained_value) { (*stream_ref) << contained_value; }, value);
    return stream;
}
class VariantRetriever {
    std::size_t index_;
    QDataStream* stream_;
    template <std::size_t idx = 0, class VariantType>
    void try_retrieve_recursive_(VariantType& value) {
        if constexpr (idx < std::variant_size_v<remove_cvref_t<decltype(value)>>) {
            using CurrentType = std::variant_alternative_t<idx, remove_cvref_t<decltype(value)>>;
            if (index_ == index_of<remove_cvref_t<CurrentType>, VariantType>()) {
                CurrentType contained_value;
                (*stream_) >> contained_value;
                value = contained_value;
                return;
            }
            try_retrieve_recursive_<idx + 1>(value);
        }
    }

   public:
    VariantRetriever(QDataStream* stream) : stream_(stream) { (*stream) >> index_; }
    template <class VariantType>
    void retrieve_to(VariantType& value) {
        try_retrieve_recursive_(value);
    }
};
template <class T>
QDataStream& operator>>(QDataStream& stream, RangedVariant<T>& value) {
    VariantRetriever retriever(&stream);
    retriever.retrieve_to(value);
    return stream;
}
template <class T>
QDataStream& operator<<(QDataStream& stream, const SameAsInput<T>& value) {
    stream << value.value;
    return stream;
}
template <class T>
QDataStream& operator>>(QDataStream& stream, SameAsInput<T>& value) {
    stream >> value.value;
    return stream;
}
template <class T>
QDataStream& operator<<(QDataStream& stream, const SelectableVariant<T>& value) {
    stream << value.index();
    auto stream_ref = &stream;
    std::visit([stream_ref](const auto& contained_value) { (*stream_ref) << contained_value; }, value);
    return stream;
}
template <class T>
QDataStream& operator>>(QDataStream& stream, SelectableVariant<T>& value) {
    VariantRetriever retriever(&stream);
    retriever.retrieve_to(value);
    return stream;
}

QDataStream& operator<<(QDataStream& stream, const VideoInfo& info);
QDataStream& operator>>(QDataStream& stream, VideoInfo& info);
}  // namespace operators
}  // namespace concat
#endif