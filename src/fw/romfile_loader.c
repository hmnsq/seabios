#अन्तर्भाव "romfile_loader.h"
#अन्तर्भाव "byteorder.h" // leXX_to_cpu/cpu_to_leXX
#अन्तर्भाव "util.h" // checksum
#अन्तर्भाव "string.h" // strcmp
#अन्तर्भाव "romfile.h" // struct romfile_s
#अन्तर्भाव "malloc.h" // Zone*, _malloc
#अन्तर्भाव "list.h" // struct hlist_node
#अन्तर्भाव "output.h" // warn_*
#अन्तर्भाव "paravirt.h" // qemu_cfg_write_file

संरचना romfile_loader_file {
    संरचना romfile_s *file;
    शून्य *data;
};
संरचना romfile_loader_files {
    पूर्णाङ्क nfiles;
    संरचना romfile_loader_file files[];
};

// Data structures for storing "write pointer" entries for possible replay
संरचना romfile_wr_pointer_entry {
    u64 pointer;
    u32 offset;
    u16 किल्ली;
    u8 ptr_size;
    संरचना hlist_गाठ गाठ;
};
स्थिर संरचना hlist_head romfile_pointer_list;

स्थिर संरचना romfile_loader_file *
romfile_loader_find(ध्रुव वर्ण *name,
                    संरचना romfile_loader_files *files)
{
    पूर्णाङ्क i;
    यदि (name[ROMFILE_LOADER_FILESZ - 1])
        प्रत्यागम रिक्त;
    प्रति (i = 0; i < files->nfiles; ++i)
        यदि (!strcmp(files->files[i].file->name, name))
            प्रत्यागम &files->files[i];
    प्रत्यागम रिक्त;
}

// Replay "write pointer" entries back to QEMU
शून्य romfile_fw_cfg_resume(शून्य)
{
    यदि (!CONFIG_QEMU)
        प्रत्यागम;

    संरचना romfile_wr_pointer_entry *entry;
    hlist_for_each_entry(entry, &romfile_pointer_list, गाठ) {
        qemu_cfg_write_file_simple(&entry->pointer, entry->किल्ली,
                                   entry->offset, entry->ptr_size);
    }
}

स्थिर शून्य romfile_loader_allocate(संरचना romfile_loader_entry_s *entry,
                                    संरचना romfile_loader_files *files)
{
    संरचना zone_s *zone;
    संरचना romfile_loader_file *file = &files->files[files->nfiles];
    शून्य *data;
    पूर्णाङ्क ret;
    unsigned alloc_align = le32_to_cpu(entry->alloc.align);

    यदि (alloc_align & (alloc_align - 1))
        गच्छ err;

    विकल्प (entry->alloc.zone) {
        पक्ष ROMFILE_LOADER_ALLOC_ZONE_वरचा:
            zone = &Zone_वरचा;
            भङ्ग;
        पक्ष ROMFILE_LOADER_ALLOC_ZONE_FSEG:
            zone = &ZoneFSeg;
            भङ्ग;
        स्वभाव:
            गच्छ err;
    }
    यदि (alloc_align < MALLOC_MIN_ALIGN)
        alloc_align = MALLOC_MIN_ALIGN;
    यदि (entry->alloc.file[ROMFILE_LOADER_FILESZ - 1])
        गच्छ err;
    file->file = romfile_find(entry->alloc.file);
    यदि (!file->file || !file->file->size)
        प्रत्यागम;
    data = _malloc(zone, file->file->size, alloc_align);
    यदि (!data) {
        warn_noalloc();
        प्रत्यागम;
    }
    ret = file->file->copy(file->file, data, file->file->size);
    यदि (ret != file->file->size)
        गच्छ file_err;
    file->data = data;
    files->nfiles++;
    प्रत्यागम;

file_err:
    मुक्त(data);
err:
    warn_internalerror();
}

स्थिर शून्य romfile_loader_add_pointer(संरचना romfile_loader_entry_s *entry,
                                       संरचना romfile_loader_files *files)
{
    संरचना romfile_loader_file *dest_file;
    संरचना romfile_loader_file *src_file;
    unsigned offset = le32_to_cpu(entry->pointer.offset);
    u64 pointer = 0;

    dest_file = romfile_loader_find(entry->pointer.dest_file, files);
    src_file = romfile_loader_find(entry->pointer.src_file, files);

    यदि (!dest_file || !src_file || !dest_file->data || !src_file->data ||
        offset + entry->pointer.size < offset ||
        offset + entry->pointer.size > dest_file->file->size ||
        entry->pointer.size < 1 || entry->pointer.size > 8 ||
        entry->pointer.size & (entry->pointer.size - 1))
        गच्छ err;

    memcpy(&pointer, dest_file->data + offset, entry->pointer.size);
    pointer = le64_to_cpu(pointer);
    pointer += (unsigned long)src_file->data;
    pointer = cpu_to_le64(pointer);
    memcpy(dest_file->data + offset, &pointer, entry->pointer.size);

    प्रत्यागम;
err:
    warn_internalerror();
}

स्थिर शून्य romfile_loader_add_checksum(संरचना romfile_loader_entry_s *entry,
                                        संरचना romfile_loader_files *files)
{
    संरचना romfile_loader_file *file;
    unsigned offset = le32_to_cpu(entry->cksum.offset);
    unsigned start = le32_to_cpu(entry->cksum.start);
    unsigned len = le32_to_cpu(entry->cksum.length);
    u8 *data;

    file = romfile_loader_find(entry->cksum.file, files);

    यदि (!file || !file->data || offset >= file->file->size ||
        start + len < start || start + len > file->file->size)
        गच्छ err;

    data = file->data + offset;
    *data -= checksum(file->data + start, len);

    प्रत्यागम;
err:
    warn_internalerror();
}

स्थिर शून्य romfile_loader_write_pointer(संरचना romfile_loader_entry_s *entry,
                                         संरचना romfile_loader_files *files)
{
    संरचना romfile_s *dest_file;
    संरचना romfile_loader_file *src_file;
    unsigned dst_offset = le32_to_cpu(entry->wr_pointer.dst_offset);
    unsigned src_offset = le32_to_cpu(entry->wr_pointer.src_offset);
    u64 pointer = 0;

    /* Writing back to a file that may not be loaded in RAM */
    dest_file = romfile_find(entry->wr_pointer.dest_file);
    src_file = romfile_loader_find(entry->wr_pointer.src_file, files);

    यदि (!dest_file || !src_file || !src_file->data ||
        dst_offset + entry->wr_pointer.size < dst_offset ||
        dst_offset + entry->wr_pointer.size > dest_file->size ||
        src_offset >= src_file->file->size ||
        entry->wr_pointer.size < 1 || entry->wr_pointer.size > 8 ||
        entry->wr_pointer.size & (entry->wr_pointer.size - 1)) {
        गच्छ err;
    }

    pointer = (unsigned long)src_file->data + src_offset;
    /* Make sure the pointer fits within wr_pointer.size */
    यदि ((entry->wr_pointer.size != परिमाण(u64)) &&
        ((pointer >> (entry->wr_pointer.size * 8)) > 0)) {
        गच्छ err;
    }
    pointer = cpu_to_le64(pointer);

    /* Only supported on QEMU */
    यदि (qemu_cfg_write_file(&pointer, dest_file, dst_offset,
                            entry->wr_pointer.size) != entry->wr_pointer.size) {
        गच्छ err;
    }

    /* Store the info so it can replayed later if necessary */
    संरचना romfile_wr_pointer_entry *store = malloc_वरचा(परिमाण(*store));
    यदि (!store) {
        warn_noalloc();
        प्रत्यागम;
    }
    store->pointer = pointer;
    store->किल्ली = qemu_get_romfile_किल्ली(dest_file);
    store->offset = dst_offset;
    store->ptr_size = entry->wr_pointer.size;
    hlist_add_head(&store->गाठ, &romfile_pointer_list);

    प्रत्यागम;
 err:
    warn_internalerror();
}

पूर्णाङ्क romfile_loader_execute(ध्रुव वर्ण *name)
{
    संरचना romfile_loader_entry_s *entry;
    पूर्णाङ्क size, offset = 0, nfiles;
    संरचना romfile_loader_files *files;
    शून्य *data = romfile_loadfile(name, &size);
    यदि (!data)
        प्रत्यागम -1;

    यदि (size % परिमाण(*entry)) {
        warn_internalerror();
        गच्छ err;
    }

    /* (over)estimate the number of files to load. */
    nfiles = size / परिमाण(*entry);
    files = malloc_tmp(परिमाण(*files) + nfiles * परिमाण(files->files[0]));
    यदि (!files) {
        warn_noalloc();
        गच्छ err;
    }
    files->nfiles = 0;

    प्रति (offset = 0; offset < size; offset += परिमाण(*entry)) {
        entry = data + offset;
        विकल्प (le32_to_cpu(entry->command)) {
                पक्ष ROMFILE_LOADER_COMMAND_ALLOCATE:
                        romfile_loader_allocate(entry, files);
                        भङ्ग;
                पक्ष ROMFILE_LOADER_COMMAND_ADD_POINTER:
                        romfile_loader_add_pointer(entry, files);
                        भङ्ग;
                पक्ष ROMFILE_LOADER_COMMAND_ADD_CHECKSUM:
                        romfile_loader_add_checksum(entry, files);
                        भङ्ग;
                पक्ष ROMFILE_LOADER_COMMAND_WRITE_POINTER:
                        romfile_loader_write_pointer(entry, files);
                        भङ्ग;
                स्वभाव:
                        /* Skip commands that we don't recognize. */
                        भङ्ग;
        }
    }

    मुक्त(files);
    मुक्त(data);
    प्रत्यागम 0;

err:
    मुक्त(data);
    प्रत्यागम -1;
}
