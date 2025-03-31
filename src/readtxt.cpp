#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <sstream>
#include <stdexcept>
#include"process_function.h"

bool readDimensionsFromTxt(const std::string& txtFilePath, int& width, int& height, int& depth) {
    // 打开TXT文件
    std::ifstream inputFile(txtFilePath);
    if (!inputFile.is_open()) {
        // 打开失败，输出错误信息
        std::cerr << "Failed to open TXT file: " << txtFilePath << std::endl;
        return false;
    }

    std::string line;
    // 定义正则表达式，匹配 Dimensions 后面的数字
    std::regex dimensionsRegex(R"(Dimensions:\s*(\d+)\s*x\s*(\d+)\s*x\s*(\d+))");

    // 逐行读取文件内容
    while (std::getline(inputFile, line)) {
        std::smatch match;
        // 使用正则表达式匹配行内容
        if (std::regex_search(line, match, dimensionsRegex)) {
            // 提取匹配的 width, height, depth
            width = std::stoi(match[1]);
            height = std::stoi(match[2]);
            depth = std::stoi(match[3]);
            return true;
        }
    }

    // 未找到 Dimensions 或格式不正确，输出错误信息
    std::cerr << "Dimensions not found or invalid format in TXT file: " << txtFilePath << std::endl;
    return false;
}




/**
 * @brief 读取当前目录下的x265_log.txt文件内容
 * @return std::string 包含文件所有内容的字符串
 */
std::string readlog() {
    const std::string filename = "x265_log.txt";
    std::ifstream file(filename);

    // 检查文件是否成功打开
    if (!file.is_open()) {
        throw std::runtime_error("无法打开文件: " + filename);
    }

    // 使用stringstream高效读取文件内容
    std::stringstream buffer;
    buffer << file.rdbuf();

    // 检查读取过程中是否发生错误
    if (!file) {
        throw std::runtime_error("读取文件时发生错误: " + filename);
    }

    return buffer.str();
}
