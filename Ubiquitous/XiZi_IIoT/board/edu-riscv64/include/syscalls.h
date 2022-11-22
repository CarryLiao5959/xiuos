/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
* @file syscalls.h
* @brief add from Canaan K210 SDK
*               https://canaan-creative.com/developer
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

#ifndef _BSP_SYSCALLS_H
#define _BSP_SYSCALLS_H

#include <machine/syscall.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

void __attribute__((noreturn)) sys_exit(int code);

void setStats(int enable);

#undef putchar
int putchar(int ch);
void printstr(const char *s);

void printhex(uint64_t x);


#ifdef __cplusplus
}
#endif

#endif /* _BSP_SYSCALLS_H */

