#include <process_function.h>
#include <iostream>
#include <x265Encoder.h>
#include<Nifti_function.h>

int main()
{

    system("chcp 65001"); 

    // 输入和输出文件路径
    std::string inputNifti = "D:/VScode/Capstone_project/CMakeProject1/resources/mni_icbm152_wm_tal_nlin_sym_09a.nii";
    std::string outputYUV = "D:/VScode/Capstone_project/CMakeProject1/resources/temp.yuv";
    std::string outputH265 = "D:/VScode/Capstone_project/CMakeProject1/resources/temp.h265";
    std::string outputNifti = "D:/VScode/Capstone_project/CMakeProject1/resources/restored.nii";

    // 打印NIfTI元数据并获取图像信息
    NiftiMetadata metadata = printNiftiMetadata(inputNifti);

    std::cout << "\n\n\n开始转换NiftiToYUV...\n\n\n";

    // 获取原始图像维度
    int width = metadata.width;   // 图像宽度
    int height = metadata.height; // 图像高度
    int depth = metadata.depth;   // 图像深度（切片数量）

    // 计算帧填充后的尺寸，确保是2的倍数（YUV420要求）
    int padded_width = (width + 1) & ~1;    // 向上取偶数
    int padded_height = (height + 1) & ~1;  // 向上取偶数

    // 调用函数将NIfTI图像转换为YUV格式
    processNiftiToYUV(inputNifti, outputYUV, padded_width, padded_height);


    // 自定义编码参数
    X265Encoder::EncoderParams params;
    params.mode = 3;  // 0为ABR模式，1为CQP模式，2为CRF模式，3为无损压缩
    params.maxNumReferences = 4;       // 最大参考帧数
    params.keyframeInterval = 63;      // 关键帧间隔
    params.bframes = 3;                // B帧数量
    params.preset = "medium";          // 编码速度预设
    params.tune = "psnr";              // 优化目标
    params.qp = 20;                    // 量化参数
    params.crf = 24;                   // CRF值
    params.maxBitrate = 1000;         // 最大码率 (kbps)
    params.enableSAO = true;          // 启用SAO（样本自适应偏移）
    params.enableLoopFilter = true;    // 启用环路滤波
    params.enableStrongIntraSmoothing = false; // 启用强帧内平滑

    std::cout << "\n\n\n开始转换YUVtoH265...\n\n\n" << std::endl;

    // 调用函数将YUV格式转换为H.265格式
    processYUVtoh265(padded_width, padded_height, outputYUV, outputH265, params);

    std::cout << "\n\n\n开始转换H265toNifti...\n\n\n" << std::endl;

    // 调用函数将H.265格式转换回NIfTI格式
    processH265ToNifti(outputH265, outputNifti, width, height, depth);

    // 显示原始NIfTI图像
    showNifti(inputNifti);
    // 显示转换后的NIfTI图像
    showNifti(outputNifti);

    // 打印转换后的NIfTI图像的元数据
    printNiftiMetadata(outputNifti);

    return 0; // 程序结束
}
