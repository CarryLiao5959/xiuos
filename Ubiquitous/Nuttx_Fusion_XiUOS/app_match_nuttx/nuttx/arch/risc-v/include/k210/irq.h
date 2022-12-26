/****************************************************************************
 * arch/risc-v/include/k210/irq.h
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

#ifndef __ARCH_RISCV_INCLUDE_K210_IRQ_H
#define __ARCH_RISCV_INCLUDE_K210_IRQ_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <arch/irq.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Map RISC-V exception code to NuttX IRQ */

#ifdef CONFIG_K210_WITH_QEMU
#define K210_IRQ_UART0    (RISCV_IRQ_MEXT + 4)
#else
#define K210_IRQ_UART0    (RISCV_IRQ_MEXT + 33)
#define K210_IRQ_UART1    (RISCV_IRQ_MEXT + 11)
#define K210_IRQ_UART2    (RISCV_IRQ_MEXT + 12)
#define K210_IRQ_UART3    (RISCV_IRQ_MEXT + 13)
#endif

/* Total number of IRQs */

#define NR_IRQS           (64 + 16 +16)

#endif /* __ARCH_RISCV_INCLUDE_K210_IRQ_H */