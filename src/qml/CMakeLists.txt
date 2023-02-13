# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

# QML plugin
add_library(kcmutilsqmlplugin qmlplugin.cpp)
target_link_libraries(kcmutilsqmlplugin PRIVATE Qt6::Qml kcmutils_proxy_model)
install (TARGETS kcmutilsqmlplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kcmutils)


install (DIRECTORY components DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kcmutils)
install (FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kcmutils)

# Library
add_library(KF6KCMUtilsQuick kquickconfigmodule.cpp kquickmanagedconfigmodule.cpp sharedqmlengine.cpp kquickconfigmoduleloader.cpp)

ecm_generate_export_header(KF6KCMUtilsQuick
    BASE_NAME KCMUtilsQuick
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
)
target_include_directories(KF6KCMUtilsQuick INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtilsQuick>")

target_link_libraries(KF6KCMUtilsQuick
  PUBLIC
    KF6::CoreAddons
    KF6::ConfigGui
    Qt6::Qml
    KF6KCMUtilsCore
  PRIVATE
    KF6::I18n
    KF6::Package
    KF6::ConfigGui
    Qt6::Quick
    kcmutils_logging_STATIC
)

set_target_properties(KF6KCMUtilsQuick PROPERTIES
                      VERSION ${KCMUTILS_VERSION}
                      SOVERSION ${KCMUTILS_SOVERSION}
                      EXPORT_NAME KCMUtilsQml)
ecm_generate_headers(KCMUtilsQml_HEADERS
  HEADER_NAMES
  KQuickConfigModule
  KQuickManagedConfigModule
  KQuickConfigModuleLoader

  REQUIRED_HEADERS KCMUtilsQml_HEADERS
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kcmutilsquick_export.h
  ${KCMUtilsQml_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtilsQuick COMPONENT Devel
)

install(TARGETS KF6KCMUtilsQuick EXPORT KF6KCMUtilsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
