#include"worker.h"
#include"process_function.h"

void Worker::compress(const X265Encoder::EncoderParams& params, const NiftiMetadata& metadata)
{
    // 调用压缩函数，对m_params和m_metadata进行压缩
    compressNifti(params, metadata, true);

    // 发送压缩完成信号，提示压缩完成，输出H265文件到out目录
    emit compressionFinished("压缩完成,输出H265文件到out目录");

}

void Worker::compressBatch(const X265Encoder::EncoderParams& params, const QList<NiftiMetadata>& metadataList)
{
    // 获取metadataList的大小
    int totalFiles = metadataList.size();
    int currentFile = 0;

    // 遍历metadataList
    for (const auto& metadata : metadataList) {
        currentFile++;
        // 调用压缩函数，对metadata进行压缩
        compressNifti(params, metadata);
        // 构造进度信息
        QString progressInfo = QString("%1/%2").arg(currentFile).arg(totalFiles);
        // 发送压缩完成信号，提示压缩完成，并显示进度信息
        emit compressionFinished("压缩完成: " + QString::fromStdString(metadata.filePath) + " - 进度: " + progressInfo);

    }

    // 发送完成信号，提示压缩完成
    emit finished();
}

void Worker::reconstructBatch(const QStringList& filePaths)
{
    // 获取filePaths的大小
    int totalFiles = filePaths.size();
    int currentFile = 0;

    // 遍历filePaths
    for (const auto& h265Path : filePaths) {
        currentFile++;

        // 获取输入文件的目录和文件名
        QFileInfo fileInfo(h265Path);
        QString inputDir = fileInfo.path(); // 输入文件所在目录
        QString baseName = fileInfo.baseName(); // 输入文件名（不含扩展名）

        // 创建 restore 目录
        QString restoreDir = inputDir + "/reconstruction";
        QDir dir(restoreDir);
        if (!dir.exists()) {
            dir.mkpath("."); // 如果目录不存在，则创建
        }

        // 设置输出文件路径
        QString outputNiftiPath = restoreDir + "/" + baseName + ".nii";

        // 调用重建函数
        processH265ToNifti(h265Path.toStdString(), outputNiftiPath.toStdString());

        // 更新进度信息
        QString progressInfo = QString("%1/%2").arg(currentFile).arg(totalFiles);
        // 发送重建完成信号，提示重建完成，并显示进度信息
        emit reconstructionFinished("重建完成: " + h265Path + " - 进度: " + progressInfo);
    }

    // 发送完成信号，提示重建完成
    emit finished();
}
