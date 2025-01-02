#pragma once
#include <x265.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>

class X265Encoder {
public:
    // 编码器参数结构
    struct EncoderParams {
        // 基本参数
        int qp = 23;                     // 量化参数 (0-51)
        int keyframeInterval = 50;       // 关键帧间隔
        int bframes = 4;                 // B帧数量
        int maxNumReferences = 3;       // 最大参考帧数

        std::string preset = "medium";   // 编码预设 [ultrafast,superfast,veryfast,faster,fast,medium,slow,slower,veryslow,placebo]
        std::string tune = "psnr";       // 优化目标 [psnr,ssim,grain,zerolatency,fastdecode]

        // 码率控制
        int crf = 28;                    // CRF值 (0-51)
        int maxBitrate = 1000;             // 最大码率 (kbps)

        // 高级参数
        bool enableSAO = false;          // 启用SAO (Sample Adaptive Offset)
        bool enableLoopFilter = false;    // 启用环路滤波
        bool enableStrongIntraSmoothing = false;  // 强帧内平滑

        int mode = 3;  //0为ABR模式，1为CQP模式，2为CRF模式，3为无损压缩

    };

private:
    size_t frameSize;                   // 帧大小 (字节)

    // x265相关指针
    x265_param* param;                  // 编码器参数
    x265_encoder* encoder;              // 编码器实例
    x265_picture* picIn;                // 输入图像

    // 帧缓冲
    std::vector<uint8_t> frameBuffer;   // YUV数据缓冲区

public:
    // 默认构造函数
    X265Encoder(const int& width, const int& height)
        : X265Encoder(width, height,EncoderParams()) {}

    // 自定义编码参数构造函数
    X265Encoder(const int& width, const int& height, const EncoderParams& params) {

        // 计算帧大小
        frameSize = width * height * 3 / 2 ;  // YUV420格式
        frameBuffer.resize(frameSize);

        // 配置x265参数
        param = x265_param_alloc();
        if (!param) {
            throw std::runtime_error("Failed to allocate x265 params");
        }

        // 设置默认参数
        x265_param_default(param);

        // 应用预设
        if (x265_param_default_preset(param, params.preset.c_str(), params.tune.c_str()) < 0) {
            throw std::runtime_error("Failed to set encoder preset and tune");
        }

        // 基本参数设置
        param->sourceWidth = width;          // 输入视频宽度
        param->sourceHeight = height;        // 输入视频高度
        param->fpsNum = 24;                // 帧率分子
        param->fpsDenom = 1;                // 帧率分母（分子/分母=实际帧率）
        param->bRepeatHeaders = 1;          // 在每个关键帧前重复写入SPS和PPS头，有利于随机访问
        param->internalCsp = X265_CSP_I420; // 色彩空间格式：YUV420（Y平面全分辨率，UV平面半分辨率）
        param->internalBitDepth = 8;        // 视频位深：8位/像素/分量

        // 线程设置
        param->frameNumThreads = 16;         // 帧级并行线程数，增加可提高编码速度

        param->keyframeMax = params.keyframeInterval;  // 最大关键帧间隔（GOP大小）
        param->keyframeMin = params.keyframeInterval;  // 最小关键帧间隔，与最大值相同时确保固定GOP
        param->maxNumReferences = params.maxNumReferences;   // 最大参考帧数，增加可提高质量但降低速度
        param->bframes = params.bframes;     // 连续B帧的最大数量，增加可提高压缩率但增加延迟

        param->bEnableLoopFilter = params.enableLoopFilter;  // 去块滤波器，减少块效应
        param->bEnableSAO = params.enableSAO;               // 样本自适应偏移，改善重建图像质量
        param->bEnableStrongIntraSmoothing = params.enableStrongIntraSmoothing;  // 强帧内平滑，改善平滑区域质量

  


        // 码率控制模式选择
        if (params.mode == 3) {
            param->rc.qp = 0;               // 无损编码时QP设为0
            param->bLossless = 1;           // 启用无损编码模式
        }
        else if (params.mode == 0 && params.maxBitrate > 0) {
            param->rc.rateControlMode = X265_RC_ABR;  // 平均码率模式(Average Bitrate)
            param->rc.bitrate = params.maxBitrate;    // 目标平均码率(kbps)
            param->rc.vbvMaxBitrate = params.maxBitrate;  // VBV最大码率限制
            param->rc.vbvBufferSize = params.maxBitrate;  // VBV缓冲区大小，影响码率波动
        }
        else if (params.mode == 1) {
            param->rc.rateControlMode = X265_RC_CQP;  // 固定QP模式(Constant Quantization Parameter)
            param->rc.qp = params.qp;                 // 固定的量化参数值
        }
        else if (params.mode == 2) {
            param->rc.rateControlMode = X265_RC_CRF;  // 恒定质量模式(Constant Rate Factor)
            param->rc.rfConstant = params.crf;        // CRF值(0-51)，值越小质量越高
        }

        // 应用profile
        if (x265_param_apply_profile(param, "main") < 0) {
            throw std::runtime_error("Failed to apply profile");
        }

        // 创建编码器
        encoder = x265_encoder_open(param);
        if (!encoder) {
            throw std::runtime_error("Failed to open x265 encoder");
        }

        // 初始化输入图像
        picIn = x265_picture_alloc();
        x265_picture_init(param, picIn);

        // 设置YUV平面
        picIn->planes[0] = frameBuffer.data();                    // Y平面
        picIn->planes[1] = frameBuffer.data() + width * height;  // U平面
        picIn->planes[2] = frameBuffer.data() + width * height * 5 / 4;  // V平面

        // 设置stride
        picIn->stride[0] = width;        // Y平面stride
        picIn->stride[1] = width / 2;    // U平面stride
        picIn->stride[2] = width / 2;    // V平面stride
    }

    // 析构函数
    ~X265Encoder() {
        if (encoder) x265_encoder_close(encoder);
        if (picIn) x265_picture_free(picIn);
        if (param) x265_param_free(param);
    }

    // 编码单个YUV文件
    void encode(const std::string& inputFile, const std::string& outputFile) {
        // 打开输入输出文件
        std::ifstream inFile(inputFile, std::ios::binary);
        std::ofstream outFile(outputFile, std::ios::binary);

        if (!inFile || !outFile) {
            throw std::runtime_error("Failed to open input or output file");
        }

        x265_nal* nals = nullptr;
        uint32_t numNals = 0;
        int ret;
        int frameCount = 0;

        // 循环读取并编码每一帧
        while (inFile.read(reinterpret_cast<char*>(frameBuffer.data()), frameSize)) {
            // 编码当前帧
            ret = x265_encoder_encode(encoder, &nals, &numNals, picIn, nullptr);

            // 写入编码后的数据
            for (uint32_t i = 0; i < numNals; i++) {
                outFile.write(reinterpret_cast<char*>(nals[i].payload), nals[i].sizeBytes);
            }

            frameCount++;
            std::cout << "Encoded " << frameCount << " frames\r" << std::flush;
        }

        // 刷新编码器，输出剩余数据
        while ((ret = x265_encoder_encode(encoder, &nals, &numNals, nullptr, nullptr))) {
            for (uint32_t i = 0; i < numNals; i++) {
                outFile.write(reinterpret_cast<char*>(nals[i].payload), nals[i].sizeBytes);
            }
        }
    }
};
