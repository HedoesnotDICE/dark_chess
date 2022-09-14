cmake_minimum_required(VERSION 3.12)

project(dark_chess)


# 0 初始化
# 0.1 设置相对路径变量
set(dev_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../../../)
set(base_dir    ${dev_dir}/base)
set(proj_dir    ${dev_dir}/)
set(common_dir  ${proj_dir}/common)
set(plt_dir     ${proj_dir}/plt)

# 0.2 设置属性
set_property(GLOBAL PROPERTY USE_FOLDERS On)

# 0.3 设置通用头文件路径
include_directories(${proj_dir} ${base_dir} ${proj_dir}/common)
