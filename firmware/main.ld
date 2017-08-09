MEMORY
{
  FLASH (rx) : ORIGIN = 0x8000000, LENGTH = 512K
  SRAM (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
  AHBB0 (xrw) : ORIGIN = 0x10000000, LENGTH = 32K
  AHBB1 (xrw) : ORIGIN = 0x10008000, LENGTH = 32K
}

SECTIONS
{
    __STACKSIZE__ = 16384; 
    __HEAPSIZE__  = 32768; 

    .text :
    {
        _text = .;
        KEEP(*(.isr_vector))
        *(.text*)
        *(.rodata*)
        /* _etext = .; */
    } > FLASH

    ARM.extab : 
    { 
        *(.ARM.extab* .gnu.linkonce.armextab.*) 
    } >  FLASH 

    __exidx_start = .; 
    .ARM.exidx : 
    { 
        *(.ARM.exidx* .gnu.linkonce.armexidx.*) 
    } >  FLASH
    __exidx_end = .; 

    _etext = .; 
    _sidata = _etext;

    .data : 
    {
        _data = .;
        *(.data*)
        *(COMMON)
        _edata = .;
    } > SRAM AT > FLASH

    .bss (NOLOAD) :
    {
        _bss = .;
        *(.bss*)
        _ebss = .;
    } > SRAM

    .heap (NOLOAD) : 
    {
      _heap = .;
      _end = .;
      end = .;
      *(.heap*)
      . = ALIGN(MAX(_heap + __HEAPSIZE__ , .), 4);
    } > AHBB0
    _eheap = _heap + SIZEOF(.heap);

    .stack (NOLOAD) : 
    {
      _stack = .;
      *(.stack*)
      . = ALIGN(MAX(_stack + __STACKSIZE__ , .), 4);
    } > AHBB1
    _estack = _stack + SIZEOF(.stack);

    /* remove the debugging information from the standard libraries */
    DISCARD :
    {
     libc.a ( * )
     libm.a ( * )
     libgcc.a ( * )
     }
  
  
}
