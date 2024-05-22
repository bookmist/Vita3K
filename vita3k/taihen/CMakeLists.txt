add_library(taihen
include/taihen/error.h
include/taihen/hen.h
include/taihen/module.h
include/taihen/patches.h
include/taihen/plugin.h
include/taihen/proc_map.h
include/taihen/slab.h
include/taihen/taihen.h
include/taihen/taihen_internal.h
src/hen.cpp
src/module.cpp
src/patches.cpp
src/plugin.cpp
src/posix-compat.cpp
src/proc_map.cpp
src/slab.cpp
src/taihen.cpp
src/taihen-user.cpp)
add_subdirectory(taihen-parser)
target_include_directories(taihen PUBLIC include)
target_link_libraries(taihen PUBLIC kernel rtc cpu mem util nids module)
target_link_libraries(taihen PRIVATE kernel vita-toolchain)
if(TRACY_ENABLE_ON_CORE_COMPONENTS)
	target_link_libraries(taihen PRIVATE tracy)
endif()
