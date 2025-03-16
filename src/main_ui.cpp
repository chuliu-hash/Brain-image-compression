#include <QApplication>
#include "mainwindow.h"

int ui_main(int argc, char* argv[])
{
    QApplication app(argc, argv);  // 创建 QApplication 对象
    MainWindow window;             // 创建主窗口
    window.show();                 // 显示窗口
    return app.exec();             // 进入事件循环
}


int main()
{
    return ui_main(0, nullptr);
}
