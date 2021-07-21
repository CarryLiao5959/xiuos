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
 * @file adapter_4G.c
 * @brief Implement the connection 4G adapter function
 * @version 1.1
 * @author AIIT XUOS Lab
 * @date 2021.06.25
 */

#include <adapter.h>

#ifdef ADAPTER_EC200T
extern AdapterProductInfoType Ec200tAttach(struct Adapter *adapter);
#endif

#define ADAPTER_4G_NAME "4G"

static int Adapter4GRegister(struct Adapter *adapter)
{
    int ret = 0;

    strncpy(adapter->name, ADAPTER_4G_NAME, NAME_NUM_MAX);

    adapter->net_protocol = IP_PROTOCOL;
    adapter->net_role = CLIENT;
    adapter->adapter_status = UNREGISTERED;

    ret = AdapterDeviceRegister(adapter);
    if (ret < 0) {
        printf("Adapter4G register error\n");
        return -1;
    }

    return ret;
}

int Adapter4GInit(void)
{
    int ret = 0;

    struct Adapter *adapter = malloc(sizeof(struct Adapter));
    if (!adapter) {
        printf("Adapter4GInit malloc error\n");
        free(adapter);
        return -1;
    }

    ret = Adapter4GRegister(adapter);
    if (ret < 0) {
        printf("Adapter4GInit register 4G adapter error\n");
        free(adapter);
        return -1;
    }

#ifdef ADAPTER_EC200T
    AdapterProductInfoType product_info = Ec200tAttach(adapter);
    if (!product_info) {
        printf("Adapter4GInit ec200t attach error\n");
        free(adapter);
        return -1;
    }

    adapter->product_info_flag = 1;
    adapter->info = product_info;
    adapter->done = product_info->model_done;

#endif

    return ret;
}

/******************TEST*********************/
static int Adapter4GTest(void)
{
    int ret = 0;


    return ret;
}
