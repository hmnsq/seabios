/*  multiboot.h - Multiboot header file.  */
/*  Copyright (C) 1999,2003,2007,2008,2009,2010  Free Software Foundation, Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ANY
 *  DEVELOPER OR DISTRIBUTOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#నిర్వచితం_కాకపోతే MULTIBOOT_శీర్షిక
#నిర్వచించు MULTIBOOT_శీర్షిక 1

/* How many bytes from the start of the file we search for the header.  */
#నిర్వచించు MULTIBOOT_శోధన			8192
#నిర్వచించు MULTIBOOT_శీర్షిక_ALIGN			4

/* The magic field should contain this.  */
#నిర్వచించు MULTIBOOT_శీర్షిక_MAGIC			0x1BADB002

/* This should be in %eax.  */
#నిర్వచించు MULTIBOOT_BOOTLOADER_MAGIC		0x2BADB002

/* Alignment of multiboot modules.  */
#నిర్వచించు MULTIBOOT_MOD_ALIGN			0x00001000

/* Alignment of the multiboot info structure.  */
#నిర్వచించు MULTIBOOT_INFO_ALIGN			0x00000004

/* Flags set in the 'flags' member of the multiboot header.  */

/* Align all boot modules on i386 page (4KB) boundaries.  */
#నిర్వచించు MULTIBOOT_పుట_ALIGN			0x00000001

/* Must pass memory information to OS.  */
#నిర్వచించు MULTIBOOT_జ్ఞాపకం_INFO			0x00000002

/* Must pass video information to OS.  */
#నిర్వచించు MULTIBOOT_VIDEO_బహుళకం			0x00000004

/* This flag indicates the use of the address fields in the header.  */
#నిర్వచించు MULTIBOOT_AOUT_KLUDGE			0x00010000

/* Flags to be set in the 'flags' member of the multiboot info structure.  */

/* is there basic lower/upper memory information? */
#నిర్వచించు MULTIBOOT_INFO_జ్ఞాపకం			0x00000001
/* is there a boot device set? */
#నిర్వచించు MULTIBOOT_INFO_BOOTDEV			0x00000002
/* is the command-line defined? */
#నిర్వచించు MULTIBOOT_INFO_CMDLINE			0x00000004
/* are there modules to do something with? */
#నిర్వచించు MULTIBOOT_INFO_MODS			0x00000008

/* These next two are mutually exclusive */

/* is there a symbol table loaded? */
#నిర్వచించు MULTIBOOT_INFO_AOUT_SYMS		0x00000010
/* is there an ELF section header table? */
#నిర్వచించు MULTIBOOT_INFO_ELF_SHDR			0X00000020

/* is there a full memory map? */
#నిర్వచించు MULTIBOOT_INFO_MEM_MAP			0x00000040

/* Is there drive info?  */
#నిర్వచించు MULTIBOOT_INFO_DRIVE_INFO		0x00000080

/* Is there a config table?  */
#నిర్వచించు MULTIBOOT_INFO_CONFIG_పట్టిక		0x00000100

/* Is there a boot loader name?  */
#నిర్వచించు MULTIBOOT_INFO_BOOT_LOADER_NAME		0x00000200

/* Is there a APM table?  */
#నిర్వచించు MULTIBOOT_INFO_APM_పట్టిక		0x00000400

/* Is there video information?  */
#నిర్వచించు MULTIBOOT_INFO_VBE_INFO		        0x00000800
#నిర్వచించు MULTIBOOT_INFO_FRAMEBUFFER_INFO	        0x00001000

#నిర్వచితం_కాకపోతే ASM_FILE

రకనిర్వచనం గుర్తులేని అక్షరం		multiboot_uint8_t;
రకనిర్వచనం గుర్తులేని పొట్టి		multiboot_uint16_t;
రకనిర్వచనం గుర్తులేని పూర్ణాంకం		multiboot_uint32_t;
రకనిర్వచనం గుర్తులేని దీర్ఘ దీర్ఘ	multiboot_uint64_t;

నిర్మాణం multiboot_శీర్షిక
{
  /* Must be MULTIBOOT_MAGIC - see above.  */
  multiboot_uint32_t magic;

  /* Feature flags.  */
  multiboot_uint32_t flags;

  /* The above fields plus this one must equal 0 mod 2^32. */
  multiboot_uint32_t checksum;

  /* These are only valid if MULTIBOOT_AOUT_KLUDGE is set.  */
  multiboot_uint32_t శీర్షిక_addr;
  multiboot_uint32_t ఎక్కించు_addr;
  multiboot_uint32_t ఎక్కించు_చివర_addr;
  multiboot_uint32_t bss_చివర_addr;
  multiboot_uint32_t నమోదు_addr;

  /* These are only valid if MULTIBOOT_VIDEO_MODE is set.  */
  multiboot_uint32_t బహుళకం_type;
  multiboot_uint32_t వెడల్పు;
  multiboot_uint32_t ఎత్తు;
  multiboot_uint32_t లోతు;
};

/* The symbol table for a.out.  */
నిర్మాణం multiboot_aout_చిహ్నం_పట్టిక
{
  multiboot_uint32_t tabsize;
  multiboot_uint32_t strsize;
  multiboot_uint32_t addr;
  multiboot_uint32_t reserved;
};
రకనిర్వచనం నిర్మాణం multiboot_aout_చిహ్నం_పట్టిక multiboot_aout_చిహ్నం_పట్టిక_t;

/* The section header table for ELF.  */
నిర్మాణం multiboot_elf_section_శీర్షిక_పట్టిక
{
  multiboot_uint32_t అంకె;
  multiboot_uint32_t కొలత;
  multiboot_uint32_t addr;
  multiboot_uint32_t shndx;
};
రకనిర్వచనం నిర్మాణం multiboot_elf_section_శీర్షిక_పట్టిక multiboot_elf_section_శీర్షిక_పట్టిక_t;

నిర్మాణం multiboot_info
{
  /* Multiboot info version number */
  multiboot_uint32_t flags;

  /* Available memory from BIOS */
  multiboot_uint32_t mem_క్రింది;
  multiboot_uint32_t mem_పై;

  /* "root" partition */
  multiboot_uint32_t boot_device;

  /* Kernel command line */
  multiboot_uint32_t cmdline;

  /* Boot-Module list */
  multiboot_uint32_t mods_లెక్క;
  multiboot_uint32_t mods_addr;

  సంఘం
  {
    multiboot_aout_చిహ్నం_పట్టిక_t aout_sym;
    multiboot_elf_section_శీర్షిక_పట్టిక_t elf_sec;
  } u;

  /* Memory Mapping buffer */
  multiboot_uint32_t mmap_నిడివి;
  multiboot_uint32_t mmap_addr;

  /* Drive Info buffer */
  multiboot_uint32_t drives_నిడివి;
  multiboot_uint32_t drives_addr;

  /* ROM configuration table */
  multiboot_uint32_t config_పట్టిక;

  /* Boot Loader Name */
  multiboot_uint32_t boot_loader_name;

  /* APM table */
  multiboot_uint32_t apm_పట్టిక;

  /* Video */
  multiboot_uint32_t vbe_control_info;
  multiboot_uint32_t vbe_బహుళకం_info;
  multiboot_uint16_t vbe_బహుళకం;
  multiboot_uint16_t vbe_interface_seg;
  multiboot_uint16_t vbe_interface_off;
  multiboot_uint16_t vbe_interface_len;

  multiboot_uint64_t framebuffer_addr;
  multiboot_uint32_t framebuffer_pitch;
  multiboot_uint32_t framebuffer_వెడల్పు;
  multiboot_uint32_t framebuffer_ఎత్తు;
  multiboot_uint8_t framebuffer_bpp;
#నిర్వచించు MULTIBOOT_FRAMEBUFFER_TYPE_సూచీకృత 0
#నిర్వచించు MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
#నిర్వచించు MULTIBOOT_FRAMEBUFFER_TYPE_EGA_పాఠ్యం	2
  multiboot_uint8_t framebuffer_type;
  సంఘం
  {
    నిర్మాణం
    {
      multiboot_uint32_t framebuffer_palette_addr;
      multiboot_uint16_t framebuffer_palette_అంకె_colors;
    };
    నిర్మాణం
    {
      multiboot_uint8_t framebuffer_red_క్షేత్రం_position;
      multiboot_uint8_t framebuffer_red_ముసుగు_కొలత;
      multiboot_uint8_t framebuffer_green_క్షేత్రం_position;
      multiboot_uint8_t framebuffer_green_ముసుగు_కొలత;
      multiboot_uint8_t framebuffer_blue_క్షేత్రం_position;
      multiboot_uint8_t framebuffer_blue_ముసుగు_కొలత;
    };
  };
};
రకనిర్వచనం నిర్మాణం multiboot_info multiboot_info_t;

నిర్మాణం multiboot_color
{
  multiboot_uint8_t red;
  multiboot_uint8_t green;
  multiboot_uint8_t blue;
};

నిర్మాణం multiboot_mmap_నమోదు
{
  multiboot_uint32_t కొలత;
  multiboot_uint64_t addr;
  multiboot_uint64_t len;
#నిర్వచించు MULTIBOOT_జ్ఞాపకం_అందుబాటు		1
#నిర్వచించు MULTIBOOT_జ్ఞాపకం_RESERVED		2
#నిర్వచించు MULTIBOOT_జ్ఞాపకం_ACPI_RECLAIMABLE       3
#నిర్వచించు MULTIBOOT_జ్ఞాపకం_NVS                    4
#నిర్వచించు MULTIBOOT_జ్ఞాపకం_BADRAM                 5
  multiboot_uint32_t type;
} __attribute__((packed));
రకనిర్వచనం నిర్మాణం multiboot_mmap_నమోదు multiboot_జ్ఞాపకం_map_t;

నిర్మాణం multiboot_mod_list
{
  /* the memory used goes from bytes 'mod_start' to 'mod_end-1' inclusive */
  multiboot_uint32_t mod_ప్రారంభం;
  multiboot_uint32_t mod_చివర;

  /* Module command line */
  multiboot_uint32_t cmdline;

  /* padding to take it to 16 bytes (must be zero) */
  multiboot_uint32_t pad;
};
రకనిర్వచనం నిర్మాణం multiboot_mod_list multiboot_module_t;

#అయితే_ముగింపు /* ! ASM_FILE */

#అయితే_ముగింపు /* ! MULTIBOOT_HEADER */
