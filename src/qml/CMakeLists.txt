# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

# QML plugin
add_library(kcmutilsqmlplugin qmlplugin.cpp)
target_link_libraries(kcmutilsqmlplugin PRIVATE Qt6::Qml kcmutils_proxy_model)
install (TARGETS kcmutilsqmlplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kcmutils)


install (DIRECTORY components DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kcmutils)
install (FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kcmutils)

# Library
add_library(KF5KCMUtilsQml configmodule.cpp managedconfigmodule.cpp)

ecm_generate_export_header(KF5KCMUtilsQml
    BASE_NAME KCMUtilsQml
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
)
target_include_directories(KF5KCMUtilsQml INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtilsQml>")

target_link_libraries(KF5KCMUtilsQml
  PUBLIC
    KF5::CoreAddons
  PRIVATE
    KF5KCMUtilsCore
    KF5::I18n
    KF5::Package
    KF5::Kirigami2
    KF5::ConfigGui
    Qt${QT_MAJOR_VERSION}::Quick
)

set_target_properties(KF5KCMUtilsQml PROPERTIES
                      VERSION ${KCMUTILS_VERSION}
                      SOVERSION ${KCMUTILS_SOVERSION}
                      EXPORT_NAME KCMUtilsQml)
ecm_generate_headers(KCMUtilsQml_HEADERS
  HEADER_NAMES
  ConfigModule
  ManagedConfigModule

  REQUIRED_HEADERS KCMUtilsQml_HEADERS
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kcmutilsqml_export.h
  ${KCMUtilsQml_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtilsQml COMPONENT Devel
)

install(TARGETS KF5KCMUtilsQml EXPORT KF5KCMUtilsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
