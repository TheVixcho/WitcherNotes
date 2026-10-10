cmake_minimum_required(VERSION 3.16)

project(WitcherNotes
    VERSION 0.0.1
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O2)
endif()

add_compile_definitions(IMGUI_USE_WCHAR32)
add_compile_definitions(GLEW_NO_GLU)

# Subdirectorios y paths
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/external/imgui-docking")
set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/vendor")

# Fuentes ImGui Docking
set(IMGUI_SOURCES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

# Fuentes de la aplicación
file(GLOB_RECURSE CORE_SOURCES "src/core/*.cpp")
file(GLOB_RECURSE UI_SOURCES   "src/ui/*.cpp")
set(ROOT_SOURCES
    "src/main.cpp"
    "src/SplashScreen.cpp"
)

add_executable(WitcherNotes
    ${ROOT_SOURCES}
    ${CORE_SOURCES}
    ${UI_SOURCES}
    ${IMGUI_SOURCES}
)

target_include_directories(WitcherNotes PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/panels"
    "${VENDOR_DIR}/stb"
    "${VENDOR_DIR}/nlohmann"
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/misc/cpp"
    "${IMGUI_DIR}/backends"
)

# Dependencias según plataforma
if(WIN32)
    add_compile_options(-fpermissive)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
    target_compile_definitions(WitcherNotes PRIVATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        GLEW_STATIC
    )
    target_include_directories(WitcherNotes PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/sdk/glfw/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/sdk/glew/include"
    )
    target_sources(WitcherNotes PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/sdk/glew/src/glew.c"
    )
    target_link_libraries(WitcherNotes PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/sdk/glfw/lib-mingw-w64/libglfw3.a"
        opengl32
        gdi32
        shell32
        dwmapi
        comdlg32
        winmm
        ws2_32
        iphlpapi
        winhttp
    )
else()
    find_package(OpenGL REQUIRED)

    find_package(glfw3 QUIET)
    if(NOT glfw3_FOUND)
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(GLFW3 QUIET glfw3)
        endif()
        if(GLFW3_FOUND)
            target_include_directories(WitcherNotes PRIVATE ${GLFW3_INCLUDE_DIRS})
            target_link_libraries(WitcherNotes PRIVATE ${GLFW3_LIBRARIES})
        else()
            include(FetchContent)
            set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
            set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
            set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
            set(GLFW_BUILD_X11      ON  CACHE BOOL "" FORCE)
            set(GLFW_BUILD_WAYLAND  ON  CACHE BOOL "" FORCE)
            set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
            FetchContent_Declare(
                glfw3_src
                GIT_REPOSITORY https://github.com/glfw/glfw.git
                GIT_TAG        3.4
                GIT_SHALLOW    TRUE
            )
            FetchContent_MakeAvailable(glfw3_src)
            target_link_libraries(WitcherNotes PRIVATE glfw)
        endif()
    else()
        target_link_libraries(WitcherNotes PRIVATE glfw)
    endif()

    find_package(GLEW QUIET)
    if(GLEW_FOUND)
        target_link_libraries(WitcherNotes PRIVATE GLEW::GLEW)
    else()
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(GLEW QUIET glew)
        endif()
        if(GLEW_FOUND)
            target_include_directories(WitcherNotes PRIVATE ${GLEW_INCLUDE_DIRS})
            target_link_libraries(WitcherNotes PRIVATE ${GLEW_LIBRARIES})
        else()
            target_sources(WitcherNotes PRIVATE "${VENDOR_DIR}/sdk/glew/src/glew.c")
            target_include_directories(WitcherNotes PRIVATE "${VENDOR_DIR}/sdk/glew/include")
            target_compile_definitions(WitcherNotes PRIVATE GLEW_STATIC)
        endif()
    endif()

    target_link_libraries(WitcherNotes PRIVATE
        OpenGL::GL
        pthread
        ${CMAKE_DL_LIBS}
    )

    if(UNIX AND NOT APPLE)
        target_link_libraries(WitcherNotes PRIVATE X11)
    endif()
endif()

# Copiar assets e iconos a la carpeta de ejecución
add_custom_command(TARGET WitcherNotes POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:WitcherNotes>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:WitcherNotes>/bin/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/witchernotes.png"
        "$<TARGET_FILE_DIR:WitcherNotes>/witchernotes.png"
    COMMENT "Copiando recursos (assets, imágenes, fuentes) al directorio del ejecutable..."
)
