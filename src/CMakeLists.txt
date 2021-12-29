add_subdirectory(core)

########### kcmutils ###############
set(kcmutils_LIB_SRCS
    kcmodulecontainer.cpp
    kcmodulecontainer.h
    kcmoduledata.cpp
    kcmoduledata.h
    kcmoduleinfo.cpp
    kcmoduleinfo.h
    kcmoduleloader.cpp
    kcmoduleloader.h
    kcmoduleproxy.cpp
    kcmoduleproxy.h
    kcmoduleproxy_p.h
    kcmoduleqml.cpp
    kcmoduleqml_p.h
    kcmultidialog.cpp
    kcmultidialog.h
    kcmultidialog_p.h
    kpluginselector.cpp
    kpluginselector.h
    kpluginselector_p.h
    kpluginwidget.cpp
    ksettings/dialog.cpp
    ksettings/dialog.h
    ksettings/dialog_p.h
    ksettings/dispatcher.cpp
    ksettings/dispatcher.h
    ksettings/dispatcher_p.h
    ksettings/pluginpage.cpp
    ksettings/pluginpage.h
    ksettingswidgetadaptor.cpp
    ksettingswidgetadaptor.h
)

add_library(KF5KCMUtils ${kcmutils_LIB_SRCS})
ecm_generate_export_header(KF5KCMUtils
    BASE_NAME KCMUtils
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 5.66 5.76 5.82 5.85 5.87 5.88 5.90
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)
target_include_directories(KF5KCMUtils INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtils>")

target_link_libraries(KF5KCMUtils
  PUBLIC
    Qt${QT_MAJOR_VERSION}::Widgets
    KF5::ConfigWidgets  # KCModule
    KF5::Service        # KService
  PRIVATE
    KF5KCMUtilsCore
    Qt${QT_MAJOR_VERSION}::DBus       # dbus usage in kcmoduleproxy.cpp
    Qt${QT_MAJOR_VERSION}::Qml
    Qt${QT_MAJOR_VERSION}::Quick
    Qt${QT_MAJOR_VERSION}::QuickWidgets
    KF5::CoreAddons
    KF5::GuiAddons # KIconUtils
    KF5::I18n
    KF5::ItemViews       # KWidgetItemDelegate
    KF5::XmlGui          # KAboutApplicationDialog
    KF5::QuickAddons
    KF5::Declarative
)
set_target_properties(KF5KCMUtils PROPERTIES
                      VERSION ${KCMUTILS_VERSION}
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
  KCModuleData
  KPluginWidget

  REQUIRED_HEADERS KCMUtils_HEADERS
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kcmutils_export.h
  ${KCMUtils_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtils COMPONENT Devel
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
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtils/ksettings COMPONENT Devel
)

install(TARGETS KF5KCMUtils EXPORT KF5KCMUtilsTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
 kcmodule.desktop
 kcmoduleinit.desktop
DESTINATION  ${KDE_INSTALL_KSERVICETYPESDIR} )

if(BUILD_QCH)
    ecm_add_qch(
        KF5KCMUtils_QCH
        NAME KCMUtils
        BASE_NAME KF5KCMUtils
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KCMUtils_HEADERS}
            ${KSettings_HEADERS}
            ksettings/README.dox
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt5Widgets_QCH
            KF5ConfigWidgets_QCH
            KF5Service_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KCMUTILS_EXPORT
            KCMUTILS_DEPRECATED
            KCMUTILS_DEPRECATED_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KCMUtils LIB_NAME KF5KCMUtils DEPS "widgets KConfigWidgets KService" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtils)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

ecm_qt_install_logging_categories(
    EXPORT KCMUTILS
    FILE kcmutils.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

