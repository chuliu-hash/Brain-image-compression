#include <filesystem>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkNiftiImageIO.h>
#include <itkExtractImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkPNGImageIO.h>
#include "process_function.h"

 /*如果 NIfTI 文件中包含 scl_slope 和 scl_inter 字段，
 ITK 会自动应用这些缩放和偏移参数，将原始数据转换为浮点类型（如 float），范围可能变为 [0, 1]
 公式：scaled_value = raw_value * scl_slope + scl_inter
  */

// 定义图像类型
using InputPixelType = float;  
using OutputPixelType = unsigned char;
using Image3DType = itk::Image<InputPixelType, 3>;
using Image2DType = itk::Image<InputPixelType, 2>;
using OutputImageType = itk::Image<OutputPixelType, 2>;


void processNiftiToYUV(const std::string& inputFile, const std::string& outputYUV, const int& padded_width, const int& padded_height) {
    try {
        // 创建临时目录存放PNG序列
        std::string tempDir = "temp_slices";
        std::filesystem::create_directories(tempDir);  // 确保临时目录存在

        // 1. 读取和处理NIfTI文件
        auto reader = itk::ImageFileReader<Image3DType>::New();
        auto niftiIO = itk::NiftiImageIO::New();
        reader->SetImageIO(niftiIO);  // 设置NIfTI格式的IO接口
        reader->SetFileName(inputFile); // 设置输入文件名
        reader->Update();  // 执行文件读取

        // 获取图像数据和区域信息
        auto inputImage = reader->GetOutput();
        auto region = inputImage->GetLargestPossibleRegion();
        auto size = region.GetSize();

        int width = size[0];   // 获取宽度
        int height = size[1];  // 获取高度
        int depth = size[2];   // 获取深度（切片数量）

        // 设置图像处理过滤器
        auto extractFilter = itk::ExtractImageFilter<Image3DType, Image2DType>::New();
        auto rescaler = itk::RescaleIntensityImageFilter<Image2DType, OutputImageType>::New();
        rescaler->SetOutputMinimum(0);     // 设置输出最小值
        rescaler->SetOutputMaximum(255);   // 设置输出最大值

        // 第一步：保存PNG序列
        for (unsigned int z = 0; z < depth; ++z) {
            // 设置提取区域，提取当前切片
            Image3DType::RegionType extractRegion = region;
            extractRegion.SetSize(2, 0);    // Z轴大小设为0，表示提取单个切片
            extractRegion.SetIndex(2, z);   // 设置当前切片的Z轴索引

            extractFilter->SetExtractionRegion(extractRegion); // 设置提取区域
            extractFilter->SetInput(inputImage);               // 设置输入图像
            extractFilter->SetDirectionCollapseToIdentity();   // 确保输出为2D图像

            rescaler->SetInput(extractFilter->GetOutput());   // 执行灰度值重映射
            rescaler->Update(); // 更新过滤器

            // 生成PNG文件名并保存
            std::string pngFilename = tempDir + "/slice_" + std::to_string(z) + ".png";

            auto writer = itk::ImageFileWriter<OutputImageType>::New();
            auto pngIO = itk::PNGImageIO::New();  // 创建PNG IO
            writer->SetImageIO(pngIO);            // 设置IO
            writer->SetFileName(pngFilename);     // 设置文件名
            writer->SetInput(rescaler->GetOutput()); // 设置要写入的图像数据
            writer->Update(); // 执行文件写入
        }

        //// 第二步：将PNG序列转换为YUV420P
        runFFmpegCommand(tempDir, outputYUV, padded_width, padded_height);
    }
    catch (const itk::ExceptionObject& e) {
        std::cerr << "ITK error: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}


