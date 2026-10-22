#यदि_अपरिभाषितम् __ROMFILE_LOADER_H
#परिभाषय __ROMFILE_LOADER_H

#अन्तर्भाव "types.h" // u8
#अन्तर्भाव "util.h" // romfile_s

#परिभाषय ROMFILE_LOADER_FILESZ 56

/* ROM file linker/loader interface. Linker uses little endian format */
संरचना romfile_loader_प्रविष्टिः_s {
    u32 command;
    सङ्घ {
        /*
         * COMMAND_ALLOCATE - allocate a table from @alloc.file
         * subject to @alloc.align alignment (must be power of 2)
         * and @alloc.zone (can be HIGH or FSEG) requirements.
         *
         * Must appear exactly once for each file, and before
         * this file is referenced by any other command.
         */
        संरचना {
            वर्ण file[ROMFILE_LOADER_FILESZ];
            u32 align;
            u8 zone;
        } alloc;

        /*
         * COMMAND_ADD_POINTER - patch the table (originating from
         * @dest_file) at @pointer.offset, by adding a pointer to the table
         * originating from @src_file. 1,2,4 or 8 byte unsigned
         * addition is used depending on @pointer.size.
         */
        संरचना {
            वर्ण dest_file[ROMFILE_LOADER_FILESZ];
            वर्ण src_file[ROMFILE_LOADER_FILESZ];
            u32 विस्थापनम्_2;
            u8 आकार;
        } pointer;

        /*
         * COMMAND_ADD_CHECKSUM - calculate checksum of the range specified by
         * @cksum.start and @cksum.length fields,
         * and then add the value at @cksum_offset.
         * Checksum simply sums -X for each byte X in the range
         * using 8-bit math.
         */
        संरचना {
            वर्ण file[ROMFILE_LOADER_FILESZ];
            u32 विस्थापनम्_2;
            u32 start;
            u32 दीर्घता;
        } cksum;

        /*
         * COMMAND_WRITE_POINTER - Write back to a host file via DMA,
         * @wr_pointer.dest_file at offset @wr_pointer.dst_offset, a pointer
         * to the table originating from @wr_pointer.src_file at offset
         * @wr_pointer.src_offset.
         * 1,2,4 or 8 byte unsigned addition is used depending on
         * @wr_pointer.size.
         */
        संरचना {
            वर्ण dest_file[ROMFILE_LOADER_FILESZ];
            वर्ण src_file[ROMFILE_LOADER_FILESZ];
            u32 dst_विस्थापनम्;
            u32 src_विस्थापनम्;
            u8 आकार;
        } wr_pointer;

        /* padding */
        वर्ण pad[124];
    };
};

गणना {
    ROMFILE_LOADER_COMMAND_ALLOCATE      = 0x1,
    ROMFILE_LOADER_COMMAND_सङ्कलय_POINTER   = 0x2,
    ROMFILE_LOADER_COMMAND_सङ्कलय_CHECKSUM  = 0x3,
    ROMFILE_LOADER_COMMAND_WRITE_POINTER = 0x4,
};

गणना {
    ROMFILE_LOADER_ALLOC_ZONE_उच्चः = 0x1,
    ROMFILE_LOADER_ALLOC_ZONE_FSEG = 0x2,
};

पूर्णाङ्क romfile_loader_execute(ध्रुव वर्ण *name);

शून्य romfile_fw_cfg_resume(शून्य);

#यदि_अन्तः
