#ifndef __ROMFILE_LOADER_H
#define __ROMFILE_LOADER_H

#ಅನ್ತರ್ಭಾವ "types.h" // u8
#ಅನ್ತರ್ಭಾವ "util.h" // romfile_s

#define ROMFILE_LOADER_FILESZ 56

/* ROM file linker/loader interface. Linker uses little endian format */
ಸಂರಚನಾ romfile_loader_entry_s {
    u32 command;
    ಸಙ್ಘ {
        /*
         * COMMAND_ALLOCATE - allocate a table from @alloc.file
         * subject to @alloc.align alignment (must be power of 2)
         * and @alloc.zone (can be HIGH or FSEG) requirements.
         *
         * Must appear exactly once for each file, and before
         * this file is referenced by any other command.
         */
        ಸಂರಚನಾ {
            ವರ್ಣ file[ROMFILE_LOADER_FILESZ];
            u32 align;
            u8 zone;
        } alloc;

        /*
         * COMMAND_ADD_POINTER - patch the table (originating from
         * @dest_file) at @pointer.offset, by adding a pointer to the table
         * originating from @src_file. 1,2,4 or 8 byte unsigned
         * addition is used depending on @pointer.size.
         */
        ಸಂರಚನಾ {
            ವರ್ಣ dest_file[ROMFILE_LOADER_FILESZ];
            ವರ್ಣ src_file[ROMFILE_LOADER_FILESZ];
            u32 offset;
            u8 size;
        } pointer;

        /*
         * COMMAND_ADD_CHECKSUM - calculate checksum of the range specified by
         * @cksum.start and @cksum.length fields,
         * and then add the value at @cksum_offset.
         * Checksum simply sums -X for each byte X in the range
         * using 8-bit math.
         */
        ಸಂರಚನಾ {
            ವರ್ಣ file[ROMFILE_LOADER_FILESZ];
            u32 offset;
            u32 start;
            u32 length;
        } cksum;

        /*
         * COMMAND_WRITE_POINTER - Write back to a host file via DMA,
         * @wr_pointer.dest_file at offset @wr_pointer.dst_offset, a pointer
         * to the table originating from @wr_pointer.src_file at offset
         * @wr_pointer.src_offset.
         * 1,2,4 or 8 byte unsigned addition is used depending on
         * @wr_pointer.size.
         */
        ಸಂರಚನಾ {
            ವರ್ಣ dest_file[ROMFILE_LOADER_FILESZ];
            ವರ್ಣ src_file[ROMFILE_LOADER_FILESZ];
            u32 dst_offset;
            u32 src_offset;
            u8 size;
        } wr_pointer;

        /* padding */
        ವರ್ಣ pad[124];
    };
};

ಗಣನಾ {
    ROMFILE_LOADER_COMMAND_ALLOCATE      = 0x1,
    ROMFILE_LOADER_COMMAND_ADD_POINTER   = 0x2,
    ROMFILE_LOADER_COMMAND_ADD_CHECKSUM  = 0x3,
    ROMFILE_LOADER_COMMAND_WRITE_POINTER = 0x4,
};

ಗಣನಾ {
    ROMFILE_LOADER_ALLOC_ZONE_ಮೇಲಿನ = 0x1,
    ROMFILE_LOADER_ALLOC_ZONE_FSEG = 0x2,
};

ಪೂರ್ಣಾಙ್ಕ romfile_loader_execute(ಧ್ರುವ ವರ್ಣ *name);

ಶೂನ್ಯ romfile_fw_cfg_resume(ಶೂನ್ಯ);

#endif
