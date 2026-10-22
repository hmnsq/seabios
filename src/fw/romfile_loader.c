#അന്തര്ഭാവ "romfile_loader.h"
#അന്തര്ഭാവ "byteorder.h" // leXX_to_cpu/cpu_to_leXX
#അന്തര്ഭാവ "util.h" // checksum
#അന്തര്ഭാവ "string.h" // strcmp
#അന്തര്ഭാവ "romfile.h" // struct romfile_s
#അന്തര്ഭാവ "malloc.h" // Zone*, _malloc
#അന്തര്ഭാവ "list.h" // struct hlist_node
#അന്തര്ഭാവ "output.h" // warn_*
#അന്തര്ഭാവ "paravirt.h" // qemu_cfg_write_file

ഘടന romfile_loader_file {
    ഘടന romfile_s *file;
    ശൂന്യം *data;
};
ഘടന romfile_loader_files {
    പൂർണ്ണസംഖ്യ nfiles;
    ഘടന romfile_loader_file files[];
};

// Data structures for storing "write pointer" entries for possible replay
ഘടന romfile_wr_pointer_entry {
    u64 pointer;
    u32 offset;
    u16 താക്കോൽ;
    u8 ptr_size;
    ഘടന hlist_കണ്ണി കണ്ണി;
};
സ്ഥിതം ഘടന hlist_head romfile_pointer_list;

സ്ഥിതം ഘടന romfile_loader_file *
romfile_loader_find(സ്ഥിരം അക്ഷരം *name,
                    ഘടന romfile_loader_files *files)
{
    പൂർണ്ണസംഖ്യ i;
    എങ്കിൽ (name[ROMFILE_LOADER_FILESZ - 1])
        തിരികെ ശൂന്യ;
    വേണ്ടി (i = 0; i < files->nfiles; ++i)
        എങ്കിൽ (!strcmp(files->files[i].file->name, name))
            തിരികെ &files->files[i];
    തിരികെ ശൂന്യ;
}

// Replay "write pointer" entries back to QEMU
ശൂന്യം romfile_fw_cfg_resume(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_QEMU)
        തിരികെ;

    ഘടന romfile_wr_pointer_entry *entry;
    hlist_for_each_entry(entry, &romfile_pointer_list, കണ്ണി) {
        qemu_cfg_write_file_simple(&entry->pointer, entry->താക്കോൽ,
                                   entry->offset, entry->ptr_size);
    }
}

സ്ഥിതം ശൂന്യം romfile_loader_allocate(ഘടന romfile_loader_entry_s *entry,
                                    ഘടന romfile_loader_files *files)
{
    ഘടന zone_s *zone;
    ഘടന romfile_loader_file *file = &files->files[files->nfiles];
    ശൂന്യം *data;
    പൂർണ്ണസംഖ്യ ret;
    unsigned alloc_align = le32_to_cpu(entry->alloc.align);

    എങ്കിൽ (alloc_align & (alloc_align - 1))
        പോകുക err;

    തിരഞ്ഞെടുപ്പ് (entry->alloc.zone) {
        അവസ്ഥ ROMFILE_LOADER_ALLOC_ZONE_മുകളിലെ:
            zone = &Zone_മുകളിലെ;
            നിർത്തുക;
        അവസ്ഥ ROMFILE_LOADER_ALLOC_ZONE_FSEG:
            zone = &ZoneFSeg;
            നിർത്തുക;
        സ്ഥിരസ്ഥിതി:
            പോകുക err;
    }
    എങ്കിൽ (alloc_align < MALLOC_MIN_ALIGN)
        alloc_align = MALLOC_MIN_ALIGN;
    എങ്കിൽ (entry->alloc.file[ROMFILE_LOADER_FILESZ - 1])
        പോകുക err;
    file->file = romfile_find(entry->alloc.file);
    എങ്കിൽ (!file->file || !file->file->size)
        തിരികെ;
    data = _malloc(zone, file->file->size, alloc_align);
    എങ്കിൽ (!data) {
        warn_noalloc();
        തിരികെ;
    }
    ret = file->file->copy(file->file, data, file->file->size);
    എങ്കിൽ (ret != file->file->size)
        പോകുക file_err;
    file->data = data;
    files->nfiles++;
    തിരികെ;

file_err:
    സ്വതന്ത്രം(data);
err:
    warn_internalerror();
}

സ്ഥിതം ശൂന്യം romfile_loader_add_pointer(ഘടന romfile_loader_entry_s *entry,
                                       ഘടന romfile_loader_files *files)
{
    ഘടന romfile_loader_file *dest_file;
    ഘടന romfile_loader_file *src_file;
    unsigned offset = le32_to_cpu(entry->pointer.offset);
    u64 pointer = 0;

    dest_file = romfile_loader_find(entry->pointer.dest_file, files);
    src_file = romfile_loader_find(entry->pointer.src_file, files);

    എങ്കിൽ (!dest_file || !src_file || !dest_file->data || !src_file->data ||
        offset + entry->pointer.size < offset ||
        offset + entry->pointer.size > dest_file->file->size ||
        entry->pointer.size < 1 || entry->pointer.size > 8 ||
        entry->pointer.size & (entry->pointer.size - 1))
        പോകുക err;

    memcpy(&pointer, dest_file->data + offset, entry->pointer.size);
    pointer = le64_to_cpu(pointer);
    pointer += (unsigned long)src_file->data;
    pointer = cpu_to_le64(pointer);
    memcpy(dest_file->data + offset, &pointer, entry->pointer.size);

    തിരികെ;
err:
    warn_internalerror();
}

സ്ഥിതം ശൂന്യം romfile_loader_add_checksum(ഘടന romfile_loader_entry_s *entry,
                                        ഘടന romfile_loader_files *files)
{
    ഘടന romfile_loader_file *file;
    unsigned offset = le32_to_cpu(entry->cksum.offset);
    unsigned start = le32_to_cpu(entry->cksum.start);
    unsigned len = le32_to_cpu(entry->cksum.length);
    u8 *data;

    file = romfile_loader_find(entry->cksum.file, files);

    എങ്കിൽ (!file || !file->data || offset >= file->file->size ||
        start + len < start || start + len > file->file->size)
        പോകുക err;

    data = file->data + offset;
    *data -= checksum(file->data + start, len);

    തിരികെ;
err:
    warn_internalerror();
}

സ്ഥിതം ശൂന്യം romfile_loader_write_pointer(ഘടന romfile_loader_entry_s *entry,
                                         ഘടന romfile_loader_files *files)
{
    ഘടന romfile_s *dest_file;
    ഘടന romfile_loader_file *src_file;
    unsigned dst_offset = le32_to_cpu(entry->wr_pointer.dst_offset);
    unsigned src_offset = le32_to_cpu(entry->wr_pointer.src_offset);
    u64 pointer = 0;

    /* Writing back to a file that may not be loaded in RAM */
    dest_file = romfile_find(entry->wr_pointer.dest_file);
    src_file = romfile_loader_find(entry->wr_pointer.src_file, files);

    എങ്കിൽ (!dest_file || !src_file || !src_file->data ||
        dst_offset + entry->wr_pointer.size < dst_offset ||
        dst_offset + entry->wr_pointer.size > dest_file->size ||
        src_offset >= src_file->file->size ||
        entry->wr_pointer.size < 1 || entry->wr_pointer.size > 8 ||
        entry->wr_pointer.size & (entry->wr_pointer.size - 1)) {
        പോകുക err;
    }

    pointer = (unsigned long)src_file->data + src_offset;
    /* Make sure the pointer fits within wr_pointer.size */
    എങ്കിൽ ((entry->wr_pointer.size != വലുപ്പം(u64)) &&
        ((pointer >> (entry->wr_pointer.size * 8)) > 0)) {
        പോകുക err;
    }
    pointer = cpu_to_le64(pointer);

    /* Only supported on QEMU */
    എങ്കിൽ (qemu_cfg_write_file(&pointer, dest_file, dst_offset,
                            entry->wr_pointer.size) != entry->wr_pointer.size) {
        പോകുക err;
    }

    /* Store the info so it can replayed later if necessary */
    ഘടന romfile_wr_pointer_entry *store = malloc_മുകളിലെ(വലുപ്പം(*store));
    എങ്കിൽ (!store) {
        warn_noalloc();
        തിരികെ;
    }
    store->pointer = pointer;
    store->താക്കോൽ = qemu_get_romfile_താക്കോൽ(dest_file);
    store->offset = dst_offset;
    store->ptr_size = entry->wr_pointer.size;
    hlist_add_head(&store->കണ്ണി, &romfile_pointer_list);

    തിരികെ;
 err:
    warn_internalerror();
}

പൂർണ്ണസംഖ്യ romfile_loader_execute(സ്ഥിരം അക്ഷരം *name)
{
    ഘടന romfile_loader_entry_s *entry;
    പൂർണ്ണസംഖ്യ size, offset = 0, nfiles;
    ഘടന romfile_loader_files *files;
    ശൂന്യം *data = romfile_loadfile(name, &size);
    എങ്കിൽ (!data)
        തിരികെ -1;

    എങ്കിൽ (size % വലുപ്പം(*entry)) {
        warn_internalerror();
        പോകുക err;
    }

    /* (over)estimate the number of files to load. */
    nfiles = size / വലുപ്പം(*entry);
    files = malloc_tmp(വലുപ്പം(*files) + nfiles * വലുപ്പം(files->files[0]));
    എങ്കിൽ (!files) {
        warn_noalloc();
        പോകുക err;
    }
    files->nfiles = 0;

    വേണ്ടി (offset = 0; offset < size; offset += വലുപ്പം(*entry)) {
        entry = data + offset;
        തിരഞ്ഞെടുപ്പ് (le32_to_cpu(entry->command)) {
                അവസ്ഥ ROMFILE_LOADER_COMMAND_ALLOCATE:
                        romfile_loader_allocate(entry, files);
                        നിർത്തുക;
                അവസ്ഥ ROMFILE_LOADER_COMMAND_ADD_POINTER:
                        romfile_loader_add_pointer(entry, files);
                        നിർത്തുക;
                അവസ്ഥ ROMFILE_LOADER_COMMAND_ADD_CHECKSUM:
                        romfile_loader_add_checksum(entry, files);
                        നിർത്തുക;
                അവസ്ഥ ROMFILE_LOADER_COMMAND_WRITE_POINTER:
                        romfile_loader_write_pointer(entry, files);
                        നിർത്തുക;
                സ്ഥിരസ്ഥിതി:
                        /* Skip commands that we don't recognize. */
                        നിർത്തുക;
        }
    }

    സ്വതന്ത്രം(files);
    സ്വതന്ത്രം(data);
    തിരികെ 0;

err:
    സ്വതന്ത്രം(data);
    തിരികെ -1;
}
