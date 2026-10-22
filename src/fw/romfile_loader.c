#شامل "romfile_loader.h"
#شامل "byteorder.h" // leXX_to_cpu/cpu_to_leXX
#شامل "util.h" // checksum
#شامل "string.h" // strcmp
#شامل "romfile.h" // struct romfile_s
#شامل "malloc.h" // Zone*, _malloc
#شامل "list.h" // struct hlist_node
#شامل "output.h" // warn_*
#شامل "paravirt.h" // qemu_cfg_write_file

ساخت romfile_loader_file {
    ساخت romfile_s *file;
    خالی *ڈیٹا;
};
ساخت romfile_loader_files {
    صحیح nfiles;
    ساخت romfile_loader_file files[];
};

// Data structures for storing "write pointer" entries for possible replay
ساخت romfile_wr_اشارہ_گر_اندراج {
    u64 اشارہ_گر;
    u32 انحراف_2;
    u16 کلید;
    u8 ptr_سائز;
    ساخت hlist_گرہ گرہ;
};
ساکن ساخت hlist_سر romfile_اشارہ_گر_list;

ساکن ساخت romfile_loader_file *
romfile_loader_ڈھونڈو(ثابت حرف *name,
                    ساخت romfile_loader_files *files)
{
    صحیح i;
    اگر (name[ROMFILE_LOADER_FILESZ - 1])
        واپس تہی;
    برائے (i = 0; i < files->nfiles; ++i)
        اگر (!strcmp(files->files[i].file->name, name))
            واپس &files->files[i];
    واپس تہی;
}

// Replay "write pointer" entries back to QEMU
خالی romfile_fw_cfg_resume(خالی)
{
    اگر (!CONFIG_QEMU)
        واپس;

    ساخت romfile_wr_اشارہ_گر_اندراج *اندراج_2;
    hlist_for_each_اندراج(اندراج_2, &romfile_اشارہ_گر_list, گرہ) {
        qemu_cfg_write_file_simple(&اندراج_2->اشارہ_گر, اندراج_2->کلید,
                                   اندراج_2->انحراف_2, اندراج_2->ptr_سائز);
    }
}

ساکن خالی romfile_loader_allocate(ساخت romfile_loader_اندراج_s *اندراج_2,
                                    ساخت romfile_loader_files *files)
{
    ساخت zone_s *zone;
    ساخت romfile_loader_file *file = &files->files[files->nfiles];
    خالی *ڈیٹا;
    صحیح ret;
    غیرعلامتی alloc_align = le32_مقصد_cpu(اندراج_2->alloc.align);

    اگر (alloc_align & (alloc_align - 1))
        جاؤ err;

    انتخاب (اندراج_2->alloc.zone) {
        حالت ROMFILE_LOADER_ALLOC_ZONE_بلند:
            zone = &Zone_بلند;
            توڑو;
        حالت ROMFILE_LOADER_ALLOC_ZONE_FSEG:
            zone = &ZoneFSeg;
            توڑو;
        طےشدہ:
            جاؤ err;
    }
    اگر (alloc_align < MALLOC_MIN_ALIGN)
        alloc_align = MALLOC_MIN_ALIGN;
    اگر (اندراج_2->alloc.file[ROMFILE_LOADER_FILESZ - 1])
        جاؤ err;
    file->file = romfile_ڈھونڈو(اندراج_2->alloc.file);
    اگر (!file->file || !file->file->سائز)
        واپس;
    ڈیٹا = _malloc(zone, file->file->سائز, alloc_align);
    اگر (!ڈیٹا) {
        warn_noalloc();
        واپس;
    }
    ret = file->file->copy(file->file, ڈیٹا, file->file->سائز);
    اگر (ret != file->file->سائز)
        جاؤ file_err;
    file->ڈیٹا = ڈیٹا;
    files->nfiles++;
    واپس;

file_err:
    آزاد(ڈیٹا);
err:
    warn_internalerror();
}

ساکن خالی romfile_loader_جوڑو_اشارہ_گر(ساخت romfile_loader_اندراج_s *اندراج_2,
                                       ساخت romfile_loader_files *files)
{
    ساخت romfile_loader_file *dest_file;
    ساخت romfile_loader_file *src_file;
    غیرعلامتی انحراف_2 = le32_مقصد_cpu(اندراج_2->اشارہ_گر.انحراف_2);
    u64 اشارہ_گر = 0;

    dest_file = romfile_loader_ڈھونڈو(اندراج_2->اشارہ_گر.dest_file, files);
    src_file = romfile_loader_ڈھونڈو(اندراج_2->اشارہ_گر.src_file, files);

    اگر (!dest_file || !src_file || !dest_file->ڈیٹا || !src_file->ڈیٹا ||
        انحراف_2 + اندراج_2->اشارہ_گر.سائز < انحراف_2 ||
        انحراف_2 + اندراج_2->اشارہ_گر.سائز > dest_file->file->سائز ||
        اندراج_2->اشارہ_گر.سائز < 1 || اندراج_2->اشارہ_گر.سائز > 8 ||
        اندراج_2->اشارہ_گر.سائز & (اندراج_2->اشارہ_گر.سائز - 1))
        جاؤ err;

    حافظہ_نقل(&اشارہ_گر, dest_file->ڈیٹا + انحراف_2, اندراج_2->اشارہ_گر.سائز);
    اشارہ_گر = le64_مقصد_cpu(اشارہ_گر);
    اشارہ_گر += (غیرعلامتی طویل)src_file->ڈیٹا;
    اشارہ_گر = cpu_مقصد_le64(اشارہ_گر);
    حافظہ_نقل(dest_file->ڈیٹا + انحراف_2, &اشارہ_گر, اندراج_2->اشارہ_گر.سائز);

    واپس;
err:
    warn_internalerror();
}

ساکن خالی romfile_loader_جوڑو_checksum(ساخت romfile_loader_اندراج_s *اندراج_2,
                                        ساخت romfile_loader_files *files)
{
    ساخت romfile_loader_file *file;
    غیرعلامتی انحراف_2 = le32_مقصد_cpu(اندراج_2->cksum.انحراف_2);
    غیرعلامتی آغاز = le32_مقصد_cpu(اندراج_2->cksum.آغاز);
    غیرعلامتی len = le32_مقصد_cpu(اندراج_2->cksum.لمبائی);
    u8 *ڈیٹا;

    file = romfile_loader_ڈھونڈو(اندراج_2->cksum.file, files);

    اگر (!file || !file->ڈیٹا || انحراف_2 >= file->file->سائز ||
        آغاز + len < آغاز || آغاز + len > file->file->سائز)
        جاؤ err;

    ڈیٹا = file->ڈیٹا + انحراف_2;
    *ڈیٹا -= checksum(file->ڈیٹا + آغاز, len);

    واپس;
err:
    warn_internalerror();
}

ساکن خالی romfile_loader_write_اشارہ_گر(ساخت romfile_loader_اندراج_s *اندراج_2,
                                         ساخت romfile_loader_files *files)
{
    ساخت romfile_s *dest_file;
    ساخت romfile_loader_file *src_file;
    غیرعلامتی dst_انحراف = le32_مقصد_cpu(اندراج_2->wr_اشارہ_گر.dst_انحراف);
    غیرعلامتی src_انحراف = le32_مقصد_cpu(اندراج_2->wr_اشارہ_گر.src_انحراف);
    u64 اشارہ_گر = 0;

    /* Writing back to a file that may not be loaded in RAM */
    dest_file = romfile_ڈھونڈو(اندراج_2->wr_اشارہ_گر.dest_file);
    src_file = romfile_loader_ڈھونڈو(اندراج_2->wr_اشارہ_گر.src_file, files);

    اگر (!dest_file || !src_file || !src_file->ڈیٹا ||
        dst_انحراف + اندراج_2->wr_اشارہ_گر.سائز < dst_انحراف ||
        dst_انحراف + اندراج_2->wr_اشارہ_گر.سائز > dest_file->سائز ||
        src_انحراف >= src_file->file->سائز ||
        اندراج_2->wr_اشارہ_گر.سائز < 1 || اندراج_2->wr_اشارہ_گر.سائز > 8 ||
        اندراج_2->wr_اشارہ_گر.سائز & (اندراج_2->wr_اشارہ_گر.سائز - 1)) {
        جاؤ err;
    }

    اشارہ_گر = (غیرعلامتی طویل)src_file->ڈیٹا + src_انحراف;
    /* Make sure the pointer fits within wr_pointer.size */
    اگر ((اندراج_2->wr_اشارہ_گر.سائز != حجم(u64)) &&
        ((اشارہ_گر >> (اندراج_2->wr_اشارہ_گر.سائز * 8)) > 0)) {
        جاؤ err;
    }
    اشارہ_گر = cpu_مقصد_le64(اشارہ_گر);

    /* Only supported on QEMU */
    اگر (qemu_cfg_write_file(&اشارہ_گر, dest_file, dst_انحراف,
                            اندراج_2->wr_اشارہ_گر.سائز) != اندراج_2->wr_اشارہ_گر.سائز) {
        جاؤ err;
    }

    /* Store the info so it can replayed later if necessary */
    ساخت romfile_wr_اشارہ_گر_اندراج *store = malloc_بلند(حجم(*store));
    اگر (!store) {
        warn_noalloc();
        واپس;
    }
    store->اشارہ_گر = اشارہ_گر;
    store->کلید = qemu_get_romfile_کلید(dest_file);
    store->انحراف_2 = dst_انحراف;
    store->ptr_سائز = اندراج_2->wr_اشارہ_گر.سائز;
    hlist_جوڑو_سر(&store->گرہ, &romfile_اشارہ_گر_list);

    واپس;
 err:
    warn_internalerror();
}

صحیح romfile_loader_execute(ثابت حرف *name)
{
    ساخت romfile_loader_اندراج_s *اندراج_2;
    صحیح سائز, انحراف_2 = 0, nfiles;
    ساخت romfile_loader_files *files;
    خالی *ڈیٹا = romfile_loadfile(name, &سائز);
    اگر (!ڈیٹا)
        واپس -1;

    اگر (سائز % حجم(*اندراج_2)) {
        warn_internalerror();
        جاؤ err;
    }

    /* (over)estimate the number of files to load. */
    nfiles = سائز / حجم(*اندراج_2);
    files = malloc_tmp(حجم(*files) + nfiles * حجم(files->files[0]));
    اگر (!files) {
        warn_noalloc();
        جاؤ err;
    }
    files->nfiles = 0;

    برائے (انحراف_2 = 0; انحراف_2 < سائز; انحراف_2 += حجم(*اندراج_2)) {
        اندراج_2 = ڈیٹا + انحراف_2;
        انتخاب (le32_مقصد_cpu(اندراج_2->command)) {
                حالت ROMFILE_LOADER_COMMAND_ALLOCATE:
                        romfile_loader_allocate(اندراج_2, files);
                        توڑو;
                حالت ROMFILE_LOADER_COMMAND_جوڑو_اشارہ_گر:
                        romfile_loader_جوڑو_اشارہ_گر(اندراج_2, files);
                        توڑو;
                حالت ROMFILE_LOADER_COMMAND_جوڑو_CHECKSUM:
                        romfile_loader_جوڑو_checksum(اندراج_2, files);
                        توڑو;
                حالت ROMFILE_LOADER_COMMAND_WRITE_اشارہ_گر:
                        romfile_loader_write_اشارہ_گر(اندراج_2, files);
                        توڑو;
                طےشدہ:
                        /* Skip commands that we don't recognize. */
                        توڑو;
        }
    }

    آزاد(files);
    آزاد(ڈیٹا);
    واپس 0;

err:
    آزاد(ڈیٹا);
    واپس -1;
}
