#ifndef __ROMFILE_LOADER_H
#define __ROMFILE_LOADER_H

#অন্তর্ভাব "types.h" // u8
#অন্তর্ভাব "util.h" // romfile_s

#define ROMFILE_LOADER_FILESZ 56

/* ROM file linker/loader interface. Linker uses little endian format */
কাঠামো romfile_loader_entry_s {
    u32 command;
    সংঘ {
        /*
         * COMMAND_ALLOCATE - allocate a table from @alloc.file
         * subject to @alloc.align alignment (must be power of 2)
         * and @alloc.zone (can be HIGH or FSEG) requirements.
         *
         * Must appear exactly once for each file, and before
         * this file is referenced by any other command.
         */
        কাঠামো {
            অক্ষর file[ROMFILE_LOADER_FILESZ];
            u32 align;
            u8 zone;
        } alloc;

        /*
         * COMMAND_ADD_POINTER - patch the table (originating from
         * @dest_file) at @pointer.offset, by adding a pointer to the table
         * originating from @src_file. 1,2,4 or 8 byte unsigned
         * addition is used depending on @pointer.size.
         */
        কাঠামো {
            অক্ষর dest_file[ROMFILE_LOADER_FILESZ];
            অক্ষর src_file[ROMFILE_LOADER_FILESZ];
            u32 offset;
            u8 আকার_2;
        } pointer;

        /*
         * COMMAND_ADD_CHECKSUM - calculate checksum of the range specified by
         * @cksum.start and @cksum.length fields,
         * and then add the value at @cksum_offset.
         * Checksum simply sums -X for each byte X in the range
         * using 8-bit math.
         */
        কাঠামো {
            অক্ষর file[ROMFILE_LOADER_FILESZ];
            u32 offset;
            u32 start;
            u32 দৈর্ঘ্য;
        } cksum;

        /*
         * COMMAND_WRITE_POINTER - Write back to a host file via DMA,
         * @wr_pointer.dest_file at offset @wr_pointer.dst_offset, a pointer
         * to the table originating from @wr_pointer.src_file at offset
         * @wr_pointer.src_offset.
         * 1,2,4 or 8 byte unsigned addition is used depending on
         * @wr_pointer.size.
         */
        কাঠামো {
            অক্ষর dest_file[ROMFILE_LOADER_FILESZ];
            অক্ষর src_file[ROMFILE_LOADER_FILESZ];
            u32 dst_offset;
            u32 src_offset;
            u8 আকার_2;
        } wr_pointer;

        /* padding */
        অক্ষর pad[124];
    };
};

গণনা {
    ROMFILE_LOADER_COMMAND_ALLOCATE      = 0x1,
    ROMFILE_LOADER_COMMAND_ADD_POINTER   = 0x2,
    ROMFILE_LOADER_COMMAND_ADD_CHECKSUM  = 0x3,
    ROMFILE_LOADER_COMMAND_WRITE_POINTER = 0x4,
};

গণনা {
    ROMFILE_LOADER_ALLOC_ZONE_উপরের = 0x1,
    ROMFILE_LOADER_ALLOC_ZONE_FSEG = 0x2,
};

পূর্ণসংখ্যা romfile_loader_execute(ধ্রুবক অক্ষর *name);

শূন্য romfile_fw_cfg_resume(শূন্য);

#endif
