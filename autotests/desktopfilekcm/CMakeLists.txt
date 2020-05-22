add_library(kcmtest MODULE kcmtest.cpp)

kcoreaddons_desktop_to_json(kcmtest kcmtest.desktop)

target_link_libraries(kcmtest KF5::CoreAddons)

