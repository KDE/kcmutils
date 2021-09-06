########### kcmutils ###############
set(kcmutils_LIB_SRCS
    kcmoduleinfo.cpp
    kcmoduledata.cpp
    kcmoduleloader.cpp
    kcmoduleqml.cpp
    kcmultidialog.cpp
    kcmoduleproxy.cpp
    kpluginselector.cpp
    kcmodulecontainer.cpp
    ksettingswidgetadaptor.cpp
    ksettings/dispatcher.cpp
    ksettings/dialog.cpp
    ksettings/pluginpage.cpp
)

ecm_qt_declare_logging_category(kcmutils_LIB_SRCS
    HEADER kcmutils_debug.h
    IDENTIFIER KCMUTILS_LOG
    CATEGORY_NAME kf.kcmutils
    DESCRIPTION "KCMUtils"
    EXPORT KCMUTILS
)

add_library(KF5KCMUtils ${kcmutils_LIB_SRCS})
ecm_generate_export_header(KF5KCMUtils
    BASE_NAME KCMUtils
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 5.66 5.76 5.82 5.85 5.87
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)
target_include_directories(KF5KCMUtils INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KCMUtils>")

target_link_libraries(KF5KCMUtils
  PUBLIC
    Qt5::Widgets
    KF5::ConfigWidgets  # KCModule
    KF5::Service        # KService
  PRIVATE
    Qt5::DBus       # dbus usage in kcmoduleproxy.cpp
    Qt5::Qml
    Qt5::Quick
    Qt5::QuickWidgets
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
ecm_generate_pri_file(BASE_NAME KCMUtils LIB_NAME KF5KCMUtils DEPS "widgets KConfigWidgets KService" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KCMUtils)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

ecm_qt_install_logging_categories(
    EXPORT KCMUTILS
    FILE kcmutils.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

