#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkNIFTIImageReader.h>
#include <vtkImageAccumulate.h> 
#include <vtkImageHistogramStatistics.h>  
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include"NiftiMetadata.h"

void NiftiMetadata::setNiftiMetadata(const std::string& filePath)  {
    this->filePath = filePath;
    // 创建 NIFTI 读取器，用于读取 NIfTI 格式的图像文件
    vtkSmartPointer<vtkNIFTIImageReader> reader =
        vtkSmartPointer<vtkNIFTIImageReader>::New();

    // 设置要读取的 NIfTI 文件名
    reader->SetFileName(filePath.c_str());
    reader->Update(); // 更新读取器以读取图像数据

    // 获取图像数据
    vtkImageData* imageData = reader->GetOutput();

    // 获取图像的维度（宽度、高度、深度）
    int* dimensions = imageData->GetDimensions();
    width = dimensions[0];   // 图像宽度
    height = dimensions[1];  // 图像高度
    depth = dimensions[2];   // 图像深度（切片数量）

    // // 获取体素间距（每个维度的物理尺寸）
    // double* spacing = imageData->GetSpacing();
    // std::copy(spacing, spacing + 3, this->spacing); // 复制到元数据结构中

    // // 获取原点坐标（图像在空间中的起始位置）
    // double* origin = imageData->GetOrigin();
    // std::copy(origin, origin + 3, this->origin); // 复制到元数据结构中

    // // 获取数据范围（最小值和最大值）
    // double* range = imageData->GetScalarRange();
    // std::copy(range, range + 2, this->dataRange); // 复制到元数据结构中

    // // 获取数据类型信息
    // dataType = imageData->GetScalarTypeAsString(); // 数据类型（如 "unsigned char", "short" 等）
    // bitsPerPixel = imageData->GetScalarSize() * 8; // 每像素位数（字节数乘以8）

    // totalVoxels = static_cast<size_t>(width) *
    //               static_cast<size_t>(height) *
    //               static_cast<size_t>(depth); // 计算总体素数

    // 计算内存使用情况
    totalMemoryBytes = imageData->GetActualMemorySize() * 1024ull; // 转换为字节

}


// 打印元数据信息
// void NiftiMetadata::printMetadata() const {
//     // 打印图像基本信息
//     std::cout << "\n=== 图像基本信息 ===" << std::endl;
//     std::cout << "维度: " << width << " x "
//               << height << " x " << depth << std::endl;
//     std::cout << "体素间距: " << spacing[0] << " x "
//               << spacing[1] << " x " << spacing[2] << std::endl;
//     std::cout << "原点: (" << origin[0] << ", "
//               << origin[1] << ", " << origin[2] << ")" << std::endl;

//     // 打印数据信息
//     std::cout << "\n=== 数据信息 ===" << std::endl;
//     std::cout << "数据类型: " << dataType << std::endl;
//     std::cout << "位深: " << bitsPerPixel << " bits" << std::endl;
//     std::cout << "数据范围: [" << dataRange[0] << ", "
//               << dataRange[1] << "]" << std::endl;

//     // 打印内存使用情况
//     std::cout << "\n=== 内存使用 ===" << std::endl;
//     std::cout << "总体素数: " << totalVoxels << std::endl;
//     std::cout << "内存使用: " << totalMemoryBytes / (1024.0 * 1024.0)
//               << " MB" << std::endl; // 转换为MB并打印
// }



