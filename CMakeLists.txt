cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "5.114.0") # handled by release scripts
set(KF_DEP_VERSION "5.114.0") # handled by release scripts
project(KCMUtils VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 5.114.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)


set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

include(ECMGenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(CMakePackageConfigHelpers)
include(ECMAddQch)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupQtPluginMacroNames)
include(ECMDeprecationSettings)
include(KDEGitCommitHooks)


include(CMakeDependentOption)
option(TOOLS_ONLY "Only build the tools and not the rest of KCMUtils" OFF)
if (TOOLS_ONLY)
    find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Core)
    add_subdirectory(tools)
    return()
endif()

set(REQUIRED_QT_VERSION 5.15.2)
find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Widgets DBus Qml Quick QuickWidgets Test)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

ecm_setup_version(PROJECT VARIABLE_PREFIX KCMUTILS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kcmutils_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5KCMUtilsConfigVersion.cmake"
                        SOVERSION 5)

find_package(KF5ItemViews ${KF_DEP_VERSION} REQUIRED)
find_package(KF5ConfigWidgets ${KF_DEP_VERSION} REQUIRED)
find_package(KF5CoreAddons ${KF_DEP_VERSION} REQUIRED)
find_package(KF5GuiAddons ${KF_DEP_VERSION} REQUIRED)
find_package(KF5I18n ${KF_DEP_VERSION} REQUIRED)
find_package(KF5Service ${KF_DEP_VERSION} REQUIRED)
find_package(KF5XmlGui ${KF_DEP_VERSION} REQUIRED)
find_package(KF5Declarative ${KF_DEP_VERSION} REQUIRED)

ecm_setup_qtplugin_macro_names(
    JSON_NONE
        "KCMODULECONTAINER"
    CONFIG_CODE_VARIABLE
        PACKAGE_SETUP_AUTOMOC_VARIABLES
)

ecm_set_disabled_deprecation_versions(
    QT 5.15.2
    KF 5.90.0
    KSERVICE 5.78 # for KPluginInfo::dependencies
    KCOREADDONS 5.78 # for KPluginInfo::dependencies
    QUICKADDONS 5.87 # for ConfigModule::aboutData
    KCONFIGWIDGETS 5.84 # for KCModule::aboutData and corresponding constructor
)

add_definitions(-DTRANSLATION_DOMAIN=\"kcmutils5\")
ki18n_install(po)
add_subdirectory(tools)
add_subdirectory(src)
if(BUILD_TESTING)
add_subdirectory(autotests)
endif()


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5KCMUtils")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF5KCMUtils_QCH
        FILE KF5KCMUtilsQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5KCMUtilsQchTargets.cmake\")")
endif()

configure_package_config_file(
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

install(
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/KF5KCMUtilsMacros.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/src/kcmutilsgeneratemoduledata.cpp.in
    ${CMAKE_CURRENT_SOURCE_DIR}/src/kcmutilsgeneratemoduledata.h.in
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(EXPORT KF5KCMUtilsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5KCMUtilsTargets.cmake NAMESPACE KF5:: )

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kcmutils_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtils COMPONENT Devel
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
