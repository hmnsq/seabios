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
    فراغ *معلومات;
};
بنية romfile_loader_files {
    صحيح nfiles;
    بنية romfile_loader_file files[];
};

// Data structures for storing "write pointer" entries for possible replay
بنية romfile_wr_اشاره_ګر_ثبت {
    u64 اشاره_ګر;
    u32 دننه_واټن_2;
    u16 کلی;
    u8 ptr_اندازه;
    بنية hlist_غوټه غوټه;
};
ساكن بنية hlist_سر romfile_اشاره_ګر_list;

ساكن بنية romfile_loader_file *
romfile_loader_پیدا_کړه(ثابت حرف *name,
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

    بنية romfile_wr_اشاره_ګر_ثبت *ثبت;
    hlist_for_each_ثبت(ثبت, &romfile_اشاره_ګر_list, غوټه) {
        qemu_cfg_write_file_simple(&ثبت->اشاره_ګر, ثبت->کلی,
                                   ثبت->دننه_واټن_2, ثبت->ptr_اندازه);
    }
}

ساكن فراغ romfile_loader_allocate(بنية romfile_loader_ثبت_s *ثبت,
                                    بنية romfile_loader_files *files)
{
    بنية zone_s *zone;
    بنية romfile_loader_file *file = &files->files[files->nfiles];
    فراغ *معلومات;
    صحيح ret;
    بینښه alloc_align = le32_منزل_cpu(ثبت->alloc.align);

    إذا (alloc_align & (alloc_align - 1))
        اذهب err;

    بدل (ثبت->alloc.zone) {
        حالة ROMFILE_LOADER_ALLOC_ZONE_لوړ:
            zone = &Zone_لوړ;
            اكسر;
        حالة ROMFILE_LOADER_ALLOC_ZONE_FSEG:
            zone = &ZoneFSeg;
            اكسر;
        افتراضي:
            اذهب err;
    }
    إذا (alloc_align < MALLOC_MIN_ALIGN)
        alloc_align = MALLOC_MIN_ALIGN;
    إذا (ثبت->alloc.file[ROMFILE_LOADER_FILESZ - 1])
        اذهب err;
    file->file = romfile_پیدا_کړه(ثبت->alloc.file);
    إذا (!file->file || !file->file->اندازه)
        أرجع;
    معلومات = _malloc(zone, file->file->اندازه, alloc_align);
    إذا (!معلومات) {
        warn_noalloc();
        أرجع;
    }
    ret = file->file->copy(file->file, معلومات, file->file->اندازه);
    إذا (ret != file->file->اندازه)
        اذهب file_err;
    file->معلومات = معلومات;
    files->nfiles++;
    أرجع;

file_err:
    حرر(معلومات);
err:
    warn_internalerror();
}

ساكن فراغ romfile_loader_جمع_کړه_اشاره_ګر(بنية romfile_loader_ثبت_s *ثبت,
                                       بنية romfile_loader_files *files)
{
    بنية romfile_loader_file *dest_file;
    بنية romfile_loader_file *src_file;
    بینښه دننه_واټن_2 = le32_منزل_cpu(ثبت->اشاره_ګر.دننه_واټن_2);
    u64 اشاره_ګر = 0;

    dest_file = romfile_loader_پیدا_کړه(ثبت->اشاره_ګر.dest_file, files);
    src_file = romfile_loader_پیدا_کړه(ثبت->اشاره_ګر.src_file, files);

    إذا (!dest_file || !src_file || !dest_file->معلومات || !src_file->معلومات ||
        دننه_واټن_2 + ثبت->اشاره_ګر.اندازه < دننه_واټن_2 ||
        دننه_واټن_2 + ثبت->اشاره_ګر.اندازه > dest_file->file->اندازه ||
        ثبت->اشاره_ګر.اندازه < 1 || ثبت->اشاره_ګر.اندازه > 8 ||
        ثبت->اشاره_ګر.اندازه & (ثبت->اشاره_ګر.اندازه - 1))
        اذهب err;

    حافظه_نقل(&اشاره_ګر, dest_file->معلومات + دننه_واټن_2, ثبت->اشاره_ګر.اندازه);
    اشاره_ګر = le64_منزل_cpu(اشاره_ګر);
    اشاره_ګر += (بینښه اوږد)src_file->معلومات;
    اشاره_ګر = cpu_منزل_le64(اشاره_ګر);
    حافظه_نقل(dest_file->معلومات + دننه_واټن_2, &اشاره_ګر, ثبت->اشاره_ګر.اندازه);

    أرجع;
err:
    warn_internalerror();
}

ساكن فراغ romfile_loader_جمع_کړه_checksum(بنية romfile_loader_ثبت_s *ثبت,
                                        بنية romfile_loader_files *files)
{
    بنية romfile_loader_file *file;
    بینښه دننه_واټن_2 = le32_منزل_cpu(ثبت->cksum.دننه_واټن_2);
    بینښه پیل = le32_منزل_cpu(ثبت->cksum.پیل);
    بینښه len = le32_منزل_cpu(ثبت->cksum.اوږدتیا);
    u8 *معلومات;

    file = romfile_loader_پیدا_کړه(ثبت->cksum.file, files);

    إذا (!file || !file->معلومات || دننه_واټن_2 >= file->file->اندازه ||
        پیل + len < پیل || پیل + len > file->file->اندازه)
        اذهب err;

    معلومات = file->معلومات + دننه_واټن_2;
    *معلومات -= checksum(file->معلومات + پیل, len);

    أرجع;
err:
    warn_internalerror();
}

ساكن فراغ romfile_loader_write_اشاره_ګر(بنية romfile_loader_ثبت_s *ثبت,
                                         بنية romfile_loader_files *files)
{
    بنية romfile_s *dest_file;
    بنية romfile_loader_file *src_file;
    بینښه dst_دننه_واټن = le32_منزل_cpu(ثبت->wr_اشاره_ګر.dst_دننه_واټن);
    بینښه src_دننه_واټن = le32_منزل_cpu(ثبت->wr_اشاره_ګر.src_دننه_واټن);
    u64 اشاره_ګر = 0;

    /* Writing back to a file that may not be loaded in RAM */
    dest_file = romfile_پیدا_کړه(ثبت->wr_اشاره_ګر.dest_file);
    src_file = romfile_loader_پیدا_کړه(ثبت->wr_اشاره_ګر.src_file, files);

    إذا (!dest_file || !src_file || !src_file->معلومات ||
        dst_دننه_واټن + ثبت->wr_اشاره_ګر.اندازه < dst_دننه_واټن ||
        dst_دننه_واټن + ثبت->wr_اشاره_ګر.اندازه > dest_file->اندازه ||
        src_دننه_واټن >= src_file->file->اندازه ||
        ثبت->wr_اشاره_ګر.اندازه < 1 || ثبت->wr_اشاره_ګر.اندازه > 8 ||
        ثبت->wr_اشاره_ګر.اندازه & (ثبت->wr_اشاره_ګر.اندازه - 1)) {
        اذهب err;
    }

    اشاره_ګر = (بینښه اوږد)src_file->معلومات + src_دننه_واټن;
    /* Make sure the pointer fits within wr_pointer.size */
    إذا ((ثبت->wr_اشاره_ګر.اندازه != حجم(u64)) &&
        ((اشاره_ګر >> (ثبت->wr_اشاره_ګر.اندازه * 8)) > 0)) {
        اذهب err;
    }
    اشاره_ګر = cpu_منزل_le64(اشاره_ګر);

    /* Only supported on QEMU */
    إذا (qemu_cfg_write_file(&اشاره_ګر, dest_file, dst_دننه_واټن,
                            ثبت->wr_اشاره_ګر.اندازه) != ثبت->wr_اشاره_ګر.اندازه) {
        اذهب err;
    }

    /* Store the info so it can replayed later if necessary */
    بنية romfile_wr_اشاره_ګر_ثبت *store = malloc_لوړ(حجم(*store));
    إذا (!store) {
        warn_noalloc();
        أرجع;
    }
    store->اشاره_ګر = اشاره_ګر;
    store->کلی = qemu_get_romfile_کلی(dest_file);
    store->دننه_واټن_2 = dst_دننه_واټن;
    store->ptr_اندازه = ثبت->wr_اشاره_ګر.اندازه;
    hlist_جمع_کړه_سر(&store->غوټه, &romfile_اشاره_ګر_list);

    أرجع;
 err:
    warn_internalerror();
}

صحيح romfile_loader_execute(ثابت حرف *name)
{
    بنية romfile_loader_ثبت_s *ثبت;
    صحيح اندازه, دننه_واټن_2 = 0, nfiles;
    بنية romfile_loader_files *files;
    فراغ *معلومات = romfile_loadfile(name, &اندازه);
    إذا (!معلومات)
        أرجع -1;

    إذا (اندازه % حجم(*ثبت)) {
        warn_internalerror();
        اذهب err;
    }

    /* (over)estimate the number of files to load. */
    nfiles = اندازه / حجم(*ثبت);
    files = malloc_tmp(حجم(*files) + nfiles * حجم(files->files[0]));
    إذا (!files) {
        warn_noalloc();
        اذهب err;
    }
    files->nfiles = 0;

    لكل (دننه_واټن_2 = 0; دننه_واټن_2 < اندازه; دننه_واټن_2 += حجم(*ثبت)) {
        ثبت = معلومات + دننه_واټن_2;
        بدل (le32_منزل_cpu(ثبت->command)) {
                حالة ROMFILE_LOADER_COMMAND_ALLOCATE:
                        romfile_loader_allocate(ثبت, files);
                        اكسر;
                حالة ROMFILE_LOADER_COMMAND_جمع_کړه_اشاره_ګر:
                        romfile_loader_جمع_کړه_اشاره_ګر(ثبت, files);
                        اكسر;
                حالة ROMFILE_LOADER_COMMAND_جمع_کړه_CHECKSUM:
                        romfile_loader_جمع_کړه_checksum(ثبت, files);
                        اكسر;
                حالة ROMFILE_LOADER_COMMAND_WRITE_اشاره_ګر:
                        romfile_loader_write_اشاره_ګر(ثبت, files);
                        اكسر;
                افتراضي:
                        /* Skip commands that we don't recognize. */
                        اكسر;
        }
    }

    حرر(files);
    حرر(معلومات);
    أرجع 0;

err:
    حرر(معلومات);
    أرجع -1;
}
