#
# Copyright (c) 2010-2016, Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2011-2015, Benjamin Girault, <benjamin dot girault at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# =======================================================
# Stand alone version of ExpoBlending tool

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories(
    $<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

set(expoblending_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/expoblending.cpp)

add_executable(expoblending ${expoblending_SRCS})

target_link_libraries(expoblending
                      digikamcore
                      libdng

                      Qt5::Gui
                      Qt5::Xml
)

# =======================================================
# Stand alone version of Calendar tool

set(calendar_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/calendar.cpp)

add_executable(calendar ${calendar_SRCS})

target_link_libraries(calendar
                      digikamcore
                      libdng

                      Qt5::Gui
                      Qt5::Xml
)

# =======================================================
# Stand alone version of Panorama tool plus test code

if(FLEX_FOUND AND BISON_FOUND AND KF5ThreadWeaver_FOUND)
    set(panorama_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/panorama.cpp)

    add_executable(panorama ${panorama_SRCS})

    target_link_libraries(panorama
                          digikamcore
                          libdng

                          Qt5::Gui
                          Qt5::Xml
    )

    set(panoparser_test_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/panoparser_test.cpp)

    add_executable(panoparser_test ${panoparser_test_SRCS})

    target_link_libraries(panoparser_test
                          digikamcore
                          libdng

                          Qt5::Gui
                          Qt5::Xml
    )
endif()
