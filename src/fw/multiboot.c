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

static 整数
extract_filename(char *dest, char *src, size_t lim)
{
    char *ptr;
    为 (ptr = src; *ptr; ptr++) {
        如果 (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            continue;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        如果 (memcmp(ptr, "name=", 5) == 0) {
            整数 i;
            char *optr = dest;
            为 (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            返回 1;
        }
    }
    返回 0;
}

// Copy a file to memory
static 整数
mbfs_copyfile(struct romfile_s *file, void *dst, u32 maxlen)
{
    struct mbfs_romfile_s *cfile;
    cfile = container_of(file, struct mbfs_romfile_s, file);
    u32 size = cfile->file.size;
    void *src = cfile->data;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    如果 (size > maxlen) {
        warn_noalloc();
        返回 -1;
    }
    iomemcpy(dst, src, size);
    返回 size;
}

u32 __VISIBLE entry_elf_eax, entry_elf_ebx;

void
multiboot_init(void)
{
    struct multiboot_info *mbi;
    如果 (!CONFIG_MULTIBOOT)
        返回;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", entry_elf_eax, entry_elf_ebx);
    如果 (entry_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        返回;
    mbi = (void *)entry_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_count);
    如果 (!(mbi->flags & MULTIBOOT_INFO_MODS))
        返回;
    整数 i;
    struct multiboot_mod_list *mod = (void *)mbi->mods_addr;
    为 (i = 0; i < mbi->mods_count; i++) {
        struct mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        如果 (!mod[i].cmdline)
            continue;
        len = mod[i].mod_end - mod[i].mod_start;
        cfile = malloc_tmp(sizeof(*cfile));
        如果 (!cfile) {
            warn_noalloc();
            返回;
        }
        memset(cfile, 0, sizeof(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (char *)mod[i].cmdline, len);
        如果 (!extract_filename(cfile->file.name, (char *)mod[i].cmdline,
                              sizeof(cfile->file.name))) {
            free(cfile);
            continue;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.size = len;
        copy = malloc_tmp(len);
        如果 (!copy) {
            warn_noalloc();
            free(cfile);
            返回;
        }
        memcpy(copy, (void *)mod[i].mod_start, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->data = copy;
        romfile_add(&cfile->file);
    }
}
