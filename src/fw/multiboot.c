// Multiboot interface support.
//
// Copyright (C) 2015  Vladimir Serbinenko <phcoder@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // romfile_add
#ਅਨ੍ਤਰ੍ਭਾਵ "std/multiboot.h" // MULTIBOOT_*
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // multiboot_init

ਬਣਤਰ mbfs_romfile_s {
    ਬਣਤਰ romfile_s file;
    ਖਾਲੀ *ਅੰਕੜੇ;
};

ਸਥਿਰ ਪੂਰਨ
ਕੱਢੋ_filename(ਅੱਖਰ *dest, ਅੱਖਰ *src, ਮਾਪ_t lim)
{
    ਅੱਖਰ *ptr;
    ਲਈ (ptr = src; *ptr; ptr++) {
        ਜੇ (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            ਜਾਰੀ;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        ਜੇ (memcmp(ptr, "name=", 5) == 0) {
            ਪੂਰਨ i;
            ਅੱਖਰ *optr = dest;
            ਲਈ (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            ਵਾਪਸ 1;
        }
    }
    ਵਾਪਸ 0;
}

// Copy a file to memory
ਸਥਿਰ ਪੂਰਨ
mbfs_copyfile(ਬਣਤਰ romfile_s *file, ਖਾਲੀ *dst, u32 maxlen)
{
    ਬਣਤਰ mbfs_romfile_s *cfile;
    cfile = container_ਦਾ(file, ਬਣਤਰ mbfs_romfile_s, file);
    u32 ਮਾਪ = cfile->file.ਮਾਪ;
    ਖਾਲੀ *src = cfile->ਅੰਕੜੇ;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", ਮਾਪ, src, maxlen, dst);
    ਜੇ (ਮਾਪ > maxlen) {
        warn_noalloc();
        ਵਾਪਸ -1;
    }
    iomemcpy(dst, src, ਮਾਪ);
    ਵਾਪਸ ਮਾਪ;
}

u32 __VISIBLE ਇੰਦਰਾਜ_elf_eax, ਇੰਦਰਾਜ_elf_ebx;

ਖਾਲੀ
multiboot_init(ਖਾਲੀ)
{
    ਬਣਤਰ multiboot_info *mbi;
    ਜੇ (!CONFIG_MULTIBOOT)
        ਵਾਪਸ;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", ਇੰਦਰਾਜ_elf_eax, ਇੰਦਰਾਜ_elf_ebx);
    ਜੇ (ਇੰਦਰਾਜ_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        ਵਾਪਸ;
    mbi = (ਖਾਲੀ *)ਇੰਦਰਾਜ_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_ਗਿਣਤੀ);
    ਜੇ (!(mbi->flags & MULTIBOOT_INFO_MODS))
        ਵਾਪਸ;
    ਪੂਰਨ i;
    ਬਣਤਰ multiboot_mod_list *mod = (ਖਾਲੀ *)mbi->mods_addr;
    ਲਈ (i = 0; i < mbi->mods_ਗਿਣਤੀ; i++) {
        ਬਣਤਰ mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        ਜੇ (!mod[i].cmdline)
            ਜਾਰੀ;
        len = mod[i].mod_ਸਿਰਾ - mod[i].mod_ਸ਼ੁਰੂ;
        cfile = malloc_tmp(ਆਕਾਰ(*cfile));
        ਜੇ (!cfile) {
            warn_noalloc();
            ਵਾਪਸ;
        }
        memset(cfile, 0, ਆਕਾਰ(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (ਅੱਖਰ *)mod[i].cmdline, len);
        ਜੇ (!ਕੱਢੋ_filename(cfile->file.name, (ਅੱਖਰ *)mod[i].cmdline,
                              ਆਕਾਰ(cfile->file.name))) {
            ਮੁਕਤ(cfile);
            ਜਾਰੀ;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.ਮਾਪ = len;
        copy = malloc_tmp(len);
        ਜੇ (!copy) {
            warn_noalloc();
            ਮੁਕਤ(cfile);
            ਵਾਪਸ;
        }
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(copy, (ਖਾਲੀ *)mod[i].mod_ਸ਼ੁਰੂ, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->ਅੰਕੜੇ = copy;
        romfile_ਜੋੜੋ(&cfile->file);
    }
}
