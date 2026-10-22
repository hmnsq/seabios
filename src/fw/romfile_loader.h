#अगर_अघोषित __ROMFILE_LOADER_H
#घोषणा __ROMFILE_LOADER_H

#समावेश "types.h" // u8
#समावेश "util.h" // romfile_s

#घोषणा ROMFILE_LOADER_FILESZ 56

/* ROM file linker/loader interface. Linker uses little endian format */
काष्ठा romfile_loader_entry_s {
    u32 command;
    जोड़ {
        /*
         * COMMAND_ALLOCATE - allocate a table from @alloc.file
         * subject to @alloc.align alignment (must be power of 2)
         * and @alloc.zone (can be HIGH or FSEG) requirements.
         *
         * Must appear exactly once for each file, and before
         * this file is referenced by any other command.
         */
        काष्ठा {
            अक्षर file[ROMFILE_LOADER_FILESZ];
            u32 align;
            u8 zone;
        } alloc;

        /*
         * COMMAND_ADD_POINTER - patch the table (originating from
         * @dest_file) at @pointer.offset, by adding a pointer to the table
         * originating from @src_file. 1,2,4 or 8 byte unsigned
         * addition is used depending on @pointer.size.
         */
        काष्ठा {
            अक्षर dest_file[ROMFILE_LOADER_FILESZ];
            अक्षर src_file[ROMFILE_LOADER_FILESZ];
            u32 offset;
            u8 आकार;
        } pointer;

        /*
         * COMMAND_ADD_CHECKSUM - calculate checksum of the range specified by
         * @cksum.start and @cksum.length fields,
         * and then add the value at @cksum_offset.
         * Checksum simply sums -X for each byte X in the range
         * using 8-bit math.
         */
        काष्ठा {
            अक्षर file[ROMFILE_LOADER_FILESZ];
            u32 offset;
            u32 start;
            u32 लंबाई;
        } cksum;

        /*
         * COMMAND_WRITE_POINTER - Write back to a host file via DMA,
         * @wr_pointer.dest_file at offset @wr_pointer.dst_offset, a pointer
         * to the table originating from @wr_pointer.src_file at offset
         * @wr_pointer.src_offset.
         * 1,2,4 or 8 byte unsigned addition is used depending on
         * @wr_pointer.size.
         */
        काष्ठा {
            अक्षर dest_file[ROMFILE_LOADER_FILESZ];
            अक्षर src_file[ROMFILE_LOADER_FILESZ];
            u32 dst_offset;
            u32 src_offset;
            u8 आकार;
        } wr_pointer;

        /* padding */
        अक्षर pad[124];
    };
};

क्रमागत {
    ROMFILE_LOADER_COMMAND_ALLOCATE      = 0x1,
    ROMFILE_LOADER_COMMAND_ADD_POINTER   = 0x2,
    ROMFILE_LOADER_COMMAND_ADD_CHECKSUM  = 0x3,
    ROMFILE_LOADER_COMMAND_WRITE_POINTER = 0x4,
};

क्रमागत {
    ROMFILE_LOADER_ALLOC_ZONE_ऊपरी = 0x1,
    ROMFILE_LOADER_ALLOC_ZONE_FSEG = 0x2,
};

पूर्णांक romfile_loader_execute(स्थिर अक्षर *name);

व्योम romfile_fw_cfg_resume(व्योम);

#पूर्ण_अगर
