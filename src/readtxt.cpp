#include <iostream>
#include <fstream>
#include <string>
#include <regex>

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