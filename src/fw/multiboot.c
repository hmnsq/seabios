// Multiboot interface support.
//
// Copyright (C) 2015  Vladimir Serbinenko <phcoder@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "malloc.h" // free
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "romfile.h" // romfile_add
#અન્તર્ભાવ "std/multiboot.h" // MULTIBOOT_*
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // multiboot_init

સંરચના mbfs_romfile_s {
    સંરચના romfile_s file;
    શૂન્ય *data;
};

સ્થિર પૂર્ણાંક
extract_filename(અક્ષર *dest, અક્ષર *src, size_t lim)
{
    અક્ષર *ptr;
    માટે (ptr = src; *ptr; ptr++) {
        જો (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            ચાલુ;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        જો (memcmp(ptr, "name=", 5) == 0) {
            પૂર્ણાંક i;
            અક્ષર *optr = dest;
            માટે (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            પાછા 1;
        }
    }
    પાછા 0;
}

// Copy a file to memory
સ્થિર પૂર્ણાંક
mbfs_copyfile(સંરચના romfile_s *file, શૂન્ય *dst, u32 maxlen)
{
    સંરચના mbfs_romfile_s *cfile;
    cfile = container_of(file, સંરચના mbfs_romfile_s, file);
    u32 size = cfile->file.size;
    શૂન્ય *src = cfile->data;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    જો (size > maxlen) {
        warn_noalloc();
        પાછા -1;
    }
    iomemcpy(dst, src, size);
    પાછા size;
}

u32 __VISIBLE entry_elf_eax, entry_elf_ebx;

શૂન્ય
multiboot_init(શૂન્ય)
{
    સંરચના multiboot_info *mbi;
    જો (!CONFIG_MULTIBOOT)
        પાછા;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", entry_elf_eax, entry_elf_ebx);
    જો (entry_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        પાછા;
    mbi = (શૂન્ય *)entry_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_ગણતરી);
    જો (!(mbi->flags & MULTIBOOT_INFO_MODS))
        પાછા;
    પૂર્ણાંક i;
    સંરચના multiboot_mod_list *mod = (શૂન્ય *)mbi->mods_addr;
    માટે (i = 0; i < mbi->mods_ગણતરી; i++) {
        સંરચના mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        જો (!mod[i].cmdline)
            ચાલુ;
        len = mod[i].mod_end - mod[i].mod_start;
        cfile = malloc_tmp(કદ(*cfile));
        જો (!cfile) {
            warn_noalloc();
            પાછા;
        }
        memset(cfile, 0, કદ(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (અક્ષર *)mod[i].cmdline, len);
        જો (!extract_filename(cfile->file.name, (અક્ષર *)mod[i].cmdline,
                              કદ(cfile->file.name))) {
            મુક્ત(cfile);
            ચાલુ;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.size = len;
        copy = malloc_tmp(len);
        જો (!copy) {
            warn_noalloc();
            મુક્ત(cfile);
            પાછા;
        }
        memcpy(copy, (શૂન્ય *)mod[i].mod_start, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->data = copy;
        romfile_add(&cfile->file);
    }
}
