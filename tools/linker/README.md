# Linker
This is made not to replace LD but to understand how the different relocatable executable format work, so a similar process can be done in the OS.

## Current targets
Load the objects used to build the loader, bundle it together and get a binary output exact to the byte. Which should mean applying the relocation has been done correctly.