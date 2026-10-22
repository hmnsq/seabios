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

ರಚನೆ mbfs_romfile_s {
    ರಚನೆ romfile_s file;
    ಶೂನ್ಯ *ದತ್ತಾಂಶ;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ಹೊರತೆಗೆ_filename(ಅಕ್ಷರ *dest, ಅಕ್ಷರ *src, ಗಾತ್ರ_t lim)
{
    ಅಕ್ಷರ *ptr;
    ಗಾಗಿ (ptr = src; *ptr; ptr++) {
        ಆದರೆ (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            ಮುಂದುವರಿ;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        ಆದರೆ (memcmp(ptr, "name=", 5) == 0) {
            ಪೂರ್ಣಾಂಕ i;
            ಅಕ್ಷರ *optr = dest;
            ಗಾಗಿ (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            ಹಿಂತಿರುಗಿ 1;
        }
    }
    ಹಿಂತಿರುಗಿ 0;
}

// Copy a file to memory
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
mbfs_copyfile(ರಚನೆ romfile_s *file, ಶೂನ್ಯ *dst, u32 maxlen)
{
    ರಚನೆ mbfs_romfile_s *cfile;
    cfile = container_ದ(file, ರಚನೆ mbfs_romfile_s, file);
    u32 ಗಾತ್ರ_2 = cfile->file.ಗಾತ್ರ_2;
    ಶೂನ್ಯ *src = cfile->ದತ್ತಾಂಶ;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", ಗಾತ್ರ_2, src, maxlen, dst);
    ಆದರೆ (ಗಾತ್ರ_2 > maxlen) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ -1;
    }
    iomemcpy(dst, src, ಗಾತ್ರ_2);
    ಹಿಂತಿರುಗಿ ಗಾತ್ರ_2;
}

u32 __VISIBLE ನಮೂದು_elf_eax, ನಮೂದು_elf_ebx;

ಶೂನ್ಯ
multiboot_init(ಶೂನ್ಯ)
{
    ರಚನೆ multiboot_info *mbi;
    ಆದರೆ (!CONFIG_MULTIBOOT)
        ಹಿಂತಿರುಗಿ;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", ನಮೂದು_elf_eax, ನಮೂದು_elf_ebx);
    ಆದರೆ (ನಮೂದು_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        ಹಿಂತಿರುಗಿ;
    mbi = (ಶೂನ್ಯ *)ನಮೂದು_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_ಎಣಿಕೆ);
    ಆದರೆ (!(mbi->flags & MULTIBOOT_INFO_MODS))
        ಹಿಂತಿರುಗಿ;
    ಪೂರ್ಣಾಂಕ i;
    ರಚನೆ multiboot_mod_list *mod = (ಶೂನ್ಯ *)mbi->mods_addr;
    ಗಾಗಿ (i = 0; i < mbi->mods_ಎಣಿಕೆ; i++) {
        ರಚನೆ mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        ಆದರೆ (!mod[i].cmdline)
            ಮುಂದುವರಿ;
        len = mod[i].mod_ಕೊನೆ - mod[i].mod_ಆರಂಭ;
        cfile = malloc_tmp(ಗಾತ್ರ(*cfile));
        ಆದರೆ (!cfile) {
            warn_noalloc();
            ಹಿಂತಿರುಗಿ;
        }
        memset(cfile, 0, ಗಾತ್ರ(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (ಅಕ್ಷರ *)mod[i].cmdline, len);
        ಆದರೆ (!ಹೊರತೆಗೆ_filename(cfile->file.name, (ಅಕ್ಷರ *)mod[i].cmdline,
                              ಗಾತ್ರ(cfile->file.name))) {
            ಬಿಡುಗಡೆ(cfile);
            ಮುಂದುವರಿ;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.ಗಾತ್ರ_2 = len;
        copy = malloc_tmp(len);
        ಆದರೆ (!copy) {
            warn_noalloc();
            ಬಿಡುಗಡೆ(cfile);
            ಹಿಂತಿರುಗಿ;
        }
        ಸ್ಮರಣೆ_ನಕಲು(copy, (ಶೂನ್ಯ *)mod[i].mod_ಆರಂಭ, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->ದತ್ತಾಂಶ = copy;
        romfile_ಕೂಡಿಸು(&cfile->file);
    }
}
