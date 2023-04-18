include(ECMAddTests)

# Ensure we do not actually install the plugins/desktop files
set(KCOREADDONS_INTERNAL_SKIP_PLUGIN_INSTALLATION TRUE)
set(KCMUTILS_INTERNAL_TEST_MODE TRUE)

add_subdirectory(jsonplugin)
add_subdirectory(desktopfilekcm)
add_subdirectory(module_data_codegen)
add_subdirectory(fakekcm)
add_subdirectory(qmlkcm)

ecm_add_tests(
    kcmultidialogtest.cpp
    kcmloadtest.cpp
    LINK_LIBRARIES KF6KCMUtils Qt6::Test
)
