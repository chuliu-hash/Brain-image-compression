#pragma once
#include "x265Encoder.h"
#include"NiftiMetadata.h"

// 压缩Nifti文件
void compressNifti(const X265Encoder::EncoderParams& params, const NiftiMetadata& metadata, const bool& restore_flag = false);

// 将Nifti文件转换为YUV格式
void processNiftiToYUV(const std::string& inputFile, const std::string& outputYUV,
    const int& padded_width, const int& padded_height);

// 将YUV格式转换为h265格式
void processYUVtoh265(const int& width, const int& height, const std::string& inputFile, const std::string& outputFile,
    const X265Encoder::EncoderParams& params);

// 将h265格式转换为Nifti格式
void processH265ToNifti(const std::string& inputHevc, const std::string& outputNifti);

// 获取视频信息
void getVideoInfo(const std::string& filePath, int& width, int& height);

// 从txt文件中读取维度信息
bool readDimensionsFromTxt(const std::string& txtFilePath, int& width, int& height, int& depth);


