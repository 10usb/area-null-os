# Kernel
The first piece of code that can take full control over the system without having to resort to loading more parts of itself from disk. This is due to the fact that the boot sector is limited to 512 bytes. Which is just enough to load some more sectors into memory and even initialize 32 bit (we don't, but be could). These more sectors have to do is unlock all memory, go into 32 bit (or 64 bit) mode. And then it could load more logic from disk into memory. This gives a bad foundation to start the core logic of the OS. So it's better to keep this part minimal and let is load this piece of code.

## Wondering thoughts...
In most OS's this part is called "kernel", but I don't like that. A kernel is a dormant seed of a fruit. And this piece of code is anything but dormant.

Now it could be called "core" but thats really generic, and it's a buzz word that gets sticked to anything to make it more appealing for marketing. So thats a big NO.

Copilot directed me to "Nucleus" which seems to be used in similare concepts, but where an acitve state is a thing. But searching a bit further I learned it latin for seed of a fruit...

My first thought of nucleus is that it has something to do which biology. The kernel is the brain of the OS. So the word "Nexus" came into my mind. In a way the kernel lets the software and hardware talk to each other. This is especially true, if it relies heavily on drivers to have the knowlege on how to actually talk to hardware. On the otherhand nexus could just be a part of the kernel.

So the name of this part it up for debate.