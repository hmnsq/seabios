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
    शून्य *दत्त;
};
संरचना romfile_loader_files {
    पूर्णाङ्क nfiles;
    संरचना romfile_loader_file files[];
};

// Data structures for storing "write pointer" entries for possible replay
संरचना romfile_wr_pointer_प्रविष्टिः {
    u64 pointer;
    u32 विस्थापनम्_2;
    u16 कुञ्चिका;
    u8 ptr_आकार;
    संरचना hlist_ग्रन्थिः ग्रन्थिः;
};
स्थिर संरचना hlist_शीर्ष romfile_pointer_list;

स्थिर संरचना romfile_loader_file *
romfile_loader_find(ध्रुव वर्ण *name,
                    संरचना romfile_loader_files *files)
{
    पूर्णाङ्क i;
    यदि (name[ROMFILE_LOADER_FILESZ - 1])
        प्रत्यागम रिक्त;
    प्रति (i = 0; i < files->nfiles; ++i)
        यदि (!वाक्यं_तुलय(files->files[i].file->name, name))
            प्रत्यागम &files->files[i];
    प्रत्यागम रिक्त;
}

// Replay "write pointer" entries back to QEMU
शून्य romfile_fw_cfg_resume(शून्य)
{
    यदि (!CONFIG_QEMU)
        प्रत्यागम;

    संरचना romfile_wr_pointer_प्रविष्टिः *प्रविष्टिः;
    hlist_for_each_प्रविष्टिः(प्रविष्टिः, &romfile_pointer_list, ग्रन्थिः) {
        qemu_cfg_write_file_simple(&प्रविष्टिः->pointer, प्रविष्टिः->कुञ्चिका,
                                   प्रविष्टिः->विस्थापनम्_2, प्रविष्टिः->ptr_आकार);
    }
}

स्थिर शून्य romfile_loader_allocate(संरचना romfile_loader_प्रविष्टिः_s *प्रविष्टिः,
                                    संरचना romfile_loader_files *files)
{
    संरचना zone_s *zone;
    संरचना romfile_loader_file *file = &files->files[files->nfiles];
    शून्य *दत्त;
    पूर्णाङ्क ret;
    अचिह्नित alloc_align = le32_गन्तव्यम्_cpu(प्रविष्टिः->alloc.align);

    यदि (alloc_align & (alloc_align - 1))
        गच्छ err;

    विकल्प (प्रविष्टिः->alloc.zone) {
        पक्ष ROMFILE_LOADER_ALLOC_ZONE_उच्चः:
            zone = &Zone_उच्चः;
            भङ्ग;
        पक्ष ROMFILE_LOADER_ALLOC_ZONE_FSEG:
            zone = &ZoneFSeg;
            भङ्ग;
        स्वभाव:
            गच्छ err;
    }
    यदि (alloc_align < MALLOC_MIN_ALIGN)
        alloc_align = MALLOC_MIN_ALIGN;
    यदि (प्रविष्टिः->alloc.file[ROMFILE_LOADER_FILESZ - 1])
        गच्छ err;
    file->file = romfile_find(प्रविष्टिः->alloc.file);
    यदि (!file->file || !file->file->आकार)
        प्रत्यागम;
    दत्त = _malloc(zone, file->file->आकार, alloc_align);
    यदि (!दत्त) {
        warn_noalloc();
        प्रत्यागम;
    }
    ret = file->file->copy(file->file, दत्त, file->file->आकार);
    यदि (ret != file->file->आकार)
        गच्छ file_err;
    file->दत्त = दत्त;
    files->nfiles++;
    प्रत्यागम;

file_err:
    मुक्त(दत्त);
err:
    warn_internalerror();
}

स्थिर शून्य romfile_loader_सङ्कलय_pointer(संरचना romfile_loader_प्रविष्टिः_s *प्रविष्टिः,
                                       संरचना romfile_loader_files *files)
{
    संरचना romfile_loader_file *dest_file;
    संरचना romfile_loader_file *src_file;
    अचिह्नित विस्थापनम्_2 = le32_गन्तव्यम्_cpu(प्रविष्टिः->pointer.विस्थापनम्_2);
    u64 pointer = 0;

    dest_file = romfile_loader_find(प्रविष्टिः->pointer.dest_file, files);
    src_file = romfile_loader_find(प्रविष्टिः->pointer.src_file, files);

    यदि (!dest_file || !src_file || !dest_file->दत्त || !src_file->दत्त ||
        विस्थापनम्_2 + प्रविष्टिः->pointer.आकार < विस्थापनम्_2 ||
        विस्थापनम्_2 + प्रविष्टिः->pointer.आकार > dest_file->file->आकार ||
        प्रविष्टिः->pointer.आकार < 1 || प्रविष्टिः->pointer.आकार > 8 ||
        प्रविष्टिः->pointer.आकार & (प्रविष्टिः->pointer.आकार - 1))
        गच्छ err;

    स्मृतिं_प्रतिलिख(&pointer, dest_file->दत्त + विस्थापनम्_2, प्रविष्टिः->pointer.आकार);
    pointer = le64_गन्तव्यम्_cpu(pointer);
    pointer += (अचिह्नित दीर्घ)src_file->दत्त;
    pointer = cpu_गन्तव्यम्_le64(pointer);
    स्मृतिं_प्रतिलिख(dest_file->दत्त + विस्थापनम्_2, &pointer, प्रविष्टिः->pointer.आकार);

    प्रत्यागम;
err:
    warn_internalerror();
}

स्थिर शून्य romfile_loader_सङ्कलय_checksum(संरचना romfile_loader_प्रविष्टिः_s *प्रविष्टिः,
                                        संरचना romfile_loader_files *files)
{
    संरचना romfile_loader_file *file;
    अचिह्नित विस्थापनम्_2 = le32_गन्तव्यम्_cpu(प्रविष्टिः->cksum.विस्थापनम्_2);
    अचिह्नित start = le32_गन्तव्यम्_cpu(प्रविष्टिः->cksum.start);
    अचिह्नित len = le32_गन्तव्यम्_cpu(प्रविष्टिः->cksum.दीर्घता);
    u8 *दत्त;

    file = romfile_loader_find(प्रविष्टिः->cksum.file, files);

    यदि (!file || !file->दत्त || विस्थापनम्_2 >= file->file->आकार ||
        start + len < start || start + len > file->file->आकार)
        गच्छ err;

    दत्त = file->दत्त + विस्थापनम्_2;
    *दत्त -= checksum(file->दत्त + start, len);

    प्रत्यागम;
err:
    warn_internalerror();
}

स्थिर शून्य romfile_loader_write_pointer(संरचना romfile_loader_प्रविष्टिः_s *प्रविष्टिः,
                                         संरचना romfile_loader_files *files)
{
    संरचना romfile_s *dest_file;
    संरचना romfile_loader_file *src_file;
    अचिह्नित dst_विस्थापनम् = le32_गन्तव्यम्_cpu(प्रविष्टिः->wr_pointer.dst_विस्थापनम्);
    अचिह्नित src_विस्थापनम् = le32_गन्तव्यम्_cpu(प्रविष्टिः->wr_pointer.src_विस्थापनम्);
    u64 pointer = 0;

    /* Writing back to a file that may not be loaded in RAM */
    dest_file = romfile_find(प्रविष्टिः->wr_pointer.dest_file);
    src_file = romfile_loader_find(प्रविष्टिः->wr_pointer.src_file, files);

    यदि (!dest_file || !src_file || !src_file->दत्त ||
        dst_विस्थापनम् + प्रविष्टिः->wr_pointer.आकार < dst_विस्थापनम् ||
        dst_विस्थापनम् + प्रविष्टिः->wr_pointer.आकार > dest_file->आकार ||
        src_विस्थापनम् >= src_file->file->आकार ||
        प्रविष्टिः->wr_pointer.आकार < 1 || प्रविष्टिः->wr_pointer.आकार > 8 ||
        प्रविष्टिः->wr_pointer.आकार & (प्रविष्टिः->wr_pointer.आकार - 1)) {
        गच्छ err;
    }

    pointer = (अचिह्नित दीर्घ)src_file->दत्त + src_विस्थापनम्;
    /* Make sure the pointer fits within wr_pointer.size */
    यदि ((प्रविष्टिः->wr_pointer.आकार != परिमाण(u64)) &&
        ((pointer >> (प्रविष्टिः->wr_pointer.आकार * 8)) > 0)) {
        गच्छ err;
    }
    pointer = cpu_गन्तव्यम्_le64(pointer);

    /* Only supported on QEMU */
    यदि (qemu_cfg_write_file(&pointer, dest_file, dst_विस्थापनम्,
                            प्रविष्टिः->wr_pointer.आकार) != प्रविष्टिः->wr_pointer.आकार) {
        गच्छ err;
    }

    /* Store the info so it can replayed later if necessary */
    संरचना romfile_wr_pointer_प्रविष्टिः *store = malloc_उच्चः(परिमाण(*store));
    यदि (!store) {
        warn_noalloc();
        प्रत्यागम;
    }
    store->pointer = pointer;
    store->कुञ्चिका = qemu_get_romfile_कुञ्चिका(dest_file);
    store->विस्थापनम्_2 = dst_विस्थापनम्;
    store->ptr_आकार = प्रविष्टिः->wr_pointer.आकार;
    hlist_सङ्कलय_शीर्ष(&store->ग्रन्थिः, &romfile_pointer_list);

    प्रत्यागम;
 err:
    warn_internalerror();
}

पूर्णाङ्क romfile_loader_execute(ध्रुव वर्ण *name)
{
    संरचना romfile_loader_प्रविष्टिः_s *प्रविष्टिः;
    पूर्णाङ्क आकार, विस्थापनम्_2 = 0, nfiles;
    संरचना romfile_loader_files *files;
    शून्य *दत्त = romfile_loadfile(name, &आकार);
    यदि (!दत्त)
        प्रत्यागम -1;

    यदि (आकार % परिमाण(*प्रविष्टिः)) {
        warn_internalerror();
        गच्छ err;
    }

    /* (over)estimate the number of files to load. */
    nfiles = आकार / परिमाण(*प्रविष्टिः);
    files = malloc_tmp(परिमाण(*files) + nfiles * परिमाण(files->files[0]));
    यदि (!files) {
        warn_noalloc();
        गच्छ err;
    }
    files->nfiles = 0;

    प्रति (विस्थापनम्_2 = 0; विस्थापनम्_2 < आकार; विस्थापनम्_2 += परिमाण(*प्रविष्टिः)) {
        प्रविष्टिः = दत्त + विस्थापनम्_2;
        विकल्प (le32_गन्तव्यम्_cpu(प्रविष्टिः->command)) {
                पक्ष ROMFILE_LOADER_COMMAND_ALLOCATE:
                        romfile_loader_allocate(प्रविष्टिः, files);
                        भङ्ग;
                पक्ष ROMFILE_LOADER_COMMAND_सङ्कलय_POINTER:
                        romfile_loader_सङ्कलय_pointer(प्रविष्टिः, files);
                        भङ्ग;
                पक्ष ROMFILE_LOADER_COMMAND_सङ्कलय_CHECKSUM:
                        romfile_loader_सङ्कलय_checksum(प्रविष्टिः, files);
                        भङ्ग;
                पक्ष ROMFILE_LOADER_COMMAND_WRITE_POINTER:
                        romfile_loader_write_pointer(प्रविष्टिः, files);
                        भङ्ग;
                स्वभाव:
                        /* Skip commands that we don't recognize. */
                        भङ्ग;
        }
    }

    मुक्त(files);
    मुक्त(दत्त);
    प्रत्यागम 0;

err:
    मुक्त(दत्त);
    प्रत्यागम -1;
}
