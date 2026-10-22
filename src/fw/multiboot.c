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

ساخت mbfs_romfile_s {
    ساخت romfile_s file;
    خالی *ڈیٹا;
};

ساکن صحیح
نکالو_filename(حرف *dest, حرف *src, سائز_t lim)
{
    حرف *ptr;
    برائے (ptr = src; *ptr; ptr++) {
        اگر (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            جاری;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        اگر (memcmp(ptr, "name=", 5) == 0) {
            صحیح i;
            حرف *optr = dest;
            برائے (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            واپس 1;
        }
    }
    واپس 0;
}

// Copy a file to memory
ساکن صحیح
mbfs_copyfile(ساخت romfile_s *file, خالی *dst, u32 maxlen)
{
    ساخت mbfs_romfile_s *cfile;
    cfile = container_کا(file, ساخت mbfs_romfile_s, file);
    u32 سائز = cfile->file.سائز;
    خالی *src = cfile->ڈیٹا;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", سائز, src, maxlen, dst);
    اگر (سائز > maxlen) {
        warn_noalloc();
        واپس -1;
    }
    iomemcpy(dst, src, سائز);
    واپس سائز;
}

u32 __VISIBLE اندراج_elf_eax, اندراج_elf_ebx;

خالی
multiboot_init(خالی)
{
    ساخت multiboot_info *mbi;
    اگر (!CONFIG_MULTIBOOT)
        واپس;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", اندراج_elf_eax, اندراج_elf_ebx);
    اگر (اندراج_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        واپس;
    mbi = (خالی *)اندراج_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_گنتی);
    اگر (!(mbi->flags & MULTIBOOT_INFO_MODS))
        واپس;
    صحیح i;
    ساخت multiboot_mod_list *mod = (خالی *)mbi->mods_addr;
    برائے (i = 0; i < mbi->mods_گنتی; i++) {
        ساخت mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        اگر (!mod[i].cmdline)
            جاری;
        len = mod[i].mod_انتہا - mod[i].mod_آغاز;
        cfile = malloc_tmp(حجم(*cfile));
        اگر (!cfile) {
            warn_noalloc();
            واپس;
        }
        memset(cfile, 0, حجم(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (حرف *)mod[i].cmdline, len);
        اگر (!نکالو_filename(cfile->file.name, (حرف *)mod[i].cmdline,
                              حجم(cfile->file.name))) {
            آزاد(cfile);
            جاری;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.سائز = len;
        copy = malloc_tmp(len);
        اگر (!copy) {
            warn_noalloc();
            آزاد(cfile);
            واپس;
        }
        حافظہ_نقل(copy, (خالی *)mod[i].mod_آغاز, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->ڈیٹا = copy;
        romfile_جوڑو(&cfile->file);
    }
}
