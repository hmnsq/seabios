// Multiboot interface support.
//
// Copyright (C) 2015  Vladimir Serbinenko <phcoder@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "malloc.h" // free
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "romfile.h" // romfile_add
#அந்தர்பாவ "std/multiboot.h" // MULTIBOOT_*
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // multiboot_init

கட்டமைப்பு mbfs_romfile_s {
    கட்டமைப்பு romfile_s file;
    வெறுமை *data;
};

நிலையான முழு
extract_filename(எழுத்து *dest, எழுத்து *src, size_t lim)
{
    எழுத்து *ptr;
    க்கு (ptr = src; *ptr; ptr++) {
        எனில் (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            தொடர்;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        எனில் (memcmp(ptr, "name=", 5) == 0) {
            முழு i;
            எழுத்து *optr = dest;
            க்கு (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            திரும்பு 1;
        }
    }
    திரும்பு 0;
}

// Copy a file to memory
நிலையான முழு
mbfs_copyfile(கட்டமைப்பு romfile_s *file, வெறுமை *dst, u32 maxlen)
{
    கட்டமைப்பு mbfs_romfile_s *cfile;
    cfile = container_of(file, கட்டமைப்பு mbfs_romfile_s, file);
    u32 size = cfile->file.size;
    வெறுமை *src = cfile->data;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    எனில் (size > maxlen) {
        warn_noalloc();
        திரும்பு -1;
    }
    iomemcpy(dst, src, size);
    திரும்பு size;
}

u32 __VISIBLE entry_elf_eax, entry_elf_ebx;

வெறுமை
multiboot_init(வெறுமை)
{
    கட்டமைப்பு multiboot_info *mbi;
    எனில் (!CONFIG_MULTIBOOT)
        திரும்பு;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", entry_elf_eax, entry_elf_ebx);
    எனில் (entry_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        திரும்பு;
    mbi = (வெறுமை *)entry_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_மொத்தம்);
    எனில் (!(mbi->flags & MULTIBOOT_INFO_MODS))
        திரும்பு;
    முழு i;
    கட்டமைப்பு multiboot_mod_list *mod = (வெறுமை *)mbi->mods_addr;
    க்கு (i = 0; i < mbi->mods_மொத்தம்; i++) {
        கட்டமைப்பு mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        எனில் (!mod[i].cmdline)
            தொடர்;
        len = mod[i].mod_end - mod[i].mod_start;
        cfile = malloc_tmp(அளவு(*cfile));
        எனில் (!cfile) {
            warn_noalloc();
            திரும்பு;
        }
        memset(cfile, 0, அளவு(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (எழுத்து *)mod[i].cmdline, len);
        எனில் (!extract_filename(cfile->file.name, (எழுத்து *)mod[i].cmdline,
                              அளவு(cfile->file.name))) {
            விடுவி(cfile);
            தொடர்;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.size = len;
        copy = malloc_tmp(len);
        எனில் (!copy) {
            warn_noalloc();
            விடுவி(cfile);
            திரும்பு;
        }
        memcpy(copy, (வெறுமை *)mod[i].mod_start, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->data = copy;
        romfile_add(&cfile->file);
    }
}
