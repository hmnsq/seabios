// Multiboot interface support.
//
// Copyright (C) 2015  Vladimir Serbinenko <phcoder@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "config.h" // CONFIG_*
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "romfile.h" // romfile_add
#ضمن "std/multiboot.h" // MULTIBOOT_*
#ضمن "string.h" // memset
#ضمن "util.h" // multiboot_init

بنية mbfs_romfile_s {
    بنية romfile_s file;
    فراغ *انکڑے;
};

ساكن صحيح
کڈھو_filename(حرف *dest, حرف *src, ناپ_t lim)
{
    حرف *ptr;
    لكل (ptr = src; *ptr; ptr++) {
        إذا (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            استمر;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        إذا (memcmp(ptr, "name=", 5) == 0) {
            صحيح i;
            حرف *optr = dest;
            لكل (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            أرجع 1;
        }
    }
    أرجع 0;
}

// Copy a file to memory
ساكن صحيح
mbfs_copyfile(بنية romfile_s *file, فراغ *dst, u32 maxlen)
{
    بنية mbfs_romfile_s *cfile;
    cfile = container_دا(file, بنية mbfs_romfile_s, file);
    u32 ناپ = cfile->file.ناپ;
    فراغ *src = cfile->انکڑے;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", ناپ, src, maxlen, dst);
    إذا (ناپ > maxlen) {
        warn_noalloc();
        أرجع -1;
    }
    iomemcpy(dst, src, ناپ);
    أرجع ناپ;
}

u32 __VISIBLE اندراج_elf_eax, اندراج_elf_ebx;

فراغ
multiboot_init(فراغ)
{
    بنية multiboot_info *mbi;
    إذا (!CONFIG_MULTIBOOT)
        أرجع;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", اندراج_elf_eax, اندراج_elf_ebx);
    إذا (اندراج_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        أرجع;
    mbi = (فراغ *)اندراج_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_گنتی);
    إذا (!(mbi->flags & MULTIBOOT_INFO_MODS))
        أرجع;
    صحيح i;
    بنية multiboot_mod_list *mod = (فراغ *)mbi->mods_addr;
    لكل (i = 0; i < mbi->mods_گنتی; i++) {
        بنية mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        إذا (!mod[i].cmdline)
            استمر;
        len = mod[i].mod_سرا - mod[i].mod_شروع;
        cfile = malloc_tmp(حجم(*cfile));
        إذا (!cfile) {
            warn_noalloc();
            أرجع;
        }
        memset(cfile, 0, حجم(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (حرف *)mod[i].cmdline, len);
        إذا (!کڈھو_filename(cfile->file.name, (حرف *)mod[i].cmdline,
                              حجم(cfile->file.name))) {
            حرر(cfile);
            استمر;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.ناپ = len;
        copy = malloc_tmp(len);
        إذا (!copy) {
            warn_noalloc();
            حرر(cfile);
            أرجع;
        }
        حافظہ_نقل(copy, (فراغ *)mod[i].mod_شروع, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->انکڑے = copy;
        romfile_جوڑو(&cfile->file);
    }
}
