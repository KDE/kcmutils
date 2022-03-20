# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

add_library(kcmutilsqmlplugin qmlplugin.cpp)
target_link_libraries(kcmutilsqmlplugin PRIVATE Qt${QT_MAJOR_VERSION}::Qml kcmutils_proxy_model)
install (TARGETS kcmutilsqmlplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kcmutils)


install (DIRECTORY components DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kcmutils)
install (FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kcmutils)
