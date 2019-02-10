set(TARGET_NAME celestia-gtk3)

include(FindGIO)

PKG_CHECK_MODULES(GTKMM REQUIRED gtkmm-3.0)

include_directories(
        .
        ${GTKMM_INCLUDE_DIRS}
        ${OPENGL_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIR}
)

glib_compile_resources("data/celestia.gresource.xml"
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data"
        SOURCE "${CMAKE_CURRENT_BINARY_DIR}/celestia-ui.c"
        C_NAME "celestia_ui_resources"
        INTERNAL)


add_executable(${TARGET_NAME}
        ${SHARED_SRC_FILES}
        main.cpp
        celestia-application.h
        celestia-appwindow.h
        celestia-application.cpp
        celestia-appwindow.cpp
        celestia-ui.c
        celestia-app-data.cpp
        celestia-app-data.h
        celestia-splash-window.cpp
        celestia-splash-window.h)

target_link_libraries(${TARGET_NAME}
        ${GTKMM_LIBRARIES}
        ${OPENGL_LIBRARIES}
        ${GLEW_LIBRARIES}
        ${CMAKE_DL_LIBS}
        ${EXTRA_LIBS}
        ${CELESTIA_LIBS}
        )

# Install GSettings schemas
glib_install_schemas(
        "data/org.celestia.gschema.xml")
