# 教育开发板k210_edu说明

## 板载资源

该教育开发板板载摄像头dvp接口，可外接（ov2640、ov5640等），同时还外接屏幕lcd（320*320），以及W5500网卡。



## 编译说明
先下载源码，进入xiuos根目录：
    
    git submodule init
    git submodule update Ubiquitous/RT-Thread_Fusion_XiUOS/rt-thread
    git submodule update Ubiquitous/RT-Thread_Fusion_XiUOS/aiit_board/k210_edu/kendryte-sdk/kendryte-sdk-source
下载好相应的源码以及勘智官方K210 sdk到指定目录

下载risc-v的工具链，[下载地址](https://github.com/xpack-dev-tools/riscv-none-embed-gcc-xpack/releases)  
    
在`rtconfig.py`中将risc-v工具链的本地路径加入文档。

然后执行scons编译：  
    scons

也可以利用env工具编译，指定工具链路径

         set RTT_EXEC_PATH=D:\xpack-riscv-none-embed-gcc-8.2.0-3.1-win32-x64\xPack\RISC-V Embedded GCC\8.2.0-3.1\bin
         
         scons

