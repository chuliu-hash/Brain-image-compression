#include "itkImage.h"
#include "itkImageFileReader.h"
#include <itkNiftiImageIO.h>
#include "itkImageRegionConstIterator.h"
#include <fstream>
#include <cstdlib>

typedef itk::Image<short, 3> ImageType; // 假设 NIfTI 文件是 16 位有符号整数
typedef itk::ImageFileReader<ImageType> ReaderType;

int jp3d()
{
    // 固定文件路径
    const char* inputFileName = "original.nii"; // 输入的 NIfTI 文件路径
    const char* outputRawFileName = "original.bin"; // 输出的原始二进制文件路径
    const char* outputImgFileName = "original.img"; // 输出的图像特性文件路径
    const char* outputJp3dFileName = "compressed.jp3d"; // 输出的 JP3D 文件路径

    // 读取 NIfTI 文件
    ReaderType::Pointer reader = ReaderType::New();
    auto niftiIO = itk::NiftiImageIO::New();
    reader->SetImageIO(niftiIO);  // 设置NIfTI格式的IO接口
    reader->SetFileName(inputFileName);

    try
    {
        reader->Update();
    }
    catch (itk::ExceptionObject& error)
    {
        std::cerr << "Error: " << error << std::endl;
        return EXIT_FAILURE;
    }

    ImageType::Pointer image = reader->GetOutput();

    // 获取图像尺寸
    ImageType::RegionType region = image->GetLargestPossibleRegion();
    ImageType::SizeType size = region.GetSize();


    // 将图像数据写入原始二进制文件
    std::ofstream rawFile(outputRawFileName, std::ios::out | std::ios::binary);
    if (!rawFile)
    {
        std::cerr << "Error: Could not open " << outputRawFileName << " for writing." << std::endl;
        return EXIT_FAILURE;
    }

    itk::ImageRegionConstIterator<ImageType> it(image, region);
    for (it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
        short pixelValue = it.Get();
        rawFile.write(reinterpret_cast<char*>(&pixelValue), sizeof(short));
    }

    rawFile.close();

    // 创建图像特性文件
    std::ofstream imgFile(outputImgFileName);
    if (!imgFile)
    {
        std::cerr << "Error: Could not open " << outputImgFileName << " for writing." << std::endl;
        return EXIT_FAILURE;
    }

    // 写入 .img 文件内容
    imgFile << "Bpp\t16" << std::endl; // 假设是 16 位数据
    imgFile << "Color Map\t1" << std::endl; // 假设是单通道图像
    imgFile << "Dimensions\t" << size[0] << " " << size[1] << " " << size[2] << std::endl; // 图像尺寸
    imgFile.close();
    std::cout << "Image characteristics file successfully written to " << outputImgFileName << std::endl;

    // 调用 opj_jp3d_compress 进行 JP3D 压缩
    std::string command = "opj_jp3d_compress -i " + std::string(outputRawFileName) +
        " -m " + std::string(outputImgFileName) +
        " -o " + std::string(outputJp3dFileName) + 
        "  -q 45,50,60"  +
       " -b 16,16,16  -M 102 -n 2,2,1";

    int result = std::system(command.c_str());

    if (result != 0)
    {
        std::cerr << "Error: JP3D compression failed." << std::endl;
        return EXIT_FAILURE;
    }


  
      return EXIT_SUCCESS;
}

//int main()
//{
//    return jp3d();
//}