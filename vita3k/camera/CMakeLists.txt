add_library(
	camera
	STATIC
	include/camera/functions.h
	include/camera/camera.h
	include/camera/state.h
	src/camera.cpp
)

target_include_directories(camera PUBLIC include)
target_link_libraries(camera PUBLIC emuenv sdl2 util)
