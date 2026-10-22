// Coreboot interface support.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "block.h" // MAXDESCSIZE
#समावेश "byteorder.h" // be32_to_cpu
#समावेश "config.h" // CONFIG_*
#समावेश "e820map.h" // e820_add
#समावेश "hw/pcidevice.h" // pci_probe_devices
#समावेश "lzmadecode.h" // LzmaDecode
#समावेश "malloc.h" // free
#समावेश "output.h" // dprintf
#समावेश "paravirt.h" // PlatformRunningOn
#समावेश "romfile.h" // romfile_findprefix
#समावेश "stacks.h" // yield
#समावेश "string.h" // memset
#समावेश "util.h" // coreboot_preinit


/****************************************************************
 * Memory map
 ****************************************************************/

काष्ठा cb_header {
    u32 signature;
    u32 header_bytes;
    u32 header_checksum;
    u32 table_bytes;
    u32 table_checksum;
    u32 table_entries;
};

#घोषणा CB_SIGNATURE 0x4f49424C // "LBIO"

काष्ठा cb_memory_range {
    u64 start;
    u64 आकार;
    u32 type;
};

#घोषणा CB_MEM_TABLE    16

काष्ठा cb_memory {
    u32 tag;
    u32 आकार;
    काष्ठा cb_memory_range map[0];
};

#घोषणा CB_TAG_MEMORY 0x01

#घोषणा MEM_RANGE_गिनती(_rec) \
        (((_rec)->आकार - माप(*(_rec))) / माप((_rec)->map[0]))

काष्ठा cb_mainboard {
    u32 tag;
    u32 आकार;
    u8  vendor_idx;
    u8  part_idx;
    अक्षर  strings[0];
};

#घोषणा CB_TAG_MAINBOARD 0x0003

काष्ठा cb_forward {
    u32 tag;
    u32 आकार;
    u64 forward;
};

#घोषणा CB_TAG_FORWARD 0x11

काष्ठा cb_cbmem_ref {
    u32 tag;
    u32 आकार;
    u64 cbmem_addr;
};

#घोषणा CB_TAG_CBMEM_CONSOLE 0x17

काष्ठा cbmem_console {
    u32 आकार;
    u32 cursor;
    u8  body[0];
} PACKED;
#घोषणा CBMC_CURSOR_MASK ((1 << 28) - 1)
#घोषणा CBMC_OVERFLOW (1 << 31)
अटल काष्ठा cbmem_console *cbcon = शून्य;

अटल u16
ipchksum(अक्षर *buf, पूर्णांक गिनती)
{
    u16 *p = (u16*)buf;
    u32 योग = 0;
    जबतक (गिनती > 1) {
        योग += GET_FARVAR(0, *p);
        p++;
        गिनती -= 2;
    }
    अगर (गिनती)
        योग += GET_FARVAR(0, *(u8*)p);
    योग = (योग >> 16) + (योग & 0xffff);
    योग += (योग >> 16);
    वापस ~योग;
}

// Try to locate the coreboot header in a given address range.
अटल काष्ठा cb_header *
find_cb_header(u32 addr, पूर्णांक len)
{
    u32 end = addr + len;
    क्रम (; addr < end; addr += 16) {
        काष्ठा cb_header *cbh = (व्योम*)addr;
        अगर (GET_FARVAR(0, cbh->signature) != CB_SIGNATURE)
            जारी;
        u32 tsize = GET_FARVAR(0, cbh->table_bytes);
        अगर (! tsize)
            जारी;
        अगर (ipchksum((व्योम*)addr, माप(*cbh)) != 0)
            जारी;
        अगर (ipchksum((व्योम*)addr + माप(*cbh), tsize)
            != GET_FARVAR(0, cbh->table_checksum))
            जारी;
        वापस cbh;
    }
    वापस शून्य;
}

// Try to find the coreboot memory table in the given coreboot table.
व्योम *
find_cb_subtable(काष्ठा cb_header *cbh, u32 tag)
{
    अक्षर *tbl = (अक्षर *)cbh + माप(*cbh);
    u32 गिनती = GET_FARVAR(0, cbh->table_entries);
    पूर्णांक i;
    क्रम (i=0; i<गिनती; i++) {
        काष्ठा cb_memory *cbm = (व्योम*)tbl;
        tbl += GET_FARVAR(0, cbm->आकार);
        अगर (GET_FARVAR(0, cbm->tag) == tag)
            वापस cbm;
    }
    वापस शून्य;
}

काष्ठा cb_header *
find_cb_table(व्योम)
{
    काष्ठा cb_header *cbh = find_cb_header(0, 0x1000);
    अगर (!cbh)
        वापस शून्य;
    काष्ठा cb_forward *cbf = find_cb_subtable(cbh, CB_TAG_FORWARD);
    अगर (cbf) {
        dprintf(3, "Found coreboot table forwarder.\n");
        cbh = find_cb_header(GET_FARVAR(0, cbf->forward), 0x100);
        अगर (!cbh)
            वापस शून्य;
    }
    वापस cbh;
}

अटल काष्ठा cb_memory *CBMemTable;
स्थिर अक्षर *CBvendor = "", *CBpart = "";

// Populate max ram and e820 map info by scanning for a coreboot table.
व्योम
coreboot_preinit(व्योम)
{
    अगर (!CONFIG_COREBOOT)
        वापस;

    dprintf(3, "Attempting to find coreboot table\n");

    // Find coreboot table.
    काष्ठा cb_header *cbh = find_cb_table();
    अगर (!cbh)
        जाओ fail;
    dprintf(3, "Now attempting to find coreboot memory map\n");
    काष्ठा cb_memory *cbm = CBMemTable = find_cb_subtable(cbh, CB_TAG_MEMORY);
    अगर (!cbm)
        जाओ fail;

    पूर्णांक i, गिनती = MEM_RANGE_गिनती(cbm);
    क्रम (i=0; i<गिनती; i++) {
        काष्ठा cb_memory_range *m = &cbm->map[i];
        u32 type = m->type;
        अगर (type == CB_MEM_TABLE)
            type = E820_RESERVED;
        e820_add(m->start, m->आकार, type);
    }

    // Ughh - coreboot likes to set a map at 0x0000-0x1000, but this
    // confuses grub.  So, override it.
    e820_add(0, 16*1024, E820_RAM);

    काष्ठा cb_cbmem_ref *cbref = find_cb_subtable(cbh, CB_TAG_CBMEM_CONSOLE);
    अगर (cbref) {
        cbcon = (व्योम*)(u32)cbref->cbmem_addr;
        debug_banner();
        dprintf(1, "Found coreboot cbmem console @ %llx\n", cbref->cbmem_addr);
    }

    काष्ठा cb_mainboard *cbmb = find_cb_subtable(cbh, CB_TAG_MAINBOARD);
    अगर (cbmb) {
        CBvendor = &cbmb->strings[cbmb->vendor_idx];
        CBpart = &cbmb->strings[cbmb->part_idx];
        dprintf(1, "Found mainboard %s %s\n", CBvendor, CBpart);
    }

    वापस;

fail:
    // No table found..  Use 16Megs as a dummy value.
    dprintf(1, "Unable to find coreboot table!\n");
    e820_add(0, 16*1024*1024, E820_RAM);
    वापस;
}

व्योम coreboot_debug_putc(अक्षर c)
{
    अगर (!CONFIG_DEBUG_COREBOOT)
        वापस;
    अगर (!cbcon)
        वापस;
    u32 cursor = cbcon->cursor & CBMC_CURSOR_MASK;
    u32 flags = cbcon->cursor & ~CBMC_CURSOR_MASK;
    अगर (cursor >= cbcon->आकार)
        वापस; // Old coreboot version with legacy overflow mechanism.
    cbcon->body[cursor++] = c;
    अगर (cursor >= cbcon->आकार) {
        cursor = 0;
        flags |= CBMC_OVERFLOW;
    }
    cbcon->cursor = flags | cursor;
}

/****************************************************************
 * BIOS table copying
 ****************************************************************/

// Attempt to find (and relocate) any standard bios tables found in a
// given address range.
अटल व्योम
scan_tables(u32 start, u32 आकार)
{
    व्योम *p = (व्योम*)ALIGN(start, 16);
    व्योम *end = (व्योम*)start + आकार;
    क्रम (; p<end; p += 16)
        copy_table(p);
}

व्योम
coreboot_platform_setup(व्योम)
{
    अगर (!CONFIG_COREBOOT)
        वापस;
    pci_probe_devices();

    काष्ठा cb_memory *cbm = CBMemTable;
    अगर (!cbm)
        वापस;

    dprintf(3, "Relocating coreboot bios tables\n");

    // Scan CB_MEM_TABLE areas for bios tables.
    पूर्णांक i, गिनती = MEM_RANGE_गिनती(cbm);
    क्रम (i=0; i<गिनती; i++) {
        काष्ठा cb_memory_range *m = &cbm->map[i];
        अगर (m->type == CB_MEM_TABLE)
            scan_tables(m->start, m->आकार);
    }

    find_acpi_features();
}


/****************************************************************
 * ulzma
 ****************************************************************/

// Uncompress data in flash to an area of memory.
अटल पूर्णांक
ulzma(u8 *dst, u32 maxlen, स्थिर u8 *src, u32 srclen)
{
    dprintf(3, "Uncompressing data %d@%p to %d@%p\n", srclen, src, maxlen, dst);
    CLzmaDecoderState state;
    पूर्णांक ret = LzmaDecodeProperties(&state.Properties, src, LZMA_PROPERTIES_आकार);
    अगर (ret != LZMA_परिणाम_OK) {
        dprintf(1, "LzmaDecodeProperties error - %d\n", ret);
        वापस -1;
    }
    u8 scratch[15980];
    पूर्णांक need = (Lzma_Get_अंक_Probs(&state.Properties) * माप(CProb));
    अगर (need > माप(scratch)) {
        dprintf(1, "LzmaDecode need %d have %d\n", need, (अचिन्हित पूर्णांक)माप(scratch));
        वापस -1;
    }
    state.Probs = (CProb *)scratch;

    u32 dstlen = *(u32*)(src + LZMA_PROPERTIES_आकार);
    अगर (dstlen > maxlen) {
        dprintf(1, "LzmaDecode too large (max %d need %d)\n", maxlen, dstlen);
        वापस -1;
    }
    u32 inProcessed, outProcessed;
    ret = LzmaDecode(&state, src + LZMA_PROPERTIES_आकार + 8, srclen
                     , &inProcessed, dst, dstlen, &outProcessed);
    अगर (ret) {
        dprintf(1, "LzmaDecode returned %d\n", ret);
        वापस -1;
    }
    वापस dstlen;
}


/****************************************************************
 * Coreboot flash format
 ****************************************************************/

#घोषणा CBFS_HEADER_MAGIC 0x4F524243
#घोषणा CBFS_VERSION1 0x31313131

काष्ठा cbfs_header {
    u32 magic;
    u32 version;
    u32 romsize;
    u32 bootblocksize;
    u32 align;
    u32 offset;
    u32 pad[2];
} PACKED;

#घोषणा CBFS_FILE_MAGIC 0x455649484352414cLL // LARCHIVE

काष्ठा cbfs_file {
    u64 magic;
    u32 len;
    u32 type;
    u32 checksum;
    u32 offset;
    अक्षर filename[0];
} PACKED;

काष्ठा cbfs_romfile_s {
    काष्ठा romfile_s file;
    काष्ठा cbfs_file *fhdr;
    व्योम *तथ्य;
    u32 rawsize, flags;
};

// Copy a file to memory (uncompressing if necessary)
अटल पूर्णांक
cbfs_copyfile(काष्ठा romfile_s *file, व्योम *dst, u32 maxlen)
{
    अगर (!CONFIG_COREBOOT_FLASH)
        वापस -1;

    काष्ठा cbfs_romfile_s *cfile;
    cfile = container_of(file, काष्ठा cbfs_romfile_s, file);
    u32 आकार = cfile->rawsize;
    व्योम *src = cfile->तथ्य;
    अगर (cfile->flags) {
        // Compressed - copy to temp ram and uncompress it.
        व्योम *अस्थायी = malloc_tmphigh(आकार);
        अगर (!अस्थायी) {
            warn_noalloc();
            वापस -1;
        }
        iomemcpy(अस्थायी, src, आकार);
        पूर्णांक ret = ulzma(dst, maxlen, अस्थायी, आकार);
        yield();
        मुक्त(अस्थायी);
        वापस ret;
    }

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", आकार, src, maxlen, dst);
    अगर (आकार > maxlen) {
        warn_noalloc();
        वापस -1;
    }
    iomemcpy(dst, src, आकार);
    वापस आकार;
}

// Process CBFS links file.  The links file is a newline separated
// file where each line has a "link name" and a "destination name"
// separated by a space character.
अटल व्योम
process_links_file(व्योम)
{
    अक्षर *links = romfile_loadfile("links", शून्य), *अग्र = links;
    जबतक (अग्र) {
        // Parse out linkname and destname
        अक्षर *linkname = अग्र;
        अग्र = म_अक्षर(linkname, '\n');
        अगर (अग्र)
            *अग्र++ = '\0';
        अक्षर *comment = म_अक्षर(linkname, '#');
        अगर (comment)
            *comment = '\0';
        linkname = nullTrailingSpace(linkname);
        अक्षर *destname = म_अक्षर(linkname, ' ');
        अगर (!destname)
            जारी;
        *destname++ = '\0';
        destname = nullTrailingSpace(destname);
        // Lookup destname and create new romfile entry for linkname
        काष्ठा romfile_s *ufile = romfile_find(destname);
        अगर (!ufile)
            जारी;
        काष्ठा cbfs_romfile_s *cufile
            = container_of(ufile, काष्ठा cbfs_romfile_s, file);
        काष्ठा cbfs_romfile_s *cfile = malloc_tmp(माप(*cfile));
        अगर (!cfile) {
            warn_noalloc();
            अवरोध;
        }
        स_नकल(cfile, cufile, माप(*cfile));
        strtcpy(cfile->file.name, linkname, माप(cfile->file.name));
        romfile_add(&cfile->file);
    }
    मुक्त(links);
}

व्योम
coreboot_cbfs_init(व्योम)
{
    अगर (!CONFIG_COREBOOT_FLASH)
        वापस;

    काष्ठा cbfs_header *hdr = *(व्योम **)(CONFIG_CBFS_LOCATION - 4);
    अगर ((u32)hdr & 0x03) {
        dprintf(1, "Invalid CBFS pointer %p\n", hdr);
        वापस;
    }
    अगर (CONFIG_CBFS_LOCATION && (u32)hdr > CONFIG_CBFS_LOCATION)
        // Looks like the pointer is relative to CONFIG_CBFS_LOCATION
        hdr = (व्योम*)hdr + CONFIG_CBFS_LOCATION;
    अगर (hdr->magic != cpu_to_be32(CBFS_HEADER_MAGIC)) {
        dprintf(1, "Unable to find CBFS (ptr=%p; got %x not %x)\n"
                , hdr, hdr->magic, cpu_to_be32(CBFS_HEADER_MAGIC));
        वापस;
    }
    dprintf(1, "Found CBFS header at %p\n", hdr);

    u32 romsize = be32_to_cpu(hdr->romsize);
    u32 romstart = CONFIG_CBFS_LOCATION - romsize;
    काष्ठा cbfs_file *fhdr = (व्योम*)romstart + be32_to_cpu(hdr->offset);
    क्रम (;;) {
        अगर ((u32)fhdr - romstart > romsize)
            अवरोध;
        u64 magic = fhdr->magic;
        अगर (magic != CBFS_FILE_MAGIC)
            अवरोध;
        काष्ठा cbfs_romfile_s *cfile = malloc_tmp(माप(*cfile));
        अगर (!cfile) {
            warn_noalloc();
            अवरोध;
        }
        स_रखो(cfile, 0, माप(*cfile));
        strtcpy(cfile->file.name, fhdr->filename, माप(cfile->file.name));
        cfile->file.आकार = cfile->rawsize = be32_to_cpu(fhdr->len);
        cfile->fhdr = fhdr;
        cfile->file.copy = cbfs_copyfile;
        cfile->तथ्य = (व्योम*)fhdr + be32_to_cpu(fhdr->offset);
        पूर्णांक len = म_माप(cfile->file.name);
        अगर (len > 5 && म_भेद(&cfile->file.name[len-5], ".lzma") == 0) {
            // Using compression.
            cfile->flags = 1;
            cfile->file.name[len-5] = '\0';
            cfile->file.आकार = *(u32*)(cfile->तथ्य + LZMA_PROPERTIES_आकार);
        }
        romfile_add(&cfile->file);

        fhdr = (व्योम*)ALIGN((u32)cfile->तथ्य + cfile->rawsize
                            , be32_to_cpu(hdr->align));
    }

    process_links_file();
}

काष्ठा cbfs_payload_segment {
    u32 type;
    u32 compression;
    u32 offset;
    u64 load_addr;
    u32 len;
    u32 mem_len;
} PACKED;

#घोषणा PAYLOAD_SEGMENT_BSS    0x20535342
#घोषणा PAYLOAD_SEGMENT_ENTRY  0x52544E45

#घोषणा CBFS_COMPRESS_NONE  0
#घोषणा CBFS_COMPRESS_LZMA  1

काष्ठा cbfs_payload {
    काष्ठा cbfs_payload_segment segments[1];
};

व्योम
cbfs_run_payload(काष्ठा cbfs_file *fhdr)
{
    अगर (!CONFIG_COREBOOT_FLASH || !fhdr)
        वापस;
    dprintf(1, "Run %s\n", fhdr->filename);
    काष्ठा cbfs_payload *pay = (व्योम*)fhdr + be32_to_cpu(fhdr->offset);
    काष्ठा cbfs_payload_segment *seg = pay->segments;
    क्रम (;;) {
        व्योम *src = (व्योम*)pay + be32_to_cpu(seg->offset);
        व्योम *dest = (व्योम*)(u32)be64_to_cpu(seg->load_addr);
        u32 src_len = be32_to_cpu(seg->len);
        u32 dest_len = be32_to_cpu(seg->mem_len);
        चयन (seg->type) {
        हाल PAYLOAD_SEGMENT_BSS:
            dprintf(3, "BSS segment %d@%p\n", dest_len, dest);
            स_रखो(dest, 0, dest_len);
            अवरोध;
        हाल PAYLOAD_SEGMENT_ENTRY: {
            dprintf(1, "Calling addr %p\n", dest);
            व्योम (*func)(व्योम) = dest;
            func();
            वापस;
        }
        शेष:
            dprintf(3, "Segment %x %d@%p -> %d@%p\n"
                    , seg->type, src_len, src, dest_len, dest);
            अगर (seg->compression == cpu_to_be32(CBFS_COMPRESS_NONE)) {
                अगर (src_len > dest_len)
                    src_len = dest_len;
                स_नकल(dest, src, src_len);
            } अन्यथा अगर (CONFIG_LZMA
                       && seg->compression == cpu_to_be32(CBFS_COMPRESS_LZMA)) {
                पूर्णांक ret = ulzma(dest, dest_len, src, src_len);
                अगर (ret < 0)
                    वापस;
                src_len = ret;
            } अन्यथा {
                dprintf(1, "No support for compression type %x\n"
                        , seg->compression);
                वापस;
            }
            अगर (dest_len > src_len)
                स_रखो(dest + src_len, 0, dest_len - src_len);
            अवरोध;
        }
        seg++;
    }
}

// Register payloads in "img/" directory with boot system.
व्योम
cbfs_payload_setup(व्योम)
{
    अगर (!CONFIG_COREBOOT_FLASH)
        वापस;
    काष्ठा romfile_s *file = शून्य;
    क्रम (;;) {
        file = romfile_findprefix("img/", file);
        अगर (!file)
            अवरोध;
        काष्ठा cbfs_romfile_s *cfile;
        cfile = container_of(file, काष्ठा cbfs_romfile_s, file);
        स्थिर अक्षर *filename = file->name;
        अक्षर *desc = znprintf(MAXDESCSIZE, "Payload [%s]", &filename[4]);
        boot_add_cbfs(cfile->fhdr, desc, bootprio_find_named_rom(filename, 0));
    }
}
