set(TARGET_NAME celestia-openvr)

PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)

## OpenVR API path
find_library(OPENVR_LIBRARIES
        NAMES
        openvr_api
        PATHS
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../thirdparty/openvr/bin/linux64
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../thirdparty/openvr/bin/linux64
        PATH_SUFFIXES
        osx32
        linux64
        ${WINDOWS_PATH_SUFFIXES}
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
        )
set(OPENVR_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../thirdparty/openvr/headers)

if(APPLE)
    if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set_source_files_properties(../shared/pathtools.cpp main.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
        find_library(FOUNDATION_FRAMEWORK Foundation)
        mark_as_advanced(FOUNDATION_FRAMEWORK)
        set(EXTRA_LIBS ${EXTRA_LIBS} ${FOUNDATION_FRAMEWORK})
    endif()
endif()

include_directories(
        .
        ${OPENGL_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIR}
        ${SDL2_INCLUDE_DIRS}
        ${QT_INCLUDE_DIRS}
        ${OPENVR_INCLUDE_DIR}
)


add_executable(${TARGET_NAME}
        ${SHARED_SRC_FILES}
        main.cpp
        )

target_link_libraries(${TARGET_NAME}
        ${OPENGL_LIBRARIES}
        ${GLEW_LIBRARIES}
        ${SDL2_LIBRARIES}
        ${OPENVR_LIBRARIES}
        ${CMAKE_DL_LIBS}
        ${EXTRA_LIBS}
        ${CELESTIA_LIBS}
        )