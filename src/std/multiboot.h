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

#جيڪڏهن_وصف_نه_ٿيل MULTIBOOT_سرو
#وصف_ڪريو MULTIBOOT_سرو 1

/* How many bytes from the start of the file we search for the header.  */
#وصف_ڪريو MULTIBOOT_ڳولا			8192
#وصف_ڪريو MULTIBOOT_سرو_ALIGN			4

/* The magic field should contain this.  */
#وصف_ڪريو MULTIBOOT_سرو_MAGIC			0x1BADB002

/* This should be in %eax.  */
#وصف_ڪريو MULTIBOOT_BOOTLOADER_MAGIC		0x2BADB002

/* Alignment of multiboot modules.  */
#وصف_ڪريو MULTIBOOT_MOD_ALIGN			0x00001000

/* Alignment of the multiboot info structure.  */
#وصف_ڪريو MULTIBOOT_INFO_ALIGN			0x00000004

/* Flags set in the 'flags' member of the multiboot header.  */

/* Align all boot modules on i386 page (4KB) boundaries.  */
#وصف_ڪريو MULTIBOOT_صفحو_ALIGN			0x00000001

/* Must pass memory information to OS.  */
#وصف_ڪريو MULTIBOOT_يادگيري_INFO			0x00000002

/* Must pass video information to OS.  */
#وصف_ڪريو MULTIBOOT_VIDEO_گهڻائي			0x00000004

/* This flag indicates the use of the address fields in the header.  */
#وصف_ڪريو MULTIBOOT_AOUT_KLUDGE			0x00010000

/* Flags to be set in the 'flags' member of the multiboot info structure.  */

/* is there basic lower/upper memory information? */
#وصف_ڪريو MULTIBOOT_INFO_يادگيري			0x00000001
/* is there a boot device set? */
#وصف_ڪريو MULTIBOOT_INFO_BOOTDEV			0x00000002
/* is the command-line defined? */
#وصف_ڪريو MULTIBOOT_INFO_CMDLINE			0x00000004
/* are there modules to do something with? */
#وصف_ڪريو MULTIBOOT_INFO_MODS			0x00000008

/* These next two are mutually exclusive */

/* is there a symbol table loaded? */
#وصف_ڪريو MULTIBOOT_INFO_AOUT_SYMS		0x00000010
/* is there an ELF section header table? */
#وصف_ڪريو MULTIBOOT_INFO_ELF_SHDR			0X00000020

/* is there a full memory map? */
#وصف_ڪريو MULTIBOOT_INFO_MEM_MAP			0x00000040

/* Is there drive info?  */
#وصف_ڪريو MULTIBOOT_INFO_DRIVE_INFO		0x00000080

/* Is there a config table?  */
#وصف_ڪريو MULTIBOOT_INFO_CONFIG_جدول		0x00000100

/* Is there a boot loader name?  */
#وصف_ڪريو MULTIBOOT_INFO_BOOT_LOADER_NAME		0x00000200

/* Is there a APM table?  */
#وصف_ڪريو MULTIBOOT_INFO_APM_جدول		0x00000400

/* Is there video information?  */
#وصف_ڪريو MULTIBOOT_INFO_VBE_INFO		        0x00000800
#وصف_ڪريو MULTIBOOT_INFO_FRAMEBUFFER_INFO	        0x00001000

#جيڪڏهن_وصف_نه_ٿيل ASM_FILE

تعريفنوع بنانشان حرف		multiboot_uint8_t;
تعريفنوع بنانشان ننڍو		multiboot_uint16_t;
تعريفنوع بنانشان صحيح		multiboot_uint32_t;
تعريفنوع بنانشان ڊگهو ڊگهو	multiboot_uint64_t;

بنية multiboot_سرو
{
  /* Must be MULTIBOOT_MAGIC - see above.  */
  multiboot_uint32_t magic;

  /* Feature flags.  */
  multiboot_uint32_t flags;

  /* The above fields plus this one must equal 0 mod 2^32. */
  multiboot_uint32_t checksum;

  /* These are only valid if MULTIBOOT_AOUT_KLUDGE is set.  */
  multiboot_uint32_t سرو_addr;
  multiboot_uint32_t ڀريو_addr;
  multiboot_uint32_t ڀريو_پڇاڙي_addr;
  multiboot_uint32_t bss_پڇاڙي_addr;
  multiboot_uint32_t داخلا_addr;

  /* These are only valid if MULTIBOOT_VIDEO_MODE is set.  */
  multiboot_uint32_t گهڻائي_type;
  multiboot_uint32_t ويڪر;
  multiboot_uint32_t اوچائي;
  multiboot_uint32_t اونهائي;
};

/* The symbol table for a.out.  */
بنية multiboot_aout_نشان_جدول
{
  multiboot_uint32_t tabsize;
  multiboot_uint32_t strsize;
  multiboot_uint32_t addr;
  multiboot_uint32_t reserved;
};
تعريفنوع بنية multiboot_aout_نشان_جدول multiboot_aout_نشان_جدول_t;

/* The section header table for ELF.  */
بنية multiboot_elf_section_سرو_جدول
{
  multiboot_uint32_t انگ;
  multiboot_uint32_t سائيز;
  multiboot_uint32_t addr;
  multiboot_uint32_t shndx;
};
تعريفنوع بنية multiboot_elf_section_سرو_جدول multiboot_elf_section_سرو_جدول_t;

بنية multiboot_info
{
  /* Multiboot info version number */
  multiboot_uint32_t flags;

  /* Available memory from BIOS */
  multiboot_uint32_t mem_هيٺاهون;
  multiboot_uint32_t mem_مٿانهون;

  /* "root" partition */
  multiboot_uint32_t boot_device;

  /* Kernel command line */
  multiboot_uint32_t cmdline;

  /* Boot-Module list */
  multiboot_uint32_t mods_ڳڻپ;
  multiboot_uint32_t mods_addr;

  اتحاد
  {
    multiboot_aout_نشان_جدول_t aout_sym;
    multiboot_elf_section_سرو_جدول_t elf_sec;
  } u;

  /* Memory Mapping buffer */
  multiboot_uint32_t mmap_ڊيگهائي;
  multiboot_uint32_t mmap_addr;

  /* Drive Info buffer */
  multiboot_uint32_t drives_ڊيگهائي;
  multiboot_uint32_t drives_addr;

  /* ROM configuration table */
  multiboot_uint32_t config_جدول;

  /* Boot Loader Name */
  multiboot_uint32_t boot_loader_name;

  /* APM table */
  multiboot_uint32_t apm_جدول;

  /* Video */
  multiboot_uint32_t vbe_control_info;
  multiboot_uint32_t vbe_گهڻائي_info;
  multiboot_uint16_t vbe_گهڻائي;
  multiboot_uint16_t vbe_interface_seg;
  multiboot_uint16_t vbe_interface_off;
  multiboot_uint16_t vbe_interface_len;

  multiboot_uint64_t framebuffer_addr;
  multiboot_uint32_t framebuffer_pitch;
  multiboot_uint32_t framebuffer_ويڪر;
  multiboot_uint32_t framebuffer_اوچائي;
  multiboot_uint8_t framebuffer_bpp;
#وصف_ڪريو MULTIBOOT_FRAMEBUFFER_TYPE_اشاريي_وارو 0
#وصف_ڪريو MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
#وصف_ڪريو MULTIBOOT_FRAMEBUFFER_TYPE_EGA_متن	2
  multiboot_uint8_t framebuffer_type;
  اتحاد
  {
    بنية
    {
      multiboot_uint32_t framebuffer_palette_addr;
      multiboot_uint16_t framebuffer_palette_انگ_colors;
    };
    بنية
    {
      multiboot_uint8_t framebuffer_red_ميدان_position;
      multiboot_uint8_t framebuffer_red_نقاب_سائيز;
      multiboot_uint8_t framebuffer_green_ميدان_position;
      multiboot_uint8_t framebuffer_green_نقاب_سائيز;
      multiboot_uint8_t framebuffer_blue_ميدان_position;
      multiboot_uint8_t framebuffer_blue_نقاب_سائيز;
    };
  };
};
تعريفنوع بنية multiboot_info multiboot_info_t;

بنية multiboot_color
{
  multiboot_uint8_t red;
  multiboot_uint8_t green;
  multiboot_uint8_t blue;
};

بنية multiboot_mmap_داخلا
{
  multiboot_uint32_t سائيز;
  multiboot_uint64_t addr;
  multiboot_uint64_t len;
#وصف_ڪريو MULTIBOOT_يادگيري_موجود		1
#وصف_ڪريو MULTIBOOT_يادگيري_RESERVED		2
#وصف_ڪريو MULTIBOOT_يادگيري_ACPI_RECLAIMABLE       3
#وصف_ڪريو MULTIBOOT_يادگيري_NVS                    4
#وصف_ڪريو MULTIBOOT_يادگيري_BADRAM                 5
  multiboot_uint32_t type;
} __attribute__((packed));
تعريفنوع بنية multiboot_mmap_داخلا multiboot_يادگيري_map_t;

بنية multiboot_mod_list
{
  /* the memory used goes from bytes 'mod_start' to 'mod_end-1' inclusive */
  multiboot_uint32_t mod_شروعات;
  multiboot_uint32_t mod_پڇاڙي;

  /* Module command line */
  multiboot_uint32_t cmdline;

  /* padding to take it to 16 bytes (must be zero) */
  multiboot_uint32_t pad;
};
تعريفنوع بنية multiboot_mod_list multiboot_module_t;

#جيڪڏهن_پورو /* ! ASM_FILE */

#جيڪڏهن_پورو /* ! MULTIBOOT_HEADER */
