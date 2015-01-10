########### kcmutils ###############
set(kcmutils_LIB_SRCS
    kcmoduleinfo.cpp
    kcmoduleloader.cpp
    kcmultidialog.cpp
    kcmoduleproxy.cpp
    kpluginselector.cpp
    kcmodulecontainer.cpp
    ksettingswidgetadaptor.cpp
    ksettings/dispatcher.cpp
    ksettings/dialog.cpp
    ksettings/pluginpage.cpp
    ksettings/componentsdialog.cpp
)

add_library(KF5KCMUtils ${kcmutils_LIB_SRCS})
generate_export_header(KF5KCMUtils BASE_NAME KCMUtils)

target_include_directories(KF5KCMUtils INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KCMUtils>")

target_link_libraries(KF5KCMUtils
  PUBLIC
    Qt5::Widgets
    KF5::ConfigWidgets  # KCModule
    KF5::Service        # KService
  PRIVATE
    Qt5::DBus       # dbus usage in kcmoduleproxy.cpp
    KF5::CoreAddons
    KF5::I18n
    KF5::IconThemes     # KIconLoader
    KF5::ItemViews       # KWidgetItemDelegate
    KF5::XmlGui          # KAboutApplicationDialog
)
set_target_properties(KF5KCMUtils PROPERTIES
                      VERSION ${KCMUTILS_VERSION_STRING}
                      SOVERSION ${KCMUTILS_SOVERSION}
                      EXPORT_NAME KCMUtils)

ecm_generate_headers(KCMUtils_HEADERS
  HEADER_NAMES
  KCModuleInfo
  KCModuleLoader
  KCMultiDialog
  KCModuleProxy
  KPluginSelector
  KCModuleContainer

  REQUIRED_HEADERS KCMUtils_HEADERS
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kcmutils_export.h
  ${KCMUtils_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KCMUtils COMPONENT Devel
)

ecm_generate_headers(KSettings_HEADERS
  HEADER_NAMES
  Dispatcher
  Dialog
  PluginPage

  RELATIVE ksettings
  REQUIRED_HEADERS KSettings_HEADERS
)
install(FILES
  ${KSettings_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KCMUtils/ksettings COMPONENT Devel
)

install(TARGETS KF5KCMUtils EXPORT KF5KCMUtilsTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
 kcmodule.desktop
 kcmoduleinit.desktop
DESTINATION  ${KDE_INSTALL_KSERVICETYPES5DIR} )

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KCMUtils LIB_NAME KF5KCMUtils DEPS "widgets KConfigWidgets KService" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KCMUtils)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
