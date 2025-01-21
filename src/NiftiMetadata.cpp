#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkNIFTIImageReader.h>
#include <vtkMatrix4x4.h>
#include <vtkImageAccumulate.h>
#include <vtkImageHistogramStatistics.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include "NiftiMetadata.h"
#include <filesystem>
#include <fstream>

void NiftiMetadata::setNiftiMetadata(const std::string& filePath) {
    this->filePath = filePath;

    // 创建 NIFTI 读取器
    vtkSmartPointer<vtkNIFTIImageReader> reader =
        vtkSmartPointer<vtkNIFTIImageReader>::New();
    reader->SetFileName(filePath.c_str());
    reader->Update();

    // 获取图像数据
    vtkImageData* imageData = reader->GetOutput();

    // 获取图像的维度
    int* dimensions = imageData->GetDimensions();
    width = dimensions[0];
    height = dimensions[1];
    depth = dimensions[2];

    // 获取体素间距
    double* spacing = imageData->GetSpacing();
    std::copy(spacing, spacing + 3, this->spacing);

    // 获取原点坐标
    double* origin = imageData->GetOrigin();
    std::copy(origin, origin + 3, this->origin);

    // 获取数据范围
    double* range = imageData->GetScalarRange();
    std::copy(range, range + 2, this->dataRange);

    // 获取数据类型信息
    dataType = imageData->GetScalarTypeAsString();
    bitsPerPixel = imageData->GetScalarSize() * 8;

    // 计算总体素数
    totalVoxels = static_cast<size_t>(width) *
                  static_cast<size_t>(height) *
                  static_cast<size_t>(depth);

    // 计算内存使用情况
    totalMemoryBytes = imageData->GetActualMemorySize() * 1024ull;

    // 将元数据保存到 TXT 文件
    saveMetadataToTxt();
}

void NiftiMetadata::saveMetadataToTxt() const {
    // 获取输出目录路径
    std::filesystem::path inputPath(filePath);
    std::filesystem::path outputDir = inputPath.parent_path() / "out";
    std::filesystem::create_directories(outputDir); // 创建输出目录（如果不存在）

    // 构建输出文件路径
    std::filesystem::path outputPath = outputDir / (inputPath.stem().string() + ".txt");

    // 打开文件并写入元数据
    std::ofstream outputFile(outputPath);
    if (outputFile.is_open()) {
        outputFile << "=== NIfTI Metadata ===" << std::endl;
        outputFile << "File Path: " << filePath << std::endl;
        outputFile << "Dimensions: " << width << " x " << height << " x " << depth << std::endl;
        outputFile << "Spacing: " << spacing[0] << " x " << spacing[1] << " x " << spacing[2] << std::endl;
        outputFile << "Origin: (" << origin[0] << ", " << origin[1] << ", " << origin[2] << ")" << std::endl;
        outputFile << "Data Range: [" << dataRange[0] << ", " << dataRange[1] << "]" << std::endl;
        outputFile << "Data Type: " << dataType << std::endl;
        outputFile << "Bits Per Pixel: " << bitsPerPixel << std::endl;
        outputFile << "Total Voxels: " << totalVoxels << std::endl;
        outputFile << "Total Memory Bytes: " << totalMemoryBytes << std::endl;
        outputFile.close();
    } else {
        std::cerr << "Failed to open output file: " << outputPath << std::endl;
    }
}
