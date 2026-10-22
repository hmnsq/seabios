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
    لكل (ptr = src; *ptr; ptr++) {
        𐤀𐤌 (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            استمر;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        𐤀𐤌 (memcmp(ptr, "name=", 5) == 0) {
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
    cfile = container_of(file, بنية mbfs_romfile_s, file);
    u32 size = cfile->file.size;
    فراغ *src = cfile->data;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    𐤀𐤌 (size > maxlen) {
        warn_noalloc();
        أرجع -1;
    }
    iomemcpy(dst, src, size);
    أرجع size;
}

u32 __VISIBLE entry_elf_eax, entry_elf_ebx;

فراغ
multiboot_init(فراغ)
{
    بنية multiboot_info *mbi;
    𐤀𐤌 (!CONFIG_MULTIBOOT)
        أرجع;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", entry_elf_eax, entry_elf_ebx);
    𐤀𐤌 (entry_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        أرجع;
    mbi = (فراغ *)entry_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_count);
    𐤀𐤌 (!(mbi->flags & MULTIBOOT_INFO_MODS))
        أرجع;
    صحيح i;
    بنية multiboot_mod_list *mod = (فراغ *)mbi->mods_addr;
    لكل (i = 0; i < mbi->mods_count; i++) {
        بنية mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        𐤀𐤌 (!mod[i].cmdline)
            استمر;
        len = mod[i].mod_end - mod[i].mod_start;
        cfile = malloc_tmp(حجم(*cfile));
        𐤀𐤌 (!cfile) {
            warn_noalloc();
            أرجع;
        }
        memset(cfile, 0, حجم(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (حرف *)mod[i].cmdline, len);
        𐤀𐤌 (!extract_filename(cfile->file.name, (حرف *)mod[i].cmdline,
                              حجم(cfile->file.name))) {
            حرر(cfile);
            استمر;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.size = len;
        copy = malloc_tmp(len);
        𐤀𐤌 (!copy) {
            warn_noalloc();
            حرر(cfile);
            أرجع;
        }
        memcpy(copy, (فراغ *)mod[i].mod_start, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->data = copy;
        romfile_add(&cfile->file);
    }
}
