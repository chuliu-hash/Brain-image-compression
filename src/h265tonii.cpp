#include <filesystem>
#include <itkImage.h>
#include <itkImageFileWriter.h>
#include <itkNiftiImageIO.h>
#include"process_function.h"


using InputPixelType = unsigned char;
using OutputPixelType = short;
using InputImageType = itk::Image<InputPixelType, 3>;
using OutputImageType = itk::Image<OutputPixelType, 3>;

void processH265ToNifti(const std::string& inputHevc, const std::string& outputNifti) {

    int padded_width = 0, padded_height = 0;
    // 获取视频信息，包括（填充后）宽度和高度
    getVideoInfo(inputHevc, padded_width, padded_height);

    // 获取 TXT 文件路径
    std::filesystem::path hevcPath(inputHevc);
    // 获取 TXT 文件的路径
    std::filesystem::path txtFilePath = hevcPath.parent_path() / (hevcPath.stem().string() + ".txt");

    int width = 0, height = 0, depth = 0;
    bool isPadded = true;

    // 从 TXT 文件中读取原始三维图像的维度
    if (readDimensionsFromTxt(txtFilePath.string(), width, height, depth)) {
        // 比较填充尺寸和原始尺寸
        if (width == padded_width && height == padded_height) {
            isPadded = false;
        }
    } else {
        // 如果 TXT 文件不存在
        std::cerr << "TXT文件不存在." << std::endl;
        return;
    }

    try {
        // 3. 创建临时缓冲区以存储填充后的数据
        std::vector<InputPixelType> paddedBuffer(padded_width * padded_height * depth);

        // 读取填充后的数据
        size_t paddedSize = padded_width * padded_height * depth;

        // 3. 执行FFmpeg命令并读取数据
        paddedBuffer = runFFmpegCommand(inputHevc, paddedSize);


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

        InputPixelType* inputBuffer = inputImage->GetBufferPointer();

        if (isPadded) {
            // 如果填充了，需要裁剪数据
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
        } else {
            // 如果未填充，直接复制数据
            size_t originalSize = width * height * depth;
            std::memcpy(inputBuffer, paddedBuffer.data(), originalSize * sizeof(InputPixelType));
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
        }
        catch (itk::ExceptionObject& e) {
            std::cerr << "保存NIFTI文件失败: " << e.what() << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "转换过程出错: " << e.what() << std::endl;
    }
}


