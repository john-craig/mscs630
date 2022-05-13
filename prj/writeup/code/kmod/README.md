https://www.kernel.org/doc/htmldocs/kernel-api/blkdev.html
https://www.kernel.org/doc/htmldocs/kernel-api/chrdev.html

Both kue_linux and maru_linux aim to use the "module_init" and "module_exit" functions
to call their respective initialization and termination methods. These methods in turn
invoke "register_chrdev" and "unregister_chrdev" methods.

The "register_chrdev" method accepts a structure,

    const struct file_operations * fops

which is essentially a list of pointers to other methods which
handle each relevant file operation. These are the other methods defined
in the kue_linux and maru_linux files.

https://tldp.org/LDP/lkmpg/2.4/html/c577.htm

My inclination is to find an existing implementation of a kernel modules which
registers a character device with the usual file operations and then try to
modify that to fit what kue_linux and maru_linux are attempting to do.

https://olegkutkov.me/2018/03/14/simple-linux-character-device-driver/
http://derekmolloy.ie/writing-a-linux-kernel-module-part-2-a-character-device/
https://linux-kernel-labs.github.io/refs/heads/master/labs/device_drivers.html
https://intelrendz.wordpress.com/2013/08/22/character-device-driver-in-linux/

Other resources:

    - https://elixir.bootlin.com/linux/latest/source/include/linux/blk_types.h#L237
    - https://elixir.bootlin.com/linux/latest/source/include/linux/bvec.h#L32
    - https://elixir.bootlin.com/linux/latest/source/include/linux/blk_types.h#L40
    - https://elixir.bootlin.com/linux/latest/source/include/linux/blk-mq.h#L518
    - https://elixir.bootlin.com/linux/latest/source/include/linux/blk_types.h#L40
    - https://elixir.bootlin.com/linux/latest/source/include/linux/types.h#L13
    - https://www.kernel.org/doc/html/latest/block/blk-mq.html
    - https://linux-kernel-labs.github.io/refs/heads/master/labs/device_drivers.html#ioctl-1
    - https://static.lwn.net/images/pdf/LDD3/ch16.pdf
    - https://linux-kernel-labs.github.io/refs/heads/master/labs/block_device_drivers.html

Outstanding stuff to un-fuck:
    - semaphores all over the place. I think these have changed a lot
    and the code involving semaphores needs to be completed rewritten.

    - maru initialization and termination functions
    - maru_berror function
    - block read and write functions for maru : might need to convert them
    to equivalent request queue handlers or whatever

    - so I realized that the callback methods for the kue api are internal which means
    we can just pass our own 'req' structures back and forth. I've updated all the
    methods on the 'maru_linux' side of things to use these 'req' structures but 
    not for the 'kue_linux' side of thing-- fortunately I think that will be a lot
    easier since the hooks in 'kue_linux' all use buffers to begin with, and that
    was the biggest pain in the ass


    So... I'm kind of at a loss for what all of this is supposed to be doing in all honesty.

    There are three parts to this:

        - hosed
        - maru_linux
        - kue_linux
    
    hosed is a daemon, maru_linux is a device driver,
    and kue_linux is a character driver.

    hosed primarily seems to communicate using the 
    'ioctl' method.

    So the hosed main loop kicks commands off to 'process_kue_request' and 'process_control_message' respectively.

    'process_control_message' deals with
    things like terminating the daemon, attaching and unattaching aspects, etc.

    'process_kue_request' goes to 'process_maru_message', which in turn generally goes to 'maru_handle_chunk'

    'maru_handle_chunk' is in the 'block' file. This seems to be what builds and sends the request object using 'maruRemapIo'. Which itself kicks off to the 'maruRemapIo' in the 'remap' file.

    Okay, so that's a rough idea of what hosed does... kind of. Now let's take a lok at 'kue_linux'.

    There's a 'kue_read' function which 
    invokes the ka_read2_hook if it is
    available or just copies data to userspace if not.

    Specifically the data it copies is
    from the 'buf' passed as a parameter to the 'kue_read' function.

    So all in all I think I have everything in a position where it will operate in a way that corresponds correctly to how it was implemented... aside for the notable and significant exception of the maru_request method.

    Although I honestly have no idea
    how that was even used originally, so....

    Whatever. I'm tempted to just try
    compiling without semaphores or
    that method and seeing what happens.