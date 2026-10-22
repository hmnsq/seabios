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

static ຈຳນວນເຕັມ
extract_filename(var_na *dest, var_na *src, size_t lim)
{
    var_na *ptr;
    ສຳລັບ (ptr = src; *ptr; ptr++) {
        ຖ້າ (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            continue;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        ຖ້າ (memcmp(ptr, "name=", 5) == 0) {
            ຈຳນວນເຕັມ i;
            var_na *optr = dest;
            ສຳລັບ (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            ສົ່ງຄືນ 1;
        }
    }
    ສົ່ງຄືນ 0;
}

// Copy a file to memory
static ຈຳນວນເຕັມ
mbfs_copyfile(sa_nracanaa romfile_s *file, void *dst, u32 maxlen)
{
    sa_nracanaa mbfs_romfile_s *cfile;
    cfile = container_of(file, sa_nracanaa mbfs_romfile_s, file);
    u32 size = cfile->file.size;
    void *src = cfile->data;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    ຖ້າ (size > maxlen) {
        warn_noalloc();
        ສົ່ງຄືນ -1;
    }
    iomemcpy(dst, src, size);
    ສົ່ງຄືນ size;
}

u32 __VISIBLE entry_elf_eax, entry_elf_ebx;

void
multiboot_init(void)
{
    sa_nracanaa multiboot_info *mbi;
    ຖ້າ (!CONFIG_MULTIBOOT)
        ສົ່ງຄືນ;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", entry_elf_eax, entry_elf_ebx);
    ຖ້າ (entry_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        ສົ່ງຄືນ;
    mbi = (void *)entry_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_count);
    ຖ້າ (!(mbi->flags & MULTIBOOT_INFO_MODS))
        ສົ່ງຄືນ;
    ຈຳນວນເຕັມ i;
    sa_nracanaa multiboot_mod_list *mod = (void *)mbi->mods_addr;
    ສຳລັບ (i = 0; i < mbi->mods_count; i++) {
        sa_nracanaa mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        ຖ້າ (!mod[i].cmdline)
            continue;
        len = mod[i].mod_end - mod[i].mod_start;
        cfile = malloc_tmp(parimaa_na(*cfile));
        ຖ້າ (!cfile) {
            warn_noalloc();
            ສົ່ງຄືນ;
        }
        memset(cfile, 0, parimaa_na(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (var_na *)mod[i].cmdline, len);
        ຖ້າ (!extract_filename(cfile->file.name, (var_na *)mod[i].cmdline,
                              parimaa_na(cfile->file.name))) {
            free(cfile);
            continue;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.size = len;
        copy = malloc_tmp(len);
        ຖ້າ (!copy) {
            warn_noalloc();
            free(cfile);
            ສົ່ງຄືນ;
        }
        memcpy(copy, (void *)mod[i].mod_start, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->data = copy;
        romfile_add(&cfile->file);
    }
}
