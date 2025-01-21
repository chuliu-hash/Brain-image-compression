#include"process_function.h"
#include"x265Encoder.h"
#include"NiftiMetadata.h"
#include <filesystem>
namespace fs = std::filesystem;

void compressNifti(const X265Encoder::EncoderParams& params,  const NiftiMetadata& metadata, const bool& restore_flag)
{
    fs::path inputPath = metadata.filePath; // 输入文件路径
    fs::path outputDir = inputPath.parent_path() / "out"; // 输出目录路径
    std::string inputStem = inputPath.stem().string();

    // 如果输出目录不存在，则创建
    if (!fs::exists(outputDir)) {
        fs::create_directory(outputDir);
    }

    std::string inputNifti  = metadata.filePath;
    std::string outputYUV = "temp.yuv";
    std::string outputH265 =(outputDir / (inputStem + ".h265")).string();
    std::string outputNifti =    inputStem + "_restored.nii";

    // 获取原始图像维度
    int width = metadata.width;   // 图像宽度
    int height = metadata.height; // 图像高度
    int depth = metadata.depth;   // 图像深度（切片数量）

    // 计算帧填充后的尺寸，确保是2的倍数（YUV420要求）
    int padded_width = (width + 1) & ~1;    // 向上取偶数
    int padded_height = (height + 1) & ~1;  // 向上取偶数

    // 调用函数将NIfTI图像转换为YUV420P格式
    processNiftiToYUV(inputNifti, outputYUV, padded_width, padded_height);

    // 调用函数将YUV格式转换为H.265格式
    processYUVtoh265(padded_width, padded_height, outputYUV, outputH265, params);

    if (restore_flag)
    {
        // 调用函数将H.265格式转换回NIfTI格式
        processH265ToNifti(outputH265, outputNifti);
    }

}
