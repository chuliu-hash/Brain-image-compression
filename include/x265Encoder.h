#pragma once
#include <x265.h>
#include<string>
#include <vector>

class X265Encoder {
public:
    // 编码器参数结构
    struct EncoderParams {
        int qp = 23;                     // 量化参数 (0-51)
        int keyframeInterval = 50;       // 关键帧间隔
        int bframes = 4;                 // B帧数量
        int maxNumReferences = 3;        // 最大参考帧数

        std::string preset = "medium";   // 编码预设
        std::string tune = "psnr";       // 优化目标

        double crf = 28;                    // CRF值 (0-51)
        int Bitrate = 1000;           // 平均码率 (bps)

        bool enableSAO = false;          // 启用SAO
        bool enableLoopFilter = false;   // 启用环路滤波
        bool enableStrongIntraSmoothing = false;  // 强帧内平滑

        std:: string mode = "Lossless";                    // 编码模式
    };

    // 构造函数
    X265Encoder(const int& width,const int& height, const EncoderParams& params);

    // 析构函数
    ~X265Encoder();

    // 编码函数
    void encode(const std::string& inputFile, const std::string& outputFile);

private:
    size_t frameSize;                   // 帧大小
    std::vector<uint8_t> frameBuffer;   // YUV数据缓冲区

    x265_param* param;                  // 编码器参数
    x265_encoder* encoder;              // 编码器实例
    x265_picture* picIn;                // 输入图像
};
