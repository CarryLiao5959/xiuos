/*
* Copyright (c) 2021 AIIT XUOS Lab
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
 * @file ua_api.c
 * @brief Demo for OpcUa function
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.11.11
 */

#include "open62541.h"
#include <stdlib.h>
#include "ua_api.h"

int ua_open(void *dev)
{
    UaParamType *param = (UaParamType *)dev;

    param->client = UA_Client_new();

    ua_pr_info("ua: [%s] start ...\n", __func__);

    if (param->client == NULL)
    {
        ua_print("ua: [%s] tcp client null\n", __func__);
        return EEMPTY;
    }

    UA_ClientConfig *config = UA_Client_getConfig(param->client);
    UA_ClientConfig_setDefault(config);

    ua_pr_info("ua: [%s] %d %s\n", __func__, strlen(param->ua_remote_ip), param->ua_remote_ip);

    UA_StatusCode retval = UA_Client_connect(param->client, param->ua_remote_ip);
    if(retval != UA_STATUSCODE_GOOD) {
        ua_pr_info("ua: [%s] deleted ret %x!\n", __func__, retval);
        return (int)retval;
    }
    return EOK;
}

void ua_close(void *dev)
{
    UaParamType *param = (UaParamType *)dev;
    UA_Client_disconnect(param->client);
    UA_Client_delete(param->client); /* Disconnects the client internally */
}

int ua_read(void *dev, void *buf, size_t len)
{
    UaParamType *param = (UaParamType *)dev;
    switch(param->act)
    {
        case UA_ACT_ATTR:
            ua_read_nodeid_value(param->client, param->ua_id, buf);
            break;
        case UA_ACT_OBJ:
            ua_test_browser_objects(param->client);
            break;
        default:
            break;
    }
    return EOK;
}

int ua_write(void *dev, const void *buf, size_t len)
{
    UaParamType *param = (UaParamType *)dev;

    switch(param->act)
    {
        case UA_ACT_ATTR:
            ua_write_nodeid_value(param->client, param->ua_id, (char *)buf);
            break;
        case UA_ACT_OBJ:
            ua_test_browser_objects(param->client);
            break;
        default:
            break;
    }
    return EOK;
}

int ua_ioctl(void *dev, int cmd, void *arg)
{
    return EOK;
}

