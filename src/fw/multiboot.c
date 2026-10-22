// Multiboot interface support.
//
// Copyright (C) 2015  Vladimir Serbinenko <phcoder@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_add
#ಅನ್ತರ್ಭಾವ "std/multiboot.h" // MULTIBOOT_*
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // multiboot_init

ಸಂರಚನಾ mbfs_romfile_s {
    ಸಂರಚನಾ romfile_s file;
    ಶೂನ್ಯ *data;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
extract_filename(ವರ್ಣ *dest, ವರ್ಣ *src, size_t lim)
{
    ವರ್ಣ *ptr;
    ಪ್ರತಿ (ptr = src; *ptr; ptr++) {
        ಯದಿ (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            ಅನುವರ್ತ;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        ಯದಿ (memcmp(ptr, "name=", 5) == 0) {
            ಪೂರ್ಣಾಙ್ಕ i;
            ವರ್ಣ *optr = dest;
            ಪ್ರತಿ (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            ಪ್ರತ್ಯಾಗಮ 1;
        }
    }
    ಪ್ರತ್ಯಾಗಮ 0;
}

// Copy a file to memory
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
mbfs_copyfile(ಸಂರಚನಾ romfile_s *file, ಶೂನ್ಯ *dst, u32 maxlen)
{
    ಸಂರಚನಾ mbfs_romfile_s *cfile;
    cfile = container_of(file, ಸಂರಚನಾ mbfs_romfile_s, file);
    u32 size = cfile->file.size;
    ಶೂನ್ಯ *src = cfile->data;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    ಯದಿ (size > maxlen) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    iomemcpy(dst, src, size);
    ಪ್ರತ್ಯಾಗಮ size;
}

u32 __VISIBLE entry_elf_eax, entry_elf_ebx;

ಶೂನ್ಯ
multiboot_init(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ multiboot_info *mbi;
    ಯದಿ (!CONFIG_MULTIBOOT)
        ಪ್ರತ್ಯಾಗಮ;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", entry_elf_eax, entry_elf_ebx);
    ಯದಿ (entry_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        ಪ್ರತ್ಯಾಗಮ;
    mbi = (ಶೂನ್ಯ *)entry_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_ಎಣಿಕೆ);
    ಯದಿ (!(mbi->flags & MULTIBOOT_INFO_MODS))
        ಪ್ರತ್ಯಾಗಮ;
    ಪೂರ್ಣಾಙ್ಕ i;
    ಸಂರಚನಾ multiboot_mod_list *mod = (ಶೂನ್ಯ *)mbi->mods_addr;
    ಪ್ರತಿ (i = 0; i < mbi->mods_ಎಣಿಕೆ; i++) {
        ಸಂರಚನಾ mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        ಯದಿ (!mod[i].cmdline)
            ಅನುವರ್ತ;
        len = mod[i].mod_end - mod[i].mod_start;
        cfile = malloc_tmp(ಪರಿಮಾಣ(*cfile));
        ಯದಿ (!cfile) {
            warn_noalloc();
            ಪ್ರತ್ಯಾಗಮ;
        }
        memset(cfile, 0, ಪರಿಮಾಣ(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (ವರ್ಣ *)mod[i].cmdline, len);
        ಯದಿ (!extract_filename(cfile->file.name, (ವರ್ಣ *)mod[i].cmdline,
                              ಪರಿಮಾಣ(cfile->file.name))) {
            ಮುಕ್ತ(cfile);
            ಅನುವರ್ತ;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.size = len;
        copy = malloc_tmp(len);
        ಯದಿ (!copy) {
            warn_noalloc();
            ಮುಕ್ತ(cfile);
            ಪ್ರತ್ಯಾಗಮ;
        }
        memcpy(copy, (ಶೂನ್ಯ *)mod[i].mod_start, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->data = copy;
        romfile_add(&cfile->file);
    }
}
