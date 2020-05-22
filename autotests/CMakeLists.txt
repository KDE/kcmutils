include(ECMAddTests)

ecm_add_tests(
 kcmoduleinfotest.cpp

 LINK_LIBRARIES KF5KCMUtils Qt5::Test
)

add_subdirectory(jsonplugin)
add_subdirectory(desktopfilekcm)
