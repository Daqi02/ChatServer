
# 配置库文件搜索路径
# link_directories()

# 需要编译的源文件列表
# set(SRC_LIST chatserver.cpp main.cpp)

# 设置可执行文件的路径为工程根目录下的bin文件夹
# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 把路径下所有源文件放入目标中
# aux_source_directory(. SRC_LIST)

# add_subdirectory(lib)


# 包含目录下所有源文件
aux_source_directory(. SRC_LIST)
aux_source_directory(./db DB_LIST)

aux_source_directory(./model MODEL_LIST)

# 生成目标可执行文件
add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${MODEL_LIST})

# 目标需要链接的库
target_link_libraries(ChatServer muduo_net muduo_base mysqlclient hiredis pthread)

