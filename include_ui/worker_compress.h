#pragma once
#include <QThread>
#include <QDebug>
#include "x265Encoder.h"
#include"NiftiMetadata.h"
#include "process_function.h"

class Worker : public QObject
{
    Q_OBJECT
public:
    Worker(const X265Encoder::EncoderParams &params,const NiftiMetadata &metadata)
        : m_params(params), m_metadata(metadata) {}

    Worker(const X265Encoder::EncoderParams &params)
        :m_params(params) {}

public slots:

    void compress()
    {
        compressNifti(m_params, m_metadata);

        emit compressionFinished("压缩完成,输出H265文件");

    }

    void compressBatch(const QList<NiftiMetadata>& metadataList)
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


signals:
    void finished();
    void compressionFinished(const QString &message);

private:
    X265Encoder::EncoderParams m_params;
    NiftiMetadata m_metadata;
};


