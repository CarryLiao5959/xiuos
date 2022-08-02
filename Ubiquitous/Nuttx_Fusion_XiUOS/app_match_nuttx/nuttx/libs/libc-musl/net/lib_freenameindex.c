/****************************************************************************
 * libs/libc-musl/net/lib_freenameindex.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <net/if.h>

#include "libc-musl.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: if_freenameindex
 *
 * Description:
 *   The if_freenameindex() function free the data structure returned by
 *   if_nameindex().
 *
 * Input Parameters:
 *   ifn - The data structure to free
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void if_freenameindex(FAR struct if_nameindex *ifn)
{
  lib_free(ifn);
}
