include(ECMAddTests)

add_subdirectory(jsonplugin)
add_subdirectory(desktopfilekcm)
add_subdirectory(module_data_codegen)
add_subdirectory(fakekcm)
add_subdirectory(qmlkcm)

ecm_add_tests(
    kcmultidialogtest.cpp
    LINK_LIBRARIES KF6KCMUtils Qt6::Test
)
