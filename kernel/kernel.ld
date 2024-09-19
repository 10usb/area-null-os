ENTRY(main)
PHDRS 
{
    text PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(6);
    readonly PT_LOAD FLAGS(4);
}
SECTIONS
{
    . = 8M + SIZEOF_HEADERS;

    .rodata :
    {
        *(.rodata*)
    } : readonly

    .text : ALIGN(4K)
    {
        *(.text)
    } : text
    
    .data : ALIGN(4K)
    {
        *(.data)
    } : data
    
    .bss : ALIGN(4K)
    {
        *(.bss)
    } : data

    /DISCARD/ :
    {
        *(.comment)
    }
    end = .;
}