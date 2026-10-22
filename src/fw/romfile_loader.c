#समावेश "romfile_loader.h"
#समावेश "byteorder.h" // leXX_to_cpu/cpu_to_leXX
#समावेश "util.h" // checksum
#समावेश "string.h" // strcmp
#समावेश "romfile.h" // struct romfile_s
#समावेश "malloc.h" // Zone*, _malloc
#समावेश "list.h" // struct hlist_node
#समावेश "output.h" // warn_*
#समावेश "paravirt.h" // qemu_cfg_write_file

काष्ठा romfile_loader_file {
    काष्ठा romfile_s *file;
    व्योम *तथ्य;
};
काष्ठा romfile_loader_files {
    पूर्णांक nfiles;
    काष्ठा romfile_loader_file files[];
};

// Data structures for storing "write pointer" entries for possible replay
काष्ठा romfile_wr_pointer_entry {
    u64 pointer;
    u32 offset;
    u16 कुंजी;
    u8 ptr_आकार;
    काष्ठा hlist_गाँठ गाँठ;
};
अटल काष्ठा hlist_शीर्ष romfile_pointer_list;

अटल काष्ठा romfile_loader_file *
romfile_loader_find(स्थिर अक्षर *name,
                    काष्ठा romfile_loader_files *files)
{
    पूर्णांक i;
    अगर (name[ROMFILE_LOADER_FILESZ - 1])
        वापस शून्य;
    क्रम (i = 0; i < files->nfiles; ++i)
        अगर (!म_भेद(files->files[i].file->name, name))
            वापस &files->files[i];
    वापस शून्य;
}

// Replay "write pointer" entries back to QEMU
व्योम romfile_fw_cfg_resume(व्योम)
{
    अगर (!CONFIG_QEMU)
        वापस;

    काष्ठा romfile_wr_pointer_entry *entry;
    hlist_for_each_entry(entry, &romfile_pointer_list, गाँठ) {
        qemu_cfg_write_file_simple(&entry->pointer, entry->कुंजी,
                                   entry->offset, entry->ptr_आकार);
    }
}

अटल व्योम romfile_loader_allocate(काष्ठा romfile_loader_entry_s *entry,
                                    काष्ठा romfile_loader_files *files)
{
    काष्ठा zone_s *zone;
    काष्ठा romfile_loader_file *file = &files->files[files->nfiles];
    व्योम *तथ्य;
    पूर्णांक ret;
    अचिन्हित alloc_align = le32_to_cpu(entry->alloc.align);

    अगर (alloc_align & (alloc_align - 1))
        जाओ err;

    चयन (entry->alloc.zone) {
        हाल ROMFILE_LOADER_ALLOC_ZONE_ऊपरी:
            zone = &Zone_ऊपरी;
            अवरोध;
        हाल ROMFILE_LOADER_ALLOC_ZONE_FSEG:
            zone = &ZoneFSeg;
            अवरोध;
        शेष:
            जाओ err;
    }
    अगर (alloc_align < MALLOC_MIN_ALIGN)
        alloc_align = MALLOC_MIN_ALIGN;
    अगर (entry->alloc.file[ROMFILE_LOADER_FILESZ - 1])
        जाओ err;
    file->file = romfile_find(entry->alloc.file);
    अगर (!file->file || !file->file->आकार)
        वापस;
    तथ्य = _malloc(zone, file->file->आकार, alloc_align);
    अगर (!तथ्य) {
        warn_noalloc();
        वापस;
    }
    ret = file->file->copy(file->file, तथ्य, file->file->आकार);
    अगर (ret != file->file->आकार)
        जाओ file_err;
    file->तथ्य = तथ्य;
    files->nfiles++;
    वापस;

file_err:
    मुक्त(तथ्य);
err:
    warn_internalerror();
}

अटल व्योम romfile_loader_add_pointer(काष्ठा romfile_loader_entry_s *entry,
                                       काष्ठा romfile_loader_files *files)
{
    काष्ठा romfile_loader_file *dest_file;
    काष्ठा romfile_loader_file *src_file;
    अचिन्हित offset = le32_to_cpu(entry->pointer.offset);
    u64 pointer = 0;

    dest_file = romfile_loader_find(entry->pointer.dest_file, files);
    src_file = romfile_loader_find(entry->pointer.src_file, files);

    अगर (!dest_file || !src_file || !dest_file->तथ्य || !src_file->तथ्य ||
        offset + entry->pointer.आकार < offset ||
        offset + entry->pointer.आकार > dest_file->file->आकार ||
        entry->pointer.आकार < 1 || entry->pointer.आकार > 8 ||
        entry->pointer.आकार & (entry->pointer.आकार - 1))
        जाओ err;

    स_नकल(&pointer, dest_file->तथ्य + offset, entry->pointer.आकार);
    pointer = le64_to_cpu(pointer);
    pointer += (अचिन्हित दीर्घ)src_file->तथ्य;
    pointer = cpu_to_le64(pointer);
    स_नकल(dest_file->तथ्य + offset, &pointer, entry->pointer.आकार);

    वापस;
err:
    warn_internalerror();
}

अटल व्योम romfile_loader_add_checksum(काष्ठा romfile_loader_entry_s *entry,
                                        काष्ठा romfile_loader_files *files)
{
    काष्ठा romfile_loader_file *file;
    अचिन्हित offset = le32_to_cpu(entry->cksum.offset);
    अचिन्हित start = le32_to_cpu(entry->cksum.start);
    अचिन्हित len = le32_to_cpu(entry->cksum.लंबाई);
    u8 *तथ्य;

    file = romfile_loader_find(entry->cksum.file, files);

    अगर (!file || !file->तथ्य || offset >= file->file->आकार ||
        start + len < start || start + len > file->file->आकार)
        जाओ err;

    तथ्य = file->तथ्य + offset;
    *तथ्य -= checksum(file->तथ्य + start, len);

    वापस;
err:
    warn_internalerror();
}

अटल व्योम romfile_loader_write_pointer(काष्ठा romfile_loader_entry_s *entry,
                                         काष्ठा romfile_loader_files *files)
{
    काष्ठा romfile_s *dest_file;
    काष्ठा romfile_loader_file *src_file;
    अचिन्हित dst_offset = le32_to_cpu(entry->wr_pointer.dst_offset);
    अचिन्हित src_offset = le32_to_cpu(entry->wr_pointer.src_offset);
    u64 pointer = 0;

    /* Writing back to a file that may not be loaded in RAM */
    dest_file = romfile_find(entry->wr_pointer.dest_file);
    src_file = romfile_loader_find(entry->wr_pointer.src_file, files);

    अगर (!dest_file || !src_file || !src_file->तथ्य ||
        dst_offset + entry->wr_pointer.आकार < dst_offset ||
        dst_offset + entry->wr_pointer.आकार > dest_file->आकार ||
        src_offset >= src_file->file->आकार ||
        entry->wr_pointer.आकार < 1 || entry->wr_pointer.आकार > 8 ||
        entry->wr_pointer.आकार & (entry->wr_pointer.आकार - 1)) {
        जाओ err;
    }

    pointer = (अचिन्हित दीर्घ)src_file->तथ्य + src_offset;
    /* Make sure the pointer fits within wr_pointer.size */
    अगर ((entry->wr_pointer.आकार != माप(u64)) &&
        ((pointer >> (entry->wr_pointer.आकार * 8)) > 0)) {
        जाओ err;
    }
    pointer = cpu_to_le64(pointer);

    /* Only supported on QEMU */
    अगर (qemu_cfg_write_file(&pointer, dest_file, dst_offset,
                            entry->wr_pointer.आकार) != entry->wr_pointer.आकार) {
        जाओ err;
    }

    /* Store the info so it can replayed later if necessary */
    काष्ठा romfile_wr_pointer_entry *store = malloc_ऊपरी(माप(*store));
    अगर (!store) {
        warn_noalloc();
        वापस;
    }
    store->pointer = pointer;
    store->कुंजी = qemu_get_romfile_कुंजी(dest_file);
    store->offset = dst_offset;
    store->ptr_आकार = entry->wr_pointer.आकार;
    hlist_add_शीर्ष(&store->गाँठ, &romfile_pointer_list);

    वापस;
 err:
    warn_internalerror();
}

पूर्णांक romfile_loader_execute(स्थिर अक्षर *name)
{
    काष्ठा romfile_loader_entry_s *entry;
    पूर्णांक आकार, offset = 0, nfiles;
    काष्ठा romfile_loader_files *files;
    व्योम *तथ्य = romfile_loadfile(name, &आकार);
    अगर (!तथ्य)
        वापस -1;

    अगर (आकार % माप(*entry)) {
        warn_internalerror();
        जाओ err;
    }

    /* (over)estimate the number of files to load. */
    nfiles = आकार / माप(*entry);
    files = malloc_tmp(माप(*files) + nfiles * माप(files->files[0]));
    अगर (!files) {
        warn_noalloc();
        जाओ err;
    }
    files->nfiles = 0;

    क्रम (offset = 0; offset < आकार; offset += माप(*entry)) {
        entry = तथ्य + offset;
        चयन (le32_to_cpu(entry->command)) {
                हाल ROMFILE_LOADER_COMMAND_ALLOCATE:
                        romfile_loader_allocate(entry, files);
                        अवरोध;
                हाल ROMFILE_LOADER_COMMAND_ADD_POINTER:
                        romfile_loader_add_pointer(entry, files);
                        अवरोध;
                हाल ROMFILE_LOADER_COMMAND_ADD_CHECKSUM:
                        romfile_loader_add_checksum(entry, files);
                        अवरोध;
                हाल ROMFILE_LOADER_COMMAND_WRITE_POINTER:
                        romfile_loader_write_pointer(entry, files);
                        अवरोध;
                शेष:
                        /* Skip commands that we don't recognize. */
                        अवरोध;
        }
    }

    मुक्त(files);
    मुक्त(तथ्य);
    वापस 0;

err:
    मुक्त(तथ्य);
    वापस -1;
}
