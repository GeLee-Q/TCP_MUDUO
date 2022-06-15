# 批量的自动查找所有的.cpp 和 .h 文件
file(GLOB mylib_sources CONFIGURE_DEPENDS mylib/*.cpp mylib/*.h)

# 将mylib设置为动态库
add_library(mylib SHARED ${mylib_sources})


target_compile_definitions(mylib PRIVATE MYLIB_EXPORT)
#设置额外的头文件查找目录
target_include_directories(mylib PUBLIC .)