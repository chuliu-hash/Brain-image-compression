#include <filesystem>
#include <itkImage.h>
#include <itkImageFileWriter.h>
#include <itkNiftiImageIO.h>
#include"process_function.h"


using InputPixelType = unsigned char;
using OutputPixelType = short;
using InputImageType = itk::Image<InputPixelType, 3>;
using OutputImageType = itk::Image<OutputPixelType, 3>;

void processH265ToNifti(const std::string& inputHevc, const std::string& outputNifti) {

    int padded_width = 0, padded_height = 0;
    // 获取视频信息，包括（填充后）宽度和高度
    getVideoInfo(inputHevc, padded_width, padded_height);

    // 获取 TXT 文件路径
    std::filesystem::path hevcPath(inputHevc);
    // 获取 TXT 文件的路径
    std::filesystem::path txtFilePath = hevcPath.parent_path() / (hevcPath.stem().string() + ".txt");

    int width = 0, height = 0, depth = 0;
    bool isPadded = true;

    // 从 TXT 文件中读取原始三维图像的维度
    if (readDimensionsFromTxt(txtFilePath.string(), width, height, depth)) {
        // 比较填充尺寸和原始尺寸
        if (width == padded_width && height == padded_height) {
            isPadded = false;
        }
    } else {
        // 如果 TXT 文件不存在
        std::cerr << "TXT文件不存在." << std::endl;
        return;
    }

    try {
        // 3. 创建临时缓冲区以存储填充后的数据
        std::vector<InputPixelType> paddedBuffer(padded_width * padded_height * depth);


        size_t paddedSize = padded_width * padded_height * depth;

        // 读取填充后的数据
        std::string ffmpegCmd = "ffmpeg -y"
                                " -loglevel quiet"
                                " -i " + inputHevc +
                                " -f rawvideo"
                                " -pix_fmt gray"  // 输出为灰度格式
                                " pipe:1";        // 将输出发送到管道




        // 安全属性设置
        SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        // 创建管道
        HANDLE hReadPipe = NULL, hWritePipe = NULL;
        if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0)) {
            throw std::runtime_error("CreatePipe failed: " + std::to_string(GetLastError()));
        }

        // 确保读端不被子进程继承
        SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

        // 进程启动信息
        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi = { 0 };
        si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
        si.wShowWindow = SW_HIDE;
        si.hStdOutput = hWritePipe;
        si.hStdError = hWritePipe;

        // 创建进程
        BOOL success = CreateProcessA(
            NULL,                   // 应用程序名称（使用命令行参数指定）
            const_cast<LPSTR>(ffmpegCmd.c_str()), // 命令行
            NULL,                   // 进程安全属性
            NULL,                   // 线程安全属性
            TRUE,                   // 继承句柄
            CREATE_NO_WINDOW,       // 创建标志
            NULL,                   // 环境变量
            NULL,                   // 当前目录
            &si,                    // 启动信息
            &pi                     // 进程信息
            );

        // 立即关闭不再需要的写管道句柄
        CloseHandle(hWritePipe);
        hWritePipe = NULL;

        if (!success) {
            CloseHandle(hReadPipe);
            throw std::runtime_error("CreateProcess failed: " + std::to_string(GetLastError()));
        }

        try {
            // 读取管道数据
            paddedBuffer.resize(paddedSize);
            DWORD totalBytesRead = 0;
            while (totalBytesRead < paddedSize) {
                DWORD bytesRead = 0;
                if (!ReadFile(hReadPipe, paddedBuffer.data() + totalBytesRead,
                              static_cast<DWORD>(paddedSize - totalBytesRead), &bytesRead, NULL)) {
                    DWORD err = GetLastError();
                    if (err != ERROR_BROKEN_PIPE) { // 正常结束会触发管道断开
                        throw std::runtime_error("ReadFile failed: " + std::to_string(err));
                    }
                    break;
                }

                if (bytesRead == 0) break; // 管道已关闭
                totalBytesRead += bytesRead;
            }

            // 检查数据完整性
            if (totalBytesRead != paddedSize) {
                std::cerr << "Warning: Expected " << paddedSize << " bytes, got " << totalBytesRead << std::endl;
                paddedBuffer.resize(totalBytesRead); // 调整缓冲区大小
            }

            // 等待进程退出（带超时机制）
            const DWORD waitTimeout = 1; // 1秒超时
            if (WaitForSingleObject(pi.hProcess, waitTimeout) == WAIT_TIMEOUT) {

                TerminateProcess(pi.hProcess, 1);
            }
        }
        catch (...) {
            // 异常时确保资源释放
            if (pi.hProcess) TerminateProcess(pi.hProcess, 1);
            throw;
        }

        // 清理资源
        if (hReadPipe) CloseHandle(hReadPipe);
        if (pi.hThread) CloseHandle(pi.hThread);
        if (pi.hProcess) CloseHandle(pi.hProcess);



        // 4. 创建输入和输出图像对象（使用原始尺寸）
        auto inputImage = InputImageType::New();
        auto outputImage = OutputImageType::New();

        // 设置原始图像尺寸
        InputImageType::SizeType size;
        size[0] = width;   // 原始宽度
        size[1] = height;  // 原始高度
        size[2] = depth;   // 深度（帧数）

        InputImageType::IndexType start;
        start.Fill(0);  // 从(0,0,0)开始

        InputImageType::RegionType region;
        region.SetSize(size);  // 设置区域大小
        region.SetIndex(start); // 设置区域起始索引

        // 分配内存
        inputImage->SetRegions(region);
        inputImage->Allocate();
        inputImage->FillBuffer(0); // 初始化为0

        InputPixelType* inputBuffer = inputImage->GetBufferPointer();

        if (isPadded) {
            // 如果填充了，需要裁剪数据
            for (int z = 0; z < depth; z++) {
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        // 计算填充后和原始图像的索引
                        size_t paddedIndex = z * (padded_width * padded_height) + y * padded_width + x;
                        size_t originalIndex = z * (width * height) + y * width + x;
                        inputBuffer[originalIndex] = paddedBuffer[paddedIndex]; // 裁剪数据
                    }
                }
            }
        } else {
            // 如果未填充，直接复制数据
            size_t originalSize = width * height * depth;
            std::memcpy(inputBuffer, paddedBuffer.data(), originalSize * sizeof(InputPixelType));
        }

        // 6. 设置输出图像并分配内存
        outputImage->SetRegions(region);
        outputImage->Allocate();

        // 7. 数据转换（将unsigned char数据映射到short范围）
        OutputPixelType* outputBuffer = outputImage->GetBufferPointer();
        const float inputMin = 0.0f;   // 输入最小值
        const float inputMax = 255.0f; // 输入最大值
        const float outputMin = -32768.0f; // 输出最小值
        const float outputMax = 32767.0f;  // 输出最大值
        size_t totalSize = width * height * depth; // 总像素数

        for (size_t i = 0; i < totalSize; ++i) {
            // 线性映射公式
            float normalizedValue = static_cast<float>(inputBuffer[i]);
            float mappedValue = (normalizedValue - inputMin) * (outputMax - outputMin) / (inputMax - inputMin) + outputMin;
            outputBuffer[i] = static_cast<OutputPixelType>(std::round(mappedValue)); // 将映射值转换为short类型
        }

        // 8. 保存为NIFTI文件
        auto niftiIO = itk::NiftiImageIO::New();
        auto writer = itk::ImageFileWriter<OutputImageType>::New();

        writer->SetFileName(outputNifti); // 设置输出文件名
        writer->SetInput(outputImage);     // 设置要写入的图像数据
        writer->SetImageIO(niftiIO);       // 设置NIFTI格式IO

        try {
            writer->Update(); // 执行文件写入
        }
        catch (itk::ExceptionObject& e) {
            std::cerr << "保存NIFTI文件失败: " << e.what() << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "转换过程出错: " << e.what() << std::endl;
    }
}


