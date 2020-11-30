cmake_minimum_required(VERSION 3.19)

project(MBountyGL
	VERSION 0.1
	DESCRIPTION "Open source clone of King's Bounty: The Conqueror's Quest for the Sega Mega Drive"
	LANGUAGES CXX
)

set(PROJECT_NAME bounty)

set(MBOUNTYGL_SRC
	src/main.cpp
	src/assets.cpp
	src/engine.cpp
	src/scene-switcher.cpp
	src/window.cpp
	src/window-engine-interface.cpp
	src/gfx/dialog.cpp
	src/gfx/font.cpp
	src/gfx/gfx.cpp
	src/gfx/rect.cpp
	src/gfx/shader.cpp
	src/gfx/sprite.cpp
	src/gfx/text.cpp
	src/gfx/textbox.cpp
	src/gfx/transformable.cpp
	src/intro/intro.cpp
	src/game/game.cpp
	src/game/hud.cpp
	src/game/map.cpp
	src/game/entity.cpp
	src/game/hero.cpp
)

add_executable(${PROJECT_NAME}
    ${MBOUNTYGL_SRC}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
	_CRT_SECURE_NO_WARNINGS
	SPDLOG_FMT_EXTERNAL
)

include(cmake/conan.cmake)
conan_cmake_run(
	CONANFILE conanfile.txt
	BASIC_SETUP
	CMAKE_TARGETS
	BUILD missing
)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::glfw CONAN_PKG::fmt CONAN_PKG::glew)

target_include_directories(${PROJECT_NAME} PRIVATE
	${CMAKE_SOURCE_DIR}/src
) 

target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR}/data)
