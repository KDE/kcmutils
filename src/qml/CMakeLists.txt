# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

install (DIRECTORY components DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kcmutils)
install (FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kcmutils)
