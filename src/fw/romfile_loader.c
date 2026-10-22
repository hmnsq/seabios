#ಅನ್ತರ್ಭಾವ "romfile_loader.h"
#ಅನ್ತರ್ಭಾವ "byteorder.h" // leXX_to_cpu/cpu_to_leXX
#ಅನ್ತರ್ಭಾವ "util.h" // checksum
#ಅನ್ತರ್ಭಾವ "string.h" // strcmp
#ಅನ್ತರ್ಭಾವ "romfile.h" // struct romfile_s
#ಅನ್ತರ್ಭಾವ "malloc.h" // Zone*, _malloc
#ಅನ್ತರ್ಭಾವ "list.h" // struct hlist_node
#ಅನ್ತರ್ಭಾವ "output.h" // warn_*
#ಅನ್ತರ್ಭಾವ "paravirt.h" // qemu_cfg_write_file

ರಚನೆ romfile_loader_file {
    ರಚನೆ romfile_s *file;
    ಶೂನ್ಯ *ದತ್ತಾಂಶ;
};
ರಚನೆ romfile_loader_files {
    ಪೂರ್ಣಾಂಕ nfiles;
    ರಚನೆ romfile_loader_file files[];
};

// Data structures for storing "write pointer" entries for possible replay
ರಚನೆ romfile_wr_ಸೂಚಕ_ನಮೂದು {
    u64 ಸೂಚಕ_2;
    u32 ಸ್ಥಾನಾಂತರ_2;
    u16 ಕೀಲಿ;
    u8 ptr_ಗಾತ್ರ;
    ರಚನೆ hlist_ಗಂಟು ಗಂಟು;
};
ಸ್ಥಿರ ರಚನೆ hlist_ತಲೆ romfile_ಸೂಚಕ_list;

ಸ್ಥಿರ ರಚನೆ romfile_loader_file *
romfile_loader_ಕಂಡುಹಿಡಿ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name,
                    ರಚನೆ romfile_loader_files *files)
{
    ಪೂರ್ಣಾಂಕ i;
    ಆದರೆ (name[ROMFILE_LOADER_FILESZ - 1])
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಗಾಗಿ (i = 0; i < files->nfiles; ++i)
        ಆದರೆ (!strcmp(files->files[i].file->name, name))
            ಹಿಂತಿರುಗಿ &files->files[i];
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

// Replay "write pointer" entries back to QEMU
ಶೂನ್ಯ romfile_fw_cfg_resume(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_QEMU)
        ಹಿಂತಿರುಗಿ;

    ರಚನೆ romfile_wr_ಸೂಚಕ_ನಮೂದು *ನಮೂದು;
    hlist_for_each_ನಮೂದು(ನಮೂದು, &romfile_ಸೂಚಕ_list, ಗಂಟು) {
        qemu_cfg_write_file_simple(&ನಮೂದು->ಸೂಚಕ_2, ನಮೂದು->ಕೀಲಿ,
                                   ನಮೂದು->ಸ್ಥಾನಾಂತರ_2, ನಮೂದು->ptr_ಗಾತ್ರ);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ romfile_loader_allocate(ರಚನೆ romfile_loader_ನಮೂದು_s *ನಮೂದು,
                                    ರಚನೆ romfile_loader_files *files)
{
    ರಚನೆ zone_s *zone;
    ರಚನೆ romfile_loader_file *file = &files->files[files->nfiles];
    ಶೂನ್ಯ *ದತ್ತಾಂಶ;
    ಪೂರ್ಣಾಂಕ ret;
    ಚಿಹ್ನೆರಹಿತ alloc_align = le32_ಗಮ್ಯ_cpu(ನಮೂದು->alloc.align);

    ಆದರೆ (alloc_align & (alloc_align - 1))
        ಹೋಗು err;

    ಆಯ್ಕೆ (ನಮೂದು->alloc.zone) {
        ಸ್ಥಿತಿ ROMFILE_LOADER_ALLOC_ZONE_ಮೇಲಿನ:
            zone = &Zone_ಮೇಲಿನ;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ ROMFILE_LOADER_ALLOC_ZONE_FSEG:
            zone = &ZoneFSeg;
            ನಿಲ್ಲಿಸು;
        ಪೂರ್ವನಿಯೋಜಿತ:
            ಹೋಗು err;
    }
    ಆದರೆ (alloc_align < MALLOC_MIN_ALIGN)
        alloc_align = MALLOC_MIN_ALIGN;
    ಆದರೆ (ನಮೂದು->alloc.file[ROMFILE_LOADER_FILESZ - 1])
        ಹೋಗು err;
    file->file = romfile_ಕಂಡುಹಿಡಿ(ನಮೂದು->alloc.file);
    ಆದರೆ (!file->file || !file->file->ಗಾತ್ರ_2)
        ಹಿಂತಿರುಗಿ;
    ದತ್ತಾಂಶ = _malloc(zone, file->file->ಗಾತ್ರ_2, alloc_align);
    ಆದರೆ (!ದತ್ತಾಂಶ) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }
    ret = file->file->copy(file->file, ದತ್ತಾಂಶ, file->file->ಗಾತ್ರ_2);
    ಆದರೆ (ret != file->file->ಗಾತ್ರ_2)
        ಹೋಗು file_err;
    file->ದತ್ತಾಂಶ = ದತ್ತಾಂಶ;
    files->nfiles++;
    ಹಿಂತಿರುಗಿ;

file_err:
    ಬಿಡುಗಡೆ(ದತ್ತಾಂಶ);
err:
    warn_internalerror();
}

ಸ್ಥಿರ ಶೂನ್ಯ romfile_loader_ಕೂಡಿಸು_ಸೂಚಕ(ರಚನೆ romfile_loader_ನಮೂದು_s *ನಮೂದು,
                                       ರಚನೆ romfile_loader_files *files)
{
    ರಚನೆ romfile_loader_file *dest_file;
    ರಚನೆ romfile_loader_file *src_file;
    ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ_2 = le32_ಗಮ್ಯ_cpu(ನಮೂದು->ಸೂಚಕ_2.ಸ್ಥಾನಾಂತರ_2);
    u64 ಸೂಚಕ_2 = 0;

    dest_file = romfile_loader_ಕಂಡುಹಿಡಿ(ನಮೂದು->ಸೂಚಕ_2.dest_file, files);
    src_file = romfile_loader_ಕಂಡುಹಿಡಿ(ನಮೂದು->ಸೂಚಕ_2.src_file, files);

    ಆದರೆ (!dest_file || !src_file || !dest_file->ದತ್ತಾಂಶ || !src_file->ದತ್ತಾಂಶ ||
        ಸ್ಥಾನಾಂತರ_2 + ನಮೂದು->ಸೂಚಕ_2.ಗಾತ್ರ_2 < ಸ್ಥಾನಾಂತರ_2 ||
        ಸ್ಥಾನಾಂತರ_2 + ನಮೂದು->ಸೂಚಕ_2.ಗಾತ್ರ_2 > dest_file->file->ಗಾತ್ರ_2 ||
        ನಮೂದು->ಸೂಚಕ_2.ಗಾತ್ರ_2 < 1 || ನಮೂದು->ಸೂಚಕ_2.ಗಾತ್ರ_2 > 8 ||
        ನಮೂದು->ಸೂಚಕ_2.ಗಾತ್ರ_2 & (ನಮೂದು->ಸೂಚಕ_2.ಗಾತ್ರ_2 - 1))
        ಹೋಗು err;

    ಸ್ಮರಣೆ_ನಕಲು(&ಸೂಚಕ_2, dest_file->ದತ್ತಾಂಶ + ಸ್ಥಾನಾಂತರ_2, ನಮೂದು->ಸೂಚಕ_2.ಗಾತ್ರ_2);
    ಸೂಚಕ_2 = le64_ಗಮ್ಯ_cpu(ಸೂಚಕ_2);
    ಸೂಚಕ_2 += (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)src_file->ದತ್ತಾಂಶ;
    ಸೂಚಕ_2 = cpu_ಗಮ್ಯ_le64(ಸೂಚಕ_2);
    ಸ್ಮರಣೆ_ನಕಲು(dest_file->ದತ್ತಾಂಶ + ಸ್ಥಾನಾಂತರ_2, &ಸೂಚಕ_2, ನಮೂದು->ಸೂಚಕ_2.ಗಾತ್ರ_2);

    ಹಿಂತಿರುಗಿ;
err:
    warn_internalerror();
}

ಸ್ಥಿರ ಶೂನ್ಯ romfile_loader_ಕೂಡಿಸು_checksum(ರಚನೆ romfile_loader_ನಮೂದು_s *ನಮೂದು,
                                        ರಚನೆ romfile_loader_files *files)
{
    ರಚನೆ romfile_loader_file *file;
    ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ_2 = le32_ಗಮ್ಯ_cpu(ನಮೂದು->cksum.ಸ್ಥಾನಾಂತರ_2);
    ಚಿಹ್ನೆರಹಿತ ಆರಂಭ = le32_ಗಮ್ಯ_cpu(ನಮೂದು->cksum.ಆರಂಭ);
    ಚಿಹ್ನೆರಹಿತ len = le32_ಗಮ್ಯ_cpu(ನಮೂದು->cksum.ಉದ್ದಳತೆ);
    u8 *ದತ್ತಾಂಶ;

    file = romfile_loader_ಕಂಡುಹಿಡಿ(ನಮೂದು->cksum.file, files);

    ಆದರೆ (!file || !file->ದತ್ತಾಂಶ || ಸ್ಥಾನಾಂತರ_2 >= file->file->ಗಾತ್ರ_2 ||
        ಆರಂಭ + len < ಆರಂಭ || ಆರಂಭ + len > file->file->ಗಾತ್ರ_2)
        ಹೋಗು err;

    ದತ್ತಾಂಶ = file->ದತ್ತಾಂಶ + ಸ್ಥಾನಾಂತರ_2;
    *ದತ್ತಾಂಶ -= checksum(file->ದತ್ತಾಂಶ + ಆರಂಭ, len);

    ಹಿಂತಿರುಗಿ;
err:
    warn_internalerror();
}

ಸ್ಥಿರ ಶೂನ್ಯ romfile_loader_write_ಸೂಚಕ(ರಚನೆ romfile_loader_ನಮೂದು_s *ನಮೂದು,
                                         ರಚನೆ romfile_loader_files *files)
{
    ರಚನೆ romfile_s *dest_file;
    ರಚನೆ romfile_loader_file *src_file;
    ಚಿಹ್ನೆರಹಿತ dst_ಸ್ಥಾನಾಂತರ = le32_ಗಮ್ಯ_cpu(ನಮೂದು->wr_ಸೂಚಕ.dst_ಸ್ಥಾನಾಂತರ);
    ಚಿಹ್ನೆರಹಿತ src_ಸ್ಥಾನಾಂತರ = le32_ಗಮ್ಯ_cpu(ನಮೂದು->wr_ಸೂಚಕ.src_ಸ್ಥಾನಾಂತರ);
    u64 ಸೂಚಕ_2 = 0;

    /* Writing back to a file that may not be loaded in RAM */
    dest_file = romfile_ಕಂಡುಹಿಡಿ(ನಮೂದು->wr_ಸೂಚಕ.dest_file);
    src_file = romfile_loader_ಕಂಡುಹಿಡಿ(ನಮೂದು->wr_ಸೂಚಕ.src_file, files);

    ಆದರೆ (!dest_file || !src_file || !src_file->ದತ್ತಾಂಶ ||
        dst_ಸ್ಥಾನಾಂತರ + ನಮೂದು->wr_ಸೂಚಕ.ಗಾತ್ರ_2 < dst_ಸ್ಥಾನಾಂತರ ||
        dst_ಸ್ಥಾನಾಂತರ + ನಮೂದು->wr_ಸೂಚಕ.ಗಾತ್ರ_2 > dest_file->ಗಾತ್ರ_2 ||
        src_ಸ್ಥಾನಾಂತರ >= src_file->file->ಗಾತ್ರ_2 ||
        ನಮೂದು->wr_ಸೂಚಕ.ಗಾತ್ರ_2 < 1 || ನಮೂದು->wr_ಸೂಚಕ.ಗಾತ್ರ_2 > 8 ||
        ನಮೂದು->wr_ಸೂಚಕ.ಗಾತ್ರ_2 & (ನಮೂದು->wr_ಸೂಚಕ.ಗಾತ್ರ_2 - 1)) {
        ಹೋಗು err;
    }

    ಸೂಚಕ_2 = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)src_file->ದತ್ತಾಂಶ + src_ಸ್ಥಾನಾಂತರ;
    /* Make sure the pointer fits within wr_pointer.size */
    ಆದರೆ ((ನಮೂದು->wr_ಸೂಚಕ.ಗಾತ್ರ_2 != ಗಾತ್ರ(u64)) &&
        ((ಸೂಚಕ_2 >> (ನಮೂದು->wr_ಸೂಚಕ.ಗಾತ್ರ_2 * 8)) > 0)) {
        ಹೋಗು err;
    }
    ಸೂಚಕ_2 = cpu_ಗಮ್ಯ_le64(ಸೂಚಕ_2);

    /* Only supported on QEMU */
    ಆದರೆ (qemu_cfg_write_file(&ಸೂಚಕ_2, dest_file, dst_ಸ್ಥಾನಾಂತರ,
                            ನಮೂದು->wr_ಸೂಚಕ.ಗಾತ್ರ_2) != ನಮೂದು->wr_ಸೂಚಕ.ಗಾತ್ರ_2) {
        ಹೋಗು err;
    }

    /* Store the info so it can replayed later if necessary */
    ರಚನೆ romfile_wr_ಸೂಚಕ_ನಮೂದು *store = malloc_ಮೇಲಿನ(ಗಾತ್ರ(*store));
    ಆದರೆ (!store) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }
    store->ಸೂಚಕ_2 = ಸೂಚಕ_2;
    store->ಕೀಲಿ = qemu_get_romfile_ಕೀಲಿ(dest_file);
    store->ಸ್ಥಾನಾಂತರ_2 = dst_ಸ್ಥಾನಾಂತರ;
    store->ptr_ಗಾತ್ರ = ನಮೂದು->wr_ಸೂಚಕ.ಗಾತ್ರ_2;
    hlist_ಕೂಡಿಸು_ತಲೆ(&store->ಗಂಟು, &romfile_ಸೂಚಕ_list);

    ಹಿಂತಿರುಗಿ;
 err:
    warn_internalerror();
}

ಪೂರ್ಣಾಂಕ romfile_loader_execute(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
    ರಚನೆ romfile_loader_ನಮೂದು_s *ನಮೂದು;
    ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2, ಸ್ಥಾನಾಂತರ_2 = 0, nfiles;
    ರಚನೆ romfile_loader_files *files;
    ಶೂನ್ಯ *ದತ್ತಾಂಶ = romfile_loadfile(name, &ಗಾತ್ರ_2);
    ಆದರೆ (!ದತ್ತಾಂಶ)
        ಹಿಂತಿರುಗಿ -1;

    ಆದರೆ (ಗಾತ್ರ_2 % ಗಾತ್ರ(*ನಮೂದು)) {
        warn_internalerror();
        ಹೋಗು err;
    }

    /* (over)estimate the number of files to load. */
    nfiles = ಗಾತ್ರ_2 / ಗಾತ್ರ(*ನಮೂದು);
    files = malloc_tmp(ಗಾತ್ರ(*files) + nfiles * ಗಾತ್ರ(files->files[0]));
    ಆದರೆ (!files) {
        warn_noalloc();
        ಹೋಗು err;
    }
    files->nfiles = 0;

    ಗಾಗಿ (ಸ್ಥಾನಾಂತರ_2 = 0; ಸ್ಥಾನಾಂತರ_2 < ಗಾತ್ರ_2; ಸ್ಥಾನಾಂತರ_2 += ಗಾತ್ರ(*ನಮೂದು)) {
        ನಮೂದು = ದತ್ತಾಂಶ + ಸ್ಥಾನಾಂತರ_2;
        ಆಯ್ಕೆ (le32_ಗಮ್ಯ_cpu(ನಮೂದು->command)) {
                ಸ್ಥಿತಿ ROMFILE_LOADER_COMMAND_ALLOCATE:
                        romfile_loader_allocate(ನಮೂದು, files);
                        ನಿಲ್ಲಿಸು;
                ಸ್ಥಿತಿ ROMFILE_LOADER_COMMAND_ಕೂಡಿಸು_ಸೂಚಕ:
                        romfile_loader_ಕೂಡಿಸು_ಸೂಚಕ(ನಮೂದು, files);
                        ನಿಲ್ಲಿಸು;
                ಸ್ಥಿತಿ ROMFILE_LOADER_COMMAND_ಕೂಡಿಸು_CHECKSUM:
                        romfile_loader_ಕೂಡಿಸು_checksum(ನಮೂದು, files);
                        ನಿಲ್ಲಿಸು;
                ಸ್ಥಿತಿ ROMFILE_LOADER_COMMAND_WRITE_ಸೂಚಕ:
                        romfile_loader_write_ಸೂಚಕ(ನಮೂದು, files);
                        ನಿಲ್ಲಿಸು;
                ಪೂರ್ವನಿಯೋಜಿತ:
                        /* Skip commands that we don't recognize. */
                        ನಿಲ್ಲಿಸು;
        }
    }

    ಬಿಡುಗಡೆ(files);
    ಬಿಡುಗಡೆ(ದತ್ತಾಂಶ);
    ಹಿಂತಿರುಗಿ 0;

err:
    ಬಿಡುಗಡೆ(ದತ್ತಾಂಶ);
    ಹಿಂತಿರುಗಿ -1;
}
