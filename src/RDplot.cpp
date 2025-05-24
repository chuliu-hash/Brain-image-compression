#include <iostream>
#include <vector>
#include <cmath>
#include <matplot/matplot.h> // 用于绘图的库


void psnr_plot() {

	std::vector<double> x265_rates = { 0.2, 0.4, 0.6, 0.8, 1.0 }; // x265的码率 (kbps)
	std::vector<double> x265_psnrs = { 55.1, 58.3, 59.9, 61.8,62.7 }; // x265的PSNR (dB)

	std::vector<double> jp3d_rates = { 0.2, 0.4, 0.6, 0.8, 1.0 }; // JP3D的码率 (kbps)
	std::vector<double> jp3d_psnrs = { 53.4, 55.2, 57.5, 58.8, 59.6 }; // JP3D的PSNR (dB)


    // 创建绘图窗口
    auto fig = matplot::figure();
    fig->position(300,200,1500,1000); // 设置窗口位置


    matplot::xlim({0.0, 1.2}); // 设置 x 轴范围
    matplot::ylim({50.0, 65.0}); // 设置 y 轴范围

    // 绘制x265的RD曲线
    matplot::hold(true); // 启用 hold 模式，防止覆盖
    matplot::plot(x265_rates, x265_psnrs, "r-o")
        ->line_width(5.0)
        .marker_size(10)
        .display_name("x265");

    // 绘制JP3D的RD曲线
    matplot::plot(jp3d_rates, jp3d_psnrs, "b-s")
        ->line_width(5.0)
        .marker_size(10)
        .display_name("JP3D");

    // 设置图表标题和轴标签
    matplot::title("PSNR-Bitrate RD Curve");
    matplot::xlabel("Bitrate (kbps)");
    matplot::ylabel("PSNR (dB)");

    // 显示图例
    auto leg = matplot::legend();
    leg->location(matplot::legend::general_alignment::bottomright);

    matplot::grid(true); // 显示网格

    // 显示图表
    matplot::show();

    // 保存为图片文件
    //const std::string filename = "rd_curve_comparison.png";
    //matplot::save(filename);
}


void ssim_plot() {

    std::vector<double> x265_rates = { 0.2, 0.4, 0.6, 0.8, 1.0 }; // x265的码率 (kbps)
    std::vector<double> x265_ssims = { 28.2, 30.3, 32.9, 35.8,40.8 }; // x265的SSIM (dB)

    std::vector<double> jp3d_rates = { 0.2, 0.4, 0.6, 0.8, 1.0 }; // JP3D的码率 (kbps)
    std::vector<double> jp3d_ssims = { 27.4,29.1, 30.5, 35.1, 38.6 }; // JP3D的SSIM (dB)


    // 创建绘图窗口
    auto fig = matplot::figure();
    fig->position(300, 200, 1500, 1000); // 设置窗口位置


    matplot::xlim({ 0.0, 1.2 }); // 设置 x 轴范围
    matplot::ylim({ 25, 45 }); // 设置 y 轴范围

    // 绘制x265的RD曲线
    matplot::hold(true); // 启用 hold 模式，防止覆盖
    matplot::plot(x265_rates, x265_ssims, "r-o")
        ->line_width(5.0)
        .marker_size(10)
        .display_name("x265");

    // 绘制JP3D的RD曲线
    matplot::plot(jp3d_rates, jp3d_ssims, "b-s")
        ->line_width(5.0)
        .marker_size(10)
        .display_name("JP3D");

    // 设置图表标题和轴标签
    matplot::title("SSIM-Bitrate RD Curve");
    matplot::xlabel("Bitrate (kbps)");
    matplot::ylabel("SSIM (dB)");

    // 显示图例
    auto leg = matplot::legend();
    leg->location(matplot::legend::general_alignment::bottomright);

    matplot::grid(true); // 显示网格

    // 显示图表
    matplot::show();

    // 保存为图片文件
    //const std::string filename = "rd_curve_comparison.png";
    //matplot::save(filename);
}

//int main() {
//    psnr_plot();
//    return 0;
//}

