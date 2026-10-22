// Multiboot interface support.
//
// Copyright (C) 2015  Vladimir Serbinenko <phcoder@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "romfile.h" // romfile_add
#अन्तर्भाव "std/multiboot.h" // MULTIBOOT_*
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // multiboot_init

संरचना mbfs_romfile_s {
    संरचना romfile_s file;
    शून्य *data;
};

स्थिर पूर्णांक
extract_filename(अक्षर *dest, अक्षर *src, माप_प्रकार lim)
{
    अक्षर *ptr;
    साठी (ptr = src; *ptr; ptr++) {
        जर (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            सुरूठेवा;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        जर (स_भेद(ptr, "name=", 5) == 0) {
            पूर्णांक i;
            अक्षर *optr = dest;
            साठी (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            परत 1;
        }
    }
    परत 0;
}

// Copy a file to memory
स्थिर पूर्णांक
mbfs_copyfile(संरचना romfile_s *file, शून्य *dst, u32 maxlen)
{
    संरचना mbfs_romfile_s *cfile;
    cfile = container_of(file, संरचना mbfs_romfile_s, file);
    u32 size = cfile->file.size;
    शून्य *src = cfile->data;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    जर (size > maxlen) {
        warn_noalloc();
        परत -1;
    }
    iomemcpy(dst, src, size);
    परत size;
}

u32 __VISIBLE entry_elf_eax, entry_elf_ebx;

शून्य
multiboot_init(शून्य)
{
    संरचना multiboot_info *mbi;
    जर (!CONFIG_MULTIBOOT)
        परत;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", entry_elf_eax, entry_elf_ebx);
    जर (entry_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        परत;
    mbi = (शून्य *)entry_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_मोजणी);
    जर (!(mbi->flags & MULTIBOOT_INFO_MODS))
        परत;
    पूर्णांक i;
    संरचना multiboot_mod_list *mod = (शून्य *)mbi->mods_addr;
    साठी (i = 0; i < mbi->mods_मोजणी; i++) {
        संरचना mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        जर (!mod[i].cmdline)
            सुरूठेवा;
        len = mod[i].mod_end - mod[i].mod_start;
        cfile = malloc_tmp(आकार(*cfile));
        जर (!cfile) {
            warn_noalloc();
            परत;
        }
        स_रखो(cfile, 0, आकार(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (अक्षर *)mod[i].cmdline, len);
        जर (!extract_filename(cfile->file.name, (अक्षर *)mod[i].cmdline,
                              आकार(cfile->file.name))) {
            मुक्त(cfile);
            सुरूठेवा;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.size = len;
        copy = malloc_tmp(len);
        जर (!copy) {
            warn_noalloc();
            मुक्त(cfile);
            परत;
        }
        स_नकल(copy, (शून्य *)mod[i].mod_start, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->data = copy;
        romfile_add(&cfile->file);
    }
}
