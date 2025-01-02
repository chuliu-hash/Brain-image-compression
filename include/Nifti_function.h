#pragma once
struct NiftiMetadata {
    int width;                  // 图像的宽度（以像素为单位）
    int height;                 // 图像的高度（以像素为单位）
    int depth;                  // 图像的深度（以切片数为单位，即Z轴的像素数）

    double spacing[3];         // 体素间距（单位：毫米），用于描述每个维度的物理尺寸
    double origin[3];          // 图像的原点坐标（单位：毫米），表示图像在空间中的起始位置
    double dataRange[2];       // 数据范围，表示图像数据的最小值和最大值（例如：灰度值范围）

    std::string dataType;      // 数据类型（例如："unsigned char", "short", "float"等），用于描述每个像素的存储格式
    int bitsPerPixel;          // 每个像素的位数（例如：8位、16位、32位），用于描述图像的色深
    size_t totalMemoryBytes;    // 图像数据在内存中占用的总字节数，方便内存管理和优化
    size_t totalVoxels;         // 总体素数（即图像中的像素总数），用于计算图像的大小和处理效率
};

void showNifti(const std::string& inputNil);

NiftiMetadata printNiftiMetadata(const std::string& inputNii);