#include"worker.h"
#include"process_function.h"

void Worker::compress()
{
    compressNifti(m_params, m_metadata, true);

    emit compressionFinished("压缩完成,输出H265文件");

}

void Worker::compressBatch(const QList<NiftiMetadata>& metadataList)
{
    int totalFiles = metadataList.size();
    int currentFile = 0;

    for (const auto& metadata : metadataList) {
        currentFile++;
        compressNifti(m_params, metadata);
        QString progressInfo = QString("%1/%2").arg(currentFile).arg(totalFiles);
        emit compressionFinished("压缩完成: " + QString::fromStdString(metadata.filePath) + " - 进度: " + progressInfo);

    }

    emit finished();
}

void Worker::reconstructBatch(const QStringList& filePaths)
{
    int totalFiles = filePaths.size();
    int currentFile = 0;

    for (const auto& h265Path : filePaths) {
        currentFile++;

        // 获取输入文件的目录和文件名
        QFileInfo fileInfo(h265Path);
        QString inputDir = fileInfo.path(); // 输入文件所在目录
        QString baseName = fileInfo.baseName(); // 输入文件名（不含扩展名）

        // 创建 restore 目录
        QString restoreDir = inputDir + "/restore";
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
        emit reconstructionFinished("重建完成: " + h265Path + " - 进度: " + progressInfo);
    }

    emit finished();
}
