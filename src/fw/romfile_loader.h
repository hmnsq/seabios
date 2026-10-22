#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __ROMFILE_LOADER_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __ROMFILE_LOADER_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u8
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // romfile_s

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ROMFILE_LOADER_FILESZ 56

/* ROM file linker/loader interface. Linker uses little endian format */
ਬਣਤਰ romfile_loader_ਇੰਦਰਾਜ_s {
    u32 command;
    ਸੰਘ {
        /*
         * COMMAND_ALLOCATE - allocate a table from @alloc.file
         * subject to @alloc.align alignment (must be power of 2)
         * and @alloc.zone (can be HIGH or FSEG) requirements.
         *
         * Must appear exactly once for each file, and before
         * this file is referenced by any other command.
         */
        ਬਣਤਰ {
            ਅੱਖਰ file[ROMFILE_LOADER_FILESZ];
            u32 align;
            u8 zone;
        } alloc;

        /*
         * COMMAND_ADD_POINTER - patch the table (originating from
         * @dest_file) at @pointer.offset, by adding a pointer to the table
         * originating from @src_file. 1,2,4 or 8 byte unsigned
         * addition is used depending on @pointer.size.
         */
        ਬਣਤਰ {
            ਅੱਖਰ dest_file[ROMFILE_LOADER_FILESZ];
            ਅੱਖਰ src_file[ROMFILE_LOADER_FILESZ];
            u32 ਵਿੱਥ_2;
            u8 ਮਾਪ;
        } ਸੂਚਕ;

        /*
         * COMMAND_ADD_CHECKSUM - calculate checksum of the range specified by
         * @cksum.start and @cksum.length fields,
         * and then add the value at @cksum_offset.
         * Checksum simply sums -X for each byte X in the range
         * using 8-bit math.
         */
        ਬਣਤਰ {
            ਅੱਖਰ file[ROMFILE_LOADER_FILESZ];
            u32 ਵਿੱਥ_2;
            u32 ਸ਼ੁਰੂ;
            u32 ਲੰਮਾਈ;
        } cksum;

        /*
         * COMMAND_WRITE_POINTER - Write back to a host file via DMA,
         * @wr_pointer.dest_file at offset @wr_pointer.dst_offset, a pointer
         * to the table originating from @wr_pointer.src_file at offset
         * @wr_pointer.src_offset.
         * 1,2,4 or 8 byte unsigned addition is used depending on
         * @wr_pointer.size.
         */
        ਬਣਤਰ {
            ਅੱਖਰ dest_file[ROMFILE_LOADER_FILESZ];
            ਅੱਖਰ src_file[ROMFILE_LOADER_FILESZ];
            u32 dst_ਵਿੱਥ;
            u32 src_ਵਿੱਥ;
            u8 ਮਾਪ;
        } wr_ਸੂਚਕ;

        /* padding */
        ਅੱਖਰ pad[124];
    };
};

ਗਿਣਤੀ {
    ROMFILE_LOADER_COMMAND_ALLOCATE      = 0x1,
    ROMFILE_LOADER_COMMAND_ਜੋੜੋ_ਸੂਚਕ   = 0x2,
    ROMFILE_LOADER_COMMAND_ਜੋੜੋ_CHECKSUM  = 0x3,
    ROMFILE_LOADER_COMMAND_WRITE_ਸੂਚਕ = 0x4,
};

ਗਿਣਤੀ {
    ROMFILE_LOADER_ALLOC_ZONE_ਉੱਚਾ = 0x1,
    ROMFILE_LOADER_ALLOC_ZONE_FSEG = 0x2,
};

ਪੂਰਨ romfile_loader_execute(ਅਚਲ ਅੱਖਰ *name);

ਖਾਲੀ romfile_fw_cfg_resume(ਖਾਲੀ);

#ਜੇ_ਅੰਤ
