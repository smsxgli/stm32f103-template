/* SPDX-License-Identifier: MIT */
/* section linker script for arm bare metal gcc */

/* _estack is used in interrupt vector table */
/* and newlib asume stack is put after heap */
PROVIDE(_estack = ORIGIN(RAM) + LENGTH(RAM));

PROVIDE(_minimum_heap_stack_size = 8K);

ENTRY(Reset_Handler)

SECTIONS {
   /*
   * for Cortex-M devices, the beginning of the startup code is stored in
   * the .isr_vector section, which goes to FLASH. 
   */
   .isr_vector : ALIGN(4)
   {
      FILL(0xFF)
      KEEP(*(.isr_vector)) /* interrupt vectors */
   } >FLASH

   .text : ALIGN(4)
   {
      FILL(0xFF)
      /*
      * These are the old initialisation sections, intended to contain
      * naked code, with the prologue/epilogue added by crti.o/crtn.o
      * when linking with startup files. The standalone startup code
      * currently does not run these, better use the init arrays below.
      */
      KEEP(*(.init))
      KEEP(*(.fini))
      . = ALIGN(4);

      PROVIDE_HIDDEN(__preinit_array_start = ABSOLUTE(.));
      KEEP(*(SORT(.preinit_array.*)))
      KEEP(*(.preinit_array))
      PROVIDE_HIDDEN(__preinit_array_end = ABSOLUTE(.));
      . = ALIGN(4);

      PROVIDE_HIDDEN(__init_array_start = ABSOLUTE(.));
      KEEP(*(SORT(.init_array.*)))
      KEEP(*(.init_array))
      PROVIDE_HIDDEN(__init_array_end = ABSOLUTE(.));
      . = ALIGN(4);

      PROVIDE_HIDDEN(__fini_array_start = ABSOLUTE(.));
      KEEP(*(SORT(.fini_array.*)))
      KEEP(*(.fini_array))
      PROVIDE_HIDDEN(__fini_array_end = ABSOLUTE(.));
      . = ALIGN(4);

      *(.text .text.*)
      *(.rodata .rodata.* .constdata .constdata.*) /* read-only data (constants) */
      *(vtable) /* C++ virtual tables */
      KEEP(*(.eh_frame*))
   } >FLASH

    /* ARM magic sections */
   .ARM.extab : ALIGN(4)
   {
      *(.ARM.extab* .gnu.linkonce.armextab.*)
   } >FLASH

   .ARM.exidx : ALIGN(4)
   {
      __exidx_start = ABSOLUTE(.);
      *(.ARM.exidx* .gnu.linkonce.armexidx.*)
      __exidx_end = ABSOLUTE(.);
   } >FLASH

   _etext = ALIGN(4);

   .data : ALIGN(4)
   {
      FILL(0xFF)
      *(.data_begin .data_begin.*)
      *(.data .data.*)
      *(.data_end .data_end.*)
      . = ALIGN(4);
   } >RAM AT>FLASH

   .bss (NOLOAD) : ALIGN(4)
   {
      *(.bss_begin .bss_begin.*)
      *(.bss .bss.*)
      *(.bss_end .bss_end.*)
      . = ALIGN(4);
   } >RAM

   _sidata = LOADADDR(.data);
   _sdata = ADDR(.data);
   _edata = ADDR(.data) + SIZEOF(.data);

   _sbss = ADDR(.bss);
   _ebss = ADDR(.bss) + SIZEOF(.bss);

   PROVIDE(_end = _ebss);

   ASSERT(_ebss + _minimum_heap_stack_size <= ORIGIN(RAM) + LENGTH(RAM), "Error: no ram left for heap and stack!")
}
