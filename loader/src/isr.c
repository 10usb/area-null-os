#include "interrupts.h"
#include "tty.h"
#include "text.h"
#include "memory.h"

#define NUM_ISRS 48

void _isr0();
void _isr1();
void _isr2();
void _isr3();
void _isr4();
void _isr5();
void _isr6();
void _isr7();
void _isr8();
void _isr9();
void _isr10();
void _isr11();
void _isr12();
void _isr13();
void _isr14();
void _isr15();
void _isr16();
void _isr17();
void _isr18();
void _isr19();
void _isr20();
void _isr21();
void _isr22();
void _isr23();
void _isr24();
void _isr25();
void _isr26();
void _isr27();
void _isr28();
void _isr29();
void _isr30();
void _isr31();
void _isr32();
void _isr33();
void _isr34();
void _isr35();
void _isr36();
void _isr37();
void _isr38();
void _isr39();
void _isr40();
void _isr41();
void _isr42();
void _isr43();
void _isr44();
void _isr45();
void _isr46();
void _isr47();

static void (*stubs[NUM_ISRS])() = {
    _isr0,
    _isr1,
    _isr2,
    _isr3,
    _isr4,
    _isr5,
    _isr6,
    _isr7,
    _isr8,
    _isr9,
    _isr10,
    _isr11,
    _isr12,
    _isr13,
    _isr14,
    _isr15,
    _isr16,
    _isr17,
    _isr18,
    _isr19,
    _isr20,
    _isr21,
    _isr22,
    _isr23,
    _isr24,
    _isr25,
    _isr26,
    _isr27,
    _isr28,
    _isr29,
    _isr30,
    _isr31,
    _isr32,
    _isr33,
    _isr34,
    _isr35,
    _isr36,
    _isr37,
    _isr38,
    _isr39,
    _isr40,
    _isr41,
    _isr42,
    _isr43,
    _isr44,
    _isr45,
    _isr46,
    _isr47,
};

static const char *exceptions[32] = {
    "Divide by zero",
    "Debug",
    "NMI",
    "Breakpoint",
    "Overflow",
    "OOB",
    "Invalid opcode",
    "No coprocessor",
    "Double fault",
    "Coprocessor segment overrun",
    "Bad TSS",
    "Segment not present",
    "Stack fault",
    "General protection fault",
    "Page fault",
    "Unrecognized interrupt",
    "Coprocessor fault",
    "Alignment check",
    "Machine check",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED"
};

typedef struct {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed)) idtptr_t;

typedef struct {
    uint16_t addressLow;
    uint16_t selector;
    uint8_t reserved;
    union {
        uint8_t flags;
        struct {
            int gateType : 4;
            int reserved2 : 1;
            int dpl : 2;
            int present : 1;
        } __attribute__((packed));  
    };
    uint16_t addressHigh;
} __attribute__((packed)) idt_entry_t;


void idt_set_gate(uint8_t index, void (*base)(), uint16_t selector, uint8_t flags) {
    idt_entry_t *entry = ((idt_entry_t*)0x6400) + index;
    entry->addressLow = ((uint32_t)base) & 0xFFFF;
    entry->addressHigh = (((uint32_t)base) >> 16) & 0xFFFF;
    entry->selector = selector;
    entry->flags = flags;
}

void isr_init() {
    // Clear all isr vectors
    memset(0, 0, 1024);

    // Clear all idt descriptors
    idt_entry_t *entries = (idt_entry_t*)0x6400;
    memset(entries, 0, sizeof(idt_entry_t) * 256);

    // Load the IDT address
    idtptr_t pointer;
    pointer.limit = sizeof(idt_entry_t) * 256 - 1;
    pointer.base = (uint32_t)entries;
    __asm__ volatile ("lidt %0" : : "m"(pointer));

    for (uint32_t index = 0; index < NUM_ISRS; index++)
        idt_set_gate(index, stubs[index], 0x08, 0x8E);
}

void isr_install(int index, isr_vector_t callback) {
    ((isr_vector_t *)0)[index] = callback;
}


static char buffer[30];

void isr_handler(isr_frame_t frame) {
    if (frame.number < 32) {
        tty_setcolors(TTY_RED, TTY_WHITE);
        tty_puts(exceptions[frame.number]);
        for (;;);
    }

    isr_vector_t callback = ((isr_vector_t *)0)[frame.number];

    if (callback) {
        callback(&frame);
    } else {
        tty_puts(itos(frame.number, buffer, 30, 16));
        tty_puts(" - Vector unknown\n");
    }
}
