/**************************************************************************
Copyright (c) 2007 Stretch, Inc. All rights reserved.  Stretch products are
protected under numerous U.S. and foreign patents, maskwork rights,
copyrights and other intellectual property laws.  

This source code and the related tools, software code and documentation, and
your use thereof, are subject to and governed by the terms and conditions of
the applicable Stretch IDE or SDK and RDK License Agreement (either as agreed
by you or found at www.stretchinc.com).  By using these items, you indicate
your acceptance of such terms and conditions between you and Stretch, Inc.
In the event that you do not agree with such terms and conditions, you may
not use any of these items and must immediately destroy any copies you have
made.
***************************************************************************/


#ifndef _SX_PORT_H
#define _SX_PORT_H

#if (defined __STRETCH_S5000__)
#include <s5000/s5000.h>
#endif

#if (defined __STRETCH_S6000__)
#include <s6000/s6000.h>
#endif

#if (defined __STRETCH_S7000__)
#include <s7000/s7000.h>
#endif

#if (defined __STRETCH__)
#include <sx-types.h>
#include <sx-event.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


#ifndef NULL
#define NULL			    ((void *)0)
#endif


// Error codes 
#define SX_QUEUE_ERR_BASE           0


#if (defined __STRETCH__) // Board
  #define SX_QUEUE_ALIGN(n)           __attribute__ ((aligned(n)))
#else   // WINDOWS
  #define SX_QUEUE_ALIGN(n)           __declspec(align(n))
#endif


/*
    For S6 onwards, the shared memory used by SCT is allocated on the board
    during sct_init(). The host driver uses PCI BAR 1 to access the shared
    memory.

    A register visible to the host through BAR0 is used to specify the
    address of the shared memory block. The register offset specified 
    below is the offset from the base of BAR0. The value in the register
    is the offset of the shared memory block from the base of BAR1.

    Pseudo-code for host to find the start of shared memory:

    bar1_offset     = *(sx_uint32 *)(bar0_base + SCT_SHARED_MEM_INFO_REG);
    shared_mem_base = bar1_base + bar1_offset;
 */
/* For S6, we use register pci->msi_addr0 */
#define S6_SCT_SHARED_MEM_REG_OFFSET    (0x000b0060)

/* For S7, we use register gblreg->memory[0] */
#define S7_SCT_SHARED_MEM_REG_OFFSET    (0x00070300)

/* Additional sync barrier for SDK/firmware prior to SCT */
#define S7_SCT_BOARD_SYNC_REG_OFFSET    (0x00070304)
#define S7_SCT_BOARD_SYNC_REG_VAL       (0xb0a4d001)


#ifdef IPC_S55DB
  // Shared memory for system
  #define SCT_SHARED_MEM_BASE        SXP1_SRAM_BASE
#else
#ifdef IPC_JAMAICA
  // Jamaica base aperture addresses mapping to S55DB BAR0/BAR1
  extern sx_uint32 jamaica_aperture_base_s55db_bar0;
  extern sx_uint32 jamaica_aperture_base_s55db_bar1;
  extern sx_uint32 jamaica_bar1_base;

  #define JAMAICA_BUFFER_BASE         (SXP1_SYSTEM_RAM_BASE_UNCACHED + 0x04000000)
  // If JAMAICA_BUFFER_BASE is in cached memory, uncomment the define below
  // so Jamaica tests can write back buffers before doing DMA on buffer
  //#define JAMAICA_CACHED_BUFFERS
#endif
#endif


// --- Data types ---

#if (defined __STRETCH__) && (!defined __STRETCH_NATIVE__)
  // Data types defined in sx-types.h
#else
#ifndef __H_SXTYPES
#define __H_SXTYPES
  typedef unsigned char             sx_uint8;    // 8  bit unsigned integer
  typedef unsigned short            sx_uint16;   // 16 bit unsigned integer
  typedef unsigned int              sx_uint32;   // 32 bit unsigned integer
  typedef unsigned long             sx_uint64;   // 64 bit unsigned integer
  typedef signed char               sx_int8;     // 8  bit signed integer
  typedef signed short              sx_int16;    // 16 bit signed integer
  typedef signed int                sx_int32;    // 32 bit signed integer
  typedef signed long               sx_int64;    // 64 bit signed integer
  typedef int                       sx_bool;
#endif
#endif


// --- Macros ---

#ifdef IPC_S55DB
  // PCI_TO_CACHED     : Map from offset in S55DB BAR1 space (DPS RAM) to S55DB cached address
  // PHYS_TO_CACHED    : Same thing
  #define PCI_TO_CACHED(offset)     ((sx_uint32)(offset) + SCT_SHARED_MEM_BASE)
  #define PHYS_TO_CACHED(offset)    PCI_TO_CACHED(offset)

  // CACHED_TO_PCI  : Map from S55DB cached space to PCI offset from BAR1 (DP SRAM)
  // CACHED_TO_PHYS : Same thing
  #define CACHED_TO_PCI(addr)       ((sx_uint32)(addr) - SCT_SHARED_MEM_BASE)
  #define CACHED_TO_PHYS(addr)      CACHED_TO_PCI(addr)

  // PCI_WIN_TO_CACHED : Map from absolute PCI address to S55DB cached address
  #define PCI_WIN_TO_CACHED(addr)   ((sx_uint32)(addr) + SXP1_PCI_APERATURE)
#else
#ifdef IPC_JAMAICA
  // PCI_TO_CACHED  : Map from offset in S55DB BAR1 space to Jamaica cached address
  // PHYS_TO_CACHED : Same thing 
  #define PCI_TO_CACHED(offset)     (jamaica_aperture_base_s55db_bar1 + (sx_uint32)(offset))  
  #define PHYS_TO_CACHED(offset)    PCI_TO_CACHED(offset)

  // CACHED_TO_PHYS    : Map from Jamaica cached address to offset into S55DB BAR1 space
  #define CACHED_TO_PHYS(addr)      ((sx_uint32)(addr) - jamaica_aperture_base_s55db_bar1)

  // CACHED_TO_PCI_WIN : Map from Jamaica DP SRAM address space to absolute PCI address
  #define CACHED_TO_PCI_WIN(addr)   ((addr) - JAMAICA_BUFFER_BASE + jamaica_bar1_base)
#else   // WINDOWS & S6
#ifdef _WIN64
  // Translate between physical address (offset into PCI BAR) and program address
  #define PHYS_TO_CACHED(offset, baseaddr)  ((ULONGLONG)(offset) + (ULONGLONG)(baseaddr))
  #define CACHED_TO_PHYS(addr, baseaddr)    ((ULONGLONG)(addr) - (ULONGLONG)(baseaddr))
#else
  // Translate between physical address (offset into PCI BAR) and program address
  #define PHYS_TO_CACHED(offset, baseaddr)  ((sx_uint32)(offset) + (sx_uint32)(baseaddr))
  #define CACHED_TO_PHYS(addr, baseaddr)    ((sx_uint32)(addr) - (sx_uint32)(baseaddr))
#endif
#endif
#endif


// Macros for sx-queue API
#if (defined __STRETCH__) // Board
// "LINE" indicates one object in sx-queue, and because sx-queue is structured
// with each variable aligned and padded to 32 bytes, "LINE" refers to 32 bytes.
// For SCT, shared memory is either
// - on the host system and accessed using uncached PCI aperture space, or
// - in board DDR memory and accessed using uncached addresses,
// so these cache macros are not actually required to do anything for board SCT.
  #define DCACHE_WBI_LINE(addr)
  #define DCACHE_INV_LINE(addr)
  #define DCACHE_WBI_REGION( addr, size )
  #define DCACHE_INV_REGION( addr, size )
  #define SYNC
  //#define DCACHE_WBI_LINE(addr)             { sx_dcache_region_writeback_inv((addr), 32); }
  //#define DCACHE_INV_LINE(addr)             { sx_dcache_region_invalidate((addr), 32); }
  //#define DCACHE_WBI_REGION( addr, size )   { sx_dcache_region_writeback_inv((addr), (size)); }
  //#define DCACHE_INV_REGION( addr, size )   { sx_dcache_region_invalidate((addr), (size));}
  //#define SYNC                              { sx_dcache_sync(); }
#else   // WINDOWS
  // CACHE Macros - Used to make sure data has been written (writeback) and will
  // be read back (invalidate) from PCI space. 'addr' or 'base' is the internal
  // Windows address that is used to access a physical PCI location (see above macros).
  #define DCACHE_INV_LINE( addr )
  #define DCACHE_WBI_LINE( addr )
  #define DCACHE_INV_REGION( base, size )
  #define DCACHE_WBI_REGION( base, size )
  #define SYNC

  // Internal Windows address that maps to PCI space, where S55DB BAR0 has been mapped
  #define BAR0_BASE               (0)
#endif


// Adjustment of Physical address space (no adjustment done for Windows)
#define ADJ_PHYS_R( addr )      (addr)
#define ADJ_PHYS_W( addr )      (addr)


// Hardware semaphore

#ifdef IPC_S55DB
  #define S5000_IO(addr)          (addr)
  #define S5000_SEM0              (SXP1_GLOBAL_REG_BASE + 0x100)
#else
#ifdef IPC_JAMAICA
  #define S5000_IO(offset)        (jamaica_aperture_base_s55db_bar0 + (sx_uint32)(offset))
  #define S5000_SEM0              0x01100100    // Offset from BAR0 Base
#else   // WINDOWS
  #define S5000_IO(offset, baseAddress)  (baseAddress + (offset))  
  #define S5000_SEM0_OFFSET               0x01100100
#endif
#endif

#if (defined IPC_S55DB || defined IPC_JAMAICA)
  #define SX_QUEUE_SEM0               ((volatile sx_uint32 *)S5000_IO(S5000_SEM0))
  #define SX_QUEUE_MUTEX_GET()        do {intflags = sx_set_intlevel(1); while( !*SX_QUEUE_SEM0 );} while (0)
  #define SX_QUEUE_MUTEX_FREE()       do {*SX_QUEUE_SEM0 = 0x1; sx_restore_intlevel(intflags);} while (0)
#else
#if (defined __STRETCH__)
  //#define S6_SEM0                     (S6_SCB_GBLREG2_BASE)
  //#define SX_QUEUE_SEM0               ((volatile sx_uint32 *)S6_SEM0)
  //#define SX_QUEUE_MUTEX_GET()        do {intflags = sx_set_intlevel(1); while( !*SX_QUEUE_SEM0 );} while (0)
  //#define SX_QUEUE_MUTEX_FREE()       do {*SX_QUEUE_SEM0 = 0x1; sx_restore_intlevel(intflags);} while (0)
  #define SX_QUEUE_MUTEX_GET(addr)        do {intflags = sx_set_intlevel(1); sx_queue_crit_section_enter(addr, 0);} while (0)
  #define SX_QUEUE_MUTEX_FREE(addr)       do {sx_queue_crit_section_exit(addr, 0); sx_restore_intlevel(intflags);} while (0)
#else   // WINDOWS
  //#define SX_QUEUE_SEM0(baseAddress)          ((volatile unsigned int *)S5000_IO(S5000_SEM0_OFFSET,baseAddress))
  //#define SX_QUEUE_MUTEX_GET(baseAddress)     do {while(!*SX_QUEUE_SEM0(baseAddress));} while (0)
  //#define SX_QUEUE_MUTEX_FREE(baseAddress)    do {*SX_QUEUE_SEM0(baseAddress) = 0x1;} while (0)
  //#define S6_IO(offset, baseaddr)       ((baseaddr) + (offset))  
  //#define S6_SEM0_OFFSET                (0x000f0000)    // offset into BAR0
  //#define SX_QUEUE_SEM0(baseaddr)       ((volatile unsigned int *)S6_IO(S6_SEM0_OFFSET, (baseaddr)))
  //#define SX_QUEUE_MUTEX_GET(baseaddr)  do {while(!*SX_QUEUE_SEM0(baseaddr));} while (0)
  //#define SX_QUEUE_MUTEX_FREE(baseaddr) do {*SX_QUEUE_SEM0(baseaddr) = 0x1;} while (0)
  #define SX_QUEUE_MUTEX_GET(addr)        do {sx_queue_crit_section_enter(addr, 1);} while (0)
  #define SX_QUEUE_MUTEX_FREE(addr)       do {sx_queue_crit_section_exit(addr, 1);} while (0)
#endif
#endif


// Defines for Windows to interrupt S55DB and reset S55DB
#define S5000_INTERRUPT_OFFSET     0x01100030
#define S5530_GPIO_OUT_OFFSET      0x00130400
#define S5530_GPIO_PIN_OFFSET	   0x00130040
#define S5530_PCI_BAR_CNTL_OFFSET  0x00000100
#define S5530_PCI_BAR1_MAP_OFFSET  0x00000108
#define S5530_PCI_GIB_MAPPING      0xFF000000	// S5530 internal PCI mapping to GIB device to drive GIB[35]
#define S5530_GIB_GIB35_OFFSET     0x00000000	// Perform a read or write to activate GIB[35]
#define S5530_PCI_BAR_CNTL_VAL     0x0000000f	// Enable bar1 and bar2 both for dword-only

/* Interrupt offsets for S6 */
#define S6_INTERRUPT_OFFSET           (0x00002754) // Host writes 1 to interrupt S6
#define S6_HOST_INTERRUPT_OFFSET      (0x00002758) // Host writes 0 to clear S6 PCIe interrupt

/* Interrupt offset for S7 */
#define S7_INTERRUPT_OFFSET           (0x00002f0c) // Host writes 1 to interrupt S7 SCP
#define S7_HOST_INTERRUPT_OFFSET      (0x00002f10) // Host writes 0 to clear S7 host interrupt

#ifdef __cplusplus
}
#endif

#endif // #ifndef _SX_PORT_H


