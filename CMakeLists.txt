cmake_minimum_required(VERSION 3.20)
project(CMakeProject1)


# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置自动生成的文件目录
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 设置目录
set(ITK_DIR "D:/Download/ITK-5.3/lib/cmake/ITK-5.3")
set(VTK_DIR "D:/Download/VTK-9.3/lib/cmake/vtk-9.3")
set(Qt6_DIR "D:/Qt/6.5.3/msvc2019_64/lib/cmake/Qt6")

# x265 配置
set(X265_ROOT_DIR "D:/Download/x265")
set(X265_INCLUDE_DIR "${X265_ROOT_DIR}/include")
set(X265_LIBRARY_DIR "${X265_ROOT_DIR}/lib")

# 查找 x265 库文件
find_library(X265_LIBRARY
    NAMES x265 libx265
    PATHS ${X265_LIBRARY_DIR}
    NO_DEFAULT_PATH
)


include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${X265_INCLUDE_DIR}
)



# 查找必要的包
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(VTK REQUIRED)
find_package(ITK REQUIRED)


file(GLOB SOURCES 
    "src/*.cpp"
)


# 添加源文件
add_executable(${PROJECT_NAME} ${SOURCES})


# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    ${VTK_LIBRARIES}
    ${ITK_LIBRARIES}
    ${X265_LIBRARY}
)


