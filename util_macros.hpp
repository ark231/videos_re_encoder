#ifndef VIDEO_RE_ENCODER_UTIL_MACROS
#define VIDEO_RE_ENCODER_UTIL_MACROS

#define VIDEO_RE_ENCODER_TRY_VARIANT try
#define VIDEO_RE_ENCODER_CATCH_VARIANT_HEADER_ \
    catch (std::bad_variant_access & e) {      \
        qDebug() << __FILE__ << __LINE__ << __func__ << QString{e.what()} << "when handling "
#define VIDEO_RE_ENCODER_INDEX_(target) "index:" << target.index()
#define VIDEO_RE_ENCODER_CATCH_VARIANT(target)                                             \
    VIDEO_RE_ENCODER_CATCH_VARIANT_HEADER_ #target "." << VIDEO_RE_ENCODER_INDEX_(target); \
    }
#define VIDEO_RE_ENCODER_CATCH_VARIANT_2(target0, target1)                                                  \
    VIDEO_RE_ENCODER_CATCH_VARIANT_HEADER_ #target0 << VIDEO_RE_ENCODER_INDEX_(target0) << "or" << #target1 \
                                                    << VIDEO_RE_ENCODER_INDEX_(target1);                    \
    }

#endif