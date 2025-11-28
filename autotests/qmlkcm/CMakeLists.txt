# SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

kcmutils_add_qml_kcm(kcm_testqml SOURCES timesettings.cpp kcm.cpp)
target_link_libraries(kcm_testqml
    Qt6::Core
    KF6::CoreAddons
    KF6::I18n
    KF6KCMUtilsQuick
)

kcmutils_add_qml_kcm2(kcm_testqml2 URI org.kde.kcm.testqml2 SOURCES timesettings.cpp kcm.cpp)
target_link_libraries(kcm_testqml2
    PRIVATE
        Qt6::Core
        KF6::CoreAddons
        KF6::I18n
        KF6KCMUtilsQuick
)
