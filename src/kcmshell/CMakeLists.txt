# SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-2-Clause
add_executable(kcmshell6 main.cpp)
target_compile_definitions(kcmshell6 PRIVATE -DPROJECT_VERSION="${KF_VERSION}")
ecm_mark_nongui_executable(kcmshell6)

target_link_libraries(kcmshell6
   KF6KCMUtils
   KF6::I18n
   kcmutils_logging_STATIC
)

if (NOT WIN32 AND NOT APPLE)
   target_link_libraries(kcmshell6 Qt6::DBus)
endif()

install(TARGETS kcmshell6 ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
