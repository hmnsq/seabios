#شامل "romfile_loader.h"
#شامل "byteorder.h" // leXX_to_cpu/cpu_to_leXX
#شامل "util.h" // checksum
#شامل "string.h" // strcmp
#شامل "romfile.h" // struct romfile_s
#شامل "malloc.h" // Zone*, _malloc
#شامل "list.h" // struct hlist_node
#شامل "output.h" // warn_*
#شامل "paravirt.h" // qemu_cfg_write_file

ساختار romfile_loader_file {
    ساختار romfile_s *file;
    تهی *data;
};
ساختار romfile_loader_files {
    صحیح nfiles;
    ساختار romfile_loader_file files[];
};

// Data structures for storing "write pointer" entries for possible replay
ساختار romfile_wr_pointer_entry {
    u64 pointer;
    u32 offset;
    u16 کلید;
    u8 ptr_size;
    ساختار hlist_گره گره;
};
ایستا ساختار hlist_head romfile_pointer_list;

ایستا ساختار romfile_loader_file *
romfile_loader_find(ثابت نویسه *name,
                    ساختار romfile_loader_files *files)
{
    صحیح i;
    اگر (name[ROMFILE_LOADER_FILESZ - 1])
        بازگشت پوچ;
    برای (i = 0; i < files->nfiles; ++i)
        اگر (!strcmp(files->files[i].file->name, name))
            بازگشت &files->files[i];
    بازگشت پوچ;
}

// Replay "write pointer" entries back to QEMU
تهی romfile_fw_cfg_resume(تهی)
{
    اگر (!CONFIG_QEMU)
        بازگشت;

    ساختار romfile_wr_pointer_entry *entry;
    hlist_for_each_entry(entry, &romfile_pointer_list, گره) {
        qemu_cfg_write_file_simple(&entry->pointer, entry->کلید,
                                   entry->offset, entry->ptr_size);
    }
}

ایستا تهی romfile_loader_allocate(ساختار romfile_loader_entry_s *entry,
                                    ساختار romfile_loader_files *files)
{
    ساختار zone_s *zone;
    ساختار romfile_loader_file *file = &files->files[files->nfiles];
    تهی *data;
    صحیح ret;
    unsigned alloc_align = le32_to_cpu(entry->alloc.align);

    اگر (alloc_align & (alloc_align - 1))
        برو err;

    انتخاب (entry->alloc.zone) {
        حالت ROMFILE_LOADER_ALLOC_ZONE_بالا:
            zone = &Zone_بالا;
            بشکن;
        حالت ROMFILE_LOADER_ALLOC_ZONE_FSEG:
            zone = &ZoneFSeg;
            بشکن;
        پیشفرض:
            برو err;
    }
    اگر (alloc_align < MALLOC_MIN_ALIGN)
        alloc_align = MALLOC_MIN_ALIGN;
    اگر (entry->alloc.file[ROMFILE_LOADER_FILESZ - 1])
        برو err;
    file->file = romfile_find(entry->alloc.file);
    اگر (!file->file || !file->file->size)
        بازگشت;
    data = _malloc(zone, file->file->size, alloc_align);
    اگر (!data) {
        warn_noalloc();
        بازگشت;
    }
    ret = file->file->copy(file->file, data, file->file->size);
    اگر (ret != file->file->size)
        برو file_err;
    file->data = data;
    files->nfiles++;
    بازگشت;

file_err:
    آزاد(data);
err:
    warn_internalerror();
}

ایستا تهی romfile_loader_add_pointer(ساختار romfile_loader_entry_s *entry,
                                       ساختار romfile_loader_files *files)
{
    ساختار romfile_loader_file *dest_file;
    ساختار romfile_loader_file *src_file;
    unsigned offset = le32_to_cpu(entry->pointer.offset);
    u64 pointer = 0;

    dest_file = romfile_loader_find(entry->pointer.dest_file, files);
    src_file = romfile_loader_find(entry->pointer.src_file, files);

    اگر (!dest_file || !src_file || !dest_file->data || !src_file->data ||
        offset + entry->pointer.size < offset ||
        offset + entry->pointer.size > dest_file->file->size ||
        entry->pointer.size < 1 || entry->pointer.size > 8 ||
        entry->pointer.size & (entry->pointer.size - 1))
        برو err;

    memcpy(&pointer, dest_file->data + offset, entry->pointer.size);
    pointer = le64_to_cpu(pointer);
    pointer += (unsigned long)src_file->data;
    pointer = cpu_to_le64(pointer);
    memcpy(dest_file->data + offset, &pointer, entry->pointer.size);

    بازگشت;
err:
    warn_internalerror();
}

ایستا تهی romfile_loader_add_checksum(ساختار romfile_loader_entry_s *entry,
                                        ساختار romfile_loader_files *files)
{
    ساختار romfile_loader_file *file;
    unsigned offset = le32_to_cpu(entry->cksum.offset);
    unsigned start = le32_to_cpu(entry->cksum.start);
    unsigned len = le32_to_cpu(entry->cksum.length);
    u8 *data;

    file = romfile_loader_find(entry->cksum.file, files);

    اگر (!file || !file->data || offset >= file->file->size ||
        start + len < start || start + len > file->file->size)
        برو err;

    data = file->data + offset;
    *data -= checksum(file->data + start, len);

    بازگشت;
err:
    warn_internalerror();
}

ایستا تهی romfile_loader_write_pointer(ساختار romfile_loader_entry_s *entry,
                                         ساختار romfile_loader_files *files)
{
    ساختار romfile_s *dest_file;
    ساختار romfile_loader_file *src_file;
    unsigned dst_offset = le32_to_cpu(entry->wr_pointer.dst_offset);
    unsigned src_offset = le32_to_cpu(entry->wr_pointer.src_offset);
    u64 pointer = 0;

    /* Writing back to a file that may not be loaded in RAM */
    dest_file = romfile_find(entry->wr_pointer.dest_file);
    src_file = romfile_loader_find(entry->wr_pointer.src_file, files);

    اگر (!dest_file || !src_file || !src_file->data ||
        dst_offset + entry->wr_pointer.size < dst_offset ||
        dst_offset + entry->wr_pointer.size > dest_file->size ||
        src_offset >= src_file->file->size ||
        entry->wr_pointer.size < 1 || entry->wr_pointer.size > 8 ||
        entry->wr_pointer.size & (entry->wr_pointer.size - 1)) {
        برو err;
    }

    pointer = (unsigned long)src_file->data + src_offset;
    /* Make sure the pointer fits within wr_pointer.size */
    اگر ((entry->wr_pointer.size != اندازه(u64)) &&
        ((pointer >> (entry->wr_pointer.size * 8)) > 0)) {
        برو err;
    }
    pointer = cpu_to_le64(pointer);

    /* Only supported on QEMU */
    اگر (qemu_cfg_write_file(&pointer, dest_file, dst_offset,
                            entry->wr_pointer.size) != entry->wr_pointer.size) {
        برو err;
    }

    /* Store the info so it can replayed later if necessary */
    ساختار romfile_wr_pointer_entry *store = malloc_بالا(اندازه(*store));
    اگر (!store) {
        warn_noalloc();
        بازگشت;
    }
    store->pointer = pointer;
    store->کلید = qemu_get_romfile_کلید(dest_file);
    store->offset = dst_offset;
    store->ptr_size = entry->wr_pointer.size;
    hlist_add_head(&store->گره, &romfile_pointer_list);

    بازگشت;
 err:
    warn_internalerror();
}

صحیح romfile_loader_execute(ثابت نویسه *name)
{
    ساختار romfile_loader_entry_s *entry;
    صحیح size, offset = 0, nfiles;
    ساختار romfile_loader_files *files;
    تهی *data = romfile_loadfile(name, &size);
    اگر (!data)
        بازگشت -1;

    اگر (size % اندازه(*entry)) {
        warn_internalerror();
        برو err;
    }

    /* (over)estimate the number of files to load. */
    nfiles = size / اندازه(*entry);
    files = malloc_tmp(اندازه(*files) + nfiles * اندازه(files->files[0]));
    اگر (!files) {
        warn_noalloc();
        برو err;
    }
    files->nfiles = 0;

    برای (offset = 0; offset < size; offset += اندازه(*entry)) {
        entry = data + offset;
        انتخاب (le32_to_cpu(entry->command)) {
                حالت ROMFILE_LOADER_COMMAND_ALLOCATE:
                        romfile_loader_allocate(entry, files);
                        بشکن;
                حالت ROMFILE_LOADER_COMMAND_ADD_POINTER:
                        romfile_loader_add_pointer(entry, files);
                        بشکن;
                حالت ROMFILE_LOADER_COMMAND_ADD_CHECKSUM:
                        romfile_loader_add_checksum(entry, files);
                        بشکن;
                حالت ROMFILE_LOADER_COMMAND_WRITE_POINTER:
                        romfile_loader_write_pointer(entry, files);
                        بشکن;
                پیشفرض:
                        /* Skip commands that we don't recognize. */
                        بشکن;
        }
    }

    آزاد(files);
    آزاد(data);
    بازگشت 0;

err:
    آزاد(data);
    بازگشت -1;
}
