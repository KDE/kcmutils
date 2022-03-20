add_library(KF5KCMUtilsCore)

ecm_qt_declare_logging_category(KF5KCMUtilsCore
    HEADER kcmutilscore_debug.h
    IDENTIFIER KCMUTILS_CORE_LOG
    CATEGORY_NAME kf.kcmutils.core
    DESCRIPTION "KCMUtilsCore"
    EXPORT KCMUTILS_CORE
)
ecm_generate_export_header(KF5KCMUtilsCore
    BASE_NAME KCMUtilsCore
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
)
target_include_directories(KF5KCMUtilsCore INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtilsCore>")

target_sources(KF5KCMUtilsCore PRIVATE kpluginmodel.cpp)
target_link_libraries(KF5KCMUtilsCore
  PUBLIC
    KF5::CoreAddons
  PRIVATE
    KF5::ConfigCore
    KF5::Service # KServiceTypeTrader compat code path
    KF5::ItemViews
)

set_target_properties(KF5KCMUtilsCore PROPERTIES
                      VERSION ${KCMUTILS_VERSION}
                      SOVERSION ${KCMUTILS_SOVERSION}
                      EXPORT_NAME KCMUtilsCore)
ecm_generate_headers(KCMUtilsCore_HEADERS
  HEADER_NAMES
  KPluginModel

  REQUIRED_HEADERS KCMUtilsCore_HEADERS
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kcmutilscore_export.h
  ${KCMUtilsCore_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtilsCore COMPONENT Devel
)

install(TARGETS KF5KCMUtilsCore EXPORT KF5KCMUtilsTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

add_library(kcmutils_proxy_model STATIC kpluginproxymodel.cpp)
# Needed to link this static lib to shared libs
set_property(TARGET kcmutils_proxy_model PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(kcmutils_proxy_model PUBLIC KF5KCMUtilsCore KF5::ItemViews)
