#pragma once
#include <QThread>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include "x265Encoder.h"
#include "NiftiMetadata.h"

// 定义一个名为Worker的类，继承自QObject
class Worker : public QObject
{
    Q_OBJECT
public:

    //默认构造函数
    Worker() = default;

public slots:
    // 压缩函数
    void compress(const X265Encoder::EncoderParams& params, const NiftiMetadata& metadata);

    // 批量压缩函数，传入NiftiMetadata列表
    void compressBatch(const X265Encoder::EncoderParams& params, const QList<NiftiMetadata>& metadataList);

    // 批量重建函数，传入文件路径列表
    void reconstructBatch(const QStringList& filePaths);

signals:
    // 完成信号
    void finished();
    // 压缩完成信号，传入消息
    void compressionFinished(const QString &message);
    // 重建完成信号，传入消息
    void reconstructionFinished(const QString &message);
};


