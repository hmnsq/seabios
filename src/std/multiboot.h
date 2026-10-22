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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MULTIBOOT_ਸਿਰਲੇਖ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_ਸਿਰਲੇਖ 1

/* How many bytes from the start of the file we search for the header.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_ਖੋਜ			8192
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_ਸਿਰਲੇਖ_ALIGN			4

/* The magic field should contain this.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_ਸਿਰਲੇਖ_MAGIC			0x1BADB002

/* This should be in %eax.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_BOOTLOADER_MAGIC		0x2BADB002

/* Alignment of multiboot modules.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_MOD_ALIGN			0x00001000

/* Alignment of the multiboot info structure.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_INFO_ALIGN			0x00000004

/* Flags set in the 'flags' member of the multiboot header.  */

/* Align all boot modules on i386 page (4KB) boundaries.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_ਪੰਨਾ_ALIGN			0x00000001

/* Must pass memory information to OS.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_ਸਮ੍ਰਿਤੀ_INFO			0x00000002

/* Must pass video information to OS.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_VIDEO_ਬਹੁਲਕ			0x00000004

/* This flag indicates the use of the address fields in the header.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_AOUT_KLUDGE			0x00010000

/* Flags to be set in the 'flags' member of the multiboot info structure.  */

/* is there basic lower/upper memory information? */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_INFO_ਸਮ੍ਰਿਤੀ			0x00000001
/* is there a boot device set? */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_INFO_BOOTDEV			0x00000002
/* is the command-line defined? */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_INFO_CMDLINE			0x00000004
/* are there modules to do something with? */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_INFO_MODS			0x00000008

/* These next two are mutually exclusive */

/* is there a symbol table loaded? */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_INFO_AOUT_SYMS		0x00000010
/* is there an ELF section header table? */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_INFO_ELF_SHDR			0X00000020

/* is there a full memory map? */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_INFO_MEM_MAP			0x00000040

/* Is there drive info?  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_INFO_DRIVE_INFO		0x00000080

/* Is there a config table?  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_INFO_CONFIG_ਸਾਰਣੀ		0x00000100

/* Is there a boot loader name?  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_INFO_BOOT_LOADER_NAME		0x00000200

/* Is there a APM table?  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_INFO_APM_ਸਾਰਣੀ		0x00000400

/* Is there video information?  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_INFO_VBE_INFO		        0x00000800
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_INFO_FRAMEBUFFER_INFO	        0x00001000

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ASM_FILE

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ		multiboot_uint8_t;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ		multiboot_uint16_t;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ		multiboot_uint32_t;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ	multiboot_uint64_t;

ਬਣਤਰ multiboot_ਸਿਰਲੇਖ
{
  /* Must be MULTIBOOT_MAGIC - see above.  */
  multiboot_uint32_t magic;

  /* Feature flags.  */
  multiboot_uint32_t flags;

  /* The above fields plus this one must equal 0 mod 2^32. */
  multiboot_uint32_t checksum;

  /* These are only valid if MULTIBOOT_AOUT_KLUDGE is set.  */
  multiboot_uint32_t ਸਿਰਲੇਖ_addr;
  multiboot_uint32_t ਲੱਦੋ_addr;
  multiboot_uint32_t ਲੱਦੋ_ਸਿਰਾ_addr;
  multiboot_uint32_t bss_ਸਿਰਾ_addr;
  multiboot_uint32_t ਇੰਦਰਾਜ_addr;

  /* These are only valid if MULTIBOOT_VIDEO_MODE is set.  */
  multiboot_uint32_t ਬਹੁਲਕ_type;
  multiboot_uint32_t ਚੌੜਾਈ;
  multiboot_uint32_t ਉਚਾਈ;
  multiboot_uint32_t ਡੂੰਘਾਈ;
};

/* The symbol table for a.out.  */
ਬਣਤਰ multiboot_aout_ਚਿੰਨ੍ਹ_ਸਾਰਣੀ
{
  multiboot_uint32_t tabsize;
  multiboot_uint32_t strsize;
  multiboot_uint32_t addr;
  multiboot_uint32_t reserved;
};
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ multiboot_aout_ਚਿੰਨ੍ਹ_ਸਾਰਣੀ multiboot_aout_ਚਿੰਨ੍ਹ_ਸਾਰਣੀ_t;

/* The section header table for ELF.  */
ਬਣਤਰ multiboot_elf_section_ਸਿਰਲੇਖ_ਸਾਰਣੀ
{
  multiboot_uint32_t ਅੰਕ;
  multiboot_uint32_t ਮਾਪ;
  multiboot_uint32_t addr;
  multiboot_uint32_t shndx;
};
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ multiboot_elf_section_ਸਿਰਲੇਖ_ਸਾਰਣੀ multiboot_elf_section_ਸਿਰਲੇਖ_ਸਾਰਣੀ_t;

ਬਣਤਰ multiboot_info
{
  /* Multiboot info version number */
  multiboot_uint32_t flags;

  /* Available memory from BIOS */
  multiboot_uint32_t mem_ਹੇਠਲਾ;
  multiboot_uint32_t mem_ਉੱਤਲਾ;

  /* "root" partition */
  multiboot_uint32_t boot_device;

  /* Kernel command line */
  multiboot_uint32_t cmdline;

  /* Boot-Module list */
  multiboot_uint32_t mods_ਗਿਣਤੀ;
  multiboot_uint32_t mods_addr;

  ਸੰਘ
  {
    multiboot_aout_ਚਿੰਨ੍ਹ_ਸਾਰਣੀ_t aout_sym;
    multiboot_elf_section_ਸਿਰਲੇਖ_ਸਾਰਣੀ_t elf_sec;
  } u;

  /* Memory Mapping buffer */
  multiboot_uint32_t mmap_ਲੰਮਾਈ;
  multiboot_uint32_t mmap_addr;

  /* Drive Info buffer */
  multiboot_uint32_t drives_ਲੰਮਾਈ;
  multiboot_uint32_t drives_addr;

  /* ROM configuration table */
  multiboot_uint32_t config_ਸਾਰਣੀ;

  /* Boot Loader Name */
  multiboot_uint32_t boot_loader_name;

  /* APM table */
  multiboot_uint32_t apm_ਸਾਰਣੀ;

  /* Video */
  multiboot_uint32_t vbe_control_info;
  multiboot_uint32_t vbe_ਬਹੁਲਕ_info;
  multiboot_uint16_t vbe_ਬਹੁਲਕ;
  multiboot_uint16_t vbe_interface_seg;
  multiboot_uint16_t vbe_interface_off;
  multiboot_uint16_t vbe_interface_len;

  multiboot_uint64_t framebuffer_addr;
  multiboot_uint32_t framebuffer_pitch;
  multiboot_uint32_t framebuffer_ਚੌੜਾਈ;
  multiboot_uint32_t framebuffer_ਉਚਾਈ;
  multiboot_uint8_t framebuffer_bpp;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_FRAMEBUFFER_TYPE_ਸੂਚਕਾਂਕਿਤ 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_FRAMEBUFFER_TYPE_EGA_ਪਾਠ	2
  multiboot_uint8_t framebuffer_type;
  ਸੰਘ
  {
    ਬਣਤਰ
    {
      multiboot_uint32_t framebuffer_palette_addr;
      multiboot_uint16_t framebuffer_palette_ਅੰਕ_colors;
    };
    ਬਣਤਰ
    {
      multiboot_uint8_t framebuffer_red_ਖੇਤਰ_position;
      multiboot_uint8_t framebuffer_red_ਨਕਾਬ_ਮਾਪ;
      multiboot_uint8_t framebuffer_green_ਖੇਤਰ_position;
      multiboot_uint8_t framebuffer_green_ਨਕਾਬ_ਮਾਪ;
      multiboot_uint8_t framebuffer_blue_ਖੇਤਰ_position;
      multiboot_uint8_t framebuffer_blue_ਨਕਾਬ_ਮਾਪ;
    };
  };
};
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ multiboot_info multiboot_info_t;

ਬਣਤਰ multiboot_color
{
  multiboot_uint8_t red;
  multiboot_uint8_t green;
  multiboot_uint8_t blue;
};

ਬਣਤਰ multiboot_mmap_ਇੰਦਰਾਜ
{
  multiboot_uint32_t ਮਾਪ;
  multiboot_uint64_t addr;
  multiboot_uint64_t len;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_ਸਮ੍ਰਿਤੀ_ਉਪਲਬਧ		1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_ਸਮ੍ਰਿਤੀ_RESERVED		2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_ਸਮ੍ਰਿਤੀ_ACPI_RECLAIMABLE       3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_ਸਮ੍ਰਿਤੀ_NVS                    4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MULTIBOOT_ਸਮ੍ਰਿਤੀ_BADRAM                 5
  multiboot_uint32_t type;
} __attribute__((packed));
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ multiboot_mmap_ਇੰਦਰਾਜ multiboot_ਸਮ੍ਰਿਤੀ_map_t;

ਬਣਤਰ multiboot_mod_list
{
  /* the memory used goes from bytes 'mod_start' to 'mod_end-1' inclusive */
  multiboot_uint32_t mod_ਸ਼ੁਰੂ;
  multiboot_uint32_t mod_ਸਿਰਾ;

  /* Module command line */
  multiboot_uint32_t cmdline;

  /* padding to take it to 16 bytes (must be zero) */
  multiboot_uint32_t pad;
};
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ multiboot_mod_list multiboot_module_t;

#ਜੇ_ਅੰਤ /* ! ASM_FILE */

#ਜੇ_ਅੰਤ /* ! MULTIBOOT_HEADER */
