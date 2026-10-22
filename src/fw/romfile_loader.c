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
    पूर्णांक nfiles;
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
romfile_loader_find(स्थिरांक अक्षर *name,
                    संरचना romfile_loader_files *files)
{
    पूर्णांक i;
    जर (name[ROMFILE_LOADER_FILESZ - 1])
        परत रिक्त;
    साठी (i = 0; i < files->nfiles; ++i)
        जर (!म_भेद(files->files[i].file->name, name))
            परत &files->files[i];
    परत रिक्त;
}

// Replay "write pointer" entries back to QEMU
शून्य romfile_fw_cfg_resume(शून्य)
{
    जर (!CONFIG_QEMU)
        परत;

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
    पूर्णांक ret;
    अचिन्हित alloc_align = le32_to_cpu(entry->alloc.align);

    जर (alloc_align & (alloc_align - 1))
        जा err;

    निवड (entry->alloc.zone) {
        स्थिती ROMFILE_LOADER_ALLOC_ZONE_वरचा:
            zone = &Zone_वरचा;
            थांबा;
        स्थिती ROMFILE_LOADER_ALLOC_ZONE_FSEG:
            zone = &ZoneFSeg;
            थांबा;
        पूर्वनिर्धारित:
            जा err;
    }
    जर (alloc_align < MALLOC_MIN_ALIGN)
        alloc_align = MALLOC_MIN_ALIGN;
    जर (entry->alloc.file[ROMFILE_LOADER_FILESZ - 1])
        जा err;
    file->file = romfile_find(entry->alloc.file);
    जर (!file->file || !file->file->size)
        परत;
    data = _malloc(zone, file->file->size, alloc_align);
    जर (!data) {
        warn_noalloc();
        परत;
    }
    ret = file->file->copy(file->file, data, file->file->size);
    जर (ret != file->file->size)
        जा file_err;
    file->data = data;
    files->nfiles++;
    परत;

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
    अचिन्हित offset = le32_to_cpu(entry->pointer.offset);
    u64 pointer = 0;

    dest_file = romfile_loader_find(entry->pointer.dest_file, files);
    src_file = romfile_loader_find(entry->pointer.src_file, files);

    जर (!dest_file || !src_file || !dest_file->data || !src_file->data ||
        offset + entry->pointer.size < offset ||
        offset + entry->pointer.size > dest_file->file->size ||
        entry->pointer.size < 1 || entry->pointer.size > 8 ||
        entry->pointer.size & (entry->pointer.size - 1))
        जा err;

    स_नकल(&pointer, dest_file->data + offset, entry->pointer.size);
    pointer = le64_to_cpu(pointer);
    pointer += (अचिन्हित दीर्घ)src_file->data;
    pointer = cpu_to_le64(pointer);
    स_नकल(dest_file->data + offset, &pointer, entry->pointer.size);

    परत;
err:
    warn_internalerror();
}

स्थिर शून्य romfile_loader_add_checksum(संरचना romfile_loader_entry_s *entry,
                                        संरचना romfile_loader_files *files)
{
    संरचना romfile_loader_file *file;
    अचिन्हित offset = le32_to_cpu(entry->cksum.offset);
    अचिन्हित start = le32_to_cpu(entry->cksum.start);
    अचिन्हित len = le32_to_cpu(entry->cksum.length);
    u8 *data;

    file = romfile_loader_find(entry->cksum.file, files);

    जर (!file || !file->data || offset >= file->file->size ||
        start + len < start || start + len > file->file->size)
        जा err;

    data = file->data + offset;
    *data -= checksum(file->data + start, len);

    परत;
err:
    warn_internalerror();
}

स्थिर शून्य romfile_loader_write_pointer(संरचना romfile_loader_entry_s *entry,
                                         संरचना romfile_loader_files *files)
{
    संरचना romfile_s *dest_file;
    संरचना romfile_loader_file *src_file;
    अचिन्हित dst_offset = le32_to_cpu(entry->wr_pointer.dst_offset);
    अचिन्हित src_offset = le32_to_cpu(entry->wr_pointer.src_offset);
    u64 pointer = 0;

    /* Writing back to a file that may not be loaded in RAM */
    dest_file = romfile_find(entry->wr_pointer.dest_file);
    src_file = romfile_loader_find(entry->wr_pointer.src_file, files);

    जर (!dest_file || !src_file || !src_file->data ||
        dst_offset + entry->wr_pointer.size < dst_offset ||
        dst_offset + entry->wr_pointer.size > dest_file->size ||
        src_offset >= src_file->file->size ||
        entry->wr_pointer.size < 1 || entry->wr_pointer.size > 8 ||
        entry->wr_pointer.size & (entry->wr_pointer.size - 1)) {
        जा err;
    }

    pointer = (अचिन्हित दीर्घ)src_file->data + src_offset;
    /* Make sure the pointer fits within wr_pointer.size */
    जर ((entry->wr_pointer.size != आकार(u64)) &&
        ((pointer >> (entry->wr_pointer.size * 8)) > 0)) {
        जा err;
    }
    pointer = cpu_to_le64(pointer);

    /* Only supported on QEMU */
    जर (qemu_cfg_write_file(&pointer, dest_file, dst_offset,
                            entry->wr_pointer.size) != entry->wr_pointer.size) {
        जा err;
    }

    /* Store the info so it can replayed later if necessary */
    संरचना romfile_wr_pointer_entry *store = malloc_वरचा(आकार(*store));
    जर (!store) {
        warn_noalloc();
        परत;
    }
    store->pointer = pointer;
    store->किल्ली = qemu_get_romfile_किल्ली(dest_file);
    store->offset = dst_offset;
    store->ptr_size = entry->wr_pointer.size;
    hlist_add_head(&store->गाठ, &romfile_pointer_list);

    परत;
 err:
    warn_internalerror();
}

पूर्णांक romfile_loader_execute(स्थिरांक अक्षर *name)
{
    संरचना romfile_loader_entry_s *entry;
    पूर्णांक size, offset = 0, nfiles;
    संरचना romfile_loader_files *files;
    शून्य *data = romfile_loadfile(name, &size);
    जर (!data)
        परत -1;

    जर (size % आकार(*entry)) {
        warn_internalerror();
        जा err;
    }

    /* (over)estimate the number of files to load. */
    nfiles = size / आकार(*entry);
    files = malloc_tmp(आकार(*files) + nfiles * आकार(files->files[0]));
    जर (!files) {
        warn_noalloc();
        जा err;
    }
    files->nfiles = 0;

    साठी (offset = 0; offset < size; offset += आकार(*entry)) {
        entry = data + offset;
        निवड (le32_to_cpu(entry->command)) {
                स्थिती ROMFILE_LOADER_COMMAND_ALLOCATE:
                        romfile_loader_allocate(entry, files);
                        थांबा;
                स्थिती ROMFILE_LOADER_COMMAND_ADD_POINTER:
                        romfile_loader_add_pointer(entry, files);
                        थांबा;
                स्थिती ROMFILE_LOADER_COMMAND_ADD_CHECKSUM:
                        romfile_loader_add_checksum(entry, files);
                        थांबा;
                स्थिती ROMFILE_LOADER_COMMAND_WRITE_POINTER:
                        romfile_loader_write_pointer(entry, files);
                        थांबा;
                पूर्वनिर्धारित:
                        /* Skip commands that we don't recognize. */
                        थांबा;
        }
    }

    मुक्त(files);
    मुक्त(data);
    परत 0;

err:
    मुक्त(data);
    परत -1;
}
