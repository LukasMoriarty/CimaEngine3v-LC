cmake_minimum_required(VERSION 3.20)

project(CimaEngine3v LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BIN "${CMAKE_SOURCE_DIR}/bin")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BIN}/${CMAKE_BUILD_TYPE}")



add_compile_definitions(ASSETS="${CMAKE_SOURCE_DIR}/assets")
#bandera para DEBUG O RELEASE
add_compile_definitions("DEBUG=$<CONFIG:Debug>")

#Descargar las dependencias (IMGUI y SFML)
message("Descargando SFML")
include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE)

if(WIN32)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY http://github.com/SFML/SFML.git
    GIT_TAG 3.0.x)
FetchContent_MakeAvailable(SFML)
else()
    find_package(SFML 3 COMPONENTS System Window Graphics Audio REQUIRED)
endif()
message("Descargando IMGUI")

FetchContent_Declare(
    imgui
    GIT_REPOSITORY http://github.com/ocornut/imgui.git
    GIT_TAG v1.91.9b-docking
    GIT_PROGRESS TRUE
    )
FetchContent_MakeAvailable(imgui)

message("Descargando ImguiSFML")

FetchContent_Declare(
    imgui-sfml
    GIT_REPOSITORY http://github.com/SFML/imgui-sfml.git
    GIT_TAG v3.0
    GIT_PROGRESS TRUE
    )

set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMGUI_SFML_FIND_SFML OFF)
set(IMGUI_SFML_IMGUI_DEMO ON)

FetchContent_MakeAvailable(imgui-sfml)
#incluir los headers
include_directories(${SFML_INCLUDE_DIRS})
include_directories(${IMGUI_DIR})
#include_directories(${S_INCLUDE_DIRS})

#incluir los folders
add_subdirectory(src)


#ligar las librerias
target_link_libraries(
    ${PROJECT_NAME} PUBLIC
    SFML::System SFML::Window SFML::Graphics SFML::Audio
    ImGui-SFML::ImGui-SFML
)
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message("Compilando con banderas de depuración")
    #add_compile_options(-ggdb -Wall -Wextra -Werror -fsanitize=adress -fsanitize=undefine)
    #target_compile_options(CimaEngine3v PRIVATE  -ggdb -Wall -Wextra  -fsanitize=address -fsanitize=undefined)
    #if(WIN32)
        #target_compile_options(CimaEngine3v PRIVATE  /W4 /Zi )
    #else()
    target_compile_options(CimaEngine3v PRIVATE  -ggdb -Wall -Wextra -Werror)
    #endif()
else()
    message("Compilando con banderas de optimización")
    #add_compile_options(-O2)
    target_compile_options(CimaEngine3v PRIVATE  -O2)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

#asignar el estandar
target_compile_features(CimaEngine3v PUBLIC cxx_std_17)

#file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/compile_commands.json" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}")
