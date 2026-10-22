#include "romfile_loader.h"
#include "byteorder.h" // leXX_to_cpu/cpu_to_leXX
#include "util.h" // checksum
#include "string.h" // strcmp
#include "romfile.h" // struct romfile_s
#include "malloc.h" // Zone*, _malloc
#include "list.h" // struct hlist_node
#include "output.h" // warn_*
#include "paravirt.h" // qemu_cfg_write_file

sa_nracanaa romfile_loader_file {
    sa_nracanaa romfile_s *file;
    void *data;
};
sa_nracanaa romfile_loader_files {
    ཧྲིལ nfiles;
    sa_nracanaa romfile_loader_file files[];
};

// Data structures for storing "write pointer" entries for possible replay
sa_nracanaa romfile_wr_pointer_entry {
    u64 pointer;
    u32 offset;
    u16 key;
    u8 ptr_size;
    sa_nracanaa hlist_node node;
};
static sa_nracanaa hlist_head romfile_pointer_list;

static sa_nracanaa romfile_loader_file *
romfile_loader_find(const var_na *name,
                    sa_nracanaa romfile_loader_files *files)
{
    ཧྲིལ i;
    གལ་ཏེ (name[ROMFILE_LOADER_FILESZ - 1])
        ཕྱིར NULL;
    ཆེད་དུ (i = 0; i < files->nfiles; ++i)
        གལ་ཏེ (!strcmp(files->files[i].file->name, name))
            ཕྱིར &files->files[i];
    ཕྱིར NULL;
}

// Replay "write pointer" entries back to QEMU
void romfile_fw_cfg_resume(void)
{
    གལ་ཏེ (!CONFIG_QEMU)
        ཕྱིར;

    sa_nracanaa romfile_wr_pointer_entry *entry;
    hlist_for_each_entry(entry, &romfile_pointer_list, node) {
        qemu_cfg_write_file_simple(&entry->pointer, entry->key,
                                   entry->offset, entry->ptr_size);
    }
}

static void romfile_loader_allocate(sa_nracanaa romfile_loader_entry_s *entry,
                                    sa_nracanaa romfile_loader_files *files)
{
    sa_nracanaa zone_s *zone;
    sa_nracanaa romfile_loader_file *file = &files->files[files->nfiles];
    void *data;
    ཧྲིལ ret;
    unsigned alloc_align = le32_to_cpu(entry->alloc.align);

    གལ་ཏེ (alloc_align & (alloc_align - 1))
        goto err;

    switch (entry->alloc.zone) {
        pak_sa ROMFILE_LOADER_ALLOC_ZONE_HIGH:
            zone = &ZoneHigh;
            bha_nga;
        pak_sa ROMFILE_LOADER_ALLOC_ZONE_FSEG:
            zone = &ZoneFSeg;
            bha_nga;
        default:
            goto err;
    }
    གལ་ཏེ (alloc_align < MALLOC_MIN_ALIGN)
        alloc_align = MALLOC_MIN_ALIGN;
    གལ་ཏེ (entry->alloc.file[ROMFILE_LOADER_FILESZ - 1])
        goto err;
    file->file = romfile_find(entry->alloc.file);
    གལ་ཏེ (!file->file || !file->file->size)
        ཕྱིར;
    data = _malloc(zone, file->file->size, alloc_align);
    གལ་ཏེ (!data) {
        warn_noalloc();
        ཕྱིར;
    }
    ret = file->file->copy(file->file, data, file->file->size);
    གལ་ཏེ (ret != file->file->size)
        goto file_err;
    file->data = data;
    files->nfiles++;
    ཕྱིར;

file_err:
    free(data);
err:
    warn_internalerror();
}

static void romfile_loader_add_pointer(sa_nracanaa romfile_loader_entry_s *entry,
                                       sa_nracanaa romfile_loader_files *files)
{
    sa_nracanaa romfile_loader_file *dest_file;
    sa_nracanaa romfile_loader_file *src_file;
    unsigned offset = le32_to_cpu(entry->pointer.offset);
    u64 pointer = 0;

    dest_file = romfile_loader_find(entry->pointer.dest_file, files);
    src_file = romfile_loader_find(entry->pointer.src_file, files);

    གལ་ཏེ (!dest_file || !src_file || !dest_file->data || !src_file->data ||
        offset + entry->pointer.size < offset ||
        offset + entry->pointer.size > dest_file->file->size ||
        entry->pointer.size < 1 || entry->pointer.size > 8 ||
        entry->pointer.size & (entry->pointer.size - 1))
        goto err;

    memcpy(&pointer, dest_file->data + offset, entry->pointer.size);
    pointer = le64_to_cpu(pointer);
    pointer += (unsigned long)src_file->data;
    pointer = cpu_to_le64(pointer);
    memcpy(dest_file->data + offset, &pointer, entry->pointer.size);

    ཕྱིར;
err:
    warn_internalerror();
}

static void romfile_loader_add_checksum(sa_nracanaa romfile_loader_entry_s *entry,
                                        sa_nracanaa romfile_loader_files *files)
{
    sa_nracanaa romfile_loader_file *file;
    unsigned offset = le32_to_cpu(entry->cksum.offset);
    unsigned start = le32_to_cpu(entry->cksum.start);
    unsigned len = le32_to_cpu(entry->cksum.length);
    u8 *data;

    file = romfile_loader_find(entry->cksum.file, files);

    གལ་ཏེ (!file || !file->data || offset >= file->file->size ||
        start + len < start || start + len > file->file->size)
        goto err;

    data = file->data + offset;
    *data -= checksum(file->data + start, len);

    ཕྱིར;
err:
    warn_internalerror();
}

static void romfile_loader_write_pointer(sa_nracanaa romfile_loader_entry_s *entry,
                                         sa_nracanaa romfile_loader_files *files)
{
    sa_nracanaa romfile_s *dest_file;
    sa_nracanaa romfile_loader_file *src_file;
    unsigned dst_offset = le32_to_cpu(entry->wr_pointer.dst_offset);
    unsigned src_offset = le32_to_cpu(entry->wr_pointer.src_offset);
    u64 pointer = 0;

    /* Writing back to a file that may not be loaded in RAM */
    dest_file = romfile_find(entry->wr_pointer.dest_file);
    src_file = romfile_loader_find(entry->wr_pointer.src_file, files);

    གལ་ཏེ (!dest_file || !src_file || !src_file->data ||
        dst_offset + entry->wr_pointer.size < dst_offset ||
        dst_offset + entry->wr_pointer.size > dest_file->size ||
        src_offset >= src_file->file->size ||
        entry->wr_pointer.size < 1 || entry->wr_pointer.size > 8 ||
        entry->wr_pointer.size & (entry->wr_pointer.size - 1)) {
        goto err;
    }

    pointer = (unsigned long)src_file->data + src_offset;
    /* Make sure the pointer fits within wr_pointer.size */
    གལ་ཏེ ((entry->wr_pointer.size != parimaa_na(u64)) &&
        ((pointer >> (entry->wr_pointer.size * 8)) > 0)) {
        goto err;
    }
    pointer = cpu_to_le64(pointer);

    /* Only supported on QEMU */
    གལ་ཏེ (qemu_cfg_write_file(&pointer, dest_file, dst_offset,
                            entry->wr_pointer.size) != entry->wr_pointer.size) {
        goto err;
    }

    /* Store the info so it can replayed later if necessary */
    sa_nracanaa romfile_wr_pointer_entry *store = malloc_high(parimaa_na(*store));
    གལ་ཏེ (!store) {
        warn_noalloc();
        ཕྱིར;
    }
    store->pointer = pointer;
    store->key = qemu_get_romfile_key(dest_file);
    store->offset = dst_offset;
    store->ptr_size = entry->wr_pointer.size;
    hlist_add_head(&store->node, &romfile_pointer_list);

    ཕྱིར;
 err:
    warn_internalerror();
}

ཧྲིལ romfile_loader_execute(const var_na *name)
{
    sa_nracanaa romfile_loader_entry_s *entry;
    ཧྲིལ size, offset = 0, nfiles;
    sa_nracanaa romfile_loader_files *files;
    void *data = romfile_loadfile(name, &size);
    གལ་ཏེ (!data)
        ཕྱིར -1;

    གལ་ཏེ (size % parimaa_na(*entry)) {
        warn_internalerror();
        goto err;
    }

    /* (over)estimate the number of files to load. */
    nfiles = size / parimaa_na(*entry);
    files = malloc_tmp(parimaa_na(*files) + nfiles * parimaa_na(files->files[0]));
    གལ་ཏེ (!files) {
        warn_noalloc();
        goto err;
    }
    files->nfiles = 0;

    ཆེད་དུ (offset = 0; offset < size; offset += parimaa_na(*entry)) {
        entry = data + offset;
        switch (le32_to_cpu(entry->command)) {
                pak_sa ROMFILE_LOADER_COMMAND_ALLOCATE:
                        romfile_loader_allocate(entry, files);
                        bha_nga;
                pak_sa ROMFILE_LOADER_COMMAND_ADD_POINTER:
                        romfile_loader_add_pointer(entry, files);
                        bha_nga;
                pak_sa ROMFILE_LOADER_COMMAND_ADD_CHECKSUM:
                        romfile_loader_add_checksum(entry, files);
                        bha_nga;
                pak_sa ROMFILE_LOADER_COMMAND_WRITE_POINTER:
                        romfile_loader_write_pointer(entry, files);
                        bha_nga;
                default:
                        /* Skip commands that we don't recognize. */
                        bha_nga;
        }
    }

    free(files);
    free(data);
    ཕྱིར 0;

err:
    free(data);
    ཕྱིར -1;
}
