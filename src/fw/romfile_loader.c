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
    فراغ *انکڑے;
};
بنية romfile_loader_files {
    صحيح nfiles;
    بنية romfile_loader_file files[];
};

// Data structures for storing "write pointer" entries for possible replay
بنية romfile_wr_اشارہ_گر_اندراج {
    u64 اشارہ_گر;
    u32 وتھ_2;
    u16 کلید;
    u8 ptr_ناپ;
    بنية hlist_گرہ گرہ;
};
ساكن بنية hlist_سر romfile_اشارہ_گر_list;

ساكن بنية romfile_loader_file *
romfile_loader_لبھو(ثابت حرف *name,
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

    بنية romfile_wr_اشارہ_گر_اندراج *اندراج;
    hlist_for_each_اندراج(اندراج, &romfile_اشارہ_گر_list, گرہ) {
        qemu_cfg_write_file_simple(&اندراج->اشارہ_گر, اندراج->کلید,
                                   اندراج->وتھ_2, اندراج->ptr_ناپ);
    }
}

ساكن فراغ romfile_loader_allocate(بنية romfile_loader_اندراج_s *اندراج,
                                    بنية romfile_loader_files *files)
{
    بنية zone_s *zone;
    بنية romfile_loader_file *file = &files->files[files->nfiles];
    فراغ *انکڑے;
    صحيح ret;
    بنانشانی alloc_align = le32_منزل_cpu(اندراج->alloc.align);

    إذا (alloc_align & (alloc_align - 1))
        اذهب err;

    بدل (اندراج->alloc.zone) {
        حالة ROMFILE_LOADER_ALLOC_ZONE_بلند:
            zone = &Zone_بلند;
            اكسر;
        حالة ROMFILE_LOADER_ALLOC_ZONE_FSEG:
            zone = &ZoneFSeg;
            اكسر;
        افتراضي:
            اذهب err;
    }
    إذا (alloc_align < MALLOC_MIN_ALIGN)
        alloc_align = MALLOC_MIN_ALIGN;
    إذا (اندراج->alloc.file[ROMFILE_LOADER_FILESZ - 1])
        اذهب err;
    file->file = romfile_لبھو(اندراج->alloc.file);
    إذا (!file->file || !file->file->ناپ)
        أرجع;
    انکڑے = _malloc(zone, file->file->ناپ, alloc_align);
    إذا (!انکڑے) {
        warn_noalloc();
        أرجع;
    }
    ret = file->file->copy(file->file, انکڑے, file->file->ناپ);
    إذا (ret != file->file->ناپ)
        اذهب file_err;
    file->انکڑے = انکڑے;
    files->nfiles++;
    أرجع;

file_err:
    حرر(انکڑے);
err:
    warn_internalerror();
}

ساكن فراغ romfile_loader_جوڑو_اشارہ_گر(بنية romfile_loader_اندراج_s *اندراج,
                                       بنية romfile_loader_files *files)
{
    بنية romfile_loader_file *dest_file;
    بنية romfile_loader_file *src_file;
    بنانشانی وتھ_2 = le32_منزل_cpu(اندراج->اشارہ_گر.وتھ_2);
    u64 اشارہ_گر = 0;

    dest_file = romfile_loader_لبھو(اندراج->اشارہ_گر.dest_file, files);
    src_file = romfile_loader_لبھو(اندراج->اشارہ_گر.src_file, files);

    إذا (!dest_file || !src_file || !dest_file->انکڑے || !src_file->انکڑے ||
        وتھ_2 + اندراج->اشارہ_گر.ناپ < وتھ_2 ||
        وتھ_2 + اندراج->اشارہ_گر.ناپ > dest_file->file->ناپ ||
        اندراج->اشارہ_گر.ناپ < 1 || اندراج->اشارہ_گر.ناپ > 8 ||
        اندراج->اشارہ_گر.ناپ & (اندراج->اشارہ_گر.ناپ - 1))
        اذهب err;

    حافظہ_نقل(&اشارہ_گر, dest_file->انکڑے + وتھ_2, اندراج->اشارہ_گر.ناپ);
    اشارہ_گر = le64_منزل_cpu(اشارہ_گر);
    اشارہ_گر += (بنانشانی لما)src_file->انکڑے;
    اشارہ_گر = cpu_منزل_le64(اشارہ_گر);
    حافظہ_نقل(dest_file->انکڑے + وتھ_2, &اشارہ_گر, اندراج->اشارہ_گر.ناپ);

    أرجع;
err:
    warn_internalerror();
}

ساكن فراغ romfile_loader_جوڑو_checksum(بنية romfile_loader_اندراج_s *اندراج,
                                        بنية romfile_loader_files *files)
{
    بنية romfile_loader_file *file;
    بنانشانی وتھ_2 = le32_منزل_cpu(اندراج->cksum.وتھ_2);
    بنانشانی شروع = le32_منزل_cpu(اندراج->cksum.شروع);
    بنانشانی len = le32_منزل_cpu(اندراج->cksum.لمائی);
    u8 *انکڑے;

    file = romfile_loader_لبھو(اندراج->cksum.file, files);

    إذا (!file || !file->انکڑے || وتھ_2 >= file->file->ناپ ||
        شروع + len < شروع || شروع + len > file->file->ناپ)
        اذهب err;

    انکڑے = file->انکڑے + وتھ_2;
    *انکڑے -= checksum(file->انکڑے + شروع, len);

    أرجع;
err:
    warn_internalerror();
}

ساكن فراغ romfile_loader_write_اشارہ_گر(بنية romfile_loader_اندراج_s *اندراج,
                                         بنية romfile_loader_files *files)
{
    بنية romfile_s *dest_file;
    بنية romfile_loader_file *src_file;
    بنانشانی dst_وتھ = le32_منزل_cpu(اندراج->wr_اشارہ_گر.dst_وتھ);
    بنانشانی src_وتھ = le32_منزل_cpu(اندراج->wr_اشارہ_گر.src_وتھ);
    u64 اشارہ_گر = 0;

    /* Writing back to a file that may not be loaded in RAM */
    dest_file = romfile_لبھو(اندراج->wr_اشارہ_گر.dest_file);
    src_file = romfile_loader_لبھو(اندراج->wr_اشارہ_گر.src_file, files);

    إذا (!dest_file || !src_file || !src_file->انکڑے ||
        dst_وتھ + اندراج->wr_اشارہ_گر.ناپ < dst_وتھ ||
        dst_وتھ + اندراج->wr_اشارہ_گر.ناپ > dest_file->ناپ ||
        src_وتھ >= src_file->file->ناپ ||
        اندراج->wr_اشارہ_گر.ناپ < 1 || اندراج->wr_اشارہ_گر.ناپ > 8 ||
        اندراج->wr_اشارہ_گر.ناپ & (اندراج->wr_اشارہ_گر.ناپ - 1)) {
        اذهب err;
    }

    اشارہ_گر = (بنانشانی لما)src_file->انکڑے + src_وتھ;
    /* Make sure the pointer fits within wr_pointer.size */
    إذا ((اندراج->wr_اشارہ_گر.ناپ != حجم(u64)) &&
        ((اشارہ_گر >> (اندراج->wr_اشارہ_گر.ناپ * 8)) > 0)) {
        اذهب err;
    }
    اشارہ_گر = cpu_منزل_le64(اشارہ_گر);

    /* Only supported on QEMU */
    إذا (qemu_cfg_write_file(&اشارہ_گر, dest_file, dst_وتھ,
                            اندراج->wr_اشارہ_گر.ناپ) != اندراج->wr_اشارہ_گر.ناپ) {
        اذهب err;
    }

    /* Store the info so it can replayed later if necessary */
    بنية romfile_wr_اشارہ_گر_اندراج *store = malloc_بلند(حجم(*store));
    إذا (!store) {
        warn_noalloc();
        أرجع;
    }
    store->اشارہ_گر = اشارہ_گر;
    store->کلید = qemu_get_romfile_کلید(dest_file);
    store->وتھ_2 = dst_وتھ;
    store->ptr_ناپ = اندراج->wr_اشارہ_گر.ناپ;
    hlist_جوڑو_سر(&store->گرہ, &romfile_اشارہ_گر_list);

    أرجع;
 err:
    warn_internalerror();
}

صحيح romfile_loader_execute(ثابت حرف *name)
{
    بنية romfile_loader_اندراج_s *اندراج;
    صحيح ناپ, وتھ_2 = 0, nfiles;
    بنية romfile_loader_files *files;
    فراغ *انکڑے = romfile_loadfile(name, &ناپ);
    إذا (!انکڑے)
        أرجع -1;

    إذا (ناپ % حجم(*اندراج)) {
        warn_internalerror();
        اذهب err;
    }

    /* (over)estimate the number of files to load. */
    nfiles = ناپ / حجم(*اندراج);
    files = malloc_tmp(حجم(*files) + nfiles * حجم(files->files[0]));
    إذا (!files) {
        warn_noalloc();
        اذهب err;
    }
    files->nfiles = 0;

    لكل (وتھ_2 = 0; وتھ_2 < ناپ; وتھ_2 += حجم(*اندراج)) {
        اندراج = انکڑے + وتھ_2;
        بدل (le32_منزل_cpu(اندراج->command)) {
                حالة ROMFILE_LOADER_COMMAND_ALLOCATE:
                        romfile_loader_allocate(اندراج, files);
                        اكسر;
                حالة ROMFILE_LOADER_COMMAND_جوڑو_اشارہ_گر:
                        romfile_loader_جوڑو_اشارہ_گر(اندراج, files);
                        اكسر;
                حالة ROMFILE_LOADER_COMMAND_جوڑو_CHECKSUM:
                        romfile_loader_جوڑو_checksum(اندراج, files);
                        اكسر;
                حالة ROMFILE_LOADER_COMMAND_WRITE_اشارہ_گر:
                        romfile_loader_write_اشارہ_گر(اندراج, files);
                        اكسر;
                افتراضي:
                        /* Skip commands that we don't recognize. */
                        اكسر;
        }
    }

    حرر(files);
    حرر(انکڑے);
    أرجع 0;

err:
    حرر(انکڑے);
    أرجع -1;
}
