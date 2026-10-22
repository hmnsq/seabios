// Multiboot interface support.
//
// Copyright (C) 2015  Vladimir Serbinenko <phcoder@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাৱ "config.h" // CONFIG_*
#অন্তর্ভাৱ "malloc.h" // free
#অন্তর্ভাৱ "output.h" // dprintf
#অন্তর্ভাৱ "romfile.h" // romfile_add
#অন্তর্ভাৱ "std/multiboot.h" // MULTIBOOT_*
#অন্তর্ভাৱ "string.h" // memset
#অন্তর্ভাৱ "util.h" // multiboot_init

সংরচনা mbfs_romfile_s {
    সংরচনা romfile_s file;
    শূন্য *data;
};

স্থির পূর্ণাঙ্ক
extract_filename(ৱর্ণ *dest, ৱর্ণ *src, size_t lim)
{
    ৱর্ণ *ptr;
    বাবে (ptr = src; *ptr; ptr++) {
        যদি (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            অনুৱর্ত;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        যদি (memcmp(ptr, "name=", 5) == 0) {
            পূর্ণাঙ্ক i;
            ৱর্ণ *optr = dest;
            বাবে (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            ঘূৰাই 1;
        }
    }
    ঘূৰাই 0;
}

// Copy a file to memory
স্থির পূর্ণাঙ্ক
mbfs_copyfile(সংরচনা romfile_s *file, শূন্য *dst, u32 maxlen)
{
    সংরচনা mbfs_romfile_s *cfile;
    cfile = container_of(file, সংরচনা mbfs_romfile_s, file);
    u32 size = cfile->file.size;
    শূন্য *src = cfile->data;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    যদি (size > maxlen) {
        warn_noalloc();
        ঘূৰাই -1;
    }
    iomemcpy(dst, src, size);
    ঘূৰাই size;
}

u32 __VISIBLE entry_elf_eax, entry_elf_ebx;

শূন্য
multiboot_init(শূন্য)
{
    সংরচনা multiboot_info *mbi;
    যদি (!CONFIG_MULTIBOOT)
        ঘূৰাই;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", entry_elf_eax, entry_elf_ebx);
    যদি (entry_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        ঘূৰাই;
    mbi = (শূন্য *)entry_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_গুনতি);
    যদি (!(mbi->flags & MULTIBOOT_INFO_MODS))
        ঘূৰাই;
    পূর্ণাঙ্ক i;
    সংরচনা multiboot_mod_list *mod = (শূন্য *)mbi->mods_addr;
    বাবে (i = 0; i < mbi->mods_গুনতি; i++) {
        সংরচনা mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        যদি (!mod[i].cmdline)
            অনুৱর্ত;
        len = mod[i].mod_end - mod[i].mod_start;
        cfile = malloc_tmp(পরিমাণ(*cfile));
        যদি (!cfile) {
            warn_noalloc();
            ঘূৰাই;
        }
        memset(cfile, 0, পরিমাণ(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (ৱর্ণ *)mod[i].cmdline, len);
        যদি (!extract_filename(cfile->file.name, (ৱর্ণ *)mod[i].cmdline,
                              পরিমাণ(cfile->file.name))) {
            মুক্ত(cfile);
            অনুৱর্ত;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.size = len;
        copy = malloc_tmp(len);
        যদি (!copy) {
            warn_noalloc();
            মুক্ত(cfile);
            ঘূৰাই;
        }
        memcpy(copy, (শূন্য *)mod[i].mod_start, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->data = copy;
        romfile_add(&cfile->file);
    }
}
