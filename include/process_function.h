#pragma once
#include "x265Encoder.h"
#include"NiftiMetadata.h"

void compressNifti(const X265Encoder::EncoderParams& params,  const NiftiMetadata& metadata,const bool& restore_flag = false);

void processNiftiToYUV(const std::string& inputFile, const std::string& outputYUV,
    const int& padded_width, const int& padded_height);

void processYUVtoh265(const int& width, const int& height, const std::string& inputFile, const std::string& outputFile,
    const X265Encoder::EncoderParams& params);

void processH265ToNifti(const std::string& inputHevc, const std::string& outputNifti);

void getVideoInfo(const std::string& filePath, int& width, int& height);

bool readDimensionsFromTxt(const std::string& txtFilePath, int& width, int& height, int& depth);

