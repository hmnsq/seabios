// Multiboot interface support.
//
// Copyright (C) 2015  Vladimir Serbinenko <phcoder@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "romfile.h" // romfile_add
#అన్తర్భావ "std/multiboot.h" // MULTIBOOT_*
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // multiboot_init

నిర్మాణం mbfs_romfile_s {
    నిర్మాణం romfile_s file;
    శూన్య *దత్తాంశం;
};

స్థిర పూర్ణాంకం
వెలికితీయి_filename(అక్షరం *dest, అక్షరం *src, కొలత_t lim)
{
    అక్షరం *ptr;
    కొరకు (ptr = src; *ptr; ptr++) {
        ఒకవేళ (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            కొనసాగు;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        ఒకవేళ (memcmp(ptr, "name=", 5) == 0) {
            పూర్ణాంకం i;
            అక్షరం *optr = dest;
            కొరకు (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            తిరిగి 1;
        }
    }
    తిరిగి 0;
}

// Copy a file to memory
స్థిర పూర్ణాంకం
mbfs_copyfile(నిర్మాణం romfile_s *file, శూన్య *dst, u32 maxlen)
{
    నిర్మాణం mbfs_romfile_s *cfile;
    cfile = container_యొక్క(file, నిర్మాణం mbfs_romfile_s, file);
    u32 కొలత = cfile->file.కొలత;
    శూన్య *src = cfile->దత్తాంశం;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", కొలత, src, maxlen, dst);
    ఒకవేళ (కొలత > maxlen) {
        warn_noalloc();
        తిరిగి -1;
    }
    iomemcpy(dst, src, కొలత);
    తిరిగి కొలత;
}

u32 __VISIBLE నమోదు_elf_eax, నమోదు_elf_ebx;

శూన్య
multiboot_init(శూన్య)
{
    నిర్మాణం multiboot_info *mbi;
    ఒకవేళ (!CONFIG_MULTIBOOT)
        తిరిగి;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", నమోదు_elf_eax, నమోదు_elf_ebx);
    ఒకవేళ (నమోదు_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        తిరిగి;
    mbi = (శూన్య *)నమోదు_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_లెక్క);
    ఒకవేళ (!(mbi->flags & MULTIBOOT_INFO_MODS))
        తిరిగి;
    పూర్ణాంకం i;
    నిర్మాణం multiboot_mod_list *mod = (శూన్య *)mbi->mods_addr;
    కొరకు (i = 0; i < mbi->mods_లెక్క; i++) {
        నిర్మాణం mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        ఒకవేళ (!mod[i].cmdline)
            కొనసాగు;
        len = mod[i].mod_చివర - mod[i].mod_ప్రారంభం;
        cfile = malloc_tmp(పరిమాణం(*cfile));
        ఒకవేళ (!cfile) {
            warn_noalloc();
            తిరిగి;
        }
        memset(cfile, 0, పరిమాణం(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (అక్షరం *)mod[i].cmdline, len);
        ఒకవేళ (!వెలికితీయి_filename(cfile->file.name, (అక్షరం *)mod[i].cmdline,
                              పరిమాణం(cfile->file.name))) {
            విడుదల(cfile);
            కొనసాగు;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.కొలత = len;
        copy = malloc_tmp(len);
        ఒకవేళ (!copy) {
            warn_noalloc();
            విడుదల(cfile);
            తిరిగి;
        }
        జ్ఞాపకం_నకలు(copy, (శూన్య *)mod[i].mod_ప్రారంభం, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->దత్తాంశం = copy;
        romfile_కూడు(&cfile->file);
    }
}
