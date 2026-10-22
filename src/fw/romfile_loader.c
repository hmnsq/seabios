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
بنية romfile_wr_اشارو_داخلا {
    u64 اشارو;
    u32 وٿي_2;
    u16 ڪنجي;
    u8 ptr_سائيز;
    بنية hlist_ڳنڍ ڳنڍ;
};
ساكن بنية hlist_سر romfile_اشارو_list;

ساكن بنية romfile_loader_file *
romfile_loader_ڳوليو(ثابت حرف *name,
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

    بنية romfile_wr_اشارو_داخلا *داخلا;
    hlist_for_each_داخلا(داخلا, &romfile_اشارو_list, ڳنڍ) {
        qemu_cfg_write_file_simple(&داخلا->اشارو, داخلا->ڪنجي,
                                   داخلا->وٿي_2, داخلا->ptr_سائيز);
    }
}

ساكن فراغ romfile_loader_allocate(بنية romfile_loader_داخلا_s *داخلا,
                                    بنية romfile_loader_files *files)
{
    بنية zone_s *zone;
    بنية romfile_loader_file *file = &files->files[files->nfiles];
    فراغ *معلومات;
    صحيح ret;
    بنانشان alloc_align = le32_منزل_cpu(داخلا->alloc.align);

    إذا (alloc_align & (alloc_align - 1))
        اذهب err;

    بدل (داخلا->alloc.zone) {
        حالة ROMFILE_LOADER_ALLOC_ZONE_مٿاهون:
            zone = &Zone_مٿاهون;
            اكسر;
        حالة ROMFILE_LOADER_ALLOC_ZONE_FSEG:
            zone = &ZoneFSeg;
            اكسر;
        افتراضي:
            اذهب err;
    }
    إذا (alloc_align < MALLOC_MIN_ALIGN)
        alloc_align = MALLOC_MIN_ALIGN;
    إذا (داخلا->alloc.file[ROMFILE_LOADER_FILESZ - 1])
        اذهب err;
    file->file = romfile_ڳوليو(داخلا->alloc.file);
    إذا (!file->file || !file->file->سائيز)
        أرجع;
    معلومات = _malloc(zone, file->file->سائيز, alloc_align);
    إذا (!معلومات) {
        warn_noalloc();
        أرجع;
    }
    ret = file->file->copy(file->file, معلومات, file->file->سائيز);
    إذا (ret != file->file->سائيز)
        اذهب file_err;
    file->معلومات = معلومات;
    files->nfiles++;
    أرجع;

file_err:
    حرر(معلومات);
err:
    warn_internalerror();
}

ساكن فراغ romfile_loader_جوڙيو_اشارو(بنية romfile_loader_داخلا_s *داخلا,
                                       بنية romfile_loader_files *files)
{
    بنية romfile_loader_file *dest_file;
    بنية romfile_loader_file *src_file;
    بنانشان وٿي_2 = le32_منزل_cpu(داخلا->اشارو.وٿي_2);
    u64 اشارو = 0;

    dest_file = romfile_loader_ڳوليو(داخلا->اشارو.dest_file, files);
    src_file = romfile_loader_ڳوليو(داخلا->اشارو.src_file, files);

    إذا (!dest_file || !src_file || !dest_file->معلومات || !src_file->معلومات ||
        وٿي_2 + داخلا->اشارو.سائيز < وٿي_2 ||
        وٿي_2 + داخلا->اشارو.سائيز > dest_file->file->سائيز ||
        داخلا->اشارو.سائيز < 1 || داخلا->اشارو.سائيز > 8 ||
        داخلا->اشارو.سائيز & (داخلا->اشارو.سائيز - 1))
        اذهب err;

    يادگيري_نقل(&اشارو, dest_file->معلومات + وٿي_2, داخلا->اشارو.سائيز);
    اشارو = le64_منزل_cpu(اشارو);
    اشارو += (بنانشان ڊگهو)src_file->معلومات;
    اشارو = cpu_منزل_le64(اشارو);
    يادگيري_نقل(dest_file->معلومات + وٿي_2, &اشارو, داخلا->اشارو.سائيز);

    أرجع;
err:
    warn_internalerror();
}

ساكن فراغ romfile_loader_جوڙيو_checksum(بنية romfile_loader_داخلا_s *داخلا,
                                        بنية romfile_loader_files *files)
{
    بنية romfile_loader_file *file;
    بنانشان وٿي_2 = le32_منزل_cpu(داخلا->cksum.وٿي_2);
    بنانشان شروعات = le32_منزل_cpu(داخلا->cksum.شروعات);
    بنانشان len = le32_منزل_cpu(داخلا->cksum.ڊيگهائي);
    u8 *معلومات;

    file = romfile_loader_ڳوليو(داخلا->cksum.file, files);

    إذا (!file || !file->معلومات || وٿي_2 >= file->file->سائيز ||
        شروعات + len < شروعات || شروعات + len > file->file->سائيز)
        اذهب err;

    معلومات = file->معلومات + وٿي_2;
    *معلومات -= checksum(file->معلومات + شروعات, len);

    أرجع;
err:
    warn_internalerror();
}

ساكن فراغ romfile_loader_write_اشارو(بنية romfile_loader_داخلا_s *داخلا,
                                         بنية romfile_loader_files *files)
{
    بنية romfile_s *dest_file;
    بنية romfile_loader_file *src_file;
    بنانشان dst_وٿي = le32_منزل_cpu(داخلا->wr_اشارو.dst_وٿي);
    بنانشان src_وٿي = le32_منزل_cpu(داخلا->wr_اشارو.src_وٿي);
    u64 اشارو = 0;

    /* Writing back to a file that may not be loaded in RAM */
    dest_file = romfile_ڳوليو(داخلا->wr_اشارو.dest_file);
    src_file = romfile_loader_ڳوليو(داخلا->wr_اشارو.src_file, files);

    إذا (!dest_file || !src_file || !src_file->معلومات ||
        dst_وٿي + داخلا->wr_اشارو.سائيز < dst_وٿي ||
        dst_وٿي + داخلا->wr_اشارو.سائيز > dest_file->سائيز ||
        src_وٿي >= src_file->file->سائيز ||
        داخلا->wr_اشارو.سائيز < 1 || داخلا->wr_اشارو.سائيز > 8 ||
        داخلا->wr_اشارو.سائيز & (داخلا->wr_اشارو.سائيز - 1)) {
        اذهب err;
    }

    اشارو = (بنانشان ڊگهو)src_file->معلومات + src_وٿي;
    /* Make sure the pointer fits within wr_pointer.size */
    إذا ((داخلا->wr_اشارو.سائيز != حجم(u64)) &&
        ((اشارو >> (داخلا->wr_اشارو.سائيز * 8)) > 0)) {
        اذهب err;
    }
    اشارو = cpu_منزل_le64(اشارو);

    /* Only supported on QEMU */
    إذا (qemu_cfg_write_file(&اشارو, dest_file, dst_وٿي,
                            داخلا->wr_اشارو.سائيز) != داخلا->wr_اشارو.سائيز) {
        اذهب err;
    }

    /* Store the info so it can replayed later if necessary */
    بنية romfile_wr_اشارو_داخلا *store = malloc_مٿاهون(حجم(*store));
    إذا (!store) {
        warn_noalloc();
        أرجع;
    }
    store->اشارو = اشارو;
    store->ڪنجي = qemu_get_romfile_ڪنجي(dest_file);
    store->وٿي_2 = dst_وٿي;
    store->ptr_سائيز = داخلا->wr_اشارو.سائيز;
    hlist_جوڙيو_سر(&store->ڳنڍ, &romfile_اشارو_list);

    أرجع;
 err:
    warn_internalerror();
}

صحيح romfile_loader_execute(ثابت حرف *name)
{
    بنية romfile_loader_داخلا_s *داخلا;
    صحيح سائيز, وٿي_2 = 0, nfiles;
    بنية romfile_loader_files *files;
    فراغ *معلومات = romfile_loadfile(name, &سائيز);
    إذا (!معلومات)
        أرجع -1;

    إذا (سائيز % حجم(*داخلا)) {
        warn_internalerror();
        اذهب err;
    }

    /* (over)estimate the number of files to load. */
    nfiles = سائيز / حجم(*داخلا);
    files = malloc_tmp(حجم(*files) + nfiles * حجم(files->files[0]));
    إذا (!files) {
        warn_noalloc();
        اذهب err;
    }
    files->nfiles = 0;

    لكل (وٿي_2 = 0; وٿي_2 < سائيز; وٿي_2 += حجم(*داخلا)) {
        داخلا = معلومات + وٿي_2;
        بدل (le32_منزل_cpu(داخلا->command)) {
                حالة ROMFILE_LOADER_COMMAND_ALLOCATE:
                        romfile_loader_allocate(داخلا, files);
                        اكسر;
                حالة ROMFILE_LOADER_COMMAND_جوڙيو_اشارو:
                        romfile_loader_جوڙيو_اشارو(داخلا, files);
                        اكسر;
                حالة ROMFILE_LOADER_COMMAND_جوڙيو_CHECKSUM:
                        romfile_loader_جوڙيو_checksum(داخلا, files);
                        اكسر;
                حالة ROMFILE_LOADER_COMMAND_WRITE_اشارو:
                        romfile_loader_write_اشارو(داخلا, files);
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
