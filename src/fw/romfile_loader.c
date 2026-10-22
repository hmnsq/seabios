#ଅନ୍ତର୍ଭାଵ "romfile_loader.h"
#ଅନ୍ତର୍ଭାଵ "byteorder.h" // leXX_to_cpu/cpu_to_leXX
#ଅନ୍ତର୍ଭାଵ "util.h" // checksum
#ଅନ୍ତର୍ଭାଵ "string.h" // strcmp
#ଅନ୍ତର୍ଭାଵ "romfile.h" // struct romfile_s
#ଅନ୍ତର୍ଭାଵ "malloc.h" // Zone*, _malloc
#ଅନ୍ତର୍ଭାଵ "list.h" // struct hlist_node
#ଅନ୍ତର୍ଭାଵ "output.h" // warn_*
#ଅନ୍ତର୍ଭାଵ "paravirt.h" // qemu_cfg_write_file

ସଂରଚନା romfile_loader_file {
    ସଂରଚନା romfile_s *file;
    ଶୂନ୍ୟ *data;
};
ସଂରଚନା romfile_loader_files {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nfiles;
    ସଂରଚନା romfile_loader_file files[];
};

// Data structures for storing "write pointer" entries for possible replay
ସଂରଚନା romfile_wr_pointer_entry {
    u64 pointer;
    u32 offset;
    u16 ଚାବି;
    u8 ptr_size;
    ସଂରଚନା hlist_ଗଣ୍ଠି ଗଣ୍ଠି;
};
ସ୍ଥିର ସଂରଚନା hlist_head romfile_pointer_list;

ସ୍ଥିର ସଂରଚନା romfile_loader_file *
romfile_loader_find(ଅଚଳ ଅକ୍ଷର *name,
                    ସଂରଚନା romfile_loader_files *files)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ଯଦି (name[ROMFILE_LOADER_FILESZ - 1])
        ଫେରନ୍ତୁ ଖାଲି;
    ପାଇଁ (i = 0; i < files->nfiles; ++i)
        ଯଦି (!strcmp(files->files[i].file->name, name))
            ଫେରନ୍ତୁ &files->files[i];
    ଫେରନ୍ତୁ ଖାଲି;
}

// Replay "write pointer" entries back to QEMU
ଶୂନ୍ୟ romfile_fw_cfg_resume(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_QEMU)
        ଫେରନ୍ତୁ;

    ସଂରଚନା romfile_wr_pointer_entry *entry;
    hlist_for_each_entry(entry, &romfile_pointer_list, ଗଣ୍ଠି) {
        qemu_cfg_write_file_simple(&entry->pointer, entry->ଚାବି,
                                   entry->offset, entry->ptr_size);
    }
}

ସ୍ଥିର ଶୂନ୍ୟ romfile_loader_allocate(ସଂରଚନା romfile_loader_entry_s *entry,
                                    ସଂରଚନା romfile_loader_files *files)
{
    ସଂରଚନା zone_s *zone;
    ସଂରଚନା romfile_loader_file *file = &files->files[files->nfiles];
    ଶୂନ୍ୟ *data;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
    unsigned alloc_align = le32_to_cpu(entry->alloc.align);

    ଯଦି (alloc_align & (alloc_align - 1))
        ଯାଅ err;

    ବାଛ (entry->alloc.zone) {
        ସ୍ଥିତି ROMFILE_LOADER_ALLOC_ZONE_ଉପର:
            zone = &Zone_ଉପର;
            ଭାଙ୍ଗ;
        ସ୍ଥିତି ROMFILE_LOADER_ALLOC_ZONE_FSEG:
            zone = &ZoneFSeg;
            ଭାଙ୍ଗ;
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
            ଯାଅ err;
    }
    ଯଦି (alloc_align < MALLOC_MIN_ALIGN)
        alloc_align = MALLOC_MIN_ALIGN;
    ଯଦି (entry->alloc.file[ROMFILE_LOADER_FILESZ - 1])
        ଯାଅ err;
    file->file = romfile_find(entry->alloc.file);
    ଯଦି (!file->file || !file->file->size)
        ଫେରନ୍ତୁ;
    data = _malloc(zone, file->file->size, alloc_align);
    ଯଦି (!data) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }
    ret = file->file->copy(file->file, data, file->file->size);
    ଯଦି (ret != file->file->size)
        ଯାଅ file_err;
    file->data = data;
    files->nfiles++;
    ଫେରନ୍ତୁ;

file_err:
    ମୁକ୍ତ(data);
err:
    warn_internalerror();
}

ସ୍ଥିର ଶୂନ୍ୟ romfile_loader_add_pointer(ସଂରଚନା romfile_loader_entry_s *entry,
                                       ସଂରଚନା romfile_loader_files *files)
{
    ସଂରଚନା romfile_loader_file *dest_file;
    ସଂରଚନା romfile_loader_file *src_file;
    unsigned offset = le32_to_cpu(entry->pointer.offset);
    u64 pointer = 0;

    dest_file = romfile_loader_find(entry->pointer.dest_file, files);
    src_file = romfile_loader_find(entry->pointer.src_file, files);

    ଯଦି (!dest_file || !src_file || !dest_file->data || !src_file->data ||
        offset + entry->pointer.size < offset ||
        offset + entry->pointer.size > dest_file->file->size ||
        entry->pointer.size < 1 || entry->pointer.size > 8 ||
        entry->pointer.size & (entry->pointer.size - 1))
        ଯାଅ err;

    memcpy(&pointer, dest_file->data + offset, entry->pointer.size);
    pointer = le64_to_cpu(pointer);
    pointer += (unsigned long)src_file->data;
    pointer = cpu_to_le64(pointer);
    memcpy(dest_file->data + offset, &pointer, entry->pointer.size);

    ଫେରନ୍ତୁ;
err:
    warn_internalerror();
}

ସ୍ଥିର ଶୂନ୍ୟ romfile_loader_add_checksum(ସଂରଚନା romfile_loader_entry_s *entry,
                                        ସଂରଚନା romfile_loader_files *files)
{
    ସଂରଚନା romfile_loader_file *file;
    unsigned offset = le32_to_cpu(entry->cksum.offset);
    unsigned start = le32_to_cpu(entry->cksum.start);
    unsigned len = le32_to_cpu(entry->cksum.length);
    u8 *data;

    file = romfile_loader_find(entry->cksum.file, files);

    ଯଦି (!file || !file->data || offset >= file->file->size ||
        start + len < start || start + len > file->file->size)
        ଯାଅ err;

    data = file->data + offset;
    *data -= checksum(file->data + start, len);

    ଫେରନ୍ତୁ;
err:
    warn_internalerror();
}

ସ୍ଥିର ଶୂନ୍ୟ romfile_loader_write_pointer(ସଂରଚନା romfile_loader_entry_s *entry,
                                         ସଂରଚନା romfile_loader_files *files)
{
    ସଂରଚନା romfile_s *dest_file;
    ସଂରଚନା romfile_loader_file *src_file;
    unsigned dst_offset = le32_to_cpu(entry->wr_pointer.dst_offset);
    unsigned src_offset = le32_to_cpu(entry->wr_pointer.src_offset);
    u64 pointer = 0;

    /* Writing back to a file that may not be loaded in RAM */
    dest_file = romfile_find(entry->wr_pointer.dest_file);
    src_file = romfile_loader_find(entry->wr_pointer.src_file, files);

    ଯଦି (!dest_file || !src_file || !src_file->data ||
        dst_offset + entry->wr_pointer.size < dst_offset ||
        dst_offset + entry->wr_pointer.size > dest_file->size ||
        src_offset >= src_file->file->size ||
        entry->wr_pointer.size < 1 || entry->wr_pointer.size > 8 ||
        entry->wr_pointer.size & (entry->wr_pointer.size - 1)) {
        ଯାଅ err;
    }

    pointer = (unsigned long)src_file->data + src_offset;
    /* Make sure the pointer fits within wr_pointer.size */
    ଯଦି ((entry->wr_pointer.size != ଆକାର(u64)) &&
        ((pointer >> (entry->wr_pointer.size * 8)) > 0)) {
        ଯାଅ err;
    }
    pointer = cpu_to_le64(pointer);

    /* Only supported on QEMU */
    ଯଦି (qemu_cfg_write_file(&pointer, dest_file, dst_offset,
                            entry->wr_pointer.size) != entry->wr_pointer.size) {
        ଯାଅ err;
    }

    /* Store the info so it can replayed later if necessary */
    ସଂରଚନା romfile_wr_pointer_entry *store = malloc_ଉପର(ଆକାର(*store));
    ଯଦି (!store) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }
    store->pointer = pointer;
    store->ଚାବି = qemu_get_romfile_ଚାବି(dest_file);
    store->offset = dst_offset;
    store->ptr_size = entry->wr_pointer.size;
    hlist_add_head(&store->ଗଣ୍ଠି, &romfile_pointer_list);

    ଫେରନ୍ତୁ;
 err:
    warn_internalerror();
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା romfile_loader_execute(ଅଚଳ ଅକ୍ଷର *name)
{
    ସଂରଚନା romfile_loader_entry_s *entry;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା size, offset = 0, nfiles;
    ସଂରଚନା romfile_loader_files *files;
    ଶୂନ୍ୟ *data = romfile_loadfile(name, &size);
    ଯଦି (!data)
        ଫେରନ୍ତୁ -1;

    ଯଦି (size % ଆକାର(*entry)) {
        warn_internalerror();
        ଯାଅ err;
    }

    /* (over)estimate the number of files to load. */
    nfiles = size / ଆକାର(*entry);
    files = malloc_tmp(ଆକାର(*files) + nfiles * ଆକାର(files->files[0]));
    ଯଦି (!files) {
        warn_noalloc();
        ଯାଅ err;
    }
    files->nfiles = 0;

    ପାଇଁ (offset = 0; offset < size; offset += ଆକାର(*entry)) {
        entry = data + offset;
        ବାଛ (le32_to_cpu(entry->command)) {
                ସ୍ଥିତି ROMFILE_LOADER_COMMAND_ALLOCATE:
                        romfile_loader_allocate(entry, files);
                        ଭାଙ୍ଗ;
                ସ୍ଥିତି ROMFILE_LOADER_COMMAND_ADD_POINTER:
                        romfile_loader_add_pointer(entry, files);
                        ଭାଙ୍ଗ;
                ସ୍ଥିତି ROMFILE_LOADER_COMMAND_ADD_CHECKSUM:
                        romfile_loader_add_checksum(entry, files);
                        ଭାଙ୍ଗ;
                ସ୍ଥିତି ROMFILE_LOADER_COMMAND_WRITE_POINTER:
                        romfile_loader_write_pointer(entry, files);
                        ଭାଙ୍ଗ;
                ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
                        /* Skip commands that we don't recognize. */
                        ଭାଙ୍ଗ;
        }
    }

    ମୁକ୍ତ(files);
    ମୁକ୍ତ(data);
    ଫେରନ୍ତୁ 0;

err:
    ମୁକ୍ତ(data);
    ଫେରନ୍ତୁ -1;
}
