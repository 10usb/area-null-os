#ifndef INTERUPTS_H
#define INTERUPTS_H

#include <stdint.h>
#include <stddef.h>

#define IDT_GATETYPE_INTERRUPT  0xE
#define IDT_GATETYPE_TRAP       0xF

typedef struct {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
} regs_t;

typedef struct {
    uint32_t address;
    uint32_t segment; // Should this be a uint16_t
} farptr_t;

typedef struct {
    regs_t gpr;
    uint32_t number;
    uint32_t errorCode;
    farptr_t returnAddress;
    uint32_t eflags;
    farptr_t userStack;
} __attribute__((packed)) isr_frame_t;

typedef void (*isr_vector_t)(isr_frame_t *frame);
typedef void (*irq_vector_t)(int index, isr_frame_t *frame);

#define sti asm volatile ("sti");
#define cli asm volatile ("cli");

void isr_init();
void isr_install(int index, isr_vector_t callback);

void irq_init();
void irg_enable(int index, int enabled);
void irq_install(int index, irq_vector_t callback);

#endif