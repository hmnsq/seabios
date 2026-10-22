#ضمن "romfile_loader.h"
#ضمن "byteorder.h" // leXX_to_cpu/cpu_to_leXX
#ضمن "util.h" // checksum
#ضمن "string.h" // strcmp
#ضمن "romfile.h" // struct romfile_s
#ضمن "malloc.h" // Zone*, _malloc
#ضمن "list.h" // struct hlist_node
#ضمن "output.h" // warn_*
#ضمن "paravirt.h" // qemu_cfg_write_file

بنية romfile_loader_file {
    بنية romfile_s *file;
    فراغ *data;
};
بنية romfile_loader_files {
    صحيح nfiles;
    بنية romfile_loader_file files[];
};

// Data structures for storing "write pointer" entries for possible replay
بنية romfile_wr_pointer_entry {
    u64 pointer;
    u32 offset;
    u16 مفتاح;
    u8 ptr_size;
    بنية hlist_عقدة عقدة;
};
ساكن بنية hlist_head romfile_pointer_list;

ساكن بنية romfile_loader_file *
romfile_loader_find(ثابت حرف *name,
                    بنية romfile_loader_files *files)
{
    صحيح i;
    إذا (name[ROMFILE_LOADER_FILESZ - 1])
        أرجع عدم;
    لكل (i = 0; i < files->nfiles; ++i)
        إذا (!strcmp(files->files[i].file->name, name))
            أرجع &files->files[i];
    أرجع عدم;
}

// Replay "write pointer" entries back to QEMU
فراغ romfile_fw_cfg_resume(فراغ)
{
    إذا (!CONFIG_QEMU)
        أرجع;

    بنية romfile_wr_pointer_entry *entry;
    hlist_for_each_entry(entry, &romfile_pointer_list, عقدة) {
        qemu_cfg_write_file_simple(&entry->pointer, entry->مفتاح,
                                   entry->offset, entry->ptr_size);
    }
}

ساكن فراغ romfile_loader_allocate(بنية romfile_loader_entry_s *entry,
                                    بنية romfile_loader_files *files)
{
    بنية zone_s *zone;
    بنية romfile_loader_file *file = &files->files[files->nfiles];
    فراغ *data;
    صحيح ret;
    unsigned alloc_align = le32_to_cpu(entry->alloc.align);

    إذا (alloc_align & (alloc_align - 1))
        اذهب err;

    بدل (entry->alloc.zone) {
        حالة ROMFILE_LOADER_ALLOC_ZONE_عالي:
            zone = &Zone_عالي;
            اكسر;
        حالة ROMFILE_LOADER_ALLOC_ZONE_FSEG:
            zone = &ZoneFSeg;
            اكسر;
        افتراضي:
            اذهب err;
    }
    إذا (alloc_align < MALLOC_MIN_ALIGN)
        alloc_align = MALLOC_MIN_ALIGN;
    إذا (entry->alloc.file[ROMFILE_LOADER_FILESZ - 1])
        اذهب err;
    file->file = romfile_find(entry->alloc.file);
    إذا (!file->file || !file->file->size)
        أرجع;
    data = _malloc(zone, file->file->size, alloc_align);
    إذا (!data) {
        warn_noalloc();
        أرجع;
    }
    ret = file->file->copy(file->file, data, file->file->size);
    إذا (ret != file->file->size)
        اذهب file_err;
    file->data = data;
    files->nfiles++;
    أرجع;

file_err:
    حرر(data);
err:
    warn_internalerror();
}

ساكن فراغ romfile_loader_add_pointer(بنية romfile_loader_entry_s *entry,
                                       بنية romfile_loader_files *files)
{
    بنية romfile_loader_file *dest_file;
    بنية romfile_loader_file *src_file;
    unsigned offset = le32_to_cpu(entry->pointer.offset);
    u64 pointer = 0;

    dest_file = romfile_loader_find(entry->pointer.dest_file, files);
    src_file = romfile_loader_find(entry->pointer.src_file, files);

    إذا (!dest_file || !src_file || !dest_file->data || !src_file->data ||
        offset + entry->pointer.size < offset ||
        offset + entry->pointer.size > dest_file->file->size ||
        entry->pointer.size < 1 || entry->pointer.size > 8 ||
        entry->pointer.size & (entry->pointer.size - 1))
        اذهب err;

    memcpy(&pointer, dest_file->data + offset, entry->pointer.size);
    pointer = le64_to_cpu(pointer);
    pointer += (unsigned long)src_file->data;
    pointer = cpu_to_le64(pointer);
    memcpy(dest_file->data + offset, &pointer, entry->pointer.size);

    أرجع;
err:
    warn_internalerror();
}

ساكن فراغ romfile_loader_add_checksum(بنية romfile_loader_entry_s *entry,
                                        بنية romfile_loader_files *files)
{
    بنية romfile_loader_file *file;
    unsigned offset = le32_to_cpu(entry->cksum.offset);
    unsigned start = le32_to_cpu(entry->cksum.start);
    unsigned len = le32_to_cpu(entry->cksum.length);
    u8 *data;

    file = romfile_loader_find(entry->cksum.file, files);

    إذا (!file || !file->data || offset >= file->file->size ||
        start + len < start || start + len > file->file->size)
        اذهب err;

    data = file->data + offset;
    *data -= checksum(file->data + start, len);

    أرجع;
err:
    warn_internalerror();
}

ساكن فراغ romfile_loader_write_pointer(بنية romfile_loader_entry_s *entry,
                                         بنية romfile_loader_files *files)
{
    بنية romfile_s *dest_file;
    بنية romfile_loader_file *src_file;
    unsigned dst_offset = le32_to_cpu(entry->wr_pointer.dst_offset);
    unsigned src_offset = le32_to_cpu(entry->wr_pointer.src_offset);
    u64 pointer = 0;

    /* Writing back to a file that may not be loaded in RAM */
    dest_file = romfile_find(entry->wr_pointer.dest_file);
    src_file = romfile_loader_find(entry->wr_pointer.src_file, files);

    إذا (!dest_file || !src_file || !src_file->data ||
        dst_offset + entry->wr_pointer.size < dst_offset ||
        dst_offset + entry->wr_pointer.size > dest_file->size ||
        src_offset >= src_file->file->size ||
        entry->wr_pointer.size < 1 || entry->wr_pointer.size > 8 ||
        entry->wr_pointer.size & (entry->wr_pointer.size - 1)) {
        اذهب err;
    }

    pointer = (unsigned long)src_file->data + src_offset;
    /* Make sure the pointer fits within wr_pointer.size */
    إذا ((entry->wr_pointer.size != حجم(u64)) &&
        ((pointer >> (entry->wr_pointer.size * 8)) > 0)) {
        اذهب err;
    }
    pointer = cpu_to_le64(pointer);

    /* Only supported on QEMU */
    إذا (qemu_cfg_write_file(&pointer, dest_file, dst_offset,
                            entry->wr_pointer.size) != entry->wr_pointer.size) {
        اذهب err;
    }

    /* Store the info so it can replayed later if necessary */
    بنية romfile_wr_pointer_entry *store = malloc_عالي(حجم(*store));
    إذا (!store) {
        warn_noalloc();
        أرجع;
    }
    store->pointer = pointer;
    store->مفتاح = qemu_get_romfile_مفتاح(dest_file);
    store->offset = dst_offset;
    store->ptr_size = entry->wr_pointer.size;
    hlist_add_head(&store->عقدة, &romfile_pointer_list);

    أرجع;
 err:
    warn_internalerror();
}

صحيح romfile_loader_execute(ثابت حرف *name)
{
    بنية romfile_loader_entry_s *entry;
    صحيح size, offset = 0, nfiles;
    بنية romfile_loader_files *files;
    فراغ *data = romfile_loadfile(name, &size);
    إذا (!data)
        أرجع -1;

    إذا (size % حجم(*entry)) {
        warn_internalerror();
        اذهب err;
    }

    /* (over)estimate the number of files to load. */
    nfiles = size / حجم(*entry);
    files = malloc_tmp(حجم(*files) + nfiles * حجم(files->files[0]));
    إذا (!files) {
        warn_noalloc();
        اذهب err;
    }
    files->nfiles = 0;

    لكل (offset = 0; offset < size; offset += حجم(*entry)) {
        entry = data + offset;
        بدل (le32_to_cpu(entry->command)) {
                حالة ROMFILE_LOADER_COMMAND_ALLOCATE:
                        romfile_loader_allocate(entry, files);
                        اكسر;
                حالة ROMFILE_LOADER_COMMAND_ADD_POINTER:
                        romfile_loader_add_pointer(entry, files);
                        اكسر;
                حالة ROMFILE_LOADER_COMMAND_ADD_CHECKSUM:
                        romfile_loader_add_checksum(entry, files);
                        اكسر;
                حالة ROMFILE_LOADER_COMMAND_WRITE_POINTER:
                        romfile_loader_write_pointer(entry, files);
                        اكسر;
                افتراضي:
                        /* Skip commands that we don't recognize. */
                        اكسر;
        }
    }

    حرر(files);
    حرر(data);
    أرجع 0;

err:
    حرر(data);
    أرجع -1;
}
