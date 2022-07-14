# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(kcmdesktopfilegenerator kcmmetadatagenerator.cpp)
add_executable(KF5::kcmdesktopfilegenerator ALIAS kcmdesktopfilegenerator )
target_link_libraries(kcmdesktopfilegenerator Qt::Core)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5KCMUtils")
install(TARGETS kcmdesktopfilegenerator EXPORT KF5KCMUtilsToolingTargets DESTINATION ${KDE_INSTALL_LIBEXECDIR_KF})
install(EXPORT KF5KCMUtilsToolingTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5KCMUtilsToolingTargets.cmake NAMESPACE KF5::)
