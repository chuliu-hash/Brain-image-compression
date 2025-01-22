#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);  // 用于体绘制
VTK_MODULE_INIT(vtkRenderingOpenGL2);        // 用于基本渲染
VTK_MODULE_INIT(vtkInteractionStyle);        // 用于交互
VTK_MODULE_INIT(vtkRenderingFreeType);       // 用于文本渲染

#include <vtkSmartPointer.h>
#include <vtkNIFTIImageReader.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkVolume.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include "NiftiMetadata.h"

void showNifti(const std::string& inputNii)
{

    // 创建 NIFTI 读取器，用于读取 NIfTI 格式的图像文件
    vtkSmartPointer<vtkNIFTIImageReader> reader =
        vtkSmartPointer<vtkNIFTIImageReader>::New();

    // 使用完整路径设置要读取的 NIfTI 文件名
    reader->SetFileName(inputNii.c_str());

    // 检查文件是否可以读取
    if (!reader->CanReadFile(reader->GetFileName()))
    {
        std::cerr << "Error: Cannot read file " << reader->GetFileName() << std::endl;
        return; // 如果无法读取文件，退出函数
    }

    // 更新读取器以读取图像数据
    reader->Update();

    // 设置体绘制映射器，用于将3D数据映射到2D图像
    vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper =
        vtkSmartPointer<vtkSmartVolumeMapper>::New();
    volumeMapper->SetInputConnection(reader->GetOutputPort()); // 将读取器的输出连接到映射器

    // 创建渲染器和窗口，用于显示图像
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renderWindow =
        vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer); // 将渲染器添加到窗口

    // 创建交互器，用于处理用户输入
    vtkSmartPointer<vtkRenderWindowInteractor> interactor =
        vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor->SetRenderWindow(renderWindow); // 将窗口与交互器关联

    // 设置体绘制属性
    vtkSmartPointer<vtkVolumeProperty> volumeProperty =
        vtkSmartPointer<vtkVolumeProperty>::New();
    volumeProperty->ShadeOn(); // 启用阴影效果

    // 创建并显示体绘制
    vtkSmartPointer<vtkVolume> volume =
        vtkSmartPointer<vtkVolume>::New();
    volume->SetMapper(volumeMapper); // 设置体绘制映射器
    volume->SetProperty(volumeProperty); // 设置体绘制属性

    // 添加不透明度传输函数，用于控制不同灰度值的透明度
    vtkSmartPointer<vtkPiecewiseFunction> opacityTransferFunction =
        vtkSmartPointer<vtkPiecewiseFunction>::New();

    // 参数说明：(灰度值, 不透明度)
    opacityTransferFunction->AddPoint(0, 0.0);    // 低灰度值完全透明
    opacityTransferFunction->AddPoint(500, 0.2);  // 中等灰度值半透明
    opacityTransferFunction->AddPoint(1000, 0.8); // 高灰度值不透明
    volumeProperty->SetScalarOpacity(opacityTransferFunction); // 设置不透明度传输函数

    // 添加颜色传输函数，用于控制不同灰度值的颜色
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction =
        vtkSmartPointer<vtkColorTransferFunction>::New();

    // 参数说明：(灰度值, R, G, B)
    colorTransferFunction->AddRGBPoint(0, 0.0, 0.0, 0.0);     // 黑色
    colorTransferFunction->AddRGBPoint(500, 0.5, 0.5, 0.5);   // 灰色
    colorTransferFunction->AddRGBPoint(1000, 1.0, 1.0, 1.0);  // 白色
    volumeProperty->SetColor(colorTransferFunction); // 设置颜色传输函数

    // 调整环境光和漫反射光的强度
    volumeProperty->SetAmbient(0.2);  // 增加环境光
    volumeProperty->SetDiffuse(0.6);  // 增加漫反射光
    volumeProperty->SetSpecular(0.2); // 添加少量镜面反射

    // 将体绘制添加到渲染器
    renderer->AddVolume(volume);
    renderer->ResetCamera(); // 重置相机以适应新添加的体绘制

    // 渲染窗口并开始交互
    renderWindow->Render(); // 渲染窗口内容
    interactor->Start();    // 启动交互器，等待用户输入
}
