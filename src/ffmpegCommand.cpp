#include <windows.h>
#include <string>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <filesystem>

// 切片处理成yuv420p格式
void runFFmpegCommand(const std::string& tempDir, const std::string& outputYUV,const int& padded_width,const int& padded_height) {
    // 构建 FFmpeg 命令
    std::string ffmpegCmd = "ffmpeg -y "
        "-loglevel quiet "
        "-f image2 "
        "-i " + tempDir + "/slice_%d.png "
        "-vf pad=" + std::to_string(padded_width) + ":" + std::to_string(padded_height) + " "
        "-pix_fmt yuv420p "
        "-f rawvideo " +
        outputYUV;

    // 将 char* 转换为 wchar_t*
    int wideCharLength = MultiByteToWideChar(CP_UTF8, 0, ffmpegCmd.c_str(), -1, nullptr, 0);
    std::wstring wideCmd(wideCharLength, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, ffmpegCmd.c_str(), -1, wideCmd.data(), wideCharLength);

    // 初始化 STARTUPINFO 和 PROCESS_INFORMATION 结构
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    // 创建进程并隐藏命令行窗口
    if (CreateProcessW(nullptr, wideCmd.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}




// 解码HEVC视频
std::vector<unsigned char> runFFmpegCommand(const std::string& inputHevc ,const size_t& expectedSize) {

    std::string ffmpegCmd = "ffmpeg -y"
        " -loglevel quiet"
        " -i " + inputHevc +
        " -f rawvideo"
        " -pix_fmt gray"  // 输出为灰度格式
        " pipe:1";        // 将输出发送到管道

    // 设置管道安全属性
    SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
    saAttr.bInheritHandle = TRUE;  // 管道句柄可继承

    // 创建管道
    HANDLE hReadPipe, hWritePipe;
    if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0)) {
        throw std::runtime_error("无法创建管道");
    }

    // 设置启动信息
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;  // 隐藏命令行窗口
    si.hStdOutput = hWritePipe;  // 将FFmpeg输出重定向到管道
    si.hStdError = hWritePipe;   // 将错误输出也重定向到管道

    // 将命令字符串转换为宽字符（Windows API需要）
    int wideCharLength = MultiByteToWideChar(CP_UTF8, 0, ffmpegCmd.c_str(), -1, nullptr, 0);
    std::wstring wideCmd(wideCharLength, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, ffmpegCmd.c_str(), -1, wideCmd.data(), wideCharLength);

    // 创建FFmpeg进程
    if (!CreateProcessW(
        nullptr,                   // 不指定可执行文件路径
        wideCmd.data(),            // 命令行
        nullptr,                   // 进程句柄不可继承
        nullptr,                   // 线程句柄不可继承
        TRUE,                      // 继承句柄
        CREATE_NO_WINDOW,          // 无窗口
        nullptr,                   // 使用父进程环境
        nullptr,                   // 使用父进程目录
        &si,                       // 启动信息
        &pi                        // 进程信息
    )) {
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        throw std::runtime_error("无法创建FFmpeg进程");
    }

    // 关闭写管道句柄（父进程不需要）
    CloseHandle(hWritePipe);

    // 创建缓冲区以存储数据
    std::vector<unsigned char> buffer(expectedSize);
    DWORD bytesRead = 0;
    DWORD totalBytesRead = 0;

    // 循环读取管道数据
    while (totalBytesRead < expectedSize) {
        // 每次读取的最大数据量
        DWORD bytesToRead = static_cast<DWORD>(expectedSize - totalBytesRead);
        if (!ReadFile(hReadPipe, buffer.data() + totalBytesRead, bytesToRead, &bytesRead, nullptr)) {
            CloseHandle(hReadPipe);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            throw std::runtime_error("无法从管道中读取数据");
        }

        // 如果没有读取到数据，说明管道已关闭
        if (bytesRead == 0) {
            break;
        }

        // 更新已读取的总字节数
        totalBytesRead += bytesRead;
    }

    // 检查读取的数据量是否正确
    if (totalBytesRead != expectedSize) {
        CloseHandle(hReadPipe);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        std::cout << "实际读取: " << totalBytesRead << " 字节，预期读取: " << expectedSize << " 字节" << std::endl;
        throw std::runtime_error("读取的数据量不足");
    }

    // 关闭读管道句柄
    CloseHandle(hReadPipe);

    // 等待FFmpeg进程结束
    WaitForSingleObject(pi.hProcess, INFINITE);

    // 关闭进程和线程句柄
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return buffer;
}


// 获取视频信息
std::string runFFmpegCommand(const std::string& filePath) {

    // 构建 ffprobe 命令获取分辨率
    std::string command = "ffprobe -v error -select_streams v:0 -show_entries stream=width,height -of csv=p=0 " + filePath;
    // 设置管道安全属性
    SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
    saAttr.bInheritHandle = TRUE;  // 管道句柄可继承

    // 创建管道
    HANDLE hReadPipe, hWritePipe;
    if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0)) {
        throw std::runtime_error("无法创建管道");
    }

    // 设置启动信息
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;  // 隐藏命令行窗口
    si.hStdOutput = hWritePipe;  // 将命令输出重定向到管道
    si.hStdError = hWritePipe;   // 将错误输出也重定向到管道

    // 将命令字符串转换为宽字符（Windows API需要）
    int wideCharLength = MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, nullptr, 0);
    std::wstring wideCmd(wideCharLength, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, wideCmd.data(), wideCharLength);

    // 创建进程
    if (!CreateProcessW(
        nullptr,                   // 不指定可执行文件路径
        wideCmd.data(),            // 命令行
        nullptr,                   // 进程句柄不可继承
        nullptr,                   // 线程句柄不可继承
        TRUE,                      // 继承句柄
        CREATE_NO_WINDOW,          // 无窗口
        nullptr,                   // 使用父进程环境
        nullptr,                   // 使用父进程目录
        &si,                       // 启动信息
        &pi                        // 进程信息
    )) {
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        throw std::runtime_error("无法创建进程");
    }

    // 关闭写管道句柄（父进程不需要）
    CloseHandle(hWritePipe);

    // 读取管道数据
    std::string output;
    char buffer[128];
    DWORD bytesRead;
    while (true) {
        if (!ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) || bytesRead == 0) {
            break;
        }
        buffer[bytesRead] = '\0';  // 确保字符串以 null 结尾
        output += buffer;
    }

    // 关闭读管道句柄
    CloseHandle(hReadPipe);

    // 等待进程结束
    WaitForSingleObject(pi.hProcess, INFINITE);

    // 关闭进程和线程句柄
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return output;
}