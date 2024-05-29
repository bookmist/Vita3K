add_library(taihen
include/taihen/error.h
include/taihen/module.h
include/taihen/patches.h
include/taihen/plugin.h
include/taihen/slab.h
include/taihen/taihen.h
include/taihen/taihen-parser/lexer.h
include/taihen/taihen-parser/parser.h
src/module.cpp
src/patches.cpp
src/plugin.cpp
src/slab.cpp
src/taihen-parser/lexer.c
src/taihen-parser/parser.c
)
target_include_directories(taihen PUBLIC include)
target_link_libraries(taihen PUBLIC kernel rtc cpu mem util nids module)
target_link_libraries(taihen PRIVATE kernel vita-toolchain io modules)
if(TRACY_ENABLE_ON_CORE_COMPONENTS)
	target_link_libraries(taihen PRIVATE tracy)
endif()
