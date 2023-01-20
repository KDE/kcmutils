# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(core)
add_subdirectory(qml)

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

add_library(KF6KCMUtils ${kcmutils_LIB_SRCS})

ecm_qt_declare_logging_category(KF6KCMUtils
    HEADER kcmutils_debug.h
    IDENTIFIER KCMUTILS_LOG
    CATEGORY_NAME kf.kcmutils
    DESCRIPTION "KCMUtils"
    EXPORT KCMUTILS
)

ecm_generate_export_header(KF6KCMUtils
    BASE_NAME KCMUtils
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 5.66 5.76 5.82 5.85 5.87 5.88 5.90
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)
target_include_directories(KF6KCMUtils INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtils>")

target_link_libraries(KF6KCMUtils
  PUBLIC
    Qt${QT_MAJOR_VERSION}::Widgets
    KF6::ConfigWidgets  # KCModule
    KF6::Service        # KService
  PRIVATE
    KF6KCMUtilsCore
    kcmutils_proxy_model
    Qt${QT_MAJOR_VERSION}::DBus       # dbus usage in kcmoduleproxy.cpp
    Qt${QT_MAJOR_VERSION}::Qml
    Qt${QT_MAJOR_VERSION}::Quick
    Qt${QT_MAJOR_VERSION}::QuickWidgets
    KF6::CoreAddons
    KF6::GuiAddons # KIconUtils
    KF6::I18n
    KF6::ItemViews       # KWidgetItemDelegate
    KF6::XmlGui          # KAboutApplicationDialog
    KF6::QuickAddons
    KF6::Declarative
)

if (QT_MAJOR_VERSION STREQUAL "6")
    # TODO: instead of relying on KF6ConfigWidgetsConfig.cmake optionally getting KAuth dependency,
    # have KF6ConfigWidgetsConfig.cmake provide a proper flag/property about using the KAuth feature
    if (TARGET KF6::AuthWidgets)
        # KF6::ConfigWidgets has only KF6::AuthCore in public interface in KF6
        target_link_libraries(KF6KCMUtils PRIVATE KF6::AuthWidgets) # KAuth::ObjectDecorator
    endif()
endif()

set_target_properties(KF6KCMUtils PROPERTIES
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

install(TARGETS KF6KCMUtils EXPORT KF6KCMUtilsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
 kcmodule.desktop
 kcmoduleinit.desktop
DESTINATION  ${KDE_INSTALL_KSERVICETYPESDIR} )

if(BUILD_QCH)
    ecm_add_qch(
        KF6KCMUtils_QCH
        NAME KCMUtils
        BASE_NAME KF6KCMUtils
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KCMUtils_HEADERS}
            ${KSettings_HEADERS}
            ksettings/README.dox
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt5Widgets_QCH
            KF6ConfigWidgets_QCH
            KF6Service_QCH
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
ecm_generate_pri_file(BASE_NAME KCMUtils LIB_NAME KF6KCMUtils DEPS "widgets KConfigWidgets KService" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtils)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

ecm_qt_install_logging_categories(
    EXPORT KCMUTILS
    FILE kcmutils.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

