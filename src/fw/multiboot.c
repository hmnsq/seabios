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
    فراغ *data;
};

ساكن صحيح
extract_filename(حرف *dest, حرف *src, size_t lim)
{
    حرف *ptr;
    עבור (ptr = src; *ptr; ptr++) {
        אם (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            استمر;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        אם (memcmp(ptr, "name=", 5) == 0) {
            صحيح i;
            حرف *optr = dest;
            עבור (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            החזר 1;
        }
    }
    החזר 0;
}

// Copy a file to memory
ساكن صحيح
mbfs_copyfile(بنية romfile_s *file, فراغ *dst, u32 maxlen)
{
    بنية mbfs_romfile_s *cfile;
    cfile = container_of(file, بنية mbfs_romfile_s, file);
    u32 size = cfile->file.size;
    فراغ *src = cfile->data;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    אם (size > maxlen) {
        warn_noalloc();
        החזר -1;
    }
    iomemcpy(dst, src, size);
    החזר size;
}

u32 __VISIBLE entry_elf_eax, entry_elf_ebx;

فراغ
multiboot_init(فراغ)
{
    بنية multiboot_info *mbi;
    אם (!CONFIG_MULTIBOOT)
        החזר;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", entry_elf_eax, entry_elf_ebx);
    אם (entry_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        החזר;
    mbi = (فراغ *)entry_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_count);
    אם (!(mbi->flags & MULTIBOOT_INFO_MODS))
        החזר;
    صحيح i;
    بنية multiboot_mod_list *mod = (فراغ *)mbi->mods_addr;
    עבור (i = 0; i < mbi->mods_count; i++) {
        بنية mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        אם (!mod[i].cmdline)
            استمر;
        len = mod[i].mod_end - mod[i].mod_start;
        cfile = malloc_tmp(حجم(*cfile));
        אם (!cfile) {
            warn_noalloc();
            החזר;
        }
        memset(cfile, 0, حجم(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (حرف *)mod[i].cmdline, len);
        אם (!extract_filename(cfile->file.name, (حرف *)mod[i].cmdline,
                              حجم(cfile->file.name))) {
            حرر(cfile);
            استمر;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.size = len;
        copy = malloc_tmp(len);
        אם (!copy) {
            warn_noalloc();
            حرر(cfile);
            החזר;
        }
        memcpy(copy, (فراغ *)mod[i].mod_start, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->data = copy;
        romfile_add(&cfile->file);
    }
}
