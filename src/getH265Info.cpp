#include <iostream>
#include <string>
#include <regex>
#include <cstdio>
#include"process_function.h"

// 获取视频信息
void getVideoInfo(const std::string& filePath, int& width, int& height) {
    // 构建 ffprobe 命令获取分辨率
    std::string resolutionCommand = "ffprobe -v error -select_streams v:0 -show_entries stream=width,height -of csv=p=0 " + filePath;

    // 调用 _popen 执行命令
    FILE* pipe = _popen(resolutionCommand.c_str(), "r");
    if (!pipe) {
        std::cerr << "无法调用 ffprobe 命令" << std::endl;
        return;
    }

    // 读取命令输出
    char buffer[128];
    std::string output;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
    _pclose(pipe);

    // 去除输出中的换行符
    output.erase(output.find_last_not_of("\n") + 1);

    // 检查输出是否为空
    if (output.empty()) {
        std::cerr << "未获取到视频信息" << std::endl;
        return;
    }

    // 解析分辨率
    std::regex resolutionRegex(R"((\d+),(\d+))");
    std::smatch resolutionMatch;
    if (!std::regex_match(output, resolutionMatch, resolutionRegex)) {
        std::cerr << "分辨率输出格式不正确: " << output << std::endl;
        return;
    }

    // 提取分辨率
    width = std::stoi(resolutionMatch[1]);
    height = std::stoi(resolutionMatch[2]);

    // // 构建 ffprobe 命令获取帧数
    // std::string frameCountCommand = "ffprobe -v error -count_frames -select_streams v:0 -show_entries stream=nb_read_frames -of csv=p=0 " + filePath;

    // // 调用 _popen 执行命令
    // pipe = _popen(frameCountCommand.c_str(), "r");
    // if (!pipe) {
    //     std::cerr << "无法调用 ffprobe 命令获取帧数" << std::endl;
    //     return;
    // }

    // output.clear();
    // while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    //     output += buffer;
    // }
    // _pclose(pipe);

    // output.erase(output.find_last_not_of("\n") + 1);

    // // 解析帧数
    // try {
    //     frameCount = std::stoi(output);
    // } catch (const std::exception& e) {
    //     std::cerr << "解析帧数失败: " << e.what() << std::endl;
    // }
}


