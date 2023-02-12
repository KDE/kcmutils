add_library(fakekcm MODULE fakekcm.cpp)

target_link_libraries(fakekcm PRIVATE KF6KCMUtils KF6::CoreAddons)

set(KCMUTILS_INTERNAL_TEST_MODE TRUE)
kcmutils_generate_desktop_file(fakekcm)
