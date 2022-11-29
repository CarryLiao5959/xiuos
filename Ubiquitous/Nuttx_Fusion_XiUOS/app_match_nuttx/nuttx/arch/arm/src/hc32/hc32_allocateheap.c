/****************************************************************************
 * arch/arm/src/hc32/hc32_allocateheap.c
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

#include <nuttx/config.h>

#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <debug.h>

#include <nuttx/arch.h>
#include <nuttx/board.h>
#include <nuttx/kmalloc.h>
#include <nuttx/userspace.h>

#include <arch/board/board.h>

#include "chip.h"

#include "mpu.h"
#include "arm_internal.h"
#include "hc32_mpuinit.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Internal SRAM is available in all members of the HC32 family. The
 * following definitions must be provided to specify the size and
 * location of internal(system) SRAM:
 *
 * CONFIG_RAM_END             : End address (+1) of SRAM (F1 family only, the
 *                            : F4 family uses the a priori end of SRAM)
 *
 * The F4 family also contains internal CCM SRAM.  This SRAM is different
 * because it cannot be used for DMA.  So if DMA needed, then the following
 * should be defined to exclude CCM SRAM from the heap:
 *
 * CONFIG_HC32_CCMEXCLUDE    : Exclude CCM SRAM from the HEAP
 *
 * In addition to internal SRAM, external RAM may also be available through
 * the FMC/FSMC. To use external RAM, the following things need to be present
 * in the NuttX configuration file:
 *
 * CONFIG_HC32_FSMC=y         : Enables the FSMC
 * CONFIG_HC32_FMC=y          : Enables the FMC
 * CONFIG_HC32_EXTERNAL_RAM=y : Indicates external RAM is available via the
 *                               FMC/FSMC (as opposed to an LCD or FLASH).
 * CONFIG_HEAP2_BASE           : The base address of the external RAM
 * CONFIG_HEAP2_SIZE           : The size of the external RAM
 * CONFIG_MM_REGIONS           : Must be set to a large enough value to
 *                               include the external RAM (as determined by
 *                               the rules provided below)
 */

#if !defined(CONFIG_HC32_FSMC) && !defined(CONFIG_HC32_FMC)
#  undef CONFIG_HC32_EXTERNAL_RAM
#endif

//#if defined(CONFIG_HC32_HC32F4A0)

/* The HC32 F4A0 have no CCM SRAM */

#  if defined(CONFIG_HC32_HC32F4A0)
#    undef CONFIG_HC32_CCMEXCLUDE
#    define CONFIG_HC32_CCMEXCLUDE 1
#  endif

/* Set the end of system SRAM */

#define SRAM1_END 0x1FFF0000


/* Set the range of CCM SRAM as well (although we may not use it) */

#define SRAM2_START 0x1FFF0000
#define SRAM2_END   0x20070000


/* There are 4 possible SRAM configurations:
 *
 * Configuration 1. System SRAM (only)
 *                  CONFIG_MM_REGIONS == 1
 *                  CONFIG_HC32_EXTERNAL_RAM NOT defined
 *                  CONFIG_HC32_CCMEXCLUDE defined
 * Configuration 2. System SRAM and CCM SRAM
 *                  CONFIG_MM_REGIONS == 2
 *                  CONFIG_HC32_EXTERNAL_RAM NOT defined
 *                  CONFIG_HC32_CCMEXCLUDE NOT defined
 * Configuration 3. System SRAM and FSMC SRAM
 *                  CONFIG_MM_REGIONS == 2
 *                  CONFIG_HC32_EXTERNAL_RAM defined
 *                  CONFIG_HC32_CCMEXCLUDE defined
 * Configuration 4. System SRAM, CCM SRAM, and FSMC SRAM
 *                  CONFIG_MM_REGIONS == 3
 *                  CONFIG_HC32_EXTERNAL_RAM defined
 *                  CONFIG_HC32_CCMEXCLUDE NOT defined
 *
 * Let's make sure that all definitions are consistent before doing
 * anything else
 */

#  if defined(CONFIG_HC32_EXTERNAL_RAM)

/* Configuration 3 or 4. External SRAM is available.  CONFIG_MM_REGIONS
 * should be at least 2.
 */

#    if CONFIG_MM_REGIONS < 2

/* Only one memory region.  Force Configuration 1 */

#      warning "FSMC SRAM (and CCM SRAM) excluded from the heap"
#      undef CONFIG_HC32_EXTERNAL_RAM
#      undef CONFIG_HC32_CCMEXCLUDE
#      define CONFIG_HC32_CCMEXCLUDE 1

/* CONFIG_MM_REGIONS may be 3 if CCM SRAM is included in the head */

#    elif CONFIG_MM_REGIONS > 2

/* More than two memory regions.  This is okay if CCM SRAM is not
 * disabled.
 */

#      if defined(CONFIG_HC32_CCMEXCLUDE)

/* Configuration 3: CONFIG_MM_REGIONS should have been 2 */

#        error "CONFIG_MM_REGIONS > 2 but I don't know what some of the region(s) are"
#        undef CONFIG_MM_REGIONS
#        define CONFIG_MM_REGIONS 2
#      else

/* Configuration 4: DMA should be disabled and CONFIG_MM_REGIONS
 * should be 3.
 */

#        ifdef CONFIG_ARCH_DMA
#          warning "CCM SRAM is included in the heap AND DMA is enabled"
#        endif

#        if CONFIG_MM_REGIONS != 3
#          error "CONFIG_MM_REGIONS > 3 but I don't know what some of the region(s) are"
#          undef CONFIG_MM_REGIONS
#          define CONFIG_MM_REGIONS 3
#        endif
#      endif

/* CONFIG_MM_REGIONS is exactly 2.  We cannot support both CCM SRAM and
 * FSMC SRAM.
 */

#    elif !defined(CONFIG_HC32_CCMEXCLUDE)
#      error "CONFIG_MM_REGIONS == 2, cannot support both CCM SRAM and FSMC SRAM"
#      undef CONFIG_HC32_CCMEXCLUDE
#      define CONFIG_HC32_CCMEXCLUDE 1
#    endif

#  elif !defined(CONFIG_HC32_CCMEXCLUDE)

/* Configuration 2: FSMC SRAM is not used, but CCM SRAM is requested.
 * DMA should be disabled and CONFIG_MM_REGIONS should be 2.
 */

#    ifdef CONFIG_ARCH_DMA
#      warning "CCM SRAM is included in the heap AND DMA is enabled"
#    endif

#    if CONFIG_MM_REGIONS < 2
#      ifdef CONFIG_HC32_HAVE_CCM
#        error "CCM SRAM excluded from the heap because CONFIG_MM_REGIONS < 2"
#      endif
#      undef CONFIG_HC32_CCMEXCLUDE
#      define CONFIG_HC32_CCMEXCLUDE 1
#    elif CONFIG_MM_REGIONS > 2
#      error "CONFIG_MM_REGIONS > 2 but I don't know what some of the region(s) are"
#      undef CONFIG_MM_REGIONS
#      define CONFIG_MM_REGIONS 2
#    endif
#  endif


/* There are 4 possible SRAM configurations:
 *
 * Configuration 1. System SRAM (only)
 *                  CONFIG_MM_REGIONS == 1
 *                  CONFIG_HC32_EXTERNAL_RAM NOT defined
 *                  CONFIG_HC32_CCMEXCLUDE defined
 * Configuration 2. System SRAM and CCM SRAM
 *                  CONFIG_MM_REGIONS == 2
 *                  CONFIG_HC32_EXTERNAL_RAM NOT defined
 *                  CONFIG_HC32_CCMEXCLUDE NOT defined
 * Configuration 3. System SRAM and FSMC SRAM
 *                  CONFIG_MM_REGIONS == 2
 *                  CONFIG_HC32_EXTERNAL_RAM defined
 *                  CONFIG_HC32_CCMEXCLUDE defined
 * Configuration 4. System SRAM, CCM SRAM, and FSMC SRAM
 *                  CONFIG_MM_REGIONS == 3
 *                  CONFIG_HC32_EXTERNAL_RAM defined
 *                  CONFIG_HC32_CCMEXCLUDE NOT defined
 *
 * Let's make sure that all definitions are consistent before doing
 * anything else
 */

#  if defined(CONFIG_HC32_EXTERNAL_RAM)
#    if (CONFIG_MM_REGIONS == 2)
/* OK: This is Configuration 3: SRAM and FSMC */

#    elif (CONFIG_MM_REGIONS == 3)
/* OK: This is Configuration 3: SRAM, CCM, and FSMC */

#    else
#      error "Expected CONFIG_MM_REGIONS to be either 2 (SRAM + FSMC) or 3 (SRAM + CCM + FSMC)!"

#    endif
#  else
#    if (CONFIG_MM_REGIONS == 1)
/* OK: Configuration 1: SRAM only. */

#    elif (CONFIG_MM_REGIONS == 2)
/* OK: Configuration 2: SRAM and CCM SRAM. */

#    else
#      error "Expected CONFIG_MM_REGIONS to be either 1 (SRAM) or 2 (SRAM + CCM)!"

#    endif
#  endif


/* If FSMC SRAM is going to be used as heap, then verify that the starting
 * address and size of the external SRAM region has been provided in the
 * configuration (as CONFIG_HEAP2_BASE and CONFIG_HEAP2_SIZE).
 */

#ifdef CONFIG_HC32_EXTERNAL_RAM
#  if !defined(CONFIG_HEAP2_BASE) || !defined(CONFIG_HEAP2_SIZE)
#    error "CONFIG_HEAP2_BASE and CONFIG_HEAP2_SIZE must be provided"
#    undef CONFIG_HC32_EXTERNAL_RAM
#  endif
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_heap_color
 *
 * Description:
 *   Set heap memory to a known, non-zero state to checking heap usage.
 *
 ****************************************************************************/

#ifdef CONFIG_HEAP_COLORATION
static inline void up_heap_color(FAR void *start, size_t size)
{
  memset(start, HEAP_COLOR, size);
}
#else
#  define up_heap_color(start,size)
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_allocate_heap
 *
 * Description:
 *   This function will be called to dynamically set aside the heap region.
 *
 *   For the kernel build (CONFIG_BUILD_PROTECTED=y) with both kernel- and
 *   user-space heaps (CONFIG_MM_KERNEL_HEAP=y), this function provides the
 *   size of the unprotected, user-space heap.
 *
 *   If a protected kernel-space heap is provided, the kernel heap must be
 *   allocated (and protected) by an analogous up_allocate_kheap().
 *
 *   The following memory map is assumed for the flat build:
 *
 *     .data region.  Size determined at link time.
 *     .bss  region  Size determined at link time.
 *     IDLE thread stack.  Size determined by CONFIG_IDLETHREAD_STACKSIZE.
 *     Heap.  Extends to the end of SRAM.
 *
 *   The following memory map is assumed for the kernel build:
 *
 *     Kernel .data region       Size determined at link time
 *     Kernel .bss  region       Size determined at link time
 *     Kernel IDLE thread stack  Size determined by
 *                                CONFIG_IDLETHREAD_STACKSIZE
 *     Padding for alignment
 *     User .data region         Size determined at link time
 *     User .bss region          Size determined at link time
 *     Kernel heap               Size determined by
 *                                CONFIG_MM_KERNEL_HEAPSIZE
 *     User heap                 Extends to the end of SRAM
 *
 ****************************************************************************/

void up_allocate_heap(FAR void **heap_start, size_t *heap_size)
{
#if defined(CONFIG_BUILD_PROTECTED) && defined(CONFIG_MM_KERNEL_HEAP)
  /* Get the unaligned size and position of the user-space heap.
   * This heap begins after the user-space .bss section at an offset
   * of CONFIG_MM_KERNEL_HEAPSIZE (subject to alignment).
   */

  uintptr_t ubase = (uintptr_t)USERSPACE->us_bssend +
    CONFIG_MM_KERNEL_HEAPSIZE;
  size_t    usize = SRAM1_END - ubase;
  int       log2;

  DEBUGASSERT(ubase < (uintptr_t)SRAM1_END);

  /* Adjust that size to account for MPU alignment requirements.
   * NOTE that there is an implicit assumption that the SRAM1_END
   * is aligned to the MPU requirement.
   */

  log2  = (int)mpu_log2regionfloor(usize);

  usize = (1 << log2);
  ubase = SRAM1_END - usize;

  /* Return the user-space heap settings */

  board_autoled_on(LED_HEAPALLOCATE);
  *heap_start = (FAR void *)ubase;
  *heap_size  = usize;

  /* Colorize the heap for debug */

  up_heap_color((FAR void *)ubase, usize);

  /* Allow user-mode access to the user heap memory */

  hc32_mpu_uheap((uintptr_t)ubase, usize);
#else

  /* Return the heap settings */

  board_autoled_on(LED_HEAPALLOCATE);
  *heap_start = (FAR void *)g_idle_topstack;
  *heap_size  = SRAM1_END - g_idle_topstack;

  /* Colorize the heap for debug */

  up_heap_color(*heap_start, *heap_size);
#endif
}

/****************************************************************************
 * Name: up_allocate_kheap
 *
 * Description:
 *   For the kernel build (CONFIG_BUILD_PROTECTED=y) with both kernel- and
 *   user-space heaps (CONFIG_MM_KERNEL_HEAP=y), this function allocates
 *   (and protects) the kernel-space heap.
 *
 ****************************************************************************/

#if defined(CONFIG_BUILD_PROTECTED) && defined(CONFIG_MM_KERNEL_HEAP)
void up_allocate_kheap(FAR void **heap_start, size_t *heap_size)
{
  /* Get the unaligned size and position of the user-space heap.
   * This heap begins after the user-space .bss section at an offset
   * of CONFIG_MM_KERNEL_HEAPSIZE (subject to alignment).
   */

  uintptr_t ubase = (uintptr_t)USERSPACE->us_bssend +
    CONFIG_MM_KERNEL_HEAPSIZE;
  size_t    usize = SRAM1_END - ubase;
  int       log2;

  DEBUGASSERT(ubase < (uintptr_t)SRAM1_END);

  /* Adjust that size to account for MPU alignment requirements.
   * NOTE that there is an implicit assumption that the SRAM1_END
   * is aligned to the MPU requirement.
   */

  log2  = (int)mpu_log2regionfloor(usize);

  usize = (1 << log2);
  ubase = SRAM1_END - usize;

  /* Return the kernel heap settings (i.e., the part of the heap region
   * that was not dedicated to the user heap).
   */

  *heap_start = (FAR void *)USERSPACE->us_bssend;
  *heap_size  = ubase - (uintptr_t)USERSPACE->us_bssend;
}
#endif

/****************************************************************************
 * Name: arm_addregion
 *
 * Description:
 *   Memory may be added in non-contiguous chunks.  Additional chunks are
 *   added by calling this function.
 *
 ****************************************************************************/

#if CONFIG_MM_REGIONS > 1
void arm_addregion(void)
{
#ifndef CONFIG_HC32_CCMEXCLUDE
#if defined(CONFIG_BUILD_PROTECTED) && defined(CONFIG_MM_KERNEL_HEAP)

  /* Allow user-mode access to the HC32F40xxx CCM SRAM heap */

  hc32_mpu_uheap((uintptr_t)SRAM2_START, SRAM2_END - SRAM2_START);

#endif

  /* Colorize the heap for debug */

  up_heap_color((FAR void *)SRAM2_START, SRAM2_END - SRAM2_START);

  /* Add the HC32F40xxx CCM SRAM user heap region. */

  kumm_addregion((FAR void *)SRAM2_START, SRAM2_END - SRAM2_START);
#endif

#ifdef CONFIG_HC32_EXTERNAL_RAM
#if defined(CONFIG_BUILD_PROTECTED) && defined(CONFIG_MM_KERNEL_HEAP)

  /* Allow user-mode access to the FSMC SRAM user heap memory */

  hc32_mpu_uheap((uintptr_t)CONFIG_HEAP2_BASE, CONFIG_HEAP2_SIZE);

#endif

  /* Colorize the heap for debug */

  up_heap_color((FAR void *)CONFIG_HEAP2_BASE, CONFIG_HEAP2_SIZE);

  /* Add the external FSMC SRAM user heap region. */

  kumm_addregion((FAR void *)CONFIG_HEAP2_BASE, CONFIG_HEAP2_SIZE);
#endif
}
#endif
