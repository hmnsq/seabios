// Multiboot interface support.
//
// Copyright (C) 2015  Vladimir Serbinenko <phcoder@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "config.h" // CONFIG_*
#include "malloc.h" // free
#include "output.h" // dprintf
#include "romfile.h" // romfile_add
#include "std/multiboot.h" // MULTIBOOT_*
#include "string.h" // memset
#include "util.h" // multiboot_init

sa_nracanaa mbfs_romfile_s {
    sa_nracanaa romfile_s file;
    void *data;
};

static puur_naa_nka
extract_filename(var_na *dest, var_na *src, size_t lim)
{
    var_na *ptr;
    for (ptr = src; *ptr; ptr++) {
        ᮚᮦᮔ᮪ (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            continue;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        ᮚᮦᮔ᮪ (memcmp(ptr, "name=", 5) == 0) {
            puur_naa_nka i;
            var_na *optr = dest;
            for (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            ᮝᮜᮤ 1;
        }
    }
    ᮝᮜᮤ 0;
}

// Copy a file to memory
static puur_naa_nka
mbfs_copyfile(sa_nracanaa romfile_s *file, void *dst, u32 maxlen)
{
    sa_nracanaa mbfs_romfile_s *cfile;
    cfile = container_of(file, sa_nracanaa mbfs_romfile_s, file);
    u32 size = cfile->file.size;
    void *src = cfile->data;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    ᮚᮦᮔ᮪ (size > maxlen) {
        warn_noalloc();
        ᮝᮜᮤ -1;
    }
    iomemcpy(dst, src, size);
    ᮝᮜᮤ size;
}

u32 __VISIBLE entry_elf_eax, entry_elf_ebx;

void
multiboot_init(void)
{
    sa_nracanaa multiboot_info *mbi;
    ᮚᮦᮔ᮪ (!CONFIG_MULTIBOOT)
        ᮝᮜᮤ;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", entry_elf_eax, entry_elf_ebx);
    ᮚᮦᮔ᮪ (entry_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        ᮝᮜᮤ;
    mbi = (void *)entry_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_count);
    ᮚᮦᮔ᮪ (!(mbi->flags & MULTIBOOT_INFO_MODS))
        ᮝᮜᮤ;
    puur_naa_nka i;
    sa_nracanaa multiboot_mod_list *mod = (void *)mbi->mods_addr;
    for (i = 0; i < mbi->mods_count; i++) {
        sa_nracanaa mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        ᮚᮦᮔ᮪ (!mod[i].cmdline)
            continue;
        len = mod[i].mod_end - mod[i].mod_start;
        cfile = malloc_tmp(parimaa_na(*cfile));
        ᮚᮦᮔ᮪ (!cfile) {
            warn_noalloc();
            ᮝᮜᮤ;
        }
        memset(cfile, 0, parimaa_na(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (var_na *)mod[i].cmdline, len);
        ᮚᮦᮔ᮪ (!extract_filename(cfile->file.name, (var_na *)mod[i].cmdline,
                              parimaa_na(cfile->file.name))) {
            free(cfile);
            continue;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.size = len;
        copy = malloc_tmp(len);
        ᮚᮦᮔ᮪ (!copy) {
            warn_noalloc();
            free(cfile);
            ᮝᮜᮤ;
        }
        memcpy(copy, (void *)mod[i].mod_start, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->data = copy;
        romfile_add(&cfile->file);
    }
}
