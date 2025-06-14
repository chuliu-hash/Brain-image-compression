#include "itkImage.h"
#include "itkImageFileReader.h"
#include <itkNiftiImageIO.h>
#include "itkImageRegionConstIterator.h" 
#include <fstream>
#include <cmath>
#include <cstdlib>

typedef itk::Image<short, 3> ImageType;
typedef itk::ImageFileReader<ImageType> ReaderType;

// 将JP3D文件解码为BIN文件
void DecompressJP3D(const std::string& inputJp3d, const std::string& outputBin) {
    std::string command = "opj_jp3d_decompress -i " + inputJp3d + " -o " + outputBin;
    int result = std::system(command.c_str());
    if (result != 0) {
        throw std::runtime_error("JP3D decompression failed");
    }
}

// 读取BIN文件到ITK图像对象
ImageType::Pointer ReadBinFile(const std::string& filename, const itk::Size<3>& size) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize != size[0] * size[1] * size[2] * sizeof(short)) {
        throw std::runtime_error("File size mismatch");
    }

    auto image = ImageType::New();
    ImageType::RegionType region;
    region.SetSize(size);
    image->SetRegions(region);
    image->Allocate();

    file.read(reinterpret_cast<char*>(image->GetBufferPointer()), fileSize);
    return image;
}

double CalculatePSNR(ImageType::Pointer original, ImageType::Pointer decompressed) {
    itk::Size<3> size = original->GetLargestPossibleRegion().GetSize();
    const size_t totalPixels = size[0] * size[1] * size[2];

    double mse = 0.0;
    itk::ImageRegionConstIterator<ImageType> itOrig(original, original->GetLargestPossibleRegion());
    itk::ImageRegionConstIterator<ImageType> itDec(decompressed, decompressed->GetLargestPossibleRegion());

    for (; !itOrig.IsAtEnd(); ++itOrig, ++itDec) {
        const double diff = itOrig.Get() - itDec.Get();
        mse += diff * diff;
    }
    mse /= totalPixels;

    const double maxVal = 32768.0;
    return 10 * std::log10((maxVal * maxVal) / mse);
}

int psnr() {
    try {
        // 步骤1：解压JP3D文件
        DecompressJP3D("compressed.jp3d", "decompressed.bin");

        // 步骤2：读取原始NIfTI文件获取元数据
        ReaderType::Pointer reader = ReaderType::New();
        auto niftiIO = itk::NiftiImageIO::New();
        reader->SetFileName("original.nii");
        reader->SetImageIO(niftiIO);
        reader->Update();
        ImageType::Pointer originalImage = reader->GetOutput();
        const itk::Size<3> imageSize = originalImage->GetLargestPossibleRegion().GetSize();

        // 步骤3：读取解压后的BIN文件
        ImageType::Pointer decompressedImage = ReadBinFile("decompressed.bin", imageSize);

        // 步骤4：计算PSNR
        const double psnr = CalculatePSNR(originalImage, decompressedImage);
        std::cout << "PSNR: " << psnr << " dB" << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
