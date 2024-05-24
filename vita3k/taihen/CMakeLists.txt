add_library(taihen
include/taihen/error.h
include/taihen/hen.h
include/taihen/module.h
include/taihen/patches.h
include/taihen/plugin.h
include/taihen/proc_map.h
include/taihen/slab.h
include/taihen/taihen.h
src/hen.cpp
src/module.cpp
src/patches.cpp
src/plugin.cpp
src/proc_map.cpp
src/slab.cpp)
add_subdirectory(taihen-parser)
target_include_directories(taihen PUBLIC include)
target_link_libraries(taihen PUBLIC kernel rtc cpu mem util nids module)
target_link_libraries(taihen PRIVATE kernel vita-toolchain io modules)
if(TRACY_ENABLE_ON_CORE_COMPONENTS)
	target_link_libraries(taihen PRIVATE tracy)
endif()
