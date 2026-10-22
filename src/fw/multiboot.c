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

static ཧྲིལ
extract_filename(var_na *dest, var_na *src, size_t lim)
{
    var_na *ptr;
    དོན་ལུ (ptr = src; *ptr; ptr++) {
        གལ་སྲིད (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            continue;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        གལ་སྲིད (memcmp(ptr, "name=", 5) == 0) {
            ཧྲིལ i;
            var_na *optr = dest;
            དོན་ལུ (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            ལོག 1;
        }
    }
    ལོག 0;
}

// Copy a file to memory
static ཧྲིལ
mbfs_copyfile(sa_nracanaa romfile_s *file, void *dst, u32 maxlen)
{
    sa_nracanaa mbfs_romfile_s *cfile;
    cfile = container_of(file, sa_nracanaa mbfs_romfile_s, file);
    u32 size = cfile->file.size;
    void *src = cfile->data;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    གལ་སྲིད (size > maxlen) {
        warn_noalloc();
        ལོག -1;
    }
    iomemcpy(dst, src, size);
    ལོག size;
}

u32 __VISIBLE entry_elf_eax, entry_elf_ebx;

void
multiboot_init(void)
{
    sa_nracanaa multiboot_info *mbi;
    གལ་སྲིད (!CONFIG_MULTIBOOT)
        ལོག;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", entry_elf_eax, entry_elf_ebx);
    གལ་སྲིད (entry_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        ལོག;
    mbi = (void *)entry_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_count);
    གལ་སྲིད (!(mbi->flags & MULTIBOOT_INFO_MODS))
        ལོག;
    ཧྲིལ i;
    sa_nracanaa multiboot_mod_list *mod = (void *)mbi->mods_addr;
    དོན་ལུ (i = 0; i < mbi->mods_count; i++) {
        sa_nracanaa mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        གལ་སྲིད (!mod[i].cmdline)
            continue;
        len = mod[i].mod_end - mod[i].mod_start;
        cfile = malloc_tmp(parimaa_na(*cfile));
        གལ་སྲིད (!cfile) {
            warn_noalloc();
            ལོག;
        }
        memset(cfile, 0, parimaa_na(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (var_na *)mod[i].cmdline, len);
        གལ་སྲིད (!extract_filename(cfile->file.name, (var_na *)mod[i].cmdline,
                              parimaa_na(cfile->file.name))) {
            free(cfile);
            continue;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.size = len;
        copy = malloc_tmp(len);
        གལ་སྲིད (!copy) {
            warn_noalloc();
            free(cfile);
            ལོག;
        }
        memcpy(copy, (void *)mod[i].mod_start, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->data = copy;
        romfile_add(&cfile->file);
    }
}
