/*
* Copyright (c) 2020 AIIT XUOS Lab
* XiUOS is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*        http://license.coscl.org.cn/MulanPSL2
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

/**
 * @file ch376_test.c
 * @brief xidatong-riscv64 ch376_test.c
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2022.10.26
 */

#include <rtthread.h>
#include <ch376.h>
#include <ch376_fs.h>

rt_uint8_t buf[64];

void CH376Demo(void)
{
    rt_uint8_t s;
	s = mInitCH376Host();
	rt_kprintf("ch376 init stat=0x%02x\n",(rt_uint16_t)s);
    
    /* Query device connections and initialize devices */
#ifdef CH376_USB_FUNCTION
    rt_kprintf( "Wait Udisk/SD...\n" );
    while ( CH376DiskConnect( ) != USB_INT_SUCCESS ) 
    {
        rt_thread_mdelay( 100 );
    }
#endif
    for ( s = 0; s < 10; s ++ ) 
    {
        rt_thread_mdelay( 50 );
        rt_kprintf("Ready ?\n");
        if ( CH376DiskMount( ) == USB_INT_SUCCESS ) break;
    }

    /* test 1 : read udisk information */
    s = CH376ReadBlock( buf );
    if ( s == sizeof( INQUIRY_DATA ) ) 
    {
        buf[ s ] = 0;
        rt_kprintf("UdiskInfo: %s\n", ((P_INQUIRY_DATA)buf) -> VendorIdStr);
    }

    /* test 2 : create file */
    rt_kprintf("Create /YEAR2022/DEMO2022.TXT... \n");
    s = CH376DirCreate((PUINT8)"/YEAR2022" );
    //rt_kprintf("CH376DirCreate:0x%02x\n",(rt_uint16_t)s );
    s = CH376FileCreatePath((PUINT8)"/YEAR2022/DEMO2022.TXT");
    //rt_kprintf("CH376FileCreatePath:0x%02x\n",(rt_uint16_t)s);
    if ( s == USB_INT_SUCCESS ) rt_kprintf("Create /YEAR2022/DEMO2022.TXT successful!\n");
    else rt_kprintf("Create /YEAR2022/DEMO2022.TXT failed.\n");

    /* test 3 : write data */
    rt_kprintf("Write some data to file...\n" );
    strcpy( (char *)buf, "This is TEST DATA.\xd\xa" );
    s = CH376ByteWrite(buf, strlen((char *)buf), NULL );
    //rt_kprintf("CH376ByteWrite:0x%02x\n",(rt_uint16_t)s );
    if ( s == USB_INT_SUCCESS ) rt_kprintf("Write data successful!\n");
    else rt_kprintf("Write data failed.\n");

    /* test 4 : close file */
    rt_kprintf( "Close...\n" );
    s = CH376FileClose( TRUE );
    //rt_kprintf( "CH376FileClose:0x%02x\n",(rt_uint16_t)s );	
    if ( s == USB_INT_SUCCESS ) rt_kprintf("Close file successful!\n");
    else rt_kprintf("Close file failed.\n");

}
MSH_CMD_EXPORT(CH376Demo,a ch376 test);