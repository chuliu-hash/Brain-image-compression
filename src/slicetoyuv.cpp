#include <filesystem>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkNiftiImageIO.h>
#include <itkExtractImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkPNGImageIO.h>

// 定义图像类型
using PixelType = float;
using OutputPixelType = unsigned char;
using Image3DType = itk::Image<PixelType, 3>;
using Image2DType = itk::Image<PixelType, 2>;
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
        std::vector<std::string> pngFiles; // 存储PNG文件名
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
            pngFiles.push_back(pngFilename); // 存储PNG文件名

            auto writer = itk::ImageFileWriter<OutputImageType>::New();
            auto pngIO = itk::PNGImageIO::New();  // 创建PNG IO
            writer->SetImageIO(pngIO);            // 设置IO
            writer->SetFileName(pngFilename);     // 设置文件名
            writer->SetInput(rescaler->GetOutput()); // 设置要写入的图像数据
            writer->Update(); // 执行文件写入

            std::cout << "Processed slice " << z + 1<< "/" << depth  << "\r"<<std::flush;
        }

        // 第二步：将PNG序列转换为YUV420P
        // 构建FFmpeg命令
        std::string ffmpegCmd = "ffmpeg -y"
            " -f image2"                    // 使用image2格式读取图片序列
            " -i " + tempDir + "/slice_%d.png"  // 输入PNG图片序列
            " -vf pad=" + std::to_string(padded_width) + ":" +
            std::to_string(padded_height) + // 进行填充确保尺寸是偶数
            " -pix_fmt yuv420p"            // 输出YUV420P格式
            " -f rawvideo "                // 输出原始视频格式
            + outputYUV;                   // 输出文件名

        // 执行FFmpeg命令
        int result = system(ffmpegCmd.c_str());
        if (result != 0) {
            throw std::runtime_error("FFmpeg conversion failed");
        }

        std::cout << "Conversion completed! Output file: " << outputYUV << std::endl;
    }
    catch (const itk::ExceptionObject& e) {
        std::cerr << "ITK error: " << e.what() << std::endl;
        throw;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        throw;
    }
}



