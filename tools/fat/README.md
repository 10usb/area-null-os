# FAT
This is a tool to create and edit images that contain a FAT12 / FAT16 / FAT32 file system.
 
## Why does it exist?
- Its initial purpose it to install a new boot sector without overriding the FAT headers. As all other tasks, can be done with existing tools.
- Its seconday purpose is to be a test tool for the shared code-base the AreaNull OS make use of to read and write, to and from a FAT file system. Therefor creating new files and directories, listing directories, reading from and writing to a file. are the basis tasks it is meant to impliment so it can be tested without having to run a full OS.
- Its third purpose it to have a tool that can be easily integrated into the build chain, without relying on an operating system specific commands to create, mount, add files, unmount an image. 