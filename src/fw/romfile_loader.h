#اگر_تعریف_نشده __ROMFILE_LOADER_H
#تعریف_کن __ROMFILE_LOADER_H

#ضمن "types.h" // u8
#ضمن "util.h" // romfile_s

#تعریف_کن ROMFILE_LOADER_FILESZ 56

/* ROM file linker/loader interface. Linker uses little endian format */
بنية romfile_loader_مدخل_s {
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
        بنية {
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
        بنية {
            حرف dest_file[ROMFILE_LOADER_FILESZ];
            حرف src_file[ROMFILE_LOADER_FILESZ];
            u32 جابه_جایی_2;
            u8 اندازه;
        } اشاره_گر;

        /*
         * COMMAND_ADD_CHECKSUM - calculate checksum of the range specified by
         * @cksum.start and @cksum.length fields,
         * and then add the value at @cksum_offset.
         * Checksum simply sums -X for each byte X in the range
         * using 8-bit math.
         */
        بنية {
            حرف file[ROMFILE_LOADER_FILESZ];
            u32 جابه_جایی_2;
            u32 شروع;
            u32 درازی;
        } cksum;

        /*
         * COMMAND_WRITE_POINTER - Write back to a host file via DMA,
         * @wr_pointer.dest_file at offset @wr_pointer.dst_offset, a pointer
         * to the table originating from @wr_pointer.src_file at offset
         * @wr_pointer.src_offset.
         * 1,2,4 or 8 byte unsigned addition is used depending on
         * @wr_pointer.size.
         */
        بنية {
            حرف dest_file[ROMFILE_LOADER_FILESZ];
            حرف src_file[ROMFILE_LOADER_FILESZ];
            u32 dst_جابه_جایی;
            u32 src_جابه_جایی;
            u8 اندازه;
        } wr_اشاره_گر;

        /* padding */
        حرف pad[124];
    };
};

تعداد {
    ROMFILE_LOADER_COMMAND_ALLOCATE      = 0x1,
    ROMFILE_LOADER_COMMAND_جمع_کن_اشاره_گر   = 0x2,
    ROMFILE_LOADER_COMMAND_جمع_کن_CHECKSUM  = 0x3,
    ROMFILE_LOADER_COMMAND_WRITE_اشاره_گر = 0x4,
};

تعداد {
    ROMFILE_LOADER_ALLOC_ZONE_بالا = 0x1,
    ROMFILE_LOADER_ALLOC_ZONE_FSEG = 0x2,
};

صحيح romfile_loader_execute(ثابت حرف *name);

فراغ romfile_fw_cfg_resume(فراغ);

#پایان_اگر
