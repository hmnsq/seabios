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

struct mbfs_romfile_s {
    struct romfile_s file;
    void *data;
};

static целое
extract_filename(char *dest, char *src, size_t lim)
{
    char *ptr;
    для (ptr = src; *ptr; ptr++) {
        если (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            continue;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        если (memcmp(ptr, "name=", 5) == 0) {
            целое i;
            char *optr = dest;
            для (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            вернуть 1;
        }
    }
    вернуть 0;
}

// Copy a file to memory
static целое
mbfs_copyfile(struct romfile_s *file, void *dst, u32 maxlen)
{
    struct mbfs_romfile_s *cfile;
    cfile = container_of(file, struct mbfs_romfile_s, file);
    u32 size = cfile->file.size;
    void *src = cfile->data;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    если (size > maxlen) {
        warn_noalloc();
        вернуть -1;
    }
    iomemcpy(dst, src, size);
    вернуть size;
}

u32 __VISIBLE entry_elf_eax, entry_elf_ebx;

void
multiboot_init(void)
{
    struct multiboot_info *mbi;
    если (!CONFIG_MULTIBOOT)
        вернуть;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", entry_elf_eax, entry_elf_ebx);
    если (entry_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        вернуть;
    mbi = (void *)entry_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_count);
    если (!(mbi->flags & MULTIBOOT_INFO_MODS))
        вернуть;
    целое i;
    struct multiboot_mod_list *mod = (void *)mbi->mods_addr;
    для (i = 0; i < mbi->mods_count; i++) {
        struct mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        если (!mod[i].cmdline)
            continue;
        len = mod[i].mod_end - mod[i].mod_start;
        cfile = malloc_tmp(sizeof(*cfile));
        если (!cfile) {
            warn_noalloc();
            вернуть;
        }
        memset(cfile, 0, sizeof(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (char *)mod[i].cmdline, len);
        если (!extract_filename(cfile->file.name, (char *)mod[i].cmdline,
                              sizeof(cfile->file.name))) {
            free(cfile);
            continue;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.size = len;
        copy = malloc_tmp(len);
        если (!copy) {
            warn_noalloc();
            free(cfile);
            вернуть;
        }
        memcpy(copy, (void *)mod[i].mod_start, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->data = copy;
        romfile_add(&cfile->file);
    }
}
