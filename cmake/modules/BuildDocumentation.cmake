#--------------------------------------------------------------------
# This file was created as a part of the LinuxDrone project:
#                http://www.linuxdrone.org
#
# Distributed under the Creative Commons Attribution-ShareAlike 4.0
# International License (see accompanying License.txt file or a copy
# at http://creativecommons.org/licenses/by-sa/4.0/legalcode)
#
# The human-readable summary of (and not a substitute for) the
# license: http://creativecommons.org/licenses/by-sa/4.0/
#--------------------------------------------------------------------

# Create and install the HTML based API documentation used Doxygen

IF(BUILD_DOCUMENTATION)

    FIND_PACKAGE(Doxygen)
    IF(NOT DOXYGEN_FOUND)
        MESSAGE(FATAL_ERROR "Doxygen is needed to build the documentation.")
    ENDIF()

    SET( doxyfile_in      ${PROJECT_SOURCE_DIR}/Doxyfile.in)
    SET( DOXY_INPUT       ${PROJECT_SOURCE_DIR})        # Pasted into Doxyfile

    SET( doxyfile_ru      ${PROJECT_BINARY_DIR}/Doxyfile.ru)
    SET( doxy_ru_index    ${PROJECT_BINARY_DIR}/ru/html/index.html)
    SET( DOXY_OUTPUT_DIR  ${PROJECT_BINARY_DIR}/ru)     # Pasted into Doxyfile.ru
    SET( DOXY_OUT_LANG    Russian)                      # Pasted into Doxyfile.ru
    CONFIGURE_FILE(${doxyfile_in} ${doxyfile_ru} @ONLY IMMEDIATE)
    ADD_CUSTOM_COMMAND(
        OUTPUT ${doxy_ru_index}
        COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile.ru
        MAIN_DEPENDENCY ${doxyfile_ru} ${doxyfile_in}
        DEPENDS sdk ${doxy_extra_files}
        COMMENT "Generating RU HTML documentation"
    )


    SET( doxyfile_en      ${PROJECT_BINARY_DIR}/Doxyfile.en)
    SET( doxy_en_index    ${PROJECT_BINARY_DIR}/en/html/index.html)
    SET( DOXY_OUTPUT_DIR  ${PROJECT_BINARY_DIR}/en)     # Pasted into Doxyfile.en
    SET( DOXY_OUT_LANG    English)                      # Pasted into Doxyfile.en
    CONFIGURE_FILE(${doxyfile_in} ${doxyfile_en} @ONLY IMMEDIATE)
    ADD_CUSTOM_COMMAND(
        OUTPUT ${doxy_en_index}
        COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile.en
        MAIN_DEPENDENCY ${doxyfile_en} ${doxyfile_in}
        DEPENDS sdk ${doxy_extra_files}
        COMMENT "Generating EN HTML documentation"
    )


    ADD_CUSTOM_TARGET( doc ALL DEPENDS ${doxy_ru_index} ${doxy_en_index})

    INSTALL( DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc )
ENDIF()
