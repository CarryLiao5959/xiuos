# 教育开发板k210_edu说明

## 板载资源

该教育开发板板载摄像头dvp接口，可外接（ov2640、ov5640等），同时还外接屏幕lcd（320*320），以及W5500网卡。

|       功能        | **引脚序号**<br />（并非原理图连接线序号） |    引脚定义    |
| :---------------: | :----------------------------------------: | :------------: |
| 高速串口作为shell |                     5                      |     ISP_RX     |
|                   |                     6                      |     ISP_TX     |
|       W5500       |                     9                      | SPI1_ENET_SCLK |
|                   |                     10                     | SPI1_ENET_MISO |
|                   |                     11                     | SPI1_ENET_MOSI |
|                   |                     12                     | SPI1_ENET_nCS  |
|                   |                     13                     | SPI1_ENET_nRST |
|                   |                     14                     |   ENET_nINT    |
|    ov2640接口     |                     47                     |    SCCB_SDA    |
|                   |                     46                     |   SCCB_SCLK    |
|                   |                     40                     |    CMOS_RST    |
|                   |                     45                     |   CMOS_VSYNC   |
|                   |                     41                     |   CMOS_PWDN    |
|                   |                     42                     |   CMOS_XCLK    |
|                   |                     43                     |   CMOS_PCLK    |
|                   |                     44                     |   CMOS_HREF    |
|      LCD接口      |                     37                     |     LCD_CS     |
|                   |                     38                     |     LCD_WR     |
|                   |                     39                     |     LCD_DC     |
|                   |                     36                     |    LCD_RST     |
|       SD卡        |                     26                     |    SPI_CLK     |
|                   |                     27                     |    SPI_MOSI    |
|                   |                     25                     |    SPI_MISO    |
|                   |                     28                     |  SOFT_SPI_CS0  |
|                   |                                            |                |
|                   |                                            |                |
|                   |                                            |                |







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

