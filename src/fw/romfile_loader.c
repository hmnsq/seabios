#అన్తర్భావ "romfile_loader.h"
#అన్తర్భావ "byteorder.h" // leXX_to_cpu/cpu_to_leXX
#అన్తర్భావ "util.h" // checksum
#అన్తర్భావ "string.h" // strcmp
#అన్తర్భావ "romfile.h" // struct romfile_s
#అన్తర్భావ "malloc.h" // Zone*, _malloc
#అన్తర్భావ "list.h" // struct hlist_node
#అన్తర్భావ "output.h" // warn_*
#అన్తర్భావ "paravirt.h" // qemu_cfg_write_file

నిర్మాణం romfile_loader_file {
    నిర్మాణం romfile_s *file;
    శూన్య *దత్తాంశం;
};
నిర్మాణం romfile_loader_files {
    పూర్ణాంకం nfiles;
    నిర్మాణం romfile_loader_file files[];
};

// Data structures for storing "write pointer" entries for possible replay
నిర్మాణం romfile_wr_సూచిక_నమోదు {
    u64 సూచిక_2;
    u32 స్థానభ్రంశం_2;
    u16 కీలకం;
    u8 ptr_కొలత;
    నిర్మాణం hlist_కణుపు కణుపు;
};
స్థిర నిర్మాణం hlist_తల romfile_సూచిక_list;

స్థిర నిర్మాణం romfile_loader_file *
romfile_loader_కనుగొను(స్థిరరాశి అక్షరం *name,
                    నిర్మాణం romfile_loader_files *files)
{
    పూర్ణాంకం i;
    ఒకవేళ (name[ROMFILE_LOADER_FILESZ - 1])
        తిరిగి ఖాళీ;
    కొరకు (i = 0; i < files->nfiles; ++i)
        ఒకవేళ (!strcmp(files->files[i].file->name, name))
            తిరిగి &files->files[i];
    తిరిగి ఖాళీ;
}

// Replay "write pointer" entries back to QEMU
శూన్య romfile_fw_cfg_resume(శూన్య)
{
    ఒకవేళ (!CONFIG_QEMU)
        తిరిగి;

    నిర్మాణం romfile_wr_సూచిక_నమోదు *నమోదు_2;
    hlist_for_each_నమోదు(నమోదు_2, &romfile_సూచిక_list, కణుపు) {
        qemu_cfg_write_file_simple(&నమోదు_2->సూచిక_2, నమోదు_2->కీలకం,
                                   నమోదు_2->స్థానభ్రంశం_2, నమోదు_2->ptr_కొలత);
    }
}

స్థిర శూన్య romfile_loader_allocate(నిర్మాణం romfile_loader_నమోదు_s *నమోదు_2,
                                    నిర్మాణం romfile_loader_files *files)
{
    నిర్మాణం zone_s *zone;
    నిర్మాణం romfile_loader_file *file = &files->files[files->nfiles];
    శూన్య *దత్తాంశం;
    పూర్ణాంకం ret;
    గుర్తులేని alloc_align = le32_గమ్యం_cpu(నమోదు_2->alloc.align);

    ఒకవేళ (alloc_align & (alloc_align - 1))
        వెళ్ళు err;

    ఎంపిక (నమోదు_2->alloc.zone) {
        స్థితి ROMFILE_LOADER_ALLOC_ZONE_ఎగువ:
            zone = &Zone_ఎగువ;
            ఆపు;
        స్థితి ROMFILE_LOADER_ALLOC_ZONE_FSEG:
            zone = &ZoneFSeg;
            ఆపు;
        అప్రమేయ:
            వెళ్ళు err;
    }
    ఒకవేళ (alloc_align < MALLOC_MIN_ALIGN)
        alloc_align = MALLOC_MIN_ALIGN;
    ఒకవేళ (నమోదు_2->alloc.file[ROMFILE_LOADER_FILESZ - 1])
        వెళ్ళు err;
    file->file = romfile_కనుగొను(నమోదు_2->alloc.file);
    ఒకవేళ (!file->file || !file->file->కొలత)
        తిరిగి;
    దత్తాంశం = _malloc(zone, file->file->కొలత, alloc_align);
    ఒకవేళ (!దత్తాంశం) {
        warn_noalloc();
        తిరిగి;
    }
    ret = file->file->copy(file->file, దత్తాంశం, file->file->కొలత);
    ఒకవేళ (ret != file->file->కొలత)
        వెళ్ళు file_err;
    file->దత్తాంశం = దత్తాంశం;
    files->nfiles++;
    తిరిగి;

file_err:
    విడుదల(దత్తాంశం);
err:
    warn_internalerror();
}

స్థిర శూన్య romfile_loader_కూడు_సూచిక(నిర్మాణం romfile_loader_నమోదు_s *నమోదు_2,
                                       నిర్మాణం romfile_loader_files *files)
{
    నిర్మాణం romfile_loader_file *dest_file;
    నిర్మాణం romfile_loader_file *src_file;
    గుర్తులేని స్థానభ్రంశం_2 = le32_గమ్యం_cpu(నమోదు_2->సూచిక_2.స్థానభ్రంశం_2);
    u64 సూచిక_2 = 0;

    dest_file = romfile_loader_కనుగొను(నమోదు_2->సూచిక_2.dest_file, files);
    src_file = romfile_loader_కనుగొను(నమోదు_2->సూచిక_2.src_file, files);

    ఒకవేళ (!dest_file || !src_file || !dest_file->దత్తాంశం || !src_file->దత్తాంశం ||
        స్థానభ్రంశం_2 + నమోదు_2->సూచిక_2.కొలత < స్థానభ్రంశం_2 ||
        స్థానభ్రంశం_2 + నమోదు_2->సూచిక_2.కొలత > dest_file->file->కొలత ||
        నమోదు_2->సూచిక_2.కొలత < 1 || నమోదు_2->సూచిక_2.కొలత > 8 ||
        నమోదు_2->సూచిక_2.కొలత & (నమోదు_2->సూచిక_2.కొలత - 1))
        వెళ్ళు err;

    జ్ఞాపకం_నకలు(&సూచిక_2, dest_file->దత్తాంశం + స్థానభ్రంశం_2, నమోదు_2->సూచిక_2.కొలత);
    సూచిక_2 = le64_గమ్యం_cpu(సూచిక_2);
    సూచిక_2 += (గుర్తులేని దీర్ఘ)src_file->దత్తాంశం;
    సూచిక_2 = cpu_గమ్యం_le64(సూచిక_2);
    జ్ఞాపకం_నకలు(dest_file->దత్తాంశం + స్థానభ్రంశం_2, &సూచిక_2, నమోదు_2->సూచిక_2.కొలత);

    తిరిగి;
err:
    warn_internalerror();
}

స్థిర శూన్య romfile_loader_కూడు_checksum(నిర్మాణం romfile_loader_నమోదు_s *నమోదు_2,
                                        నిర్మాణం romfile_loader_files *files)
{
    నిర్మాణం romfile_loader_file *file;
    గుర్తులేని స్థానభ్రంశం_2 = le32_గమ్యం_cpu(నమోదు_2->cksum.స్థానభ్రంశం_2);
    గుర్తులేని ప్రారంభం = le32_గమ్యం_cpu(నమోదు_2->cksum.ప్రారంభం);
    గుర్తులేని len = le32_గమ్యం_cpu(నమోదు_2->cksum.నిడివి);
    u8 *దత్తాంశం;

    file = romfile_loader_కనుగొను(నమోదు_2->cksum.file, files);

    ఒకవేళ (!file || !file->దత్తాంశం || స్థానభ్రంశం_2 >= file->file->కొలత ||
        ప్రారంభం + len < ప్రారంభం || ప్రారంభం + len > file->file->కొలత)
        వెళ్ళు err;

    దత్తాంశం = file->దత్తాంశం + స్థానభ్రంశం_2;
    *దత్తాంశం -= checksum(file->దత్తాంశం + ప్రారంభం, len);

    తిరిగి;
err:
    warn_internalerror();
}

స్థిర శూన్య romfile_loader_write_సూచిక(నిర్మాణం romfile_loader_నమోదు_s *నమోదు_2,
                                         నిర్మాణం romfile_loader_files *files)
{
    నిర్మాణం romfile_s *dest_file;
    నిర్మాణం romfile_loader_file *src_file;
    గుర్తులేని dst_స్థానభ్రంశం = le32_గమ్యం_cpu(నమోదు_2->wr_సూచిక.dst_స్థానభ్రంశం);
    గుర్తులేని src_స్థానభ్రంశం = le32_గమ్యం_cpu(నమోదు_2->wr_సూచిక.src_స్థానభ్రంశం);
    u64 సూచిక_2 = 0;

    /* Writing back to a file that may not be loaded in RAM */
    dest_file = romfile_కనుగొను(నమోదు_2->wr_సూచిక.dest_file);
    src_file = romfile_loader_కనుగొను(నమోదు_2->wr_సూచిక.src_file, files);

    ఒకవేళ (!dest_file || !src_file || !src_file->దత్తాంశం ||
        dst_స్థానభ్రంశం + నమోదు_2->wr_సూచిక.కొలత < dst_స్థానభ్రంశం ||
        dst_స్థానభ్రంశం + నమోదు_2->wr_సూచిక.కొలత > dest_file->కొలత ||
        src_స్థానభ్రంశం >= src_file->file->కొలత ||
        నమోదు_2->wr_సూచిక.కొలత < 1 || నమోదు_2->wr_సూచిక.కొలత > 8 ||
        నమోదు_2->wr_సూచిక.కొలత & (నమోదు_2->wr_సూచిక.కొలత - 1)) {
        వెళ్ళు err;
    }

    సూచిక_2 = (గుర్తులేని దీర్ఘ)src_file->దత్తాంశం + src_స్థానభ్రంశం;
    /* Make sure the pointer fits within wr_pointer.size */
    ఒకవేళ ((నమోదు_2->wr_సూచిక.కొలత != పరిమాణం(u64)) &&
        ((సూచిక_2 >> (నమోదు_2->wr_సూచిక.కొలత * 8)) > 0)) {
        వెళ్ళు err;
    }
    సూచిక_2 = cpu_గమ్యం_le64(సూచిక_2);

    /* Only supported on QEMU */
    ఒకవేళ (qemu_cfg_write_file(&సూచిక_2, dest_file, dst_స్థానభ్రంశం,
                            నమోదు_2->wr_సూచిక.కొలత) != నమోదు_2->wr_సూచిక.కొలత) {
        వెళ్ళు err;
    }

    /* Store the info so it can replayed later if necessary */
    నిర్మాణం romfile_wr_సూచిక_నమోదు *store = malloc_ఎగువ(పరిమాణం(*store));
    ఒకవేళ (!store) {
        warn_noalloc();
        తిరిగి;
    }
    store->సూచిక_2 = సూచిక_2;
    store->కీలకం = qemu_get_romfile_కీలకం(dest_file);
    store->స్థానభ్రంశం_2 = dst_స్థానభ్రంశం;
    store->ptr_కొలత = నమోదు_2->wr_సూచిక.కొలత;
    hlist_కూడు_తల(&store->కణుపు, &romfile_సూచిక_list);

    తిరిగి;
 err:
    warn_internalerror();
}

పూర్ణాంకం romfile_loader_execute(స్థిరరాశి అక్షరం *name)
{
    నిర్మాణం romfile_loader_నమోదు_s *నమోదు_2;
    పూర్ణాంకం కొలత, స్థానభ్రంశం_2 = 0, nfiles;
    నిర్మాణం romfile_loader_files *files;
    శూన్య *దత్తాంశం = romfile_loadfile(name, &కొలత);
    ఒకవేళ (!దత్తాంశం)
        తిరిగి -1;

    ఒకవేళ (కొలత % పరిమాణం(*నమోదు_2)) {
        warn_internalerror();
        వెళ్ళు err;
    }

    /* (over)estimate the number of files to load. */
    nfiles = కొలత / పరిమాణం(*నమోదు_2);
    files = malloc_tmp(పరిమాణం(*files) + nfiles * పరిమాణం(files->files[0]));
    ఒకవేళ (!files) {
        warn_noalloc();
        వెళ్ళు err;
    }
    files->nfiles = 0;

    కొరకు (స్థానభ్రంశం_2 = 0; స్థానభ్రంశం_2 < కొలత; స్థానభ్రంశం_2 += పరిమాణం(*నమోదు_2)) {
        నమోదు_2 = దత్తాంశం + స్థానభ్రంశం_2;
        ఎంపిక (le32_గమ్యం_cpu(నమోదు_2->command)) {
                స్థితి ROMFILE_LOADER_COMMAND_ALLOCATE:
                        romfile_loader_allocate(నమోదు_2, files);
                        ఆపు;
                స్థితి ROMFILE_LOADER_COMMAND_కూడు_సూచిక:
                        romfile_loader_కూడు_సూచిక(నమోదు_2, files);
                        ఆపు;
                స్థితి ROMFILE_LOADER_COMMAND_కూడు_CHECKSUM:
                        romfile_loader_కూడు_checksum(నమోదు_2, files);
                        ఆపు;
                స్థితి ROMFILE_LOADER_COMMAND_WRITE_సూచిక:
                        romfile_loader_write_సూచిక(నమోదు_2, files);
                        ఆపు;
                అప్రమేయ:
                        /* Skip commands that we don't recognize. */
                        ఆపు;
        }
    }

    విడుదల(files);
    విడుదల(దత్తాంశం);
    తిరిగి 0;

err:
    విడుదల(దత్తాంశం);
    తిరిగి -1;
}
