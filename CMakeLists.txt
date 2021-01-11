cmake_minimum_required(VERSION 3.17)

project(MBountyGL
	VERSION 0.1
	DESCRIPTION "Open source clone of King's Bounty: The Conqueror's Quest for the Sega Mega Drive"
	LANGUAGES CXX
)

set(PROJECT_NAME bounty)

set(MBOUNTYGL_SRC
	src/main.cpp
	src/engine/texture-cache.cpp
	src/engine/engine.cpp
	src/engine/dialog.cpp
	src/engine/textbox.cpp
	src/engine/scene-stack.cpp
	src/engine/dialog-stack.cpp
	src/engine/timer.cpp
	src/game/game-controls.cpp
	src/game/garrison.cpp
	src/game/victory.cpp
	src/game/wizard.cpp
	src/game/intro.cpp
	src/game/hud.cpp
	src/game/map.cpp
	src/game/ingame.cpp
	src/game/entity.cpp
	src/game/defeat.cpp
	src/game/hero.cpp
	src/game/view-army.cpp
	src/game/view-character.cpp
	src/game/view-continent.cpp
	src/game/view-contract.cpp
	src/game/view-puzzle.cpp
	src/game/town.cpp
	src/game/kings-castle.cpp
	src/game/shop.cpp
	src/game/shop-gen.cpp
	src/game/recruit-input.cpp
	src/game/chest.cpp
	src/game/army-gen.cpp
	src/game/battle.cpp
	src/game/save.cpp
	src/gfx/font.cpp
	src/gfx/gfx.cpp
	src/gfx/rect.cpp
	src/gfx/shader.cpp
	src/gfx/sprite.cpp
	src/gfx/text.cpp
	src/gfx/transformable.cpp
	src/window/window.cpp
	src/window/window-engine-interface.cpp
)

add_executable(${PROJECT_NAME}
    ${MBOUNTYGL_SRC}
)

find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(GLM CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_compile_definitions(${PROJECT_NAME} PRIVATE
	_CRT_SECURE_NO_WARNINGS
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	glfw GLEW::GLEW ${OPENGL_LIBRARIES} spdlog::spdlog
)

target_include_directories(${PROJECT_NAME} PRIVATE
	${CMAKE_SOURCE_DIR}/src
) 

target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Wno-deprecated-volatile>
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})
