#اگر_غیر_تعریف_شدہ __ROMFILE_LOADER_H
#تعریف_کرو __ROMFILE_LOADER_H

#شامل "types.h" // u8
#شامل "util.h" // romfile_s

#تعریف_کرو ROMFILE_LOADER_FILESZ 56

/* ROM file linker/loader interface. Linker uses little endian format */
ساخت romfile_loader_اندراج_s {
    u32 command;
    اتحاد {
        /*
         * COMMAND_ALLOCATE - allocate a table from @alloc.file
         * subject to @alloc.align alignment (must be power of 2)
         * and @alloc.zone (can be HIGH or FSEG) requirements.
         *
         * Must appear exactly once for each file, and before
         * this file is referenced by any other command.
         */
        ساخت {
            حرف file[ROMFILE_LOADER_FILESZ];
            u32 align;
            u8 zone;
        } alloc;

        /*
         * COMMAND_ADD_POINTER - patch the table (originating from
         * @dest_file) at @pointer.offset, by adding a pointer to the table
         * originating from @src_file. 1,2,4 or 8 byte unsigned
         * addition is used depending on @pointer.size.
         */
        ساخت {
            حرف dest_file[ROMFILE_LOADER_FILESZ];
            حرف src_file[ROMFILE_LOADER_FILESZ];
            u32 انحراف_2;
            u8 سائز;
        } اشارہ_گر;

        /*
         * COMMAND_ADD_CHECKSUM - calculate checksum of the range specified by
         * @cksum.start and @cksum.length fields,
         * and then add the value at @cksum_offset.
         * Checksum simply sums -X for each byte X in the range
         * using 8-bit math.
         */
        ساخت {
            حرف file[ROMFILE_LOADER_FILESZ];
            u32 انحراف_2;
            u32 آغاز;
            u32 لمبائی;
        } cksum;

        /*
         * COMMAND_WRITE_POINTER - Write back to a host file via DMA,
         * @wr_pointer.dest_file at offset @wr_pointer.dst_offset, a pointer
         * to the table originating from @wr_pointer.src_file at offset
         * @wr_pointer.src_offset.
         * 1,2,4 or 8 byte unsigned addition is used depending on
         * @wr_pointer.size.
         */
        ساخت {
            حرف dest_file[ROMFILE_LOADER_FILESZ];
            حرف src_file[ROMFILE_LOADER_FILESZ];
            u32 dst_انحراف;
            u32 src_انحراف;
            u8 سائز;
        } wr_اشارہ_گر;

        /* padding */
        حرف pad[124];
    };
};

شمار {
    ROMFILE_LOADER_COMMAND_ALLOCATE      = 0x1,
    ROMFILE_LOADER_COMMAND_جوڑو_اشارہ_گر   = 0x2,
    ROMFILE_LOADER_COMMAND_جوڑو_CHECKSUM  = 0x3,
    ROMFILE_LOADER_COMMAND_WRITE_اشارہ_گر = 0x4,
};

شمار {
    ROMFILE_LOADER_ALLOC_ZONE_بلند = 0x1,
    ROMFILE_LOADER_ALLOC_ZONE_FSEG = 0x2,
};

صحیح romfile_loader_execute(ثابت حرف *name);

خالی romfile_fw_cfg_resume(خالی);

#اگر_ختم
