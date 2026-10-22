#ಅನ್ತರ್ಭಾವ "romfile_loader.h"
#ಅನ್ತರ್ಭಾವ "byteorder.h" // leXX_to_cpu/cpu_to_leXX
#ಅನ್ತರ್ಭಾವ "util.h" // checksum
#ಅನ್ತರ್ಭಾವ "string.h" // strcmp
#ಅನ್ತರ್ಭಾವ "romfile.h" // struct romfile_s
#ಅನ್ತರ್ಭಾವ "malloc.h" // Zone*, _malloc
#ಅನ್ತರ್ಭಾವ "list.h" // struct hlist_node
#ಅನ್ತರ್ಭಾವ "output.h" // warn_*
#ಅನ್ತರ್ಭಾವ "paravirt.h" // qemu_cfg_write_file

ಸಂರಚನಾ romfile_loader_file {
    ಸಂರಚನಾ romfile_s *file;
    ಶೂನ್ಯ *data;
};
ಸಂರಚನಾ romfile_loader_files {
    ಪೂರ್ಣಾಙ್ಕ nfiles;
    ಸಂರಚನಾ romfile_loader_file files[];
};

// Data structures for storing "write pointer" entries for possible replay
ಸಂರಚನಾ romfile_wr_pointer_entry {
    u64 pointer;
    u32 offset;
    u16 ಕೀಲಿ;
    u8 ptr_size;
    ಸಂರಚನಾ hlist_ಗಂಟು ಗಂಟು;
};
ಸ್ಥಿರ ಸಂರಚನಾ hlist_head romfile_pointer_list;

ಸ್ಥಿರ ಸಂರಚನಾ romfile_loader_file *
romfile_loader_find(ಧ್ರುವ ವರ್ಣ *name,
                    ಸಂರಚನಾ romfile_loader_files *files)
{
    ಪೂರ್ಣಾಙ್ಕ i;
    ಯದಿ (name[ROMFILE_LOADER_FILESZ - 1])
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    ಪ್ರತಿ (i = 0; i < files->nfiles; ++i)
        ಯದಿ (!strcmp(files->files[i].file->name, name))
            ಪ್ರತ್ಯಾಗಮ &files->files[i];
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

// Replay "write pointer" entries back to QEMU
ಶೂನ್ಯ romfile_fw_cfg_resume(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_QEMU)
        ಪ್ರತ್ಯಾಗಮ;

    ಸಂರಚನಾ romfile_wr_pointer_entry *entry;
    hlist_for_each_entry(entry, &romfile_pointer_list, ಗಂಟು) {
        qemu_cfg_write_file_simple(&entry->pointer, entry->ಕೀಲಿ,
                                   entry->offset, entry->ptr_size);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ romfile_loader_allocate(ಸಂರಚನಾ romfile_loader_entry_s *entry,
                                    ಸಂರಚನಾ romfile_loader_files *files)
{
    ಸಂರಚನಾ zone_s *zone;
    ಸಂರಚನಾ romfile_loader_file *file = &files->files[files->nfiles];
    ಶೂನ್ಯ *data;
    ಪೂರ್ಣಾಙ್ಕ ret;
    unsigned alloc_align = le32_to_cpu(entry->alloc.align);

    ಯದಿ (alloc_align & (alloc_align - 1))
        ಗಚ್ಛ err;

    ವಿಕಲ್ಪ (entry->alloc.zone) {
        ಪಕ್ಷ ROMFILE_LOADER_ALLOC_ZONE_ಮೇಲಿನ:
            zone = &Zone_ಮೇಲಿನ;
            ಭಙ್ಗ;
        ಪಕ್ಷ ROMFILE_LOADER_ALLOC_ZONE_FSEG:
            zone = &ZoneFSeg;
            ಭಙ್ಗ;
        ಸ್ವಭಾವ:
            ಗಚ್ಛ err;
    }
    ಯದಿ (alloc_align < MALLOC_MIN_ALIGN)
        alloc_align = MALLOC_MIN_ALIGN;
    ಯದಿ (entry->alloc.file[ROMFILE_LOADER_FILESZ - 1])
        ಗಚ್ಛ err;
    file->file = romfile_find(entry->alloc.file);
    ಯದಿ (!file->file || !file->file->size)
        ಪ್ರತ್ಯಾಗಮ;
    data = _malloc(zone, file->file->size, alloc_align);
    ಯದಿ (!data) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }
    ret = file->file->copy(file->file, data, file->file->size);
    ಯದಿ (ret != file->file->size)
        ಗಚ್ಛ file_err;
    file->data = data;
    files->nfiles++;
    ಪ್ರತ್ಯಾಗಮ;

file_err:
    ಮುಕ್ತ(data);
err:
    warn_internalerror();
}

ಸ್ಥಿರ ಶೂನ್ಯ romfile_loader_add_pointer(ಸಂರಚನಾ romfile_loader_entry_s *entry,
                                       ಸಂರಚನಾ romfile_loader_files *files)
{
    ಸಂರಚನಾ romfile_loader_file *dest_file;
    ಸಂರಚನಾ romfile_loader_file *src_file;
    unsigned offset = le32_to_cpu(entry->pointer.offset);
    u64 pointer = 0;

    dest_file = romfile_loader_find(entry->pointer.dest_file, files);
    src_file = romfile_loader_find(entry->pointer.src_file, files);

    ಯದಿ (!dest_file || !src_file || !dest_file->data || !src_file->data ||
        offset + entry->pointer.size < offset ||
        offset + entry->pointer.size > dest_file->file->size ||
        entry->pointer.size < 1 || entry->pointer.size > 8 ||
        entry->pointer.size & (entry->pointer.size - 1))
        ಗಚ್ಛ err;

    memcpy(&pointer, dest_file->data + offset, entry->pointer.size);
    pointer = le64_to_cpu(pointer);
    pointer += (unsigned long)src_file->data;
    pointer = cpu_to_le64(pointer);
    memcpy(dest_file->data + offset, &pointer, entry->pointer.size);

    ಪ್ರತ್ಯಾಗಮ;
err:
    warn_internalerror();
}

ಸ್ಥಿರ ಶೂನ್ಯ romfile_loader_add_checksum(ಸಂರಚನಾ romfile_loader_entry_s *entry,
                                        ಸಂರಚನಾ romfile_loader_files *files)
{
    ಸಂರಚನಾ romfile_loader_file *file;
    unsigned offset = le32_to_cpu(entry->cksum.offset);
    unsigned start = le32_to_cpu(entry->cksum.start);
    unsigned len = le32_to_cpu(entry->cksum.length);
    u8 *data;

    file = romfile_loader_find(entry->cksum.file, files);

    ಯದಿ (!file || !file->data || offset >= file->file->size ||
        start + len < start || start + len > file->file->size)
        ಗಚ್ಛ err;

    data = file->data + offset;
    *data -= checksum(file->data + start, len);

    ಪ್ರತ್ಯಾಗಮ;
err:
    warn_internalerror();
}

ಸ್ಥಿರ ಶೂನ್ಯ romfile_loader_write_pointer(ಸಂರಚನಾ romfile_loader_entry_s *entry,
                                         ಸಂರಚನಾ romfile_loader_files *files)
{
    ಸಂರಚನಾ romfile_s *dest_file;
    ಸಂರಚನಾ romfile_loader_file *src_file;
    unsigned dst_offset = le32_to_cpu(entry->wr_pointer.dst_offset);
    unsigned src_offset = le32_to_cpu(entry->wr_pointer.src_offset);
    u64 pointer = 0;

    /* Writing back to a file that may not be loaded in RAM */
    dest_file = romfile_find(entry->wr_pointer.dest_file);
    src_file = romfile_loader_find(entry->wr_pointer.src_file, files);

    ಯದಿ (!dest_file || !src_file || !src_file->data ||
        dst_offset + entry->wr_pointer.size < dst_offset ||
        dst_offset + entry->wr_pointer.size > dest_file->size ||
        src_offset >= src_file->file->size ||
        entry->wr_pointer.size < 1 || entry->wr_pointer.size > 8 ||
        entry->wr_pointer.size & (entry->wr_pointer.size - 1)) {
        ಗಚ್ಛ err;
    }

    pointer = (unsigned long)src_file->data + src_offset;
    /* Make sure the pointer fits within wr_pointer.size */
    ಯದಿ ((entry->wr_pointer.size != ಪರಿಮಾಣ(u64)) &&
        ((pointer >> (entry->wr_pointer.size * 8)) > 0)) {
        ಗಚ್ಛ err;
    }
    pointer = cpu_to_le64(pointer);

    /* Only supported on QEMU */
    ಯದಿ (qemu_cfg_write_file(&pointer, dest_file, dst_offset,
                            entry->wr_pointer.size) != entry->wr_pointer.size) {
        ಗಚ್ಛ err;
    }

    /* Store the info so it can replayed later if necessary */
    ಸಂರಚನಾ romfile_wr_pointer_entry *store = malloc_ಮೇಲಿನ(ಪರಿಮಾಣ(*store));
    ಯದಿ (!store) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }
    store->pointer = pointer;
    store->ಕೀಲಿ = qemu_get_romfile_ಕೀಲಿ(dest_file);
    store->offset = dst_offset;
    store->ptr_size = entry->wr_pointer.size;
    hlist_add_head(&store->ಗಂಟು, &romfile_pointer_list);

    ಪ್ರತ್ಯಾಗಮ;
 err:
    warn_internalerror();
}

ಪೂರ್ಣಾಙ್ಕ romfile_loader_execute(ಧ್ರುವ ವರ್ಣ *name)
{
    ಸಂರಚನಾ romfile_loader_entry_s *entry;
    ಪೂರ್ಣಾಙ್ಕ size, offset = 0, nfiles;
    ಸಂರಚನಾ romfile_loader_files *files;
    ಶೂನ್ಯ *data = romfile_loadfile(name, &size);
    ಯದಿ (!data)
        ಪ್ರತ್ಯಾಗಮ -1;

    ಯದಿ (size % ಪರಿಮಾಣ(*entry)) {
        warn_internalerror();
        ಗಚ್ಛ err;
    }

    /* (over)estimate the number of files to load. */
    nfiles = size / ಪರಿಮಾಣ(*entry);
    files = malloc_tmp(ಪರಿಮಾಣ(*files) + nfiles * ಪರಿಮಾಣ(files->files[0]));
    ಯದಿ (!files) {
        warn_noalloc();
        ಗಚ್ಛ err;
    }
    files->nfiles = 0;

    ಪ್ರತಿ (offset = 0; offset < size; offset += ಪರಿಮಾಣ(*entry)) {
        entry = data + offset;
        ವಿಕಲ್ಪ (le32_to_cpu(entry->command)) {
                ಪಕ್ಷ ROMFILE_LOADER_COMMAND_ALLOCATE:
                        romfile_loader_allocate(entry, files);
                        ಭಙ್ಗ;
                ಪಕ್ಷ ROMFILE_LOADER_COMMAND_ADD_POINTER:
                        romfile_loader_add_pointer(entry, files);
                        ಭಙ್ಗ;
                ಪಕ್ಷ ROMFILE_LOADER_COMMAND_ADD_CHECKSUM:
                        romfile_loader_add_checksum(entry, files);
                        ಭಙ್ಗ;
                ಪಕ್ಷ ROMFILE_LOADER_COMMAND_WRITE_POINTER:
                        romfile_loader_write_pointer(entry, files);
                        ಭಙ್ಗ;
                ಸ್ವಭಾವ:
                        /* Skip commands that we don't recognize. */
                        ಭಙ್ಗ;
        }
    }

    ಮುಕ್ತ(files);
    ಮುಕ್ತ(data);
    ಪ್ರತ್ಯಾಗಮ 0;

err:
    ಮುಕ್ತ(data);
    ಪ್ರತ್ಯಾಗಮ -1;
}
