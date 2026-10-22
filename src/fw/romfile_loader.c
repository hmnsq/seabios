#অন্তর্ভাৱ "romfile_loader.h"
#অন্তর্ভাৱ "byteorder.h" // leXX_to_cpu/cpu_to_leXX
#অন্তর্ভাৱ "util.h" // checksum
#অন্তর্ভাৱ "string.h" // strcmp
#অন্তর্ভাৱ "romfile.h" // struct romfile_s
#অন্তর্ভাৱ "malloc.h" // Zone*, _malloc
#অন্তর্ভাৱ "list.h" // struct hlist_node
#অন্তর্ভাৱ "output.h" // warn_*
#অন্তর্ভাৱ "paravirt.h" // qemu_cfg_write_file

সংরচনা romfile_loader_file {
    সংরচনা romfile_s *file;
    শূন্য *data;
};
সংরচনা romfile_loader_files {
    পূর্ণাঙ্ক nfiles;
    সংরচনা romfile_loader_file files[];
};

// Data structures for storing "write pointer" entries for possible replay
সংরচনা romfile_wr_pointer_entry {
    u64 pointer;
    u32 offset;
    u16 চাবি;
    u8 ptr_size;
    সংরচনা hlist_গ্ৰন্থি গ্ৰন্থি;
};
স্থির সংরচনা hlist_head romfile_pointer_list;

স্থির সংরচনা romfile_loader_file *
romfile_loader_find(ধ্রুৱ ৱর্ণ *name,
                    সংরচনা romfile_loader_files *files)
{
    পূর্ণাঙ্ক i;
    যদি (name[ROMFILE_LOADER_FILESZ - 1])
        ঘূৰাই রিক্ত;
    বাবে (i = 0; i < files->nfiles; ++i)
        যদি (!strcmp(files->files[i].file->name, name))
            ঘূৰাই &files->files[i];
    ঘূৰাই রিক্ত;
}

// Replay "write pointer" entries back to QEMU
শূন্য romfile_fw_cfg_resume(শূন্য)
{
    যদি (!CONFIG_QEMU)
        ঘূৰাই;

    সংরচনা romfile_wr_pointer_entry *entry;
    hlist_for_each_entry(entry, &romfile_pointer_list, গ্ৰন্থি) {
        qemu_cfg_write_file_simple(&entry->pointer, entry->চাবি,
                                   entry->offset, entry->ptr_size);
    }
}

স্থির শূন্য romfile_loader_allocate(সংরচনা romfile_loader_entry_s *entry,
                                    সংরচনা romfile_loader_files *files)
{
    সংরচনা zone_s *zone;
    সংরচনা romfile_loader_file *file = &files->files[files->nfiles];
    শূন্য *data;
    পূর্ণাঙ্ক ret;
    unsigned alloc_align = le32_to_cpu(entry->alloc.align);

    যদি (alloc_align & (alloc_align - 1))
        গচ্ছ err;

    ৱিকল্প (entry->alloc.zone) {
        পক্ষ ROMFILE_LOADER_ALLOC_ZONE_উপৰেৰ:
            zone = &Zone_উপৰেৰ;
            ভঙ্গ;
        পক্ষ ROMFILE_LOADER_ALLOC_ZONE_FSEG:
            zone = &ZoneFSeg;
            ভঙ্গ;
        স্ৱভাৱ:
            গচ্ছ err;
    }
    যদি (alloc_align < MALLOC_MIN_ALIGN)
        alloc_align = MALLOC_MIN_ALIGN;
    যদি (entry->alloc.file[ROMFILE_LOADER_FILESZ - 1])
        গচ্ছ err;
    file->file = romfile_find(entry->alloc.file);
    যদি (!file->file || !file->file->size)
        ঘূৰাই;
    data = _malloc(zone, file->file->size, alloc_align);
    যদি (!data) {
        warn_noalloc();
        ঘূৰাই;
    }
    ret = file->file->copy(file->file, data, file->file->size);
    যদি (ret != file->file->size)
        গচ্ছ file_err;
    file->data = data;
    files->nfiles++;
    ঘূৰাই;

file_err:
    মুক্ত(data);
err:
    warn_internalerror();
}

স্থির শূন্য romfile_loader_add_pointer(সংরচনা romfile_loader_entry_s *entry,
                                       সংরচনা romfile_loader_files *files)
{
    সংরচনা romfile_loader_file *dest_file;
    সংরচনা romfile_loader_file *src_file;
    unsigned offset = le32_to_cpu(entry->pointer.offset);
    u64 pointer = 0;

    dest_file = romfile_loader_find(entry->pointer.dest_file, files);
    src_file = romfile_loader_find(entry->pointer.src_file, files);

    যদি (!dest_file || !src_file || !dest_file->data || !src_file->data ||
        offset + entry->pointer.size < offset ||
        offset + entry->pointer.size > dest_file->file->size ||
        entry->pointer.size < 1 || entry->pointer.size > 8 ||
        entry->pointer.size & (entry->pointer.size - 1))
        গচ্ছ err;

    memcpy(&pointer, dest_file->data + offset, entry->pointer.size);
    pointer = le64_to_cpu(pointer);
    pointer += (unsigned long)src_file->data;
    pointer = cpu_to_le64(pointer);
    memcpy(dest_file->data + offset, &pointer, entry->pointer.size);

    ঘূৰাই;
err:
    warn_internalerror();
}

স্থির শূন্য romfile_loader_add_checksum(সংরচনা romfile_loader_entry_s *entry,
                                        সংরচনা romfile_loader_files *files)
{
    সংরচনা romfile_loader_file *file;
    unsigned offset = le32_to_cpu(entry->cksum.offset);
    unsigned start = le32_to_cpu(entry->cksum.start);
    unsigned len = le32_to_cpu(entry->cksum.length);
    u8 *data;

    file = romfile_loader_find(entry->cksum.file, files);

    যদি (!file || !file->data || offset >= file->file->size ||
        start + len < start || start + len > file->file->size)
        গচ্ছ err;

    data = file->data + offset;
    *data -= checksum(file->data + start, len);

    ঘূৰাই;
err:
    warn_internalerror();
}

স্থির শূন্য romfile_loader_write_pointer(সংরচনা romfile_loader_entry_s *entry,
                                         সংরচনা romfile_loader_files *files)
{
    সংরচনা romfile_s *dest_file;
    সংরচনা romfile_loader_file *src_file;
    unsigned dst_offset = le32_to_cpu(entry->wr_pointer.dst_offset);
    unsigned src_offset = le32_to_cpu(entry->wr_pointer.src_offset);
    u64 pointer = 0;

    /* Writing back to a file that may not be loaded in RAM */
    dest_file = romfile_find(entry->wr_pointer.dest_file);
    src_file = romfile_loader_find(entry->wr_pointer.src_file, files);

    যদি (!dest_file || !src_file || !src_file->data ||
        dst_offset + entry->wr_pointer.size < dst_offset ||
        dst_offset + entry->wr_pointer.size > dest_file->size ||
        src_offset >= src_file->file->size ||
        entry->wr_pointer.size < 1 || entry->wr_pointer.size > 8 ||
        entry->wr_pointer.size & (entry->wr_pointer.size - 1)) {
        গচ্ছ err;
    }

    pointer = (unsigned long)src_file->data + src_offset;
    /* Make sure the pointer fits within wr_pointer.size */
    যদি ((entry->wr_pointer.size != পরিমাণ(u64)) &&
        ((pointer >> (entry->wr_pointer.size * 8)) > 0)) {
        গচ্ছ err;
    }
    pointer = cpu_to_le64(pointer);

    /* Only supported on QEMU */
    যদি (qemu_cfg_write_file(&pointer, dest_file, dst_offset,
                            entry->wr_pointer.size) != entry->wr_pointer.size) {
        গচ্ছ err;
    }

    /* Store the info so it can replayed later if necessary */
    সংরচনা romfile_wr_pointer_entry *store = malloc_উপৰেৰ(পরিমাণ(*store));
    যদি (!store) {
        warn_noalloc();
        ঘূৰাই;
    }
    store->pointer = pointer;
    store->চাবি = qemu_get_romfile_চাবি(dest_file);
    store->offset = dst_offset;
    store->ptr_size = entry->wr_pointer.size;
    hlist_add_head(&store->গ্ৰন্থি, &romfile_pointer_list);

    ঘূৰাই;
 err:
    warn_internalerror();
}

পূর্ণাঙ্ক romfile_loader_execute(ধ্রুৱ ৱর্ণ *name)
{
    সংরচনা romfile_loader_entry_s *entry;
    পূর্ণাঙ্ক size, offset = 0, nfiles;
    সংরচনা romfile_loader_files *files;
    শূন্য *data = romfile_loadfile(name, &size);
    যদি (!data)
        ঘূৰাই -1;

    যদি (size % পরিমাণ(*entry)) {
        warn_internalerror();
        গচ্ছ err;
    }

    /* (over)estimate the number of files to load. */
    nfiles = size / পরিমাণ(*entry);
    files = malloc_tmp(পরিমাণ(*files) + nfiles * পরিমাণ(files->files[0]));
    যদি (!files) {
        warn_noalloc();
        গচ্ছ err;
    }
    files->nfiles = 0;

    বাবে (offset = 0; offset < size; offset += পরিমাণ(*entry)) {
        entry = data + offset;
        ৱিকল্প (le32_to_cpu(entry->command)) {
                পক্ষ ROMFILE_LOADER_COMMAND_ALLOCATE:
                        romfile_loader_allocate(entry, files);
                        ভঙ্গ;
                পক্ষ ROMFILE_LOADER_COMMAND_ADD_POINTER:
                        romfile_loader_add_pointer(entry, files);
                        ভঙ্গ;
                পক্ষ ROMFILE_LOADER_COMMAND_ADD_CHECKSUM:
                        romfile_loader_add_checksum(entry, files);
                        ভঙ্গ;
                পক্ষ ROMFILE_LOADER_COMMAND_WRITE_POINTER:
                        romfile_loader_write_pointer(entry, files);
                        ভঙ্গ;
                স্ৱভাৱ:
                        /* Skip commands that we don't recognize. */
                        ভঙ্গ;
        }
    }

    মুক্ত(files);
    মুক্ত(data);
    ঘূৰাই 0;

err:
    মুক্ত(data);
    ঘূৰাই -1;
}
