#અન્તર્ભાવ "romfile_loader.h"
#અન્તર્ભાવ "byteorder.h" // leXX_to_cpu/cpu_to_leXX
#અન્તર્ભાવ "util.h" // checksum
#અન્તર્ભાવ "string.h" // strcmp
#અન્તર્ભાવ "romfile.h" // struct romfile_s
#અન્તર્ભાવ "malloc.h" // Zone*, _malloc
#અન્તર્ભાવ "list.h" // struct hlist_node
#અન્તર્ભાવ "output.h" // warn_*
#અન્તર્ભાવ "paravirt.h" // qemu_cfg_write_file

સંરચના romfile_loader_file {
    સંરચના romfile_s *file;
    શૂન્ય *data;
};
સંરચના romfile_loader_files {
    પૂર્ણાંક nfiles;
    સંરચના romfile_loader_file files[];
};

// Data structures for storing "write pointer" entries for possible replay
સંરચના romfile_wr_pointer_entry {
    u64 pointer;
    u32 offset;
    u16 ચાવી;
    u8 ptr_size;
    સંરચના hlist_ગાંઠ ગાંઠ;
};
સ્થિર સંરચના hlist_head romfile_pointer_list;

સ્થિર સંરચના romfile_loader_file *
romfile_loader_find(અચલ અક્ષર *name,
                    સંરચના romfile_loader_files *files)
{
    પૂર્ણાંક i;
    જો (name[ROMFILE_LOADER_FILESZ - 1])
        પાછા ખાલી;
    માટે (i = 0; i < files->nfiles; ++i)
        જો (!strcmp(files->files[i].file->name, name))
            પાછા &files->files[i];
    પાછા ખાલી;
}

// Replay "write pointer" entries back to QEMU
શૂન્ય romfile_fw_cfg_resume(શૂન્ય)
{
    જો (!CONFIG_QEMU)
        પાછા;

    સંરચના romfile_wr_pointer_entry *entry;
    hlist_for_each_entry(entry, &romfile_pointer_list, ગાંઠ) {
        qemu_cfg_write_file_simple(&entry->pointer, entry->ચાવી,
                                   entry->offset, entry->ptr_size);
    }
}

સ્થિર શૂન્ય romfile_loader_allocate(સંરચના romfile_loader_entry_s *entry,
                                    સંરચના romfile_loader_files *files)
{
    સંરચના zone_s *zone;
    સંરચના romfile_loader_file *file = &files->files[files->nfiles];
    શૂન્ય *data;
    પૂર્ણાંક ret;
    unsigned alloc_align = le32_to_cpu(entry->alloc.align);

    જો (alloc_align & (alloc_align - 1))
        જાઓ err;

    પસંદગી (entry->alloc.zone) {
        સ્થિતિ ROMFILE_LOADER_ALLOC_ZONE_ઉપલું:
            zone = &Zone_ઉપલું;
            તોડો;
        સ્થિતિ ROMFILE_LOADER_ALLOC_ZONE_FSEG:
            zone = &ZoneFSeg;
            તોડો;
        મૂળભૂત:
            જાઓ err;
    }
    જો (alloc_align < MALLOC_MIN_ALIGN)
        alloc_align = MALLOC_MIN_ALIGN;
    જો (entry->alloc.file[ROMFILE_LOADER_FILESZ - 1])
        જાઓ err;
    file->file = romfile_find(entry->alloc.file);
    જો (!file->file || !file->file->size)
        પાછા;
    data = _malloc(zone, file->file->size, alloc_align);
    જો (!data) {
        warn_noalloc();
        પાછા;
    }
    ret = file->file->copy(file->file, data, file->file->size);
    જો (ret != file->file->size)
        જાઓ file_err;
    file->data = data;
    files->nfiles++;
    પાછા;

file_err:
    મુક્ત(data);
err:
    warn_internalerror();
}

સ્થિર શૂન્ય romfile_loader_add_pointer(સંરચના romfile_loader_entry_s *entry,
                                       સંરચના romfile_loader_files *files)
{
    સંરચના romfile_loader_file *dest_file;
    સંરચના romfile_loader_file *src_file;
    unsigned offset = le32_to_cpu(entry->pointer.offset);
    u64 pointer = 0;

    dest_file = romfile_loader_find(entry->pointer.dest_file, files);
    src_file = romfile_loader_find(entry->pointer.src_file, files);

    જો (!dest_file || !src_file || !dest_file->data || !src_file->data ||
        offset + entry->pointer.size < offset ||
        offset + entry->pointer.size > dest_file->file->size ||
        entry->pointer.size < 1 || entry->pointer.size > 8 ||
        entry->pointer.size & (entry->pointer.size - 1))
        જાઓ err;

    memcpy(&pointer, dest_file->data + offset, entry->pointer.size);
    pointer = le64_to_cpu(pointer);
    pointer += (unsigned long)src_file->data;
    pointer = cpu_to_le64(pointer);
    memcpy(dest_file->data + offset, &pointer, entry->pointer.size);

    પાછા;
err:
    warn_internalerror();
}

સ્થિર શૂન્ય romfile_loader_add_checksum(સંરચના romfile_loader_entry_s *entry,
                                        સંરચના romfile_loader_files *files)
{
    સંરચના romfile_loader_file *file;
    unsigned offset = le32_to_cpu(entry->cksum.offset);
    unsigned start = le32_to_cpu(entry->cksum.start);
    unsigned len = le32_to_cpu(entry->cksum.length);
    u8 *data;

    file = romfile_loader_find(entry->cksum.file, files);

    જો (!file || !file->data || offset >= file->file->size ||
        start + len < start || start + len > file->file->size)
        જાઓ err;

    data = file->data + offset;
    *data -= checksum(file->data + start, len);

    પાછા;
err:
    warn_internalerror();
}

સ્થિર શૂન્ય romfile_loader_write_pointer(સંરચના romfile_loader_entry_s *entry,
                                         સંરચના romfile_loader_files *files)
{
    સંરચના romfile_s *dest_file;
    સંરચના romfile_loader_file *src_file;
    unsigned dst_offset = le32_to_cpu(entry->wr_pointer.dst_offset);
    unsigned src_offset = le32_to_cpu(entry->wr_pointer.src_offset);
    u64 pointer = 0;

    /* Writing back to a file that may not be loaded in RAM */
    dest_file = romfile_find(entry->wr_pointer.dest_file);
    src_file = romfile_loader_find(entry->wr_pointer.src_file, files);

    જો (!dest_file || !src_file || !src_file->data ||
        dst_offset + entry->wr_pointer.size < dst_offset ||
        dst_offset + entry->wr_pointer.size > dest_file->size ||
        src_offset >= src_file->file->size ||
        entry->wr_pointer.size < 1 || entry->wr_pointer.size > 8 ||
        entry->wr_pointer.size & (entry->wr_pointer.size - 1)) {
        જાઓ err;
    }

    pointer = (unsigned long)src_file->data + src_offset;
    /* Make sure the pointer fits within wr_pointer.size */
    જો ((entry->wr_pointer.size != કદ(u64)) &&
        ((pointer >> (entry->wr_pointer.size * 8)) > 0)) {
        જાઓ err;
    }
    pointer = cpu_to_le64(pointer);

    /* Only supported on QEMU */
    જો (qemu_cfg_write_file(&pointer, dest_file, dst_offset,
                            entry->wr_pointer.size) != entry->wr_pointer.size) {
        જાઓ err;
    }

    /* Store the info so it can replayed later if necessary */
    સંરચના romfile_wr_pointer_entry *store = malloc_ઉપલું(કદ(*store));
    જો (!store) {
        warn_noalloc();
        પાછા;
    }
    store->pointer = pointer;
    store->ચાવી = qemu_get_romfile_ચાવી(dest_file);
    store->offset = dst_offset;
    store->ptr_size = entry->wr_pointer.size;
    hlist_add_head(&store->ગાંઠ, &romfile_pointer_list);

    પાછા;
 err:
    warn_internalerror();
}

પૂર્ણાંક romfile_loader_execute(અચલ અક્ષર *name)
{
    સંરચના romfile_loader_entry_s *entry;
    પૂર્ણાંક size, offset = 0, nfiles;
    સંરચના romfile_loader_files *files;
    શૂન્ય *data = romfile_loadfile(name, &size);
    જો (!data)
        પાછા -1;

    જો (size % કદ(*entry)) {
        warn_internalerror();
        જાઓ err;
    }

    /* (over)estimate the number of files to load. */
    nfiles = size / કદ(*entry);
    files = malloc_tmp(કદ(*files) + nfiles * કદ(files->files[0]));
    જો (!files) {
        warn_noalloc();
        જાઓ err;
    }
    files->nfiles = 0;

    માટે (offset = 0; offset < size; offset += કદ(*entry)) {
        entry = data + offset;
        પસંદગી (le32_to_cpu(entry->command)) {
                સ્થિતિ ROMFILE_LOADER_COMMAND_ALLOCATE:
                        romfile_loader_allocate(entry, files);
                        તોડો;
                સ્થિતિ ROMFILE_LOADER_COMMAND_ADD_POINTER:
                        romfile_loader_add_pointer(entry, files);
                        તોડો;
                સ્થિતિ ROMFILE_LOADER_COMMAND_ADD_CHECKSUM:
                        romfile_loader_add_checksum(entry, files);
                        તોડો;
                સ્થિતિ ROMFILE_LOADER_COMMAND_WRITE_POINTER:
                        romfile_loader_write_pointer(entry, files);
                        તોડો;
                મૂળભૂત:
                        /* Skip commands that we don't recognize. */
                        તોડો;
        }
    }

    મુક્ત(files);
    મુક્ત(data);
    પાછા 0;

err:
    મુક્ત(data);
    પાછા -1;
}
