
add_library(jsonplugin MODULE jsonplugin.cpp)

kcoreaddons_desktop_to_json(jsonplugin jsonplugin.desktop)

target_link_libraries(jsonplugin KF5::CoreAddons)

set_target_properties(jsonplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/testplugins")
