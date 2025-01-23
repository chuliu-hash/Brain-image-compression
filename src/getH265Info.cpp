#include <windows.h>
#include <iostream>
#include <string>
#include <regex>
#include <stdexcept>
#include "process_function.h"


// 获取视频信息
void getVideoInfo(const std::string& filePath, int& width, int& height) {

    try {
        // 执行命令并获取输出
        std::string output = runFFmpegCommand(filePath);

        // 去除输出中的换行符
        output.erase(output.find_last_not_of("\n") + 1);

        // 检查输出是否为空
        if (output.empty()) {
            throw std::runtime_error("未获取到视频信息");
        }

        // 解析分辨率
        std::regex resolutionRegex(R"(\s*(\d+)\s*,\s*(\d+)\s*)");
        std::smatch resolutionMatch;
        if (!std::regex_match(output, resolutionMatch, resolutionRegex)) {
            throw std::runtime_error("分辨率输出格式不正确: " + output);
        }

        // 提取分辨率
        width = std::stoi(resolutionMatch[1]);
        height = std::stoi(resolutionMatch[2]);
    }
    catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        width = 0;
        height = 0;
    }
}


