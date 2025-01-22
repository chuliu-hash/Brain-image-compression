#include "mainwindow.h"
#include"ui_mainwindow.h"
#include <QFileDialog>
#include"worker.h"
#include <filesystem>
namespace fs = std::filesystem;


#include <vtkNIFTIImageReader.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>

// 构造函数
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)  // 调用基类构造函数
    , ui(new Ui::MainWindow)  // 初始化 ui 指针
{
    ui->setupUi(this);  // 设置 UI
    this->setFixedSize(900, 700);
}

// 析构函数
MainWindow::~MainWindow()
{
    delete ui;  // 释放 ui 指针
}



void MainWindow::on_openfile_action_triggered()
{
   // 弹出文件选择对话框，选择单个NIfTI文件
   QString temp =  QFileDialog::getOpenFileName(this, "选择单个NIfTI文件 ", "", "NIfTI Files (*.nii *.nii.gz)");
    if (!temp.isEmpty())
    {
       // 设置NIfTI文件的元数据
       metadata.setNiftiMetadata(temp.toStdString());
        // 在文本框中追加打开的文件名
        ui->plainTextEdit->appendPlainText("打开文件: "+temp);
        // 初始化VTK渲染器
        initializeVTKRenderer(ui->widget_orign, metadata.filePath);
        // 打印NIfTI数据
        printNiftidata();
    }

}
void MainWindow::printNiftidata()
{
    // 将维度信息格式化为字符串
    QString dimensionText = QString("维度: %1 x %2 x %3")
                                .arg(metadata.width)
                                .arg(metadata.height)
                                .arg(metadata.depth);

    // 将内存大小转换为 MB 并格式化为字符串
    double totalMemoryMB = static_cast<double>(metadata.totalMemoryBytes) / (1024 * 1024);
    QString memoryText = QString("原始文件内存占用: %1 MB").arg(totalMemoryMB, 0, 'f', 2); // 保留两位小数


    // 将字符串输出到 QPlainTextEdit
    ui->plainTextEdit->appendPlainText(dimensionText);
    ui->plainTextEdit->appendPlainText(memoryText);

}

void MainWindow::printH265data()
{
    fs::path inputPath = metadata.filePath; // 输入文件路径
    fs::path outputDir = inputPath.parent_path() / "out"; // 输出目录路径
    std::string inputStem = inputPath.stem().string();
    std::string h265Path = (outputDir / (inputStem + ".h265")).string();


    // 获取文件大小（以字节为单位）
    uintmax_t fileSizeBytes = fs::file_size(h265Path);

    // 将文件大小转换为 MB
    double fileSizeMB = static_cast<double>(fileSizeBytes) / (1024 * 1024);

    // 格式化文件大小字符串
    QString fileSizeText = QString("压缩后内存占用: %1 MB").arg(fileSizeMB, 0, 'f', 2); // 保留两位小数

    double totalMemoryMB = static_cast<double>(metadata.totalMemoryBytes) / (1024 * 1024);

    double compressionRatio = totalMemoryMB / fileSizeMB;

    // 格式化压缩比字符串
    QString compressionRatioText = QString("压缩比: %1").arg(compressionRatio, 0, 'f', 2); // 保留两位小数

    // 将文件大小信息输出到 QPlainTextEdit
    ui->plainTextEdit->appendPlainText(fileSizeText);

    ui->plainTextEdit->appendPlainText(compressionRatioText);
}

void MainWindow::on_pushButton_compress_clicked()
{
    // 设置参数
    setParams();

    // 如果文件路径不为空
    if(!metadata.filePath.empty())
    {
        // 在plainTextEdit中添加文本
        ui->plainTextEdit->appendPlainText("开始压缩文件");
        // 禁用组件
        unenableComponent();
        // 开始压缩
        startCompression();

    }
    // 如果文件路径为空
    else
        // 在plainTextEdit中添加文本
        ui->plainTextEdit->appendPlainText("未选择文件");

}

void MainWindow::setParams()
{
    // 1. 获取所有 QlineEdit 的值
    params.crf = ui->doubleSpinBox_crf->value();
    params.qp = ui->spinBox_qp->value();
    params.Bitrate = ui->spinBox_bps->value();
    params.maxNumReferences = ui->spinBox_mrf->value();
    params.keyframeInterval = ui->spinBox_kf->value();
    params.bframes = ui->spinBox_bf->value();

    // 2. 获取所有 ComboBox 的值
    params.mode = ui->comboBox_mode->currentText().toStdString();
    params.preset = ui->comboBox_preset->currentText().toStdString();
    params.tune = ui->comboBox_optimize->currentText().toStdString();


    // 3. 获取所有 CheckBox 的状态
    params.enableSAO = ui->checkBox_SAO->isChecked();
    params.enableLoopFilter = ui->checkBox_LF->isChecked();
    params.enableStrongIntraSmoothing = ui->checkBox_SS->isChecked();
}

void MainWindow::startCompression()
{
    // 创建一个新的线程和一个Worker对象
    QThread *thread = new QThread;
    Worker *worker = new Worker(params, metadata);

    // 将Worker对象移动到新线程中
    worker->moveToThread(thread);

    // 连接信号和槽
    connect(thread, &QThread::started, worker, &Worker::compress);
    connect(worker, &Worker::finished, thread, &QThread::quit);
    connect(worker, &Worker::finished, worker, &Worker::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(worker, &Worker::compressionFinished, this, &MainWindow::do_CompressionFinished);

    // 启动线程
    thread->start();

}

void MainWindow::startBatchCompression(const QList<NiftiMetadata>& metadataList)
{
    // 创建一个新的线程和一个Worker对象
    QThread *thread = new QThread;
    Worker *worker = new Worker(params);

    // 将Worker对象移动到新线程中
    worker->moveToThread(thread);

    // 连接信号和槽
    connect(thread, &QThread::started, worker, [worker, metadataList]() {
        worker->compressBatch(metadataList);
    });
    connect(worker, &Worker::finished, thread, &QThread::quit);
    connect(worker, &Worker::finished, worker, &Worker::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(worker, &Worker::compressionFinished, this, &MainWindow::do_BatchCompressionProgressed);
    connect(worker, &Worker::finished, this, &MainWindow::do_BatchCompressionFinished);
    thread->start();
}

void MainWindow::do_CompressionFinished(const QString &message)
{
    // 在文本框中添加压缩完成的消息
    ui->plainTextEdit->appendPlainText(message);
    // 启用组件
    enableComponent();
    // 打印H265数据
    printH265data();

    // 获取输入文件路径
    fs::path inputPath = metadata.filePath; // 输入文件路径
    std::string inputStem = inputPath.stem().string();
    std::string NiftiPath = inputStem + "_restored.nii";
    // 初始化VTK渲染器
    initializeVTKRenderer(ui->widget_restore, NiftiPath);
}

void MainWindow::do_BatchCompressionFinished()
{
    // 在文本框中添加批量压缩完成的消息
    ui->plainTextEdit->appendPlainText("批量压缩完成，输出文件到out目录");
    // 启用组件
    enableComponent();
}

void MainWindow::do_BatchCompressionProgressed(const QString &message)
{
    // 在文本框中添加批量压缩进度的消息
    ui->plainTextEdit->appendPlainText(message);
}

void MainWindow::initializeVTKRenderer(QVTKOpenGLNativeWidget * widget, const std::string& filePath) {
    // 初始化 VTK 渲染器
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    widget->renderWindow()->AddRenderer(renderer);

    // 加载 NIfTI 文件
    vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
    reader->SetFileName(filePath.c_str());
    reader->Update();

    // 创建 Volume Mapper
    vtkSmartPointer<vtkGPUVolumeRayCastMapper> volumeMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
    volumeMapper->SetInputData(reader->GetOutput());

    // 创建 Volume Property
    vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
    volumeProperty->SetInterpolationTypeToLinear();

    volumeProperty->ShadeOn(); // 启用阴影效果

    // 创建不透明度传输函数
    vtkSmartPointer<vtkPiecewiseFunction> opacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
    opacityTransferFunction->AddPoint(0, 0.0);    // 低灰度值完全透明
    opacityTransferFunction->AddPoint(200, 0.1);  // 较低灰度值较低不透明度
    opacityTransferFunction->AddPoint(500, 0.5);  // 中等灰度值中等不透明度
    opacityTransferFunction->AddPoint(1000, 1.0); // 高灰度值完全不透明
    volumeProperty->SetScalarOpacity(opacityTransferFunction);

    // 创建颜色传输函数
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
    colorTransferFunction->AddRGBPoint(0, 0.0, 0.0, 0.0);     // 黑色
    colorTransferFunction->AddRGBPoint(200, 0.2, 0.2, 0.2);   // 较暗的灰色
    colorTransferFunction->AddRGBPoint(500, 0.5, 0.5, 0.5);   // 中等灰色
    colorTransferFunction->AddRGBPoint(1000, 1.0, 1.0, 1.0);  // 白色
    volumeProperty->SetColor(colorTransferFunction);

    // 调整光照参数
    volumeProperty->SetAmbient(0);  // 增加环境光
    volumeProperty->SetDiffuse(1);  // 增加漫反射光
    volumeProperty->SetSpecular(0); // 增加镜面反射

    // 创建 Volume
    vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
    volume->SetMapper(volumeMapper);
    volume->SetProperty(volumeProperty);

    // 添加 Volume 到渲染器
    renderer->AddVolume(volume);

    // 设置背景颜色
    renderer->SetBackground(0.0, 0.0, 0.0); // 设置背景为黑色

    // 手动刷新渲染窗口
    widget->renderWindow()->Render();
}

void MainWindow::on_batchcompress_action_triggered()
{

    // 打开文件管理器，选择多个 .nii 文件
    QStringList filePaths = QFileDialog::getOpenFileNames(
        this,
        "选择若干个NIfTI文件",
        "",
        "NIfTI Files (*.nii *.nii.gz)"
        );

    if (filePaths.isEmpty()) {
        return;
    }

    setParams();
    unenableComponent();

    // 存储所有文件的 metadata
    QList<NiftiMetadata> metadataList;
    NiftiMetadata metadata_temp;

    // 遍历选中的文件
    for (const QString& filePath : filePaths) {
        metadata_temp.setNiftiMetadata(filePath.toStdString()); // 设置 metadata
        metadataList.append(metadata_temp); // 添加到列表

        // 在 UI 中显示文件信息
    }

    // 调用批量压缩函数
    ui->plainTextEdit->appendPlainText("开始批量压缩");
    startBatchCompression(metadataList);
}
void MainWindow::unenableComponent()
{
    // 禁用组件
    ui->pushButton_compress->setEnabled(false);
    ui->openfile_action->setEnabled(false);
    ui->batchcompress_action->setEnabled(false);
    ui->restore_action->setEnabled(false);
}
void MainWindow::enableComponent()
{
    // 启用组件
    ui->pushButton_compress->setEnabled(true);
    ui->openfile_action->setEnabled(true);
    ui->batchcompress_action->setEnabled(true);
    ui->restore_action->setEnabled(true);
}

void MainWindow::on_restore_action_triggered()
{
    // 获取打开文件对话框中选择的文件路径
    QStringList filePaths = QFileDialog::getOpenFileNames(
        this,
        "选择若干个h265文件",
        "",
        "h265 Files (*.h265 *.hevc)"
        );
    if (filePaths.isEmpty()) {
        return;
    }
    unenableComponent();
    ui->plainTextEdit->appendPlainText("开始批量重建");
    startBatchReconstruction(filePaths);
}

void MainWindow::startBatchReconstruction(const QStringList& filePaths)
{
    // 创建新线程和Worker对象
    QThread *thread = new QThread;
    Worker *worker = new Worker(params);

    worker->moveToThread(thread);

    // 连接信号和槽
    connect(thread, &QThread::started, worker, [worker, filePaths]() {
        worker->reconstructBatch(filePaths);
    });
    connect(worker, &Worker::finished, thread, &QThread::quit);
    connect(worker, &Worker::finished, worker, &Worker::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(worker, &Worker::reconstructionFinished, this, &MainWindow::do_BatchReconstructionProgressed);
    connect(worker, &Worker::finished, this, &MainWindow::do_BatchReconstructionFinished);
    thread->start();
}
void MainWindow::do_BatchReconstructionProgressed(const QString &message)
{
    // 更新进度文本框
    ui->plainTextEdit->appendPlainText(message);
}

void MainWindow::do_BatchReconstructionFinished()
{
    // 批量重建完成，输出文件到reconstruction目录
    ui->plainTextEdit->appendPlainText("批量重建完成，输出文件到reconstruction目录");
    enableComponent();
}
