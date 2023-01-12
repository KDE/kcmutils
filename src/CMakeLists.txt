# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(core)
add_subdirectory(qml)

########### kcmutils ###############
set(kcmutils_LIB_SRCS
    kcmoduledata.cpp
    kcmoduledata.h
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
    kpluginwidget.cpp
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
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)
target_include_directories(KF6KCMUtils INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtils>")

target_link_libraries(KF6KCMUtils
  PUBLIC
    Qt6::Widgets
    KF6::CoreAddons # KPluginMetaData
    KF6::ConfigWidgets  # KCModule
  PRIVATE
    KF6KCMUtilsCore
    kcmutils_proxy_model
    Qt6::DBus       # dbus usage in kcmoduleproxy.cpp
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickWidgets
    KF6::GuiAddons # KIconUtils
    KF6::I18n
    KF6::ItemViews       # KWidgetItemDelegate
    KF6::XmlGui          # KAboutApplicationDialog
    KF6::QuickAddons
    KF6::Kirigami2
)
# On Windows, this target does not exist
if (TARGET KF6::AuthWidgets)
    target_link_libraries(KF6KCMUtils PRIVATE KF6::AuthWidgets) # KAuth::ObjectDecorator
endif()

set_target_properties(KF6KCMUtils PROPERTIES
                      VERSION ${KCMUTILS_VERSION}
                      SOVERSION ${KCMUTILS_SOVERSION}
                      EXPORT_NAME KCMUtils)

ecm_generate_headers(KCMUtils_HEADERS
  HEADER_NAMES
  KCModuleLoader
  KCMultiDialog
  KCModuleProxy
  KCModuleData
  KPluginWidget

  REQUIRED_HEADERS KCMUtils_HEADERS
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kcmutils_export.h
  ${KCMUtils_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtils COMPONENT Devel
)

install(TARGETS KF6KCMUtils EXPORT KF6KCMUtilsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

if(BUILD_QCH)
    ecm_add_qch(
        KF6KCMUtils_QCH
        NAME KCMUtils
        BASE_NAME KF6KCMUtils
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KCMUtils_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Widgets_QCH
            KF6ConfigWidgets_QCH
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
ecm_generate_pri_file(BASE_NAME KCMUtils LIB_NAME KF6KCMUtils DEPS "widgets KConfigWidgets" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtils)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

ecm_qt_install_logging_categories(
    EXPORT KCMUTILS
    FILE kcmutils.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

add_subdirectory(kcmshell)
