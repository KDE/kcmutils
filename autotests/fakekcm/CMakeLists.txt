kcoreaddons_add_plugin(fakekcm SOURCES fakekcm.cpp INSTALL_NAMESPACE "plasma/kcms/systemsettings_qwidgets")
target_link_libraries(fakekcm PRIVATE KF6KCMUtils KF6::CoreAddons)
kcmutils_generate_desktop_file(fakekcm)
