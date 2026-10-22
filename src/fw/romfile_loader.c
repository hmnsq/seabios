#অন্তর্ভাব "romfile_loader.h"
#অন্তর্ভাব "byteorder.h" // leXX_to_cpu/cpu_to_leXX
#অন্তর্ভাব "util.h" // checksum
#অন্তর্ভাব "string.h" // strcmp
#অন্তর্ভাব "romfile.h" // struct romfile_s
#অন্তর্ভাব "malloc.h" // Zone*, _malloc
#অন্তর্ভাব "list.h" // struct hlist_node
#অন্তর্ভাব "output.h" // warn_*
#অন্তর্ভাব "paravirt.h" // qemu_cfg_write_file

কাঠামো romfile_loader_file {
    কাঠামো romfile_s *file;
    শূন্য *তথ্য;
};
কাঠামো romfile_loader_files {
    পূর্ণসংখ্যা nfiles;
    কাঠামো romfile_loader_file files[];
};

// Data structures for storing "write pointer" entries for possible replay
কাঠামো romfile_wr_pointer_entry {
    u64 pointer;
    u32 offset;
    u16 চাবি;
    u8 ptr_আকার;
    কাঠামো hlist_গ্রন্থি গ্রন্থি;
};
স্থির কাঠামো hlist_শীর্ষ romfile_pointer_list;

স্থির কাঠামো romfile_loader_file *
romfile_loader_find(ধ্রুবক অক্ষর *name,
                    কাঠামো romfile_loader_files *files)
{
    পূর্ণসংখ্যা i;
    যদি (name[ROMFILE_LOADER_FILESZ - 1])
        ফেরত ফাঁকা;
    জন্য (i = 0; i < files->nfiles; ++i)
        যদি (!strcmp(files->files[i].file->name, name))
            ফেরত &files->files[i];
    ফেরত ফাঁকা;
}

// Replay "write pointer" entries back to QEMU
শূন্য romfile_fw_cfg_resume(শূন্য)
{
    যদি (!CONFIG_QEMU)
        ফেরত;

    কাঠামো romfile_wr_pointer_entry *entry;
    hlist_for_each_entry(entry, &romfile_pointer_list, গ্রন্থি) {
        qemu_cfg_write_file_simple(&entry->pointer, entry->চাবি,
                                   entry->offset, entry->ptr_আকার);
    }
}

স্থির শূন্য romfile_loader_allocate(কাঠামো romfile_loader_entry_s *entry,
                                    কাঠামো romfile_loader_files *files)
{
    কাঠামো zone_s *zone;
    কাঠামো romfile_loader_file *file = &files->files[files->nfiles];
    শূন্য *তথ্য;
    পূর্ণসংখ্যা ret;
    unsigned alloc_align = le32_to_cpu(entry->alloc.align);

    যদি (alloc_align & (alloc_align - 1))
        যাও err;

    নির্বাচন (entry->alloc.zone) {
        ক্ষেত্র ROMFILE_LOADER_ALLOC_ZONE_উপরের:
            zone = &Zone_উপরের;
            বিরতি;
        ক্ষেত্র ROMFILE_LOADER_ALLOC_ZONE_FSEG:
            zone = &ZoneFSeg;
            বিরতি;
        পূর্বনির্ধারিত:
            যাও err;
    }
    যদি (alloc_align < MALLOC_MIN_ALIGN)
        alloc_align = MALLOC_MIN_ALIGN;
    যদি (entry->alloc.file[ROMFILE_LOADER_FILESZ - 1])
        যাও err;
    file->file = romfile_find(entry->alloc.file);
    যদি (!file->file || !file->file->আকার_2)
        ফেরত;
    তথ্য = _malloc(zone, file->file->আকার_2, alloc_align);
    যদি (!তথ্য) {
        warn_noalloc();
        ফেরত;
    }
    ret = file->file->copy(file->file, তথ্য, file->file->আকার_2);
    যদি (ret != file->file->আকার_2)
        যাও file_err;
    file->তথ্য = তথ্য;
    files->nfiles++;
    ফেরত;

file_err:
    মুক্ত(তথ্য);
err:
    warn_internalerror();
}

স্থির শূন্য romfile_loader_add_pointer(কাঠামো romfile_loader_entry_s *entry,
                                       কাঠামো romfile_loader_files *files)
{
    কাঠামো romfile_loader_file *dest_file;
    কাঠামো romfile_loader_file *src_file;
    unsigned offset = le32_to_cpu(entry->pointer.offset);
    u64 pointer = 0;

    dest_file = romfile_loader_find(entry->pointer.dest_file, files);
    src_file = romfile_loader_find(entry->pointer.src_file, files);

    যদি (!dest_file || !src_file || !dest_file->তথ্য || !src_file->তথ্য ||
        offset + entry->pointer.আকার_2 < offset ||
        offset + entry->pointer.আকার_2 > dest_file->file->আকার_2 ||
        entry->pointer.আকার_2 < 1 || entry->pointer.আকার_2 > 8 ||
        entry->pointer.আকার_2 & (entry->pointer.আকার_2 - 1))
        যাও err;

    memcpy(&pointer, dest_file->তথ্য + offset, entry->pointer.আকার_2);
    pointer = le64_to_cpu(pointer);
    pointer += (unsigned long)src_file->তথ্য;
    pointer = cpu_to_le64(pointer);
    memcpy(dest_file->তথ্য + offset, &pointer, entry->pointer.আকার_2);

    ফেরত;
err:
    warn_internalerror();
}

স্থির শূন্য romfile_loader_add_checksum(কাঠামো romfile_loader_entry_s *entry,
                                        কাঠামো romfile_loader_files *files)
{
    কাঠামো romfile_loader_file *file;
    unsigned offset = le32_to_cpu(entry->cksum.offset);
    unsigned start = le32_to_cpu(entry->cksum.start);
    unsigned len = le32_to_cpu(entry->cksum.দৈর্ঘ্য);
    u8 *তথ্য;

    file = romfile_loader_find(entry->cksum.file, files);

    যদি (!file || !file->তথ্য || offset >= file->file->আকার_2 ||
        start + len < start || start + len > file->file->আকার_2)
        যাও err;

    তথ্য = file->তথ্য + offset;
    *তথ্য -= checksum(file->তথ্য + start, len);

    ফেরত;
err:
    warn_internalerror();
}

স্থির শূন্য romfile_loader_write_pointer(কাঠামো romfile_loader_entry_s *entry,
                                         কাঠামো romfile_loader_files *files)
{
    কাঠামো romfile_s *dest_file;
    কাঠামো romfile_loader_file *src_file;
    unsigned dst_offset = le32_to_cpu(entry->wr_pointer.dst_offset);
    unsigned src_offset = le32_to_cpu(entry->wr_pointer.src_offset);
    u64 pointer = 0;

    /* Writing back to a file that may not be loaded in RAM */
    dest_file = romfile_find(entry->wr_pointer.dest_file);
    src_file = romfile_loader_find(entry->wr_pointer.src_file, files);

    যদি (!dest_file || !src_file || !src_file->তথ্য ||
        dst_offset + entry->wr_pointer.আকার_2 < dst_offset ||
        dst_offset + entry->wr_pointer.আকার_2 > dest_file->আকার_2 ||
        src_offset >= src_file->file->আকার_2 ||
        entry->wr_pointer.আকার_2 < 1 || entry->wr_pointer.আকার_2 > 8 ||
        entry->wr_pointer.আকার_2 & (entry->wr_pointer.আকার_2 - 1)) {
        যাও err;
    }

    pointer = (unsigned long)src_file->তথ্য + src_offset;
    /* Make sure the pointer fits within wr_pointer.size */
    যদি ((entry->wr_pointer.আকার_2 != আকার(u64)) &&
        ((pointer >> (entry->wr_pointer.আকার_2 * 8)) > 0)) {
        যাও err;
    }
    pointer = cpu_to_le64(pointer);

    /* Only supported on QEMU */
    যদি (qemu_cfg_write_file(&pointer, dest_file, dst_offset,
                            entry->wr_pointer.আকার_2) != entry->wr_pointer.আকার_2) {
        যাও err;
    }

    /* Store the info so it can replayed later if necessary */
    কাঠামো romfile_wr_pointer_entry *store = malloc_উপরের(আকার(*store));
    যদি (!store) {
        warn_noalloc();
        ফেরত;
    }
    store->pointer = pointer;
    store->চাবি = qemu_get_romfile_চাবি(dest_file);
    store->offset = dst_offset;
    store->ptr_আকার = entry->wr_pointer.আকার_2;
    hlist_add_শীর্ষ(&store->গ্রন্থি, &romfile_pointer_list);

    ফেরত;
 err:
    warn_internalerror();
}

পূর্ণসংখ্যা romfile_loader_execute(ধ্রুবক অক্ষর *name)
{
    কাঠামো romfile_loader_entry_s *entry;
    পূর্ণসংখ্যা আকার_2, offset = 0, nfiles;
    কাঠামো romfile_loader_files *files;
    শূন্য *তথ্য = romfile_loadfile(name, &আকার_2);
    যদি (!তথ্য)
        ফেরত -1;

    যদি (আকার_2 % আকার(*entry)) {
        warn_internalerror();
        যাও err;
    }

    /* (over)estimate the number of files to load. */
    nfiles = আকার_2 / আকার(*entry);
    files = malloc_tmp(আকার(*files) + nfiles * আকার(files->files[0]));
    যদি (!files) {
        warn_noalloc();
        যাও err;
    }
    files->nfiles = 0;

    জন্য (offset = 0; offset < আকার_2; offset += আকার(*entry)) {
        entry = তথ্য + offset;
        নির্বাচন (le32_to_cpu(entry->command)) {
                ক্ষেত্র ROMFILE_LOADER_COMMAND_ALLOCATE:
                        romfile_loader_allocate(entry, files);
                        বিরতি;
                ক্ষেত্র ROMFILE_LOADER_COMMAND_ADD_POINTER:
                        romfile_loader_add_pointer(entry, files);
                        বিরতি;
                ক্ষেত্র ROMFILE_LOADER_COMMAND_ADD_CHECKSUM:
                        romfile_loader_add_checksum(entry, files);
                        বিরতি;
                ক্ষেত্র ROMFILE_LOADER_COMMAND_WRITE_POINTER:
                        romfile_loader_write_pointer(entry, files);
                        বিরতি;
                পূর্বনির্ধারিত:
                        /* Skip commands that we don't recognize. */
                        বিরতি;
        }
    }

    মুক্ত(files);
    মুক্ত(তথ্য);
    ফেরত 0;

err:
    মুক্ত(তথ্য);
    ফেরত -1;
}
