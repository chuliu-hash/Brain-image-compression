#include <iostream>
#include <fstream>
#include <string>
#include <regex>

bool readDimensionsFromTxt(const std::string& txtFilePath, int& width, int& height, int& depth) {
    std::ifstream inputFile(txtFilePath);
    if (!inputFile.is_open()) {
        std::cerr << "Failed to open TXT file: " << txtFilePath << std::endl;
        return false;
    }

    std::string line;
    std::regex dimensionsRegex(R"(Dimensions:\s*(\d+)\s*x\s*(\d+)\s*x\s*(\d+))"); // 正则表达式

    while (std::getline(inputFile, line)) {
        std::smatch match;
        if (std::regex_search(line, match, dimensionsRegex)) {
            // 提取匹配的 width, height, depth
            width = std::stoi(match[1]);
            height = std::stoi(match[2]);
            depth = std::stoi(match[3]);
            return true;
        }
    }

    std::cerr << "Dimensions not found or invalid format in TXT file: " << txtFilePath << std::endl;
    return false;
}
