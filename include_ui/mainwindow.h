#pragma once
#include <QMainWindow>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkImageData.h>
#include <vtkVolume.h>
#include"x265Encoder.h"
#include"NiftiMetadata.h"
#include"streambuffer.h"

// 前向声明

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // 构造函数，初始化主窗口
    explicit MainWindow(QWidget* parent = nullptr);

    // 析构函数，销毁主窗口
    ~MainWindow();

private slots:
    // 打开文件
    void on_openfile_action_triggered();

    // 压缩按钮点击事件
    void on_pushButton_compress_clicked();

    // 压缩完成事件
    void do_CompressionFinished(const QString &message);

    // 批量压缩进度事件
    void do_BatchCompressionProgressed(const QString &message);

    // 批量压缩完成事件
    void do_BatchCompressionFinished();

    // 批量压缩按钮点击事件
    void on_batchcompress_action_triggered();

    // 恢复按钮点击事件
    void on_restore_action_triggered();

    // 批量重建进度事件
    void do_BatchReconstructionProgressed(const QString &message);

    // 批量重建完成事件
    void do_BatchReconstructionFinished();

private:
    // 用户界面
    Ui::MainWindow* ui;

    // Nifti元数据
    NiftiMetadata metadata;
    // X265编码器参数
    X265Encoder::EncoderParams params;

    std::unique_ptr<StreamBuffer> m_coutBuffer;  // 用于重定向 std::cout
    std::unique_ptr<StreamBuffer> m_cerrBuffer;  // 用于重定向 std::cerr

    // 设置参数
    void setParams();
    // 开始压缩
    void startCompression();
    // 开始批量压缩
    void startBatchCompression(const QList<NiftiMetadata>& metadataList);
    // 开始批量重建
    void startBatchReconstruction(const QStringList& filePaths);
    // 初始化VTK渲染器
    void initializeVTKRenderer(QVTKOpenGLNativeWidget * widget, const std::string& filePath);
    // 打印Nifti数据
    void printNiftidata();
    // 打印H265数据
    void printH265data();
    // 禁用组件
    void unenableComponent();
    // 启用组件
    void enableComponent();
};