# 设置 CMake 的最低版本要求
cmake_minimum_required(VERSION 3.20)

# 定义项目名称
project("Compress_System")

# 设置 C++ 标准为 C++17，并强制要求使用该标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 将当前源目录和生成目录添加到包含路径中，方便查找头文件
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 启用自动生成 UI 文件（.ui -> ui_*.h）、MOC 文件（Q_OBJECT 宏处理）和资源文件（.qrc -> qrc_*.cpp）
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 设置第三方库的 CMake 配置文件路径
set(ITK_DIR "D:/VScode/Capstone_project/third_party/ITK-5.3/lib/cmake/ITK-5.3")
set(VTK_DIR "D:/VScode/Capstone_project/third_party/VTK-9.3/lib/cmake/vtk-9.3")
set(Qt6_DIR "D:/Qt/6.5.3/msvc2019_64/lib/cmake/Qt6")
set(X265_ROOT_DIR "D:/VScode/Capstone_project/third_party/x265")
set(matplot++_DIR "D:/VScode/Capstone_project/third_party/Matplot/lib/cmake/Matplot++")
# 查找 x265 库文件
find_library(X265_LIBRARY
    NAMES x265 libx265  # 库的名称
    PATHS ${X265_ROOT_DIR}/lib  # 库的搜索路径
)

# 查找必要的第三方库
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)  # 查找 Qt6 的核心、GUI 和 Widgets 模块
find_package(VTK REQUIRED)  # 查找 VTK 库
find_package(ITK REQUIRED)  # 查找 ITK 库
find_package(matplot++ REQUIRED)

# 定义 .ui 文件列表
set(UI_FILES
    ui/mainwindow.ui  # 主窗口的 UI 文件
)

# 定义 .qrc 文件列表
set(RESOURCE_FILES
    resources/ui.qrc  #  资源文件
)

# 使用 qt_wrap_ui 将 .ui 文件转换为对应的 ui_*.h 文件
qt_wrap_ui(UI_HEADERS ${UI_FILES})

# 使用 GLOB 自动收集 include目录下的所有 .h 文件
file(GLOB HEADERS "include_ui/*.h" "include/*.h")

# 使用 GLOB 自动收集 src 目录下的所有 .cpp 文件
file(GLOB SOURCES "src/*.cpp")

# 添加可执行文件，并指定源文件、生成的 UI 头文件和 UI 头文件
add_executable( ${PROJECT_NAME} WIN32
    ${SOURCES}    # 源文件（.cpp）
    ${UI_HEADERS} # 生成的 UI 头文件（ui_*.h）
    ${HEADERS}    # 头文件（.h）
    ${RESOURCE_FILES}   # 资源文件（.qrc）
)

# 设置包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include  # 项目的 include 目录
    ${PROJECT_SOURCE_DIR}/include_ui  # 项目的 include_ui 目录
    ${X265_ROOT_DIR}/include  # x265 库的头文件目录
)

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core  # Qt6 核心模块
    Qt6::Gui   # Qt6 GUI 模块
    Qt6::Widgets  # Qt6 Widgets 模块
    ${VTK_LIBRARIES}  # VTK 库
    ${ITK_LIBRARIES}  # ITK 库
    ${X265_LIBRARY}  # x265 库
    Matplot++::matplot 
)
