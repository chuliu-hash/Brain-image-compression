#include "X265Encoder.h"
#include <fstream>
#include<iostream>
// 默认构造函数
X265Encoder::X265Encoder(const int& width,const  int& height)
    : X265Encoder(width, height, EncoderParams()) {}

// 自定义编码参数构造函数
X265Encoder::X265Encoder(const int& width,const int& height, const EncoderParams& params) {
    // 计算帧大小（YUV420格式：Y平面全分辨率，U和V平面半分辨率）
    frameSize = width * height * 3 / 2;
    frameBuffer.resize(frameSize);

    // 配置x265参数
    param = x265_param_alloc();
    if (!param) {
        throw std::runtime_error("Failed to allocate x265 params");
    }

    // 设置默认参数
    x265_param_default(param);

    // 应用预设和优化目标
    if (x265_param_default_preset(param, params.preset.c_str(), params.tune.c_str()) < 0) {
        throw std::runtime_error("Failed to set encoder preset and tune");
    }

    // 基本参数设置
    param->sourceWidth = width;          // 输入视频宽度
    param->sourceHeight = height;        // 输入视频高度
    param->fpsNum = 24;                  // 帧率分子
    param->fpsDenom = 1;                 // 帧率分母（分子/分母=实际帧率）
    param->bRepeatHeaders = 1;           // 在每个关键帧前重复写入SPS和PPS头，有利于随机访问
    param->internalCsp = X265_CSP_I420;  // 色彩空间格式：YUV420
    param->internalBitDepth = 8;         // 视频位深：8位/像素/分量

    // 线程设置
    param->frameNumThreads = 16;         // 帧级并行线程数，增加可提高编码速度

    // GOP（Group of Pictures）设置
    param->keyframeMax = params.keyframeInterval;  // 最大关键帧间隔（GOP大小）
    param->keyframeMin = params.keyframeInterval;  // 最小关键帧间隔，与最大值相同时确保固定GOP
    param->maxNumReferences = params.maxNumReferences;  // 最大参考帧数，增加可提高质量但降低速度
    param->bframes = params.bframes;     // 连续B帧的最大数量，增加可提高压缩率但增加延迟

    // 高级编码工具
    param->bEnableLoopFilter = params.enableLoopFilter;  // 去块滤波器，减少块效应
    param->bEnableSAO = params.enableSAO;               // 样本自适应偏移，改善重建图像质量
    param->bEnableStrongIntraSmoothing = params.enableStrongIntraSmoothing;  // 强帧内平滑，改善平滑区域质量

    // 码率控制模式选择
    if (params.mode == "Lossless") {
        param->rc.qp = 0;               // 无损编码时QP设为0
        param->bLossless = 1;           // 启用无损编码模式
    }
    else if (params.mode == "CBR" ) {
        param->rc.rateControlMode = X265_RC_ABR;  // 平均码率模式(Average Bitrate)
        param->rc.bitrate = params.Bitrate;    // 目标平均码率(kbps)
        param->rc.vbvMaxBitrate = params.Bitrate;  // VBV最大码率限制
        param->rc.vbvBufferSize = params.Bitrate;  // VBV缓冲区大小，影响码率波动
    }
    else if (params.mode == "CQP") {
        param->rc.rateControlMode = X265_RC_CQP;  // 固定QP模式(Constant Quantization Parameter)
        param->rc.qp = params.qp;                 // 固定的量化参数值
    }
    else if (params.mode == "CRF") {
        param->rc.rateControlMode = X265_RC_CRF;  // 恒定质量模式(Constant Rate Factor)
        param->rc.rfConstant = params.crf;        // CRF值(0-51)，值越小质量越高
    }

    // 应用profile（编码配置文件）
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
    picIn->planes[1] = frameBuffer.data() + width * height;   // U平面
    picIn->planes[2] = frameBuffer.data() + width * height * 5 / 4;  // V平面

    // 设置stride（步长）
    picIn->stride[0] = width;        // Y平面stride
    picIn->stride[1] = width / 2;    // U平面stride
    picIn->stride[2] = width / 2;    // V平面stride
}

// 析构函数
X265Encoder::~X265Encoder() {
    if (encoder) x265_encoder_close(encoder);  // 关闭编码器
    if (picIn) x265_picture_free(picIn);       // 释放输入图像
    if (param) x265_param_free(param);         // 释放编码器参数
}

// 编码函数
void X265Encoder::encode(const std::string& inputFile, const std::string& outputFile) {
    // 打开输入输出文件
    std::ifstream inFile(inputFile, std::ios::binary);
    std::ofstream outFile(outputFile, std::ios::binary);

    if (!inFile || !outFile) {
        throw std::runtime_error("Failed to open input or output file");
    }

    x265_nal* nals = nullptr;  // NAL单元指针
    uint32_t numNals = 0;      // NAL单元数量
    int ret;                   // 编码返回值
    int frameCount = 0;        // 已编码帧数

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
