#include <itkImage.h>
#include <itkImageFileWriter.h>
#include <itkNiftiImageIO.h> 
#include <cstdio>
#include <vector>
#include <string>
#include <iostream>

using InputPixelType = unsigned char;
using OutputPixelType = short;
using InputImageType = itk::Image<InputPixelType, 3>;
using OutputImageType = itk::Image<OutputPixelType, 3>;

void processH265ToNifti(const std::string& inputHevc, const std::string& outputNifti, const int& width, const int& height, const int& depth) {
    try {
        // 计算填充后的HEVC视频尺寸，确保宽高是2的倍数
        int padded_width = (width + 1) & ~1;    // 例如：197 -> 198
        int padded_height = (height + 1) & ~1;  // 例如：233 -> 234

        // 1. 构建FFmpeg命令以解码HEVC视频
        std::string ffmpegCmd = "ffmpeg -y"
            " -i " + inputHevc +
            " -f rawvideo"
            " -pix_fmt gray"  // 输出为灰度格式
            " pipe:1";        // 将输出发送到管道

        std::cout << "执行FFmpeg命令: " << ffmpegCmd << std::endl;

        // 2. 创建管道以读取FFmpeg输出
        FILE* pipe = _popen(ffmpegCmd.c_str(), "rb");
        if (!pipe) {
            throw std::runtime_error("无法创建FFmpeg管道");
        }

        // 3. 创建临时缓冲区以存储填充后的数据
        std::vector<InputPixelType> paddedBuffer(padded_width * padded_height * depth);

        // 读取填充后的数据
        size_t paddedSize = padded_width * padded_height * depth;
        size_t readSize = fread(paddedBuffer.data(), sizeof(InputPixelType), paddedSize, pipe);

        // 关闭管道
        int pipeStatus = _pclose(pipe);
        if (pipeStatus != 0) {
            throw std::runtime_error("FFmpeg执行失败");
        }

        // 检查读取的数据是否完整
        if (readSize != paddedSize) {
            throw std::runtime_error("数据读取不完整: 预期 " + std::to_string(paddedSize) +
                " 字节，实际读取 " + std::to_string(readSize) + " 字节");
        }

        // 4. 创建输入和输出图像对象（使用原始尺寸）
        auto inputImage = InputImageType::New();
        auto outputImage = OutputImageType::New();

        // 设置原始图像尺寸
        InputImageType::SizeType size;
        size[0] = width;   // 原始宽度
        size[1] = height;  // 原始高度
        size[2] = depth;   // 深度（帧数）

        InputImageType::IndexType start;
        start.Fill(0);  // 从(0,0,0)开始

        InputImageType::RegionType region;
        region.SetSize(size);  // 设置区域大小
        region.SetIndex(start); // 设置区域起始索引

        // 分配内存
        inputImage->SetRegions(region);
        inputImage->Allocate();
        inputImage->FillBuffer(0); // 初始化为0

        // 5. 从填充数据中裁剪到原始尺寸
        InputPixelType* inputBuffer = inputImage->GetBufferPointer();
        for (int z = 0; z < depth; z++) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    // 计算填充后和原始图像的索引
                    size_t paddedIndex = z * (padded_width * padded_height) + y * padded_width + x;
                    size_t originalIndex = z * (width * height) + y * width + x;
                    inputBuffer[originalIndex] = paddedBuffer[paddedIndex]; // 裁剪数据
                }
            }
        }

        // 6. 设置输出图像并分配内存
        outputImage->SetRegions(region);
        outputImage->Allocate();

        // 7. 数据转换（将unsigned char数据映射到short范围）
        OutputPixelType* outputBuffer = outputImage->GetBufferPointer();
        const float inputMin = 0.0f;   // 输入最小值
        const float inputMax = 255.0f; // 输入最大值
        const float outputMin = -32768.0f; // 输出最小值
        const float outputMax = 32767.0f;  // 输出最大值
        size_t totalSize = width * height * depth; // 总像素数

        for (size_t i = 0; i < totalSize; ++i) {
            // 线性映射公式
            float normalizedValue = static_cast<float>(inputBuffer[i]);
            float mappedValue = (normalizedValue - inputMin) * (outputMax - outputMin) / (inputMax - inputMin) + outputMin;
            outputBuffer[i] = static_cast<OutputPixelType>(std::round(mappedValue)); // 将映射值转换为short类型
        }

        // 8. 保存为NIFTI文件
        auto niftiIO = itk::NiftiImageIO::New();
        auto writer = itk::ImageFileWriter<OutputImageType>::New();

        writer->SetFileName(outputNifti); // 设置输出文件名
        writer->SetInput(outputImage);     // 设置要写入的图像数据
        writer->SetImageIO(niftiIO);       // 设置NIFTI格式IO

        try {
            writer->Update(); // 执行文件写入
            std::cout << "NIFTI文件保存成功" << std::endl;
        }
        catch (itk::ExceptionObject& e) {
            throw std::runtime_error(std::string("保存NIFTI文件失败: ") + e.what());
        }
    }
    catch (const std::exception& e) {
        std::cerr << "转换过程出错: " << e.what() << std::endl;
        throw;
    }
}


