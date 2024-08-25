use16
org 0x7C00 ; Startposition in memory

; Jump over image descriptor
jmp start
nop

; Make space for FAT family header
header: times 91 - ( $ - $$ ) db 0

; Start of boot sector
start:
   ; Make sure the data segments are cleared
   xor ax, ax
   mov ds, ax
   mov es, ax

   ; Now we can store the drive we are on
   ; FAT12/16 header has an field for the drive number. The value in it is
   ; not reliable but we can use its location to store the real value.
   mov [header +  33], dl

   ; Move the stack just before we life
   mov ax, 0x7c0
   mov ss, ax

   mov ah, 0x0e
   mov al, 'A'
   int 0x10
   call newline

   ; DriveNumber
   mov al, [header + 33]
   xor ah, ah
   mov cx, 10
   call printx
   call newline

   ; ReservedSectors
   mov al, [header + 11]
   xor ah, ah
   mov cx, 10
   call printx
   call newline

   ; SectorsPerTrack
   mov ax, [header + 21]
   mov cx, 10
   call printx
   call newline

   ; NumberOfHeads
   mov ax, [header + 23]
   mov cx, 10
   call printx
   call newline

   ; Address to load
   mov ax, [address]
   mov cx, 16
   call printx
   call newline

   ; Destination to load the second stage (es:bx)
   mov bx, [address]

   mov cx, 5
   .retryRead:
      push cx

      mov ah, 0x0e
      mov al, cl
      add al, '0'
      mov ah, 0x0e
      int 0x10

      ; Parameters
      ; AH	02h
      ; AL	Sectors To Read Count
      ; CH	Cylinder
      ; CL	Sector
      ; DH	Head
      ; DL	Drive
      ; ES:BX	Buffer Address Pointer
      mov ax, [header + 25]   ; FAT: HiddenSectors
      inc ax                  ; Skip current sector

      xor dx, dx
      div word [header +  21] ; FAT: SectorsPerTrack
      inc dx                  ; SectorOffset 1-64
      mov cx, dx              ; Set CL with the SectorOffset

      xor dx, dx
      div word [header +  23] ; FAT: NumberOfHeads
      mov ch, al              ; Set CH with (the cylinder index)

      shl ah, 6               ; Shift so only 2 bits remain at 6-7
      or  cl, ah              ; Merge it to the end of CL
      mov dh, dl              ; Set track offset

      mov dl, [header +  33]  ; FAT: DriveNumber

      mov al, [header + 11]   ; ReservedSectors
      dec al                  ; Number of sectors to Read

      mov ah, 0x02            ; I/O disk Read function
      int 0x13                ; Execute! and read

      jnc .success            ; If CF set indicating a failure
      pop cx
   loop .retryRead

   .error:
   mov ah, 0x0E
   mov al, 'D'
   int 0x10
   hlt
   jmp $
   .success:

   mov al, 1
   mov cl, 2
   mov ch, 0
   mov dh, 0
   mov bx, 0x8000
   mov ah, 0x02
   int 0x13
   jc .error

   mov al, 1
   mov cl, 3
   mov ch, 0
   mov dh, 0
   mov bx, 0x8200
   mov ah, 0x02
   int 0x13
   jc .error


   mov ah, 0x0E
   mov al, '+'
   int 0x10


   
   mov ah, 0x0E
   mov al, 10
   int 0x10

   mov ah, 0x0E
   mov al, 13
   int 0x10

   mov ah, 0x01
   mov cx, 0x2607
   int 0x10

   ; Jump to second stage
   push word 0
   push word [address]
   retf

   newline:      
      mov ah, 0x0E
      mov al, 10
      int 0x10
      mov al, 13
      int 0x10
      ret

   printx:
      mov si, buffer + 16

      .next:
         xor dx, dx
         div cx
         cmp dl, 10
         jb @f
            add dl, 'A' - '9' - 1
         @@:
         add dl, '0'
         mov [si], dl
         dec si
         cmp ax, 0
      jnz .next

      @@:
         inc si
         mov al, [si]
         mov ah, 0x0E
         int 0x10
         cmp si, buffer + 16
      jl @b
      ret
eoc:

; Filling up to 510 bytes with zero's
times 512 - (buffer - footer) - ( $ - $$ ) nop


footer:
   dw eoc - start 
   ; Where do we want our second stage to be loaded
   ; address dw 0x7E00
   address dw 0x8000
   ; Boot loader signature
   dw 0xAA55
buffer: