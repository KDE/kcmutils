# SPDX-FileCopyrightText: Year Author <email@company.com>
#
# SPDX-License-Identifier: BSD-2-Clause

kcoreaddons_add_plugin(kcm_time INSTALL_NAMESPACE "plasma/kcms/systemsettings")

target_sources(kcm_time PRIVATE timesettings.cpp)

target_link_libraries(kcm_time
    Qt6::Core
    KF6::CoreAddons
    KF6::I18n
    KF6KCMUtilsQuick
)

kpackage_install_package(package kcm_time kcms)

kcmutils_generate_desktop_file(kcm_time)
