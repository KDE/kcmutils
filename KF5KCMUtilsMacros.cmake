# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

#.rst:
# KF5KCMUtilsGenerateModuleData
# ---------------------------
#
# This module provides the ``kcmutils_generate_module_data`` function for
# generating basic module data classes. Class derivated from KCModuleData
# ::
#
#   kcmutils_generate_module_data(<sources_var>
#       MODULE_DATA_CLASS_NAME <class_name>
#       MODULE_DATA_HEADER <header_file_name>
#       SETTINGS_HEADERS <setting_header.h> [<second_setting_header.h> [...]]]
#       SETTINGS_CLASSES <SettingClass> [<SecondSettingClass> [...]]]
#   )
#
# A header file, ``<header_file_name>``, will be generated along with a corresponding
# source file, which will be added to ``<sources_var>``. These will provide a
# KCModuleData that can be referred to from C++ code using ``<class_name>``.
# This module will autoregister settings declared on ``setting_header.h`` with class name ``SettingClass``.
# Multiple settings classes / settings headers can be specified.

include(CMakeParseArguments)

set(_KCMODULE_DATA_TEMPLATE_CPP "${CMAKE_CURRENT_LIST_DIR}/kcmutilsgeneratemoduledata.cpp.in")
set(_KCMODULE_DATA_TEMPLATE_H   "${CMAKE_CURRENT_LIST_DIR}/kcmutilsgeneratemoduledata.h.in")

function(kcmutils_generate_module_data sources_var)
    set(options)
    set(oneValueArgs MODULE_DATA_CLASS_NAME MODULE_DATA_HEADER NAMESPACE)
    set(multiValueArgs SETTINGS_HEADERS SETTINGS_CLASSES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unexpected arguments to kcmutils_generate_module_data: ${ARG_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT ARG_MODULE_DATA_HEADER)
        message(FATAL_ERROR "Missing MODULE_DATA_HEADER argument for kcmutils_generate_module_data")
    endif()

    if(NOT ARG_MODULE_DATA_CLASS_NAME)
        message(FATAL_ERROR "Missing MODULE_DATA_CLASS_NAME argument for kcmutils_generate_module_data")
    endif()

    if(NOT ARG_SETTINGS_HEADERS)
        message(FATAL_ERROR "Missing SETTINGS_HEADERS argument for kcmutils_generate_module_data")
    endif()

    if(NOT ARG_SETTINGS_CLASSES)
        message(FATAL_ERROR "Missing SETTINGS_CLASSES argument for kcmutils_generate_module_data")
    endif()

    set(MODULE_DATA_CLASS_NAME ${ARG_MODULE_DATA_CLASS_NAME})
    get_filename_component(HEADER_NAME "${ARG_MODULE_DATA_HEADER}" NAME)

    string(REGEX REPLACE "[^a-zA-Z0-9]" "_" GUARD_NAME "${HEADER_NAME}")
    string(TOUPPER "${GUARD_NAME}" GUARD_NAME)

    string(FIND "${ARG_MODULE_DATA_HEADER}" "." pos REVERSE)
    if (pos EQUAL -1)
        set(cpp_filename "${ARG_MODULE_DATA_HEADER}.cpp")
    else()
        string(SUBSTRING "${ARG_MODULE_DATA_HEADER}" 0 ${pos} base_filename)
        set(cpp_filename "${base_filename}.cpp")
    endif()

    set(INCLUDES_SETTINGS)
    foreach(_header ${ARG_SETTINGS_HEADERS})
        set(INCLUDES_SETTINGS "${INCLUDES_SETTINGS}#include \"${_header}\"\n")
    endforeach()

    set(SETTINGS_FORWARD_DECLARATION)
    set(SETTINGS_CONSTRUCTOR_INITIALIZATION)
    set(SETTINGS_METHOD_DEFINITION)
    set(SETTINGS_METHOD_DECLARATION)
    list(LENGTH ARG_SETTINGS_CLASSES _nb_settings)
    foreach(_class ${ARG_SETTINGS_CLASSES})
        if(_nb_settings EQUAL 1)
            set(_setting_attribute "m_settings")
            set(_setting_getter "settings")
        else()
            # lower first letter
            string(SUBSTRING ${_class} 0 1 _first_letter)
            string(TOLOWER ${_first_letter} _first_letter)
            string(REGEX REPLACE "^.(.*)" "${_first_letter}\\1" _method_name "${_class}")
            set(_setting_attribute "m_${_method_name}")
            set(_setting_getter "${_method_name}")
        endif()

        set(SETTINGS_FORWARD_DECLARATION "${SETTINGS_FORWARD_DECLARATION}class ${_class};\n")
        set(SETTINGS_CONSTRUCTOR_INITIALIZATION "${SETTINGS_CONSTRUCTOR_INITIALIZATION}, ${_setting_attribute}(new ${_class}(this))\n")
        set(SETTINGS_METHOD_DECLARATION "${SETTINGS_METHOD_DECLARATION}${_class} *${_setting_getter}() const;\n")
        set(SETTINGS_ATTRIBUTE_DECLARATION "${SETTINGS_ATTRIBUTE_DECLARATION}${_class} *${_setting_attribute};\n")
        set(SETTINGS_METHOD_DEFINITION "${SETTINGS_METHOD_DEFINITION}${_class} *${ARG_MODULE_DATA_CLASS_NAME}::${_setting_getter}() const
{
    return ${_setting_attribute};
}\n\n")
    endforeach()

    if(ARG_NAMESPACE)
        set(OPEN_NAMESPACE "namespace ${ARG_NAMESPACE} {")
        set(CLOSE_NAMESPACE "}")
    endif()
    configure_file("${_KCMODULE_DATA_TEMPLATE_CPP}" "${cpp_filename}")
    configure_file("${_KCMODULE_DATA_TEMPLATE_H}" "${ARG_MODULE_DATA_HEADER}")

    set(sources "${${sources_var}}")
    list(APPEND sources "${cpp_filename}")
    set(${sources_var} "${sources}" PARENT_SCOPE)
endfunction()


# kcmutils_generate_desktop_file(kcm_target_nam)
#
# This macro generates a desktop file for the given KCM.
# This desktop file has the following attributes:
# Type=Application
# NoDisplay=true
# Icon=icon from the .json file
# Name=name from the .json file
# Name[foo]=translated name from the .json file
# and an Exec launching it in systemsettings
# The .json file must have the same basename as the kcm_target parameter.
# Since 5.97

function(kcmutils_generate_desktop_file kcm_target)
    set(OUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${kcm_target}.desktop)
    set(IN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${kcm_target}.json)
    if (NOT EXISTS ${IN_FILE})
        message(FATAL_ERROR "Could not find metadata file for ${kcm_target}, expected path was ${IN_FILE}")
    endif()

    set(IN_SOURCE_DESKTOP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${kcm_target}.desktop)
    if (EXISTS ${IN_SOURCE_DESKTOP_FILE})
        message(FATAL_ERROR "A metadata desktop file for the KCM already exists in ${IN_SOURCE_DESKTOP_FILE} Remove this file or remove the method call to generate_kcm_desktop_file")
    endif()

    if(NOT KDE_INSTALL_APPDIR)
        include(KDEInstallDirs)
    endif()

    add_custom_target(${kcm_target}-kcm-desktop-gen
                    COMMAND KF5::kcmdesktopfilegenerator ${IN_FILE} ${OUT_FILE}
                    DEPENDS ${IN_FILE})
    add_dependencies(${kcm_target} ${kcm_target}-kcm-desktop-gen)
    if (NOT KCMUTILS_INTERNAL_TEST_MODE)
        install(FILES ${OUT_FILE} DESTINATION ${KDE_INSTALL_APPDIR})
    endif()
endfunction()
