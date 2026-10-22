// Multiboot interface support.
//
// Copyright (C) 2015  Vladimir Serbinenko <phcoder@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "romfile.h" // romfile_add
#ଅନ୍ତର୍ଭାଵ "std/multiboot.h" // MULTIBOOT_*
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // multiboot_init

ସଂରଚନା mbfs_romfile_s {
    ସଂରଚନା romfile_s file;
    ଶୂନ୍ୟ *data;
};

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
extract_filename(ଅକ୍ଷର *dest, ଅକ୍ଷର *src, size_t lim)
{
    ଅକ୍ଷର *ptr;
    ପାଇଁ (ptr = src; *ptr; ptr++) {
        ଯଦି (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            ଜାରି;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        ଯଦି (memcmp(ptr, "name=", 5) == 0) {
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
            ଅକ୍ଷର *optr = dest;
            ପାଇଁ (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            ଫେରନ୍ତୁ 1;
        }
    }
    ଫେରନ୍ତୁ 0;
}

// Copy a file to memory
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
mbfs_copyfile(ସଂରଚନା romfile_s *file, ଶୂନ୍ୟ *dst, u32 maxlen)
{
    ସଂରଚନା mbfs_romfile_s *cfile;
    cfile = container_of(file, ସଂରଚନା mbfs_romfile_s, file);
    u32 size = cfile->file.size;
    ଶୂନ୍ୟ *src = cfile->data;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    ଯଦି (size > maxlen) {
        warn_noalloc();
        ଫେରନ୍ତୁ -1;
    }
    iomemcpy(dst, src, size);
    ଫେରନ୍ତୁ size;
}

u32 __VISIBLE entry_elf_eax, entry_elf_ebx;

ଶୂନ୍ୟ
multiboot_init(ଶୂନ୍ୟ)
{
    ସଂରଚନା multiboot_info *mbi;
    ଯଦି (!CONFIG_MULTIBOOT)
        ଫେରନ୍ତୁ;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", entry_elf_eax, entry_elf_ebx);
    ଯଦି (entry_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        ଫେରନ୍ତୁ;
    mbi = (ଶୂନ୍ୟ *)entry_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_ଗଣନା);
    ଯଦି (!(mbi->flags & MULTIBOOT_INFO_MODS))
        ଫେରନ୍ତୁ;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ସଂରଚନା multiboot_mod_list *mod = (ଶୂନ୍ୟ *)mbi->mods_addr;
    ପାଇଁ (i = 0; i < mbi->mods_ଗଣନା; i++) {
        ସଂରଚନା mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        ଯଦି (!mod[i].cmdline)
            ଜାରି;
        len = mod[i].mod_end - mod[i].mod_start;
        cfile = malloc_tmp(ଆକାର(*cfile));
        ଯଦି (!cfile) {
            warn_noalloc();
            ଫେରନ୍ତୁ;
        }
        memset(cfile, 0, ଆକାର(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (ଅକ୍ଷର *)mod[i].cmdline, len);
        ଯଦି (!extract_filename(cfile->file.name, (ଅକ୍ଷର *)mod[i].cmdline,
                              ଆକାର(cfile->file.name))) {
            ମୁକ୍ତ(cfile);
            ଜାରି;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.size = len;
        copy = malloc_tmp(len);
        ଯଦି (!copy) {
            warn_noalloc();
            ମୁକ୍ତ(cfile);
            ଫେରନ୍ତୁ;
        }
        memcpy(copy, (ଶୂନ୍ୟ *)mod[i].mod_start, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->data = copy;
        romfile_add(&cfile->file);
    }
}
