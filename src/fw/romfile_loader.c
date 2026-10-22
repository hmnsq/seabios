#அந்தர்பாவ "romfile_loader.h"
#அந்தர்பாவ "byteorder.h" // leXX_to_cpu/cpu_to_leXX
#அந்தர்பாவ "util.h" // checksum
#அந்தர்பாவ "string.h" // strcmp
#அந்தர்பாவ "romfile.h" // struct romfile_s
#அந்தர்பாவ "malloc.h" // Zone*, _malloc
#அந்தர்பாவ "list.h" // struct hlist_node
#அந்தர்பாவ "output.h" // warn_*
#அந்தர்பாவ "paravirt.h" // qemu_cfg_write_file

கட்டமைப்பு romfile_loader_file {
    கட்டமைப்பு romfile_s *file;
    வெறுமை *data;
};
கட்டமைப்பு romfile_loader_files {
    முழு nfiles;
    கட்டமைப்பு romfile_loader_file files[];
};

// Data structures for storing "write pointer" entries for possible replay
கட்டமைப்பு romfile_wr_pointer_entry {
    u64 pointer;
    u32 offset;
    u16 சாவி;
    u8 ptr_size;
    கட்டமைப்பு hlist_முனை முனை;
};
நிலையான கட்டமைப்பு hlist_head romfile_pointer_list;

நிலையான கட்டமைப்பு romfile_loader_file *
romfile_loader_find(மாறிலி எழுத்து *name,
                    கட்டமைப்பு romfile_loader_files *files)
{
    முழு i;
    எனில் (name[ROMFILE_LOADER_FILESZ - 1])
        திரும்பு வெற்று;
    க்கு (i = 0; i < files->nfiles; ++i)
        எனில் (!strcmp(files->files[i].file->name, name))
            திரும்பு &files->files[i];
    திரும்பு வெற்று;
}

// Replay "write pointer" entries back to QEMU
வெறுமை romfile_fw_cfg_resume(வெறுமை)
{
    எனில் (!CONFIG_QEMU)
        திரும்பு;

    கட்டமைப்பு romfile_wr_pointer_entry *entry;
    hlist_for_each_entry(entry, &romfile_pointer_list, முனை) {
        qemu_cfg_write_file_simple(&entry->pointer, entry->சாவி,
                                   entry->offset, entry->ptr_size);
    }
}

நிலையான வெறுமை romfile_loader_allocate(கட்டமைப்பு romfile_loader_entry_s *entry,
                                    கட்டமைப்பு romfile_loader_files *files)
{
    கட்டமைப்பு zone_s *zone;
    கட்டமைப்பு romfile_loader_file *file = &files->files[files->nfiles];
    வெறுமை *data;
    முழு ret;
    unsigned alloc_align = le32_to_cpu(entry->alloc.align);

    எனில் (alloc_align & (alloc_align - 1))
        செல் err;

    தேர்வு (entry->alloc.zone) {
        நிலை ROMFILE_LOADER_ALLOC_ZONE_உயர்வு:
            zone = &Zone_உயர்வு;
            நிறுத்து;
        நிலை ROMFILE_LOADER_ALLOC_ZONE_FSEG:
            zone = &ZoneFSeg;
            நிறுத்து;
        இயல்பு:
            செல் err;
    }
    எனில் (alloc_align < MALLOC_MIN_ALIGN)
        alloc_align = MALLOC_MIN_ALIGN;
    எனில் (entry->alloc.file[ROMFILE_LOADER_FILESZ - 1])
        செல் err;
    file->file = romfile_find(entry->alloc.file);
    எனில் (!file->file || !file->file->size)
        திரும்பு;
    data = _malloc(zone, file->file->size, alloc_align);
    எனில் (!data) {
        warn_noalloc();
        திரும்பு;
    }
    ret = file->file->copy(file->file, data, file->file->size);
    எனில் (ret != file->file->size)
        செல் file_err;
    file->data = data;
    files->nfiles++;
    திரும்பு;

file_err:
    விடுவி(data);
err:
    warn_internalerror();
}

நிலையான வெறுமை romfile_loader_add_pointer(கட்டமைப்பு romfile_loader_entry_s *entry,
                                       கட்டமைப்பு romfile_loader_files *files)
{
    கட்டமைப்பு romfile_loader_file *dest_file;
    கட்டமைப்பு romfile_loader_file *src_file;
    unsigned offset = le32_to_cpu(entry->pointer.offset);
    u64 pointer = 0;

    dest_file = romfile_loader_find(entry->pointer.dest_file, files);
    src_file = romfile_loader_find(entry->pointer.src_file, files);

    எனில் (!dest_file || !src_file || !dest_file->data || !src_file->data ||
        offset + entry->pointer.size < offset ||
        offset + entry->pointer.size > dest_file->file->size ||
        entry->pointer.size < 1 || entry->pointer.size > 8 ||
        entry->pointer.size & (entry->pointer.size - 1))
        செல் err;

    memcpy(&pointer, dest_file->data + offset, entry->pointer.size);
    pointer = le64_to_cpu(pointer);
    pointer += (unsigned long)src_file->data;
    pointer = cpu_to_le64(pointer);
    memcpy(dest_file->data + offset, &pointer, entry->pointer.size);

    திரும்பு;
err:
    warn_internalerror();
}

நிலையான வெறுமை romfile_loader_add_checksum(கட்டமைப்பு romfile_loader_entry_s *entry,
                                        கட்டமைப்பு romfile_loader_files *files)
{
    கட்டமைப்பு romfile_loader_file *file;
    unsigned offset = le32_to_cpu(entry->cksum.offset);
    unsigned start = le32_to_cpu(entry->cksum.start);
    unsigned len = le32_to_cpu(entry->cksum.length);
    u8 *data;

    file = romfile_loader_find(entry->cksum.file, files);

    எனில் (!file || !file->data || offset >= file->file->size ||
        start + len < start || start + len > file->file->size)
        செல் err;

    data = file->data + offset;
    *data -= checksum(file->data + start, len);

    திரும்பு;
err:
    warn_internalerror();
}

நிலையான வெறுமை romfile_loader_write_pointer(கட்டமைப்பு romfile_loader_entry_s *entry,
                                         கட்டமைப்பு romfile_loader_files *files)
{
    கட்டமைப்பு romfile_s *dest_file;
    கட்டமைப்பு romfile_loader_file *src_file;
    unsigned dst_offset = le32_to_cpu(entry->wr_pointer.dst_offset);
    unsigned src_offset = le32_to_cpu(entry->wr_pointer.src_offset);
    u64 pointer = 0;

    /* Writing back to a file that may not be loaded in RAM */
    dest_file = romfile_find(entry->wr_pointer.dest_file);
    src_file = romfile_loader_find(entry->wr_pointer.src_file, files);

    எனில் (!dest_file || !src_file || !src_file->data ||
        dst_offset + entry->wr_pointer.size < dst_offset ||
        dst_offset + entry->wr_pointer.size > dest_file->size ||
        src_offset >= src_file->file->size ||
        entry->wr_pointer.size < 1 || entry->wr_pointer.size > 8 ||
        entry->wr_pointer.size & (entry->wr_pointer.size - 1)) {
        செல் err;
    }

    pointer = (unsigned long)src_file->data + src_offset;
    /* Make sure the pointer fits within wr_pointer.size */
    எனில் ((entry->wr_pointer.size != அளவு(u64)) &&
        ((pointer >> (entry->wr_pointer.size * 8)) > 0)) {
        செல் err;
    }
    pointer = cpu_to_le64(pointer);

    /* Only supported on QEMU */
    எனில் (qemu_cfg_write_file(&pointer, dest_file, dst_offset,
                            entry->wr_pointer.size) != entry->wr_pointer.size) {
        செல் err;
    }

    /* Store the info so it can replayed later if necessary */
    கட்டமைப்பு romfile_wr_pointer_entry *store = malloc_உயர்வு(அளவு(*store));
    எனில் (!store) {
        warn_noalloc();
        திரும்பு;
    }
    store->pointer = pointer;
    store->சாவி = qemu_get_romfile_சாவி(dest_file);
    store->offset = dst_offset;
    store->ptr_size = entry->wr_pointer.size;
    hlist_add_head(&store->முனை, &romfile_pointer_list);

    திரும்பு;
 err:
    warn_internalerror();
}

முழு romfile_loader_execute(மாறிலி எழுத்து *name)
{
    கட்டமைப்பு romfile_loader_entry_s *entry;
    முழு size, offset = 0, nfiles;
    கட்டமைப்பு romfile_loader_files *files;
    வெறுமை *data = romfile_loadfile(name, &size);
    எனில் (!data)
        திரும்பு -1;

    எனில் (size % அளவு(*entry)) {
        warn_internalerror();
        செல் err;
    }

    /* (over)estimate the number of files to load. */
    nfiles = size / அளவு(*entry);
    files = malloc_tmp(அளவு(*files) + nfiles * அளவு(files->files[0]));
    எனில் (!files) {
        warn_noalloc();
        செல் err;
    }
    files->nfiles = 0;

    க்கு (offset = 0; offset < size; offset += அளவு(*entry)) {
        entry = data + offset;
        தேர்வு (le32_to_cpu(entry->command)) {
                நிலை ROMFILE_LOADER_COMMAND_ALLOCATE:
                        romfile_loader_allocate(entry, files);
                        நிறுத்து;
                நிலை ROMFILE_LOADER_COMMAND_ADD_POINTER:
                        romfile_loader_add_pointer(entry, files);
                        நிறுத்து;
                நிலை ROMFILE_LOADER_COMMAND_ADD_CHECKSUM:
                        romfile_loader_add_checksum(entry, files);
                        நிறுத்து;
                நிலை ROMFILE_LOADER_COMMAND_WRITE_POINTER:
                        romfile_loader_write_pointer(entry, files);
                        நிறுத்து;
                இயல்பு:
                        /* Skip commands that we don't recognize. */
                        நிறுத்து;
        }
    }

    விடுவி(files);
    விடுவி(data);
    திரும்பு 0;

err:
    விடுவி(data);
    திரும்பு -1;
}
