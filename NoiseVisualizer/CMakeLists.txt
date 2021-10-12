# CMakeList.txt : projet CMake pour NoiseVisualizer, incluez la source et définissez
# la logique spécifique au projet ici.
#
cmake_minimum_required (VERSION 3.8)


SET(TARGETS 
	"Source.cpp"
	"src/ColorMapSelector.h"		"src/ColorMapSelector.cpp"
	"src/Functions.h"				"src/Functions.cpp"
	"src/PerlinNoise.h"				"src/PerlinNoise.cpp"
	"src/Graph.h"					"src/Graph.cpp"
	"src/Setup.h"					"src/Setup.cpp"
	"src/Terrain.h"					"src/Terrain.cpp"
	"src/Texture.h"					"src/Texture.cpp"
	"src/opengl/Shader.h"			"src/opengl/Shader.cpp"
	)

include(FetchContent) 

#mahi
FetchContent_Declare(mahi-gui GIT_REPOSITORY https://github.com/mahilab/mahi-gui.git) 
FetchContent_MakeAvailable(mahi-gui)


add_executable(NoiseVisualizer ${TARGETS} )
target_include_directories(NoiseVisualizer PRIVATE ${GlmIncludeDir})
target_link_libraries(NoiseVisualizer mahi::gui)

#Copy res folder in final build directory
file(COPY ${CMAKE_CURRENT_LIST_DIR}/res DESTINATION /)

