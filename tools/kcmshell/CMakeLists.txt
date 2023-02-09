add_executable(kcmshell6 main.cpp)
target_compile_definitions(kcmshell6 PRIVATE -DPROJECT_VERSION="${KF_VERSION}")
ecm_mark_nongui_executable(kcmshell6)

target_link_libraries(kcmshell6
   Qt6::DBus
   KF6KCMUtils
   KF6::I18n
   KF6::Activities
   Qt6::GuiPrivate
)

install(TARGETS kcmshell6 ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
