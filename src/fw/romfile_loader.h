#నిర్వచితం_కాకపోతే __ROMFILE_LOADER_H
#నిర్వచించు __ROMFILE_LOADER_H

#అన్తర్భావ "types.h" // u8
#అన్తర్భావ "util.h" // romfile_s

#నిర్వచించు ROMFILE_LOADER_FILESZ 56

/* ROM file linker/loader interface. Linker uses little endian format */
నిర్మాణం romfile_loader_నమోదు_s {
    u32 command;
    సంఘం {
        /*
         * COMMAND_ALLOCATE - allocate a table from @alloc.file
         * subject to @alloc.align alignment (must be power of 2)
         * and @alloc.zone (can be HIGH or FSEG) requirements.
         *
         * Must appear exactly once for each file, and before
         * this file is referenced by any other command.
         */
        నిర్మాణం {
            అక్షరం file[ROMFILE_LOADER_FILESZ];
            u32 align;
            u8 zone;
        } alloc;

        /*
         * COMMAND_ADD_POINTER - patch the table (originating from
         * @dest_file) at @pointer.offset, by adding a pointer to the table
         * originating from @src_file. 1,2,4 or 8 byte unsigned
         * addition is used depending on @pointer.size.
         */
        నిర్మాణం {
            అక్షరం dest_file[ROMFILE_LOADER_FILESZ];
            అక్షరం src_file[ROMFILE_LOADER_FILESZ];
            u32 స్థానభ్రంశం_2;
            u8 కొలత;
        } సూచిక_2;

        /*
         * COMMAND_ADD_CHECKSUM - calculate checksum of the range specified by
         * @cksum.start and @cksum.length fields,
         * and then add the value at @cksum_offset.
         * Checksum simply sums -X for each byte X in the range
         * using 8-bit math.
         */
        నిర్మాణం {
            అక్షరం file[ROMFILE_LOADER_FILESZ];
            u32 స్థానభ్రంశం_2;
            u32 ప్రారంభం;
            u32 నిడివి;
        } cksum;

        /*
         * COMMAND_WRITE_POINTER - Write back to a host file via DMA,
         * @wr_pointer.dest_file at offset @wr_pointer.dst_offset, a pointer
         * to the table originating from @wr_pointer.src_file at offset
         * @wr_pointer.src_offset.
         * 1,2,4 or 8 byte unsigned addition is used depending on
         * @wr_pointer.size.
         */
        నిర్మాణం {
            అక్షరం dest_file[ROMFILE_LOADER_FILESZ];
            అక్షరం src_file[ROMFILE_LOADER_FILESZ];
            u32 dst_స్థానభ్రంశం;
            u32 src_స్థానభ్రంశం;
            u8 కొలత;
        } wr_సూచిక;

        /* padding */
        అక్షరం pad[124];
    };
};

గణన {
    ROMFILE_LOADER_COMMAND_ALLOCATE      = 0x1,
    ROMFILE_LOADER_COMMAND_కూడు_సూచిక   = 0x2,
    ROMFILE_LOADER_COMMAND_కూడు_CHECKSUM  = 0x3,
    ROMFILE_LOADER_COMMAND_WRITE_సూచిక = 0x4,
};

గణన {
    ROMFILE_LOADER_ALLOC_ZONE_ఎగువ = 0x1,
    ROMFILE_LOADER_ALLOC_ZONE_FSEG = 0x2,
};

పూర్ణాంకం romfile_loader_execute(స్థిరరాశి అక్షరం *name);

శూన్య romfile_fw_cfg_resume(శూన్య);

#అయితే_ముగింపు
