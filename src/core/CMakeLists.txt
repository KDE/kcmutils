add_library(KF5KCMUtilsCore STATIC)

ecm_qt_declare_logging_category(KF5KCMUtilsCore
    HEADER kcmutils_debug.h
    IDENTIFIER KCMUTILS_LOG
    CATEGORY_NAME kf.kcmutils
    DESCRIPTION "KCMUtils"
    EXPORT KCMUTILS
)


target_sources(KF5KCMUtilsCore PRIVATE kpluginmodel.cpp)
target_link_libraries(KF5KCMUtilsCore
  PUBLIC
    KF5::CoreAddons
  PRIVATE
    KF5::Service # KServiceTypeTrader compat code path
    KF5::ItemViews
)

# install this lib only in case of static builds, it is internal only
if (NOT BUILD_SHARED_LIBS)
    install(TARGETS KF5KCMUtilsCore EXPORT KF5KCMUtilsTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
endif()
