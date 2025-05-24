#include"process_function.h"
#include"x265Encoder.h"
#include"NiftiMetadata.h"
int test()
{


     X265Encoder::EncoderParams params;

    NiftiMetadata metadata; // 创建元数据对象
    metadata.setNiftiMetadata("D:/Download/1.nii"); // 设置NIfTI文件属性

    bool restore_flag = true; // 是否需要恢复NIfTI文件
    compressNifti(params, metadata, restore_flag); // 调用压缩函数
    return 0;

  // processH265ToNifti("D:/Download/1.h265", "D:/Download/2.nii");
}
