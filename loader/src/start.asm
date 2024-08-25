global start
extern main

section .text
   use16
   start:
      call a20line

      mov ah, 0x4F
      mov esi, hallo
      mov edi, 0xb8000
      print:
         mov al, [esi]
         inc esi
         or al, al
         jz endprint
         mov byte [edi], al
         inc edi
         mov byte [edi], ah
         inc edi
      jmp print
      endprint:

      cli
      lgdt [gdt_descriptor]

      mov eax, cr0
      or eax, 1
      mov cr0, eax

      jmp 08h:entry32
   use32
   entry32:
      ; Point all segment registers to the data segment
      mov ax, 0x10
      mov ds, ax
      mov es, ax
      mov fs, ax
      mov gs, ax
      mov ss, ax
      
      ; Move the stack pointer to 0.9 mb boundry
      mov esp, 0x090000

      ; Jump into salty C water
      call main

      halt:
      hlt
      jmp halt
   use16
   a20line:
      ; Disable interups so we know nothing it eating our data
      cli
      ; Disable first PS/2 port
      mov ah, 0xAD
      call i8042_writeCommand
      ; Read Controller Output Port
      mov ah, 0xD0
      call i8042_writeCommand
      ; Read data
      call i8042_readData
      mov cl, al
      ; Write Controller Output Port
      mov ah, 0xD1
      call i8042_writeCommand
      ; Write data
      or cl, 2
      call i8042_writeData
      ; Enable first PS/2 port
      mov ah, 0xAE
      call i8042_writeCommand
      ; Disable interups so we know nothing it eating our data
      sti
      ret

   i8042_writeCommand:
      in al, 0x64
      test al, 2
      jnz i8042_writeCommand
      mov al, ah
      out 0x64, al
      ret

   i8042_readData:
      in al, 0x64
      test al, 1
      jz i8042_readData
      in al, 0x60 ; Read data
      ret

   i8042_writeData:
      in al, 0x64
      test al, 2
      jnz i8042_writeData
      mov al, cl
      out 0x60, al
      ret

section .data
   gdt_descriptor:
      dw gdt_end - gdt_start - 1
      dd gdt_start
   
   align 16
   gdt_start:
      ; Null segment
      dd 0
      dd 0
      ; Code segment
      dw 0xFFFF
      dw 0
      db 0
      db 10011010b
      db 11001111b
      db 0
      ; Data segment
      dw 0xFFFF
      dw 0
      db 0
      db 10010010b
      db 11001111b
      db 0
   gdt_end:

   hallo: db "Entering the world of 32bit.                                                    ", 0