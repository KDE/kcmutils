cmake_minimum_required(VERSION 2.8.12)

project(KCMUtils)

find_package(ECM 1.5.0 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

set(REQUIRED_QT_VERSION "5.2.0")
find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Widgets DBus)

include(FeatureSummary)
include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMPackageConfigHelpers)

set(KF5_VERSION "5.5.0") # handled by release scripts
set(KF5_DEP_VERSION "5.5.0") # handled by release scripts
ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KCMUTILS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kcmutils_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5KCMUtilsConfigVersion.cmake"
                        SOVERSION 5)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    find_package(KF5ItemViews ${KF5_DEP_VERSION} REQUIRED)
    find_package(KF5ConfigWidgets ${KF5_DEP_VERSION} REQUIRED)
    find_package(KF5CoreAddons ${KF5_DEP_VERSION} REQUIRED)
    find_package(KF5I18n ${KF5_DEP_VERSION} REQUIRED)
    find_package(KF5IconThemes ${KF5_DEP_VERSION} REQUIRED)
    find_package(KF5Service ${KF5_DEP_VERSION} REQUIRED)
    find_package(KF5XmlGui ${KF5_DEP_VERSION} REQUIRED)
endif()

add_definitions(-DTRANSLATION_DOMAIN=\"kcmutils5\")
if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ki18n_install(po)
endif()
add_subdirectory(src)


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5KCMUtils")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5KCMUtilsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5KCMUtilsConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5KCMUtilsConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5KCMUtilsConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5KCMUtilsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5KCMUtilsTargets.cmake NAMESPACE KF5:: )

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kcmutils_version.h
  DESTINATION ${KF5_INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
