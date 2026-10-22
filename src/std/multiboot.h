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

#اگر_غیر_تعریف_شدہ MULTIBOOT_سرخی
#تعریف_کرو MULTIBOOT_سرخی 1

/* How many bytes from the start of the file we search for the header.  */
#تعریف_کرو MULTIBOOT_تلاش			8192
#تعریف_کرو MULTIBOOT_سرخی_ALIGN			4

/* The magic field should contain this.  */
#تعریف_کرو MULTIBOOT_سرخی_MAGIC			0x1BADB002

/* This should be in %eax.  */
#تعریف_کرو MULTIBOOT_BOOTLOADER_MAGIC		0x2BADB002

/* Alignment of multiboot modules.  */
#تعریف_کرو MULTIBOOT_MOD_ALIGN			0x00001000

/* Alignment of the multiboot info structure.  */
#تعریف_کرو MULTIBOOT_INFO_ALIGN			0x00000004

/* Flags set in the 'flags' member of the multiboot header.  */

/* Align all boot modules on i386 page (4KB) boundaries.  */
#تعریف_کرو MULTIBOOT_صفحہ_ALIGN			0x00000001

/* Must pass memory information to OS.  */
#تعریف_کرو MULTIBOOT_حافظہ_INFO			0x00000002

/* Must pass video information to OS.  */
#تعریف_کرو MULTIBOOT_VIDEO_منوال			0x00000004

/* This flag indicates the use of the address fields in the header.  */
#تعریف_کرو MULTIBOOT_AOUT_KLUDGE			0x00010000

/* Flags to be set in the 'flags' member of the multiboot info structure.  */

/* is there basic lower/upper memory information? */
#تعریف_کرو MULTIBOOT_INFO_حافظہ			0x00000001
/* is there a boot device set? */
#تعریف_کرو MULTIBOOT_INFO_BOOTDEV			0x00000002
/* is the command-line defined? */
#تعریف_کرو MULTIBOOT_INFO_CMDLINE			0x00000004
/* are there modules to do something with? */
#تعریف_کرو MULTIBOOT_INFO_MODS			0x00000008

/* These next two are mutually exclusive */

/* is there a symbol table loaded? */
#تعریف_کرو MULTIBOOT_INFO_AOUT_SYMS		0x00000010
/* is there an ELF section header table? */
#تعریف_کرو MULTIBOOT_INFO_ELF_SHDR			0X00000020

/* is there a full memory map? */
#تعریف_کرو MULTIBOOT_INFO_MEM_MAP			0x00000040

/* Is there drive info?  */
#تعریف_کرو MULTIBOOT_INFO_DRIVE_INFO		0x00000080

/* Is there a config table?  */
#تعریف_کرو MULTIBOOT_INFO_CONFIG_جدول		0x00000100

/* Is there a boot loader name?  */
#تعریف_کرو MULTIBOOT_INFO_BOOT_LOADER_NAME		0x00000200

/* Is there a APM table?  */
#تعریف_کرو MULTIBOOT_INFO_APM_جدول		0x00000400

/* Is there video information?  */
#تعریف_کرو MULTIBOOT_INFO_VBE_INFO		        0x00000800
#تعریف_کرو MULTIBOOT_INFO_FRAMEBUFFER_INFO	        0x00001000

#اگر_غیر_تعریف_شدہ ASM_FILE

قسمتعریف غیرعلامتی حرف		multiboot_uint8_t;
قسمتعریف غیرعلامتی مختصر		multiboot_uint16_t;
قسمتعریف غیرعلامتی صحیح		multiboot_uint32_t;
قسمتعریف غیرعلامتی طویل طویل	multiboot_uint64_t;

ساخت multiboot_سرخی
{
  /* Must be MULTIBOOT_MAGIC - see above.  */
  multiboot_uint32_t magic;

  /* Feature flags.  */
  multiboot_uint32_t flags;

  /* The above fields plus this one must equal 0 mod 2^32. */
  multiboot_uint32_t checksum;

  /* These are only valid if MULTIBOOT_AOUT_KLUDGE is set.  */
  multiboot_uint32_t سرخی_addr;
  multiboot_uint32_t بھرو_addr;
  multiboot_uint32_t بھرو_انتہا_addr;
  multiboot_uint32_t bss_انتہا_addr;
  multiboot_uint32_t اندراج_addr;

  /* These are only valid if MULTIBOOT_VIDEO_MODE is set.  */
  multiboot_uint32_t منوال_type;
  multiboot_uint32_t چوڑائی;
  multiboot_uint32_t اونچائی;
  multiboot_uint32_t گہرائی;
};

/* The symbol table for a.out.  */
ساخت multiboot_aout_علامت_جدول
{
  multiboot_uint32_t tabsize;
  multiboot_uint32_t strsize;
  multiboot_uint32_t addr;
  multiboot_uint32_t reserved;
};
قسمتعریف ساخت multiboot_aout_علامت_جدول multiboot_aout_علامت_جدول_t;

/* The section header table for ELF.  */
ساخت multiboot_elf_section_سرخی_جدول
{
  multiboot_uint32_t عدد;
  multiboot_uint32_t سائز;
  multiboot_uint32_t addr;
  multiboot_uint32_t shndx;
};
قسمتعریف ساخت multiboot_elf_section_سرخی_جدول multiboot_elf_section_سرخی_جدول_t;

ساخت multiboot_info
{
  /* Multiboot info version number */
  multiboot_uint32_t flags;

  /* Available memory from BIOS */
  multiboot_uint32_t mem_زیریں;
  multiboot_uint32_t mem_بالائی;

  /* "root" partition */
  multiboot_uint32_t boot_device;

  /* Kernel command line */
  multiboot_uint32_t cmdline;

  /* Boot-Module list */
  multiboot_uint32_t mods_گنتی;
  multiboot_uint32_t mods_addr;

  اتحاد
  {
    multiboot_aout_علامت_جدول_t aout_sym;
    multiboot_elf_section_سرخی_جدول_t elf_sec;
  } u;

  /* Memory Mapping buffer */
  multiboot_uint32_t mmap_لمبائی;
  multiboot_uint32_t mmap_addr;

  /* Drive Info buffer */
  multiboot_uint32_t drives_لمبائی;
  multiboot_uint32_t drives_addr;

  /* ROM configuration table */
  multiboot_uint32_t config_جدول;

  /* Boot Loader Name */
  multiboot_uint32_t boot_loader_name;

  /* APM table */
  multiboot_uint32_t apm_جدول;

  /* Video */
  multiboot_uint32_t vbe_control_info;
  multiboot_uint32_t vbe_منوال_info;
  multiboot_uint16_t vbe_منوال;
  multiboot_uint16_t vbe_interface_seg;
  multiboot_uint16_t vbe_interface_off;
  multiboot_uint16_t vbe_interface_len;

  multiboot_uint64_t framebuffer_addr;
  multiboot_uint32_t framebuffer_pitch;
  multiboot_uint32_t framebuffer_چوڑائی;
  multiboot_uint32_t framebuffer_اونچائی;
  multiboot_uint8_t framebuffer_bpp;
#تعریف_کرو MULTIBOOT_FRAMEBUFFER_TYPE_اشاریہ_دار 0
#تعریف_کرو MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
#تعریف_کرو MULTIBOOT_FRAMEBUFFER_TYPE_EGA_متن	2
  multiboot_uint8_t framebuffer_type;
  اتحاد
  {
    ساخت
    {
      multiboot_uint32_t framebuffer_palette_addr;
      multiboot_uint16_t framebuffer_palette_عدد_colors;
    };
    ساخت
    {
      multiboot_uint8_t framebuffer_red_میدان_position;
      multiboot_uint8_t framebuffer_red_نقاب_سائز;
      multiboot_uint8_t framebuffer_green_میدان_position;
      multiboot_uint8_t framebuffer_green_نقاب_سائز;
      multiboot_uint8_t framebuffer_blue_میدان_position;
      multiboot_uint8_t framebuffer_blue_نقاب_سائز;
    };
  };
};
قسمتعریف ساخت multiboot_info multiboot_info_t;

ساخت multiboot_color
{
  multiboot_uint8_t red;
  multiboot_uint8_t green;
  multiboot_uint8_t blue;
};

ساخت multiboot_mmap_اندراج
{
  multiboot_uint32_t سائز;
  multiboot_uint64_t addr;
  multiboot_uint64_t len;
#تعریف_کرو MULTIBOOT_حافظہ_دستیاب		1
#تعریف_کرو MULTIBOOT_حافظہ_RESERVED		2
#تعریف_کرو MULTIBOOT_حافظہ_ACPI_RECLAIMABLE       3
#تعریف_کرو MULTIBOOT_حافظہ_NVS                    4
#تعریف_کرو MULTIBOOT_حافظہ_BADRAM                 5
  multiboot_uint32_t type;
} __attribute__((packed));
قسمتعریف ساخت multiboot_mmap_اندراج multiboot_حافظہ_map_t;

ساخت multiboot_mod_list
{
  /* the memory used goes from bytes 'mod_start' to 'mod_end-1' inclusive */
  multiboot_uint32_t mod_آغاز;
  multiboot_uint32_t mod_انتہا;

  /* Module command line */
  multiboot_uint32_t cmdline;

  /* padding to take it to 16 bytes (must be zero) */
  multiboot_uint32_t pad;
};
قسمتعریف ساخت multiboot_mod_list multiboot_module_t;

#اگر_ختم /* ! ASM_FILE */

#اگر_ختم /* ! MULTIBOOT_HEADER */
