include(ECMAddTests)

if(NOT EXCLUDE_DEPRECATED_BEFORE_AND_AT STREQUAL "CURRENT" AND
        EXCLUDE_DEPRECATED_BEFORE_AND_AT VERSION_LESS 5.88.0)
ecm_add_tests(
 kcmoduleinfotest.cpp
 kcmultidialogtest.cpp

 LINK_LIBRARIES KF5KCMUtils Qt${QT_MAJOR_VERSION}::Test
)
endif()

add_subdirectory(jsonplugin)
add_subdirectory(desktopfilekcm)
add_subdirectory(module_data_codegen)
add_subdirectory(fakekcm)
