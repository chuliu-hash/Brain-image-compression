#pragma once
#include <QThread>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include "x265Encoder.h"
#include"NiftiMetadata.h"

class Worker : public QObject
{
    Q_OBJECT
public:
    Worker(const X265Encoder::EncoderParams &params,const NiftiMetadata &metadata)
        : m_params(params), m_metadata(metadata) {}

    Worker(const X265Encoder::EncoderParams &params)
        :m_params(params) {}

public slots:

    void compress();

    void compressBatch(const QList<NiftiMetadata>& metadataList);

    void reconstructBatch(const QStringList& filePaths);

signals:
    void finished();
    void compressionFinished(const QString &message);
    void reconstructionFinished(const QString &message);

private:
    X265Encoder::EncoderParams m_params;
    NiftiMetadata m_metadata;
};


