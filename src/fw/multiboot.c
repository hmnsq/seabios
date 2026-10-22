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

static số_nguyên
extract_filename(char *dest, char *src, size_t lim)
{
    char *ptr;
    cho (ptr = src; *ptr; ptr++) {
        nếu (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            continue;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        nếu (memcmp(ptr, "name=", 5) == 0) {
            số_nguyên i;
            char *optr = dest;
            cho (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            trả_về 1;
        }
    }
    trả_về 0;
}

// Copy a file to memory
static số_nguyên
mbfs_copyfile(struct romfile_s *file, void *dst, u32 maxlen)
{
    struct mbfs_romfile_s *cfile;
    cfile = container_of(file, struct mbfs_romfile_s, file);
    u32 size = cfile->file.size;
    void *src = cfile->data;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    nếu (size > maxlen) {
        warn_noalloc();
        trả_về -1;
    }
    iomemcpy(dst, src, size);
    trả_về size;
}

u32 __VISIBLE entry_elf_eax, entry_elf_ebx;

void
multiboot_init(void)
{
    struct multiboot_info *mbi;
    nếu (!CONFIG_MULTIBOOT)
        trả_về;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", entry_elf_eax, entry_elf_ebx);
    nếu (entry_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        trả_về;
    mbi = (void *)entry_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_count);
    nếu (!(mbi->flags & MULTIBOOT_INFO_MODS))
        trả_về;
    số_nguyên i;
    struct multiboot_mod_list *mod = (void *)mbi->mods_addr;
    cho (i = 0; i < mbi->mods_count; i++) {
        struct mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        nếu (!mod[i].cmdline)
            continue;
        len = mod[i].mod_end - mod[i].mod_start;
        cfile = malloc_tmp(sizeof(*cfile));
        nếu (!cfile) {
            warn_noalloc();
            trả_về;
        }
        memset(cfile, 0, sizeof(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (char *)mod[i].cmdline, len);
        nếu (!extract_filename(cfile->file.name, (char *)mod[i].cmdline,
                              sizeof(cfile->file.name))) {
            free(cfile);
            continue;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.size = len;
        copy = malloc_tmp(len);
        nếu (!copy) {
            warn_noalloc();
            free(cfile);
            trả_về;
        }
        memcpy(copy, (void *)mod[i].mod_start, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->data = copy;
        romfile_add(&cfile->file);
    }
}
