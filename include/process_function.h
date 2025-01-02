#pragma once
#include<string>
#include <x265Encoder.h>

void processNiftiToYUV(const std::string& inputFile, const std::string& outputYUV,
    const int& padded_width, const int& padded_height);

void processYUVtoh265(const int& width, const int& height, const std::string& inputFile, const std::string& outputFile,
    const X265Encoder::EncoderParams& params);

void processH265ToNifti(const std::string& inputHevc, const std::string& outputNifti, const int& width, const int& height, const int& depth);
