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

    // 构造函数，传入X265Encoder::EncoderParams和NiftiMetadata参数
    Worker(const X265Encoder::EncoderParams &params,const NiftiMetadata &metadata)
        : m_params(params), m_metadata(metadata) {}

    // 构造函数，传入X265Encoder::EncoderParams参数
    Worker(const X265Encoder::EncoderParams &params)
        :m_params(params) {}

public slots:
    // 压缩函数
    void compress();

    // 批量压缩函数，传入NiftiMetadata列表
    void compressBatch(const QList<NiftiMetadata>& metadataList);

    // 批量重建函数，传入文件路径列表
    void reconstructBatch(const QStringList& filePaths);

signals:
    // 完成信号
    void finished();
    // 压缩完成信号，传入消息
    void compressionFinished(const QString &message);
    // 重建完成信号，传入消息
    void reconstructionFinished(const QString &message);

private:
    // X265Encoder参数
    X265Encoder::EncoderParams m_params;
    // NiftiMetadata参数
    NiftiMetadata m_metadata;
};


