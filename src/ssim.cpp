#include "itkImage.h"
#include "itkImageFileReader.h"
#include <itkNiftiImageIO.h>
#include "itkImageRegionConstIterator.h"
#include <fstream>
#include <cmath>
#include <vector>
#include <numeric>

typedef itk::Image<short, 3> ImageType;
typedef itk::ImageFileReader<ImageType> ReaderType;

// 计算局部窗口的均值和方差
void ComputeLocalStats(ImageType::Pointer image,
    const itk::Index<3>& centerIdx,
    int windowSize,
    double& mean,
    double& variance)
{
    itk::Size<3> radius;
    radius.Fill(windowSize / 2);

    itk::Size<3> imageSize = image->GetLargestPossibleRegion().GetSize();
    itk::Index<3> startIdx;

    int count = 0;
    double sum = 0.0;
    double sumSq = 0.0;

    // 遍历局部窗口
    for (int k = -radius[2]; k <= radius[2]; ++k) {
        for (int j = -radius[1]; j <= radius[1]; ++j) {
            for (int i = -radius[0]; i <= radius[0]; ++i) {
                itk::Index<3> idx = {
                    centerIdx[0] + i,
                    centerIdx[1] + j,
                    centerIdx[2] + k
                };

                // 边界检查
                if (idx[0] >= 0 && idx[0] < imageSize[0] &&
                    idx[1] >= 0 && idx[1] < imageSize[1] &&
                    idx[2] >= 0 && idx[2] < imageSize[2])
                {
                    double val = image->GetPixel(idx);
                    sum += val;
                    sumSq += val * val;
                    count++;
                }
            }
        }
    }

    mean = sum / count;
    variance = (sumSq / count) - (mean * mean);
}

// 计算两幅图像的SSIM
double CalculateSSIM(ImageType::Pointer original,
    ImageType::Pointer decompressed,
    int windowSize = 11,
    double K1 = 0.01,
    double K2 = 0.03)
{
    const double L = 32768.0; // 最大像素值(short类型)
    const double C1 = (K1 * L) * (K1 * L);
    const double C2 = (K2 * L) * (K2 * L);

    itk::Size<3> size = original->GetLargestPossibleRegion().GetSize();
    double totalSSIM = 0.0;
    int validWindows = 0;

    // 遍历图像中的每个像素作为窗口中心
    itk::ImageRegionConstIterator<ImageType> itOrig(original, original->GetLargestPossibleRegion());
    itk::ImageRegionConstIterator<ImageType> itDec(decompressed, decompressed->GetLargestPossibleRegion());

    while (!itOrig.IsAtEnd()) {
        itk::Index<3> idx = itOrig.GetIndex();

        // 计算原始图像和解压图像的局部统计量
        double meanOrig, varOrig, meanDec, varDec, covar;
        ComputeLocalStats(original, idx, windowSize, meanOrig, varOrig);
        ComputeLocalStats(decompressed, idx, windowSize, meanDec, varDec);

        // 计算协方差
        double crossSum = 0.0;
        int count = 0;
        itk::Size<3> radius;
        radius.Fill(windowSize / 2);

        for (int k = -radius[2]; k <= radius[2]; ++k) {
            for (int j = -radius[1]; j <= radius[1]; ++j) {
                for (int i = -radius[0]; i <= radius[0]; ++i) {
                    itk::Index<3> localIdx = {
                        idx[0] + i,
                        idx[1] + j,
                        idx[2] + k
                    };

                    if (localIdx[0] >= 0 && localIdx[0] < size[0] &&
                        localIdx[1] >= 0 && localIdx[1] < size[1] &&
                        localIdx[2] >= 0 && localIdx[2] < size[2])
                    {
                        double valOrig = original->GetPixel(localIdx);
                        double valDec = decompressed->GetPixel(localIdx);
                        crossSum += (valOrig - meanOrig) * (valDec - meanDec);
                        count++;
                    }
                }
            }
        }
        covar = crossSum / count;

        // 计算当前窗口的SSIM
        double numerator = (2 * meanOrig * meanDec + C1) * (2 * covar + C2);
        double denominator = (meanOrig * meanOrig + meanDec * meanDec + C1) * (varOrig + varDec + C2);
        double ssim = numerator / denominator;

        totalSSIM += ssim;
        validWindows++;

        ++itOrig;
        ++itDec;
    }

    return totalSSIM / validWindows;
}
// 读取BIN文件到ITK图像对象
ImageType::Pointer ReadFile(const std::string& filename, const itk::Size<3>& size) {
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

int ssim() {
    try {
        // 读取原始和解压图像
        ReaderType::Pointer reader = ReaderType::New();
        auto niftiIO = itk::NiftiImageIO::New();
        reader->SetFileName("original.nii");
        reader->SetImageIO(niftiIO);
        reader->Update();
        ImageType::Pointer originalImage = reader->GetOutput();

        ImageType::Pointer decompressedImage = ReadFile("decompressed.bin",
            originalImage->GetLargestPossibleRegion().GetSize());

        // 计算SSIM
        const double ssim = CalculateSSIM(originalImage, decompressedImage);
        std::cout << "SSIM: " << ssim << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
