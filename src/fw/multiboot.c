// Multiboot interface support.
//
// Copyright (C) 2015  Vladimir Serbinenko <phcoder@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "romfile.h" // romfile_add
#অন্তর্ভাব "std/multiboot.h" // MULTIBOOT_*
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // multiboot_init

কাঠামো mbfs_romfile_s {
    কাঠামো romfile_s file;
    শূন্য *তথ্য;
};

স্থির পূর্ণসংখ্যা
extract_filename(অক্ষর *dest, অক্ষর *src, আকার_t lim)
{
    অক্ষর *ptr;
    জন্য (ptr = src; *ptr; ptr++) {
        যদি (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            চালু;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        যদি (memcmp(ptr, "name=", 5) == 0) {
            পূর্ণসংখ্যা i;
            অক্ষর *optr = dest;
            জন্য (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            ফেরত 1;
        }
    }
    ফেরত 0;
}

// Copy a file to memory
স্থির পূর্ণসংখ্যা
mbfs_copyfile(কাঠামো romfile_s *file, শূন্য *dst, u32 maxlen)
{
    কাঠামো mbfs_romfile_s *cfile;
    cfile = container_of(file, কাঠামো mbfs_romfile_s, file);
    u32 আকার_2 = cfile->file.আকার_2;
    শূন্য *src = cfile->তথ্য;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", আকার_2, src, maxlen, dst);
    যদি (আকার_2 > maxlen) {
        warn_noalloc();
        ফেরত -1;
    }
    iomemcpy(dst, src, আকার_2);
    ফেরত আকার_2;
}

u32 __VISIBLE entry_elf_eax, entry_elf_ebx;

শূন্য
multiboot_init(শূন্য)
{
    কাঠামো multiboot_info *mbi;
    যদি (!CONFIG_MULTIBOOT)
        ফেরত;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", entry_elf_eax, entry_elf_ebx);
    যদি (entry_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        ফেরত;
    mbi = (শূন্য *)entry_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_গুনতি);
    যদি (!(mbi->flags & MULTIBOOT_INFO_MODS))
        ফেরত;
    পূর্ণসংখ্যা i;
    কাঠামো multiboot_mod_list *mod = (শূন্য *)mbi->mods_addr;
    জন্য (i = 0; i < mbi->mods_গুনতি; i++) {
        কাঠামো mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        যদি (!mod[i].cmdline)
            চালু;
        len = mod[i].mod_end - mod[i].mod_start;
        cfile = malloc_tmp(আকার(*cfile));
        যদি (!cfile) {
            warn_noalloc();
            ফেরত;
        }
        memset(cfile, 0, আকার(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (অক্ষর *)mod[i].cmdline, len);
        যদি (!extract_filename(cfile->file.name, (অক্ষর *)mod[i].cmdline,
                              আকার(cfile->file.name))) {
            মুক্ত(cfile);
            চালু;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.আকার_2 = len;
        copy = malloc_tmp(len);
        যদি (!copy) {
            warn_noalloc();
            মুক্ত(cfile);
            ফেরত;
        }
        memcpy(copy, (শূন্য *)mod[i].mod_start, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->তথ্য = copy;
        romfile_add(&cfile->file);
    }
}
