#ضمن "romfile_loader.h"
#ضمن "byteorder.h" // leXX_to_cpu/cpu_to_leXX
#ضمن "util.h" // checksum
#ضمن "string.h" // strcmp
#ضمن "romfile.h" // struct romfile_s
#ضمن "malloc.h" // Zone*, _malloc
#ضمن "list.h" // struct hlist_node
#ضمن "output.h" // warn_*
#ضمن "paravirt.h" // qemu_cfg_write_file

بنية romfile_loader_file {
    بنية romfile_s *file;
    فراغ *داده;
};
بنية romfile_loader_files {
    صحيح nfiles;
    بنية romfile_loader_file files[];
};

// Data structures for storing "write pointer" entries for possible replay
بنية romfile_wr_اشاره_گر_مدخل {
    u64 اشاره_گر;
    u32 جابه_جایی_2;
    u16 کلید;
    u8 ptr_اندازه;
    بنية hlist_گره گره;
};
ساكن بنية hlist_سر romfile_اشاره_گر_list;

ساكن بنية romfile_loader_file *
romfile_loader_پیدا_کن(ثابت حرف *name,
                    بنية romfile_loader_files *files)
{
    صحيح i;
    إذا (name[ROMFILE_LOADER_FILESZ - 1])
        أرجع عدم;
    لكل (i = 0; i < files->nfiles; ++i)
        إذا (!strcmp(files->files[i].file->name, name))
            أرجع &files->files[i];
    أرجع عدم;
}

// Replay "write pointer" entries back to QEMU
فراغ romfile_fw_cfg_resume(فراغ)
{
    إذا (!CONFIG_QEMU)
        أرجع;

    بنية romfile_wr_اشاره_گر_مدخل *مدخل;
    hlist_for_each_مدخل(مدخل, &romfile_اشاره_گر_list, گره) {
        qemu_cfg_write_file_simple(&مدخل->اشاره_گر, مدخل->کلید,
                                   مدخل->جابه_جایی_2, مدخل->ptr_اندازه);
    }
}

ساكن فراغ romfile_loader_allocate(بنية romfile_loader_مدخل_s *مدخل,
                                    بنية romfile_loader_files *files)
{
    بنية zone_s *zone;
    بنية romfile_loader_file *file = &files->files[files->nfiles];
    فراغ *داده;
    صحيح ret;
    بیعلامت alloc_align = le32_مقصد_cpu(مدخل->alloc.align);

    إذا (alloc_align & (alloc_align - 1))
        اذهب err;

    بدل (مدخل->alloc.zone) {
        حالة ROMFILE_LOADER_ALLOC_ZONE_بالا:
            zone = &Zone_بالا;
            اكسر;
        حالة ROMFILE_LOADER_ALLOC_ZONE_FSEG:
            zone = &ZoneFSeg;
            اكسر;
        افتراضي:
            اذهب err;
    }
    إذا (alloc_align < MALLOC_MIN_ALIGN)
        alloc_align = MALLOC_MIN_ALIGN;
    إذا (مدخل->alloc.file[ROMFILE_LOADER_FILESZ - 1])
        اذهب err;
    file->file = romfile_پیدا_کن(مدخل->alloc.file);
    إذا (!file->file || !file->file->اندازه)
        أرجع;
    داده = _malloc(zone, file->file->اندازه, alloc_align);
    إذا (!داده) {
        warn_noalloc();
        أرجع;
    }
    ret = file->file->copy(file->file, داده, file->file->اندازه);
    إذا (ret != file->file->اندازه)
        اذهب file_err;
    file->داده = داده;
    files->nfiles++;
    أرجع;

file_err:
    حرر(داده);
err:
    warn_internalerror();
}

ساكن فراغ romfile_loader_جمع_کن_اشاره_گر(بنية romfile_loader_مدخل_s *مدخل,
                                       بنية romfile_loader_files *files)
{
    بنية romfile_loader_file *dest_file;
    بنية romfile_loader_file *src_file;
    بیعلامت جابه_جایی_2 = le32_مقصد_cpu(مدخل->اشاره_گر.جابه_جایی_2);
    u64 اشاره_گر = 0;

    dest_file = romfile_loader_پیدا_کن(مدخل->اشاره_گر.dest_file, files);
    src_file = romfile_loader_پیدا_کن(مدخل->اشاره_گر.src_file, files);

    إذا (!dest_file || !src_file || !dest_file->داده || !src_file->داده ||
        جابه_جایی_2 + مدخل->اشاره_گر.اندازه < جابه_جایی_2 ||
        جابه_جایی_2 + مدخل->اشاره_گر.اندازه > dest_file->file->اندازه ||
        مدخل->اشاره_گر.اندازه < 1 || مدخل->اشاره_گر.اندازه > 8 ||
        مدخل->اشاره_گر.اندازه & (مدخل->اشاره_گر.اندازه - 1))
        اذهب err;

    کپی_حافظه(&اشاره_گر, dest_file->داده + جابه_جایی_2, مدخل->اشاره_گر.اندازه);
    اشاره_گر = le64_مقصد_cpu(اشاره_گر);
    اشاره_گر += (بیعلامت دراز)src_file->داده;
    اشاره_گر = cpu_مقصد_le64(اشاره_گر);
    کپی_حافظه(dest_file->داده + جابه_جایی_2, &اشاره_گر, مدخل->اشاره_گر.اندازه);

    أرجع;
err:
    warn_internalerror();
}

ساكن فراغ romfile_loader_جمع_کن_checksum(بنية romfile_loader_مدخل_s *مدخل,
                                        بنية romfile_loader_files *files)
{
    بنية romfile_loader_file *file;
    بیعلامت جابه_جایی_2 = le32_مقصد_cpu(مدخل->cksum.جابه_جایی_2);
    بیعلامت شروع = le32_مقصد_cpu(مدخل->cksum.شروع);
    بیعلامت len = le32_مقصد_cpu(مدخل->cksum.درازی);
    u8 *داده;

    file = romfile_loader_پیدا_کن(مدخل->cksum.file, files);

    إذا (!file || !file->داده || جابه_جایی_2 >= file->file->اندازه ||
        شروع + len < شروع || شروع + len > file->file->اندازه)
        اذهب err;

    داده = file->داده + جابه_جایی_2;
    *داده -= checksum(file->داده + شروع, len);

    أرجع;
err:
    warn_internalerror();
}

ساكن فراغ romfile_loader_write_اشاره_گر(بنية romfile_loader_مدخل_s *مدخل,
                                         بنية romfile_loader_files *files)
{
    بنية romfile_s *dest_file;
    بنية romfile_loader_file *src_file;
    بیعلامت dst_جابه_جایی = le32_مقصد_cpu(مدخل->wr_اشاره_گر.dst_جابه_جایی);
    بیعلامت src_جابه_جایی = le32_مقصد_cpu(مدخل->wr_اشاره_گر.src_جابه_جایی);
    u64 اشاره_گر = 0;

    /* Writing back to a file that may not be loaded in RAM */
    dest_file = romfile_پیدا_کن(مدخل->wr_اشاره_گر.dest_file);
    src_file = romfile_loader_پیدا_کن(مدخل->wr_اشاره_گر.src_file, files);

    إذا (!dest_file || !src_file || !src_file->داده ||
        dst_جابه_جایی + مدخل->wr_اشاره_گر.اندازه < dst_جابه_جایی ||
        dst_جابه_جایی + مدخل->wr_اشاره_گر.اندازه > dest_file->اندازه ||
        src_جابه_جایی >= src_file->file->اندازه ||
        مدخل->wr_اشاره_گر.اندازه < 1 || مدخل->wr_اشاره_گر.اندازه > 8 ||
        مدخل->wr_اشاره_گر.اندازه & (مدخل->wr_اشاره_گر.اندازه - 1)) {
        اذهب err;
    }

    اشاره_گر = (بیعلامت دراز)src_file->داده + src_جابه_جایی;
    /* Make sure the pointer fits within wr_pointer.size */
    إذا ((مدخل->wr_اشاره_گر.اندازه != حجم(u64)) &&
        ((اشاره_گر >> (مدخل->wr_اشاره_گر.اندازه * 8)) > 0)) {
        اذهب err;
    }
    اشاره_گر = cpu_مقصد_le64(اشاره_گر);

    /* Only supported on QEMU */
    إذا (qemu_cfg_write_file(&اشاره_گر, dest_file, dst_جابه_جایی,
                            مدخل->wr_اشاره_گر.اندازه) != مدخل->wr_اشاره_گر.اندازه) {
        اذهب err;
    }

    /* Store the info so it can replayed later if necessary */
    بنية romfile_wr_اشاره_گر_مدخل *store = malloc_بالا(حجم(*store));
    إذا (!store) {
        warn_noalloc();
        أرجع;
    }
    store->اشاره_گر = اشاره_گر;
    store->کلید = qemu_get_romfile_کلید(dest_file);
    store->جابه_جایی_2 = dst_جابه_جایی;
    store->ptr_اندازه = مدخل->wr_اشاره_گر.اندازه;
    hlist_جمع_کن_سر(&store->گره, &romfile_اشاره_گر_list);

    أرجع;
 err:
    warn_internalerror();
}

صحيح romfile_loader_execute(ثابت حرف *name)
{
    بنية romfile_loader_مدخل_s *مدخل;
    صحيح اندازه, جابه_جایی_2 = 0, nfiles;
    بنية romfile_loader_files *files;
    فراغ *داده = romfile_loadfile(name, &اندازه);
    إذا (!داده)
        أرجع -1;

    إذا (اندازه % حجم(*مدخل)) {
        warn_internalerror();
        اذهب err;
    }

    /* (over)estimate the number of files to load. */
    nfiles = اندازه / حجم(*مدخل);
    files = malloc_tmp(حجم(*files) + nfiles * حجم(files->files[0]));
    إذا (!files) {
        warn_noalloc();
        اذهب err;
    }
    files->nfiles = 0;

    لكل (جابه_جایی_2 = 0; جابه_جایی_2 < اندازه; جابه_جایی_2 += حجم(*مدخل)) {
        مدخل = داده + جابه_جایی_2;
        بدل (le32_مقصد_cpu(مدخل->command)) {
                حالة ROMFILE_LOADER_COMMAND_ALLOCATE:
                        romfile_loader_allocate(مدخل, files);
                        اكسر;
                حالة ROMFILE_LOADER_COMMAND_جمع_کن_اشاره_گر:
                        romfile_loader_جمع_کن_اشاره_گر(مدخل, files);
                        اكسر;
                حالة ROMFILE_LOADER_COMMAND_جمع_کن_CHECKSUM:
                        romfile_loader_جمع_کن_checksum(مدخل, files);
                        اكسر;
                حالة ROMFILE_LOADER_COMMAND_WRITE_اشاره_گر:
                        romfile_loader_write_اشاره_گر(مدخل, files);
                        اكسر;
                افتراضي:
                        /* Skip commands that we don't recognize. */
                        اكسر;
        }
    }

    حرر(files);
    حرر(داده);
    أرجع 0;

err:
    حرر(داده);
    أرجع -1;
}
