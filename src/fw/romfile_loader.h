#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __ROMFILE_LOADER_H
#ವ್ಯಾಖ್ಯಾನಿಸು __ROMFILE_LOADER_H

#ಅನ್ತರ್ಭಾವ "types.h" // u8
#ಅನ್ತರ್ಭಾವ "util.h" // romfile_s

#ವ್ಯಾಖ್ಯಾನಿಸು ROMFILE_LOADER_FILESZ 56

/* ROM file linker/loader interface. Linker uses little endian format */
ರಚನೆ romfile_loader_ನಮೂದು_s {
    u32 command;
    ಒಕ್ಕೂಟ {
        /*
         * COMMAND_ALLOCATE - allocate a table from @alloc.file
         * subject to @alloc.align alignment (must be power of 2)
         * and @alloc.zone (can be HIGH or FSEG) requirements.
         *
         * Must appear exactly once for each file, and before
         * this file is referenced by any other command.
         */
        ರಚನೆ {
            ಅಕ್ಷರ file[ROMFILE_LOADER_FILESZ];
            u32 align;
            u8 zone;
        } alloc;

        /*
         * COMMAND_ADD_POINTER - patch the table (originating from
         * @dest_file) at @pointer.offset, by adding a pointer to the table
         * originating from @src_file. 1,2,4 or 8 byte unsigned
         * addition is used depending on @pointer.size.
         */
        ರಚನೆ {
            ಅಕ್ಷರ dest_file[ROMFILE_LOADER_FILESZ];
            ಅಕ್ಷರ src_file[ROMFILE_LOADER_FILESZ];
            u32 ಸ್ಥಾನಾಂತರ_2;
            u8 ಗಾತ್ರ_2;
        } ಸೂಚಕ_2;

        /*
         * COMMAND_ADD_CHECKSUM - calculate checksum of the range specified by
         * @cksum.start and @cksum.length fields,
         * and then add the value at @cksum_offset.
         * Checksum simply sums -X for each byte X in the range
         * using 8-bit math.
         */
        ರಚನೆ {
            ಅಕ್ಷರ file[ROMFILE_LOADER_FILESZ];
            u32 ಸ್ಥಾನಾಂತರ_2;
            u32 ಆರಂಭ;
            u32 ಉದ್ದಳತೆ;
        } cksum;

        /*
         * COMMAND_WRITE_POINTER - Write back to a host file via DMA,
         * @wr_pointer.dest_file at offset @wr_pointer.dst_offset, a pointer
         * to the table originating from @wr_pointer.src_file at offset
         * @wr_pointer.src_offset.
         * 1,2,4 or 8 byte unsigned addition is used depending on
         * @wr_pointer.size.
         */
        ರಚನೆ {
            ಅಕ್ಷರ dest_file[ROMFILE_LOADER_FILESZ];
            ಅಕ್ಷರ src_file[ROMFILE_LOADER_FILESZ];
            u32 dst_ಸ್ಥಾನಾಂತರ;
            u32 src_ಸ್ಥಾನಾಂತರ;
            u8 ಗಾತ್ರ_2;
        } wr_ಸೂಚಕ;

        /* padding */
        ಅಕ್ಷರ pad[124];
    };
};

ಎಣಿಕೆ {
    ROMFILE_LOADER_COMMAND_ALLOCATE      = 0x1,
    ROMFILE_LOADER_COMMAND_ಕೂಡಿಸು_ಸೂಚಕ   = 0x2,
    ROMFILE_LOADER_COMMAND_ಕೂಡಿಸು_CHECKSUM  = 0x3,
    ROMFILE_LOADER_COMMAND_WRITE_ಸೂಚಕ = 0x4,
};

ಎಣಿಕೆ {
    ROMFILE_LOADER_ALLOC_ZONE_ಮೇಲಿನ = 0x1,
    ROMFILE_LOADER_ALLOC_ZONE_FSEG = 0x2,
};

ಪೂರ್ಣಾಂಕ romfile_loader_execute(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name);

ಶೂನ್ಯ romfile_fw_cfg_resume(ಶೂನ್ಯ);

#ಆದರೆ_ಅಂತ್ಯ
