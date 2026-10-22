#ਅਨ੍ਤਰ੍ਭਾਵ "romfile_loader.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "byteorder.h" // leXX_to_cpu/cpu_to_leXX
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // checksum
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // strcmp
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // struct romfile_s
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // Zone*, _malloc
#ਅਨ੍ਤਰ੍ਭਾਵ "list.h" // struct hlist_node
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // warn_*
#ਅਨ੍ਤਰ੍ਭਾਵ "paravirt.h" // qemu_cfg_write_file

ਬਣਤਰ romfile_loader_file {
    ਬਣਤਰ romfile_s *file;
    ਖਾਲੀ *ਅੰਕੜੇ;
};
ਬਣਤਰ romfile_loader_files {
    ਪੂਰਨ nfiles;
    ਬਣਤਰ romfile_loader_file files[];
};

// Data structures for storing "write pointer" entries for possible replay
ਬਣਤਰ romfile_wr_ਸੂਚਕ_ਇੰਦਰਾਜ {
    u64 ਸੂਚਕ;
    u32 ਵਿੱਥ_2;
    u16 ਕੁੰਜੀ;
    u8 ptr_ਮਾਪ;
    ਬਣਤਰ hlist_ਗੰਢ ਗੰਢ;
};
ਸਥਿਰ ਬਣਤਰ hlist_ਸਿਰ romfile_ਸੂਚਕ_list;

ਸਥਿਰ ਬਣਤਰ romfile_loader_file *
romfile_loader_ਲੱਭੋ(ਅਚਲ ਅੱਖਰ *name,
                    ਬਣਤਰ romfile_loader_files *files)
{
    ਪੂਰਨ i;
    ਜੇ (name[ROMFILE_LOADER_FILESZ - 1])
        ਵਾਪਸ ਨਿਰਾ;
    ਲਈ (i = 0; i < files->nfiles; ++i)
        ਜੇ (!strcmp(files->files[i].file->name, name))
            ਵਾਪਸ &files->files[i];
    ਵਾਪਸ ਨਿਰਾ;
}

// Replay "write pointer" entries back to QEMU
ਖਾਲੀ romfile_fw_cfg_resume(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_QEMU)
        ਵਾਪਸ;

    ਬਣਤਰ romfile_wr_ਸੂਚਕ_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ;
    hlist_for_each_ਇੰਦਰਾਜ(ਇੰਦਰਾਜ, &romfile_ਸੂਚਕ_list, ਗੰਢ) {
        qemu_cfg_write_file_simple(&ਇੰਦਰਾਜ->ਸੂਚਕ, ਇੰਦਰਾਜ->ਕੁੰਜੀ,
                                   ਇੰਦਰਾਜ->ਵਿੱਥ_2, ਇੰਦਰਾਜ->ptr_ਮਾਪ);
    }
}

ਸਥਿਰ ਖਾਲੀ romfile_loader_allocate(ਬਣਤਰ romfile_loader_ਇੰਦਰਾਜ_s *ਇੰਦਰਾਜ,
                                    ਬਣਤਰ romfile_loader_files *files)
{
    ਬਣਤਰ zone_s *zone;
    ਬਣਤਰ romfile_loader_file *file = &files->files[files->nfiles];
    ਖਾਲੀ *ਅੰਕੜੇ;
    ਪੂਰਨ ret;
    ਬਿਨਾਨਿਸ਼ਾਨੀ alloc_align = le32_ਮੰਜ਼ਿਲ_cpu(ਇੰਦਰਾਜ->alloc.align);

    ਜੇ (alloc_align & (alloc_align - 1))
        ਜਾਓ err;

    ਚੋਣ (ਇੰਦਰਾਜ->alloc.zone) {
        ਹਾਲਤ ROMFILE_LOADER_ALLOC_ZONE_ਉੱਚਾ:
            zone = &Zone_ਉੱਚਾ;
            ਰੋਕੋ;
        ਹਾਲਤ ROMFILE_LOADER_ALLOC_ZONE_FSEG:
            zone = &ZoneFSeg;
            ਰੋਕੋ;
        ਮੂਲ:
            ਜਾਓ err;
    }
    ਜੇ (alloc_align < MALLOC_MIN_ALIGN)
        alloc_align = MALLOC_MIN_ALIGN;
    ਜੇ (ਇੰਦਰਾਜ->alloc.file[ROMFILE_LOADER_FILESZ - 1])
        ਜਾਓ err;
    file->file = romfile_ਲੱਭੋ(ਇੰਦਰਾਜ->alloc.file);
    ਜੇ (!file->file || !file->file->ਮਾਪ)
        ਵਾਪਸ;
    ਅੰਕੜੇ = _malloc(zone, file->file->ਮਾਪ, alloc_align);
    ਜੇ (!ਅੰਕੜੇ) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    ret = file->file->copy(file->file, ਅੰਕੜੇ, file->file->ਮਾਪ);
    ਜੇ (ret != file->file->ਮਾਪ)
        ਜਾਓ file_err;
    file->ਅੰਕੜੇ = ਅੰਕੜੇ;
    files->nfiles++;
    ਵਾਪਸ;

file_err:
    ਮੁਕਤ(ਅੰਕੜੇ);
err:
    warn_internalerror();
}

ਸਥਿਰ ਖਾਲੀ romfile_loader_ਜੋੜੋ_ਸੂਚਕ(ਬਣਤਰ romfile_loader_ਇੰਦਰਾਜ_s *ਇੰਦਰਾਜ,
                                       ਬਣਤਰ romfile_loader_files *files)
{
    ਬਣਤਰ romfile_loader_file *dest_file;
    ਬਣਤਰ romfile_loader_file *src_file;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ_2 = le32_ਮੰਜ਼ਿਲ_cpu(ਇੰਦਰਾਜ->ਸੂਚਕ.ਵਿੱਥ_2);
    u64 ਸੂਚਕ = 0;

    dest_file = romfile_loader_ਲੱਭੋ(ਇੰਦਰਾਜ->ਸੂਚਕ.dest_file, files);
    src_file = romfile_loader_ਲੱਭੋ(ਇੰਦਰਾਜ->ਸੂਚਕ.src_file, files);

    ਜੇ (!dest_file || !src_file || !dest_file->ਅੰਕੜੇ || !src_file->ਅੰਕੜੇ ||
        ਵਿੱਥ_2 + ਇੰਦਰਾਜ->ਸੂਚਕ.ਮਾਪ < ਵਿੱਥ_2 ||
        ਵਿੱਥ_2 + ਇੰਦਰਾਜ->ਸੂਚਕ.ਮਾਪ > dest_file->file->ਮਾਪ ||
        ਇੰਦਰਾਜ->ਸੂਚਕ.ਮਾਪ < 1 || ਇੰਦਰਾਜ->ਸੂਚਕ.ਮਾਪ > 8 ||
        ਇੰਦਰਾਜ->ਸੂਚਕ.ਮਾਪ & (ਇੰਦਰਾਜ->ਸੂਚਕ.ਮਾਪ - 1))
        ਜਾਓ err;

    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&ਸੂਚਕ, dest_file->ਅੰਕੜੇ + ਵਿੱਥ_2, ਇੰਦਰਾਜ->ਸੂਚਕ.ਮਾਪ);
    ਸੂਚਕ = le64_ਮੰਜ਼ਿਲ_cpu(ਸੂਚਕ);
    ਸੂਚਕ += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)src_file->ਅੰਕੜੇ;
    ਸੂਚਕ = cpu_ਮੰਜ਼ਿਲ_le64(ਸੂਚਕ);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(dest_file->ਅੰਕੜੇ + ਵਿੱਥ_2, &ਸੂਚਕ, ਇੰਦਰਾਜ->ਸੂਚਕ.ਮਾਪ);

    ਵਾਪਸ;
err:
    warn_internalerror();
}

ਸਥਿਰ ਖਾਲੀ romfile_loader_ਜੋੜੋ_checksum(ਬਣਤਰ romfile_loader_ਇੰਦਰਾਜ_s *ਇੰਦਰਾਜ,
                                        ਬਣਤਰ romfile_loader_files *files)
{
    ਬਣਤਰ romfile_loader_file *file;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ_2 = le32_ਮੰਜ਼ਿਲ_cpu(ਇੰਦਰਾਜ->cksum.ਵਿੱਥ_2);
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ = le32_ਮੰਜ਼ਿਲ_cpu(ਇੰਦਰਾਜ->cksum.ਸ਼ੁਰੂ);
    ਬਿਨਾਨਿਸ਼ਾਨੀ len = le32_ਮੰਜ਼ਿਲ_cpu(ਇੰਦਰਾਜ->cksum.ਲੰਮਾਈ);
    u8 *ਅੰਕੜੇ;

    file = romfile_loader_ਲੱਭੋ(ਇੰਦਰਾਜ->cksum.file, files);

    ਜੇ (!file || !file->ਅੰਕੜੇ || ਵਿੱਥ_2 >= file->file->ਮਾਪ ||
        ਸ਼ੁਰੂ + len < ਸ਼ੁਰੂ || ਸ਼ੁਰੂ + len > file->file->ਮਾਪ)
        ਜਾਓ err;

    ਅੰਕੜੇ = file->ਅੰਕੜੇ + ਵਿੱਥ_2;
    *ਅੰਕੜੇ -= checksum(file->ਅੰਕੜੇ + ਸ਼ੁਰੂ, len);

    ਵਾਪਸ;
err:
    warn_internalerror();
}

ਸਥਿਰ ਖਾਲੀ romfile_loader_write_ਸੂਚਕ(ਬਣਤਰ romfile_loader_ਇੰਦਰਾਜ_s *ਇੰਦਰਾਜ,
                                         ਬਣਤਰ romfile_loader_files *files)
{
    ਬਣਤਰ romfile_s *dest_file;
    ਬਣਤਰ romfile_loader_file *src_file;
    ਬਿਨਾਨਿਸ਼ਾਨੀ dst_ਵਿੱਥ = le32_ਮੰਜ਼ਿਲ_cpu(ਇੰਦਰਾਜ->wr_ਸੂਚਕ.dst_ਵਿੱਥ);
    ਬਿਨਾਨਿਸ਼ਾਨੀ src_ਵਿੱਥ = le32_ਮੰਜ਼ਿਲ_cpu(ਇੰਦਰਾਜ->wr_ਸੂਚਕ.src_ਵਿੱਥ);
    u64 ਸੂਚਕ = 0;

    /* Writing back to a file that may not be loaded in RAM */
    dest_file = romfile_ਲੱਭੋ(ਇੰਦਰਾਜ->wr_ਸੂਚਕ.dest_file);
    src_file = romfile_loader_ਲੱਭੋ(ਇੰਦਰਾਜ->wr_ਸੂਚਕ.src_file, files);

    ਜੇ (!dest_file || !src_file || !src_file->ਅੰਕੜੇ ||
        dst_ਵਿੱਥ + ਇੰਦਰਾਜ->wr_ਸੂਚਕ.ਮਾਪ < dst_ਵਿੱਥ ||
        dst_ਵਿੱਥ + ਇੰਦਰਾਜ->wr_ਸੂਚਕ.ਮਾਪ > dest_file->ਮਾਪ ||
        src_ਵਿੱਥ >= src_file->file->ਮਾਪ ||
        ਇੰਦਰਾਜ->wr_ਸੂਚਕ.ਮਾਪ < 1 || ਇੰਦਰਾਜ->wr_ਸੂਚਕ.ਮਾਪ > 8 ||
        ਇੰਦਰਾਜ->wr_ਸੂਚਕ.ਮਾਪ & (ਇੰਦਰਾਜ->wr_ਸੂਚਕ.ਮਾਪ - 1)) {
        ਜਾਓ err;
    }

    ਸੂਚਕ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)src_file->ਅੰਕੜੇ + src_ਵਿੱਥ;
    /* Make sure the pointer fits within wr_pointer.size */
    ਜੇ ((ਇੰਦਰਾਜ->wr_ਸੂਚਕ.ਮਾਪ != ਆਕਾਰ(u64)) &&
        ((ਸੂਚਕ >> (ਇੰਦਰਾਜ->wr_ਸੂਚਕ.ਮਾਪ * 8)) > 0)) {
        ਜਾਓ err;
    }
    ਸੂਚਕ = cpu_ਮੰਜ਼ਿਲ_le64(ਸੂਚਕ);

    /* Only supported on QEMU */
    ਜੇ (qemu_cfg_write_file(&ਸੂਚਕ, dest_file, dst_ਵਿੱਥ,
                            ਇੰਦਰਾਜ->wr_ਸੂਚਕ.ਮਾਪ) != ਇੰਦਰਾਜ->wr_ਸੂਚਕ.ਮਾਪ) {
        ਜਾਓ err;
    }

    /* Store the info so it can replayed later if necessary */
    ਬਣਤਰ romfile_wr_ਸੂਚਕ_ਇੰਦਰਾਜ *store = malloc_ਉੱਚਾ(ਆਕਾਰ(*store));
    ਜੇ (!store) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    store->ਸੂਚਕ = ਸੂਚਕ;
    store->ਕੁੰਜੀ = qemu_get_romfile_ਕੁੰਜੀ(dest_file);
    store->ਵਿੱਥ_2 = dst_ਵਿੱਥ;
    store->ptr_ਮਾਪ = ਇੰਦਰਾਜ->wr_ਸੂਚਕ.ਮਾਪ;
    hlist_ਜੋੜੋ_ਸਿਰ(&store->ਗੰਢ, &romfile_ਸੂਚਕ_list);

    ਵਾਪਸ;
 err:
    warn_internalerror();
}

ਪੂਰਨ romfile_loader_execute(ਅਚਲ ਅੱਖਰ *name)
{
    ਬਣਤਰ romfile_loader_ਇੰਦਰਾਜ_s *ਇੰਦਰਾਜ;
    ਪੂਰਨ ਮਾਪ, ਵਿੱਥ_2 = 0, nfiles;
    ਬਣਤਰ romfile_loader_files *files;
    ਖਾਲੀ *ਅੰਕੜੇ = romfile_loadfile(name, &ਮਾਪ);
    ਜੇ (!ਅੰਕੜੇ)
        ਵਾਪਸ -1;

    ਜੇ (ਮਾਪ % ਆਕਾਰ(*ਇੰਦਰਾਜ)) {
        warn_internalerror();
        ਜਾਓ err;
    }

    /* (over)estimate the number of files to load. */
    nfiles = ਮਾਪ / ਆਕਾਰ(*ਇੰਦਰਾਜ);
    files = malloc_tmp(ਆਕਾਰ(*files) + nfiles * ਆਕਾਰ(files->files[0]));
    ਜੇ (!files) {
        warn_noalloc();
        ਜਾਓ err;
    }
    files->nfiles = 0;

    ਲਈ (ਵਿੱਥ_2 = 0; ਵਿੱਥ_2 < ਮਾਪ; ਵਿੱਥ_2 += ਆਕਾਰ(*ਇੰਦਰਾਜ)) {
        ਇੰਦਰਾਜ = ਅੰਕੜੇ + ਵਿੱਥ_2;
        ਚੋਣ (le32_ਮੰਜ਼ਿਲ_cpu(ਇੰਦਰਾਜ->command)) {
                ਹਾਲਤ ROMFILE_LOADER_COMMAND_ALLOCATE:
                        romfile_loader_allocate(ਇੰਦਰਾਜ, files);
                        ਰੋਕੋ;
                ਹਾਲਤ ROMFILE_LOADER_COMMAND_ਜੋੜੋ_ਸੂਚਕ:
                        romfile_loader_ਜੋੜੋ_ਸੂਚਕ(ਇੰਦਰਾਜ, files);
                        ਰੋਕੋ;
                ਹਾਲਤ ROMFILE_LOADER_COMMAND_ਜੋੜੋ_CHECKSUM:
                        romfile_loader_ਜੋੜੋ_checksum(ਇੰਦਰਾਜ, files);
                        ਰੋਕੋ;
                ਹਾਲਤ ROMFILE_LOADER_COMMAND_WRITE_ਸੂਚਕ:
                        romfile_loader_write_ਸੂਚਕ(ਇੰਦਰਾਜ, files);
                        ਰੋਕੋ;
                ਮੂਲ:
                        /* Skip commands that we don't recognize. */
                        ਰੋਕੋ;
        }
    }

    ਮੁਕਤ(files);
    ਮੁਕਤ(ਅੰਕੜੇ);
    ਵਾਪਸ 0;

err:
    ਮੁਕਤ(ਅੰਕੜੇ);
    ਵਾਪਸ -1;
}
