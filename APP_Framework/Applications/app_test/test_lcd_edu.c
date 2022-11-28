#include <stdio.h>
#include <string.h>
#include <transform.h>

#define GRAPHIC_CTRL_RECT_UPDATE 0x00
#define LCD_STRING_TYPE 0
#define LCD_DOT_TYPE 1
#define LCD_FONT_RECT_WIDTH 150
#define LCD_FONT_RECT_HEIGHT 50
#define NULL_PARAMETER 0

void TestLcd(void)
{
    int lcd_fd = PrivOpen(EDU_LCD_DEV_DRIVER, O_RDWR);
    if (lcd_fd < 0)
    {
        printf("open lcd fd error:%d\n", lcd_fd);
        return;
    }

    // draw text
    LcdWriteParam graph_param;
    graph_param.type = LCD_STRING_TYPE;
    graph_param.string_info.x_pos = 0;
    graph_param.string_info.y_pos = 0;
    graph_param.string_info.width = 250;
    graph_param.string_info.height = 24;
    graph_param.string_info.font_size = 24;
    graph_param.string_info.back_color = 0xFFFF;
    graph_param.string_info.font_color = 0x0000;
    graph_param.string_info.addr = "hello_world!";

    PrivWrite(lcd_fd, &graph_param, NULL_PARAMETER);

    uint16 color_select = 0xF800;
    for (int i = 0; i < 5; i++)
    {
        graph_param.type = LCD_DOT_TYPE;
        graph_param.pixel_info.x_startpos = 0;
        graph_param.pixel_info.y_startpos = 50 * i;
        graph_param.pixel_info.x_endpos = 320;
        graph_param.pixel_info.y_endpos = 50 * i;
        graph_param.pixel_info.pixel_color = &color_select;
        PrivWrite(lcd_fd, &graph_param, NULL_PARAMETER);
    }

    PrivClose(lcd_fd);
}

PRIV_SHELL_CMD_FUNCTION(TestLcd, a lcd test sample, PRIV_SHELL_CMD_MAIN_ATTR);