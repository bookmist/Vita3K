add_library(
	camera
	STATIC
	include/camera/functions.h
	include/camera/camera.h
	include/camera/state.h
	src/camera.cpp
	src/camera_state.cpp
)

target_include_directories(camera PUBLIC include)
find_package(OpenCV REQUIRED PATHS "c:/Soft/Develop/opencv/build" NO_DEFAULT_PATH)
target_link_libraries( camera PRIVATE ${OpenCV_LIBS} )
target_link_libraries(camera PUBLIC emuenv sdl2 util rtc)
