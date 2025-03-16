#include "x265Encoder.h"
#include<iostream>

void processYUVtoh265(const int& width, const int& height, const std:: string& inputFile, const std::string& outputFile,
    const X265Encoder::EncoderParams& params) {
    try {
         //重定向stderr到log文件
         freopen("x265_log.txt", "w", stderr);

        // 创建编码器并执行编码
        X265Encoder encoder(width, height, params);
        encoder.encode(inputFile, outputFile);

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

}
