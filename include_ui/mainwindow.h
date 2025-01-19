#pragma once
#include <QMainWindow>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkImageData.h>
#include <vtkVolume.h>
#include"x265Encoder.h"
#include"NiftiMetadata.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

    ~MainWindow();

private slots:
    void on_openfile_action_triggered();

    void on_pushButton_compress_clicked();

    void do_CompressionFinished(const QString &message);

    void do_BatchCompressionProgressed(const QString &message);

    void do_BatchCompressionFinished();

    void on_batchcompress_action_triggered();

    void on_restore_action_triggered();

    void do_BatchReconstructionProgressed(const QString &message);

    void do_BatchReconstructionFinished();

private:
    Ui::MainWindow* ui;

    NiftiMetadata metadata;
    X265Encoder::EncoderParams params;

    void setParams();
    void startCompression();
    void startBatchCompression(const QList<NiftiMetadata>& metadataList);
    void startBatchReconstruction(const QStringList& filePaths);
    void initializeVTKRenderer(QVTKOpenGLNativeWidget * widget, const std::string& filePath);
    void printNiftidata();
    void printH265data();
    void unenableComponent();
    void enableComponent();
};
