// Multiboot interface support.
//
// Copyright (C) 2015  Vladimir Serbinenko <phcoder@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "config.h" // CONFIG_*
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "romfile.h" // romfile_add
#شامل "std/multiboot.h" // MULTIBOOT_*
#شامل "string.h" // memset
#شامل "util.h" // multiboot_init

ساختار mbfs_romfile_s {
    ساختار romfile_s file;
    تهی *data;
};

ایستا صحیح
extract_filename(نویسه *dest, نویسه *src, size_t lim)
{
    نویسه *ptr;
    برای (ptr = src; *ptr; ptr++) {
        اگر (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            ادامه;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        اگر (memcmp(ptr, "name=", 5) == 0) {
            صحیح i;
            نویسه *optr = dest;
            برای (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            بازگشت 1;
        }
    }
    بازگشت 0;
}

// Copy a file to memory
ایستا صحیح
mbfs_copyfile(ساختار romfile_s *file, تهی *dst, u32 maxlen)
{
    ساختار mbfs_romfile_s *cfile;
    cfile = container_of(file, ساختار mbfs_romfile_s, file);
    u32 size = cfile->file.size;
    تهی *src = cfile->data;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    اگر (size > maxlen) {
        warn_noalloc();
        بازگشت -1;
    }
    iomemcpy(dst, src, size);
    بازگشت size;
}

u32 __VISIBLE entry_elf_eax, entry_elf_ebx;

تهی
multiboot_init(تهی)
{
    ساختار multiboot_info *mbi;
    اگر (!CONFIG_MULTIBOOT)
        بازگشت;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", entry_elf_eax, entry_elf_ebx);
    اگر (entry_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        بازگشت;
    mbi = (تهی *)entry_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_شمار);
    اگر (!(mbi->flags & MULTIBOOT_INFO_MODS))
        بازگشت;
    صحیح i;
    ساختار multiboot_mod_list *mod = (تهی *)mbi->mods_addr;
    برای (i = 0; i < mbi->mods_شمار; i++) {
        ساختار mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        اگر (!mod[i].cmdline)
            ادامه;
        len = mod[i].mod_end - mod[i].mod_start;
        cfile = malloc_tmp(اندازه(*cfile));
        اگر (!cfile) {
            warn_noalloc();
            بازگشت;
        }
        memset(cfile, 0, اندازه(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (نویسه *)mod[i].cmdline, len);
        اگر (!extract_filename(cfile->file.name, (نویسه *)mod[i].cmdline,
                              اندازه(cfile->file.name))) {
            آزاد(cfile);
            ادامه;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.size = len;
        copy = malloc_tmp(len);
        اگر (!copy) {
            warn_noalloc();
            آزاد(cfile);
            بازگشت;
        }
        memcpy(copy, (تهی *)mod[i].mod_start, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->data = copy;
        romfile_add(&cfile->file);
    }
}
