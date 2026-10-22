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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ MULTIBOOT_ಶೀರ್ಷಿಕೆ
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_ಶೀರ್ಷಿಕೆ 1

/* How many bytes from the start of the file we search for the header.  */
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_ಹುಡುಕು			8192
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_ಶೀರ್ಷಿಕೆ_ALIGN			4

/* The magic field should contain this.  */
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_ಶೀರ್ಷಿಕೆ_MAGIC			0x1BADB002

/* This should be in %eax.  */
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_BOOTLOADER_MAGIC		0x2BADB002

/* Alignment of multiboot modules.  */
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_MOD_ALIGN			0x00001000

/* Alignment of the multiboot info structure.  */
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_INFO_ALIGN			0x00000004

/* Flags set in the 'flags' member of the multiboot header.  */

/* Align all boot modules on i386 page (4KB) boundaries.  */
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_ಪುಟ_ALIGN			0x00000001

/* Must pass memory information to OS.  */
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_ಸ್ಮರಣೆ_INFO			0x00000002

/* Must pass video information to OS.  */
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_VIDEO_ಬಹುಲಕ			0x00000004

/* This flag indicates the use of the address fields in the header.  */
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_AOUT_KLUDGE			0x00010000

/* Flags to be set in the 'flags' member of the multiboot info structure.  */

/* is there basic lower/upper memory information? */
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_INFO_ಸ್ಮರಣೆ			0x00000001
/* is there a boot device set? */
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_INFO_BOOTDEV			0x00000002
/* is the command-line defined? */
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_INFO_CMDLINE			0x00000004
/* are there modules to do something with? */
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_INFO_MODS			0x00000008

/* These next two are mutually exclusive */

/* is there a symbol table loaded? */
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_INFO_AOUT_SYMS		0x00000010
/* is there an ELF section header table? */
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_INFO_ELF_SHDR			0X00000020

/* is there a full memory map? */
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_INFO_MEM_MAP			0x00000040

/* Is there drive info?  */
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_INFO_DRIVE_INFO		0x00000080

/* Is there a config table?  */
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_INFO_CONFIG_ಕೋಷ್ಟಕ		0x00000100

/* Is there a boot loader name?  */
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_INFO_BOOT_LOADER_NAME		0x00000200

/* Is there a APM table?  */
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_INFO_APM_ಕೋಷ್ಟಕ		0x00000400

/* Is there video information?  */
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_INFO_VBE_INFO		        0x00000800
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_INFO_FRAMEBUFFER_INFO	        0x00001000

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ASM_FILE

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ		multiboot_uint8_t;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ		multiboot_uint16_t;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ		multiboot_uint32_t;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ	multiboot_uint64_t;

ರಚನೆ multiboot_ಶೀರ್ಷಿಕೆ
{
  /* Must be MULTIBOOT_MAGIC - see above.  */
  multiboot_uint32_t magic;

  /* Feature flags.  */
  multiboot_uint32_t flags;

  /* The above fields plus this one must equal 0 mod 2^32. */
  multiboot_uint32_t checksum;

  /* These are only valid if MULTIBOOT_AOUT_KLUDGE is set.  */
  multiboot_uint32_t ಶೀರ್ಷಿಕೆ_addr;
  multiboot_uint32_t ತುಂಬು_addr;
  multiboot_uint32_t ತುಂಬು_ಕೊನೆ_addr;
  multiboot_uint32_t bss_ಕೊನೆ_addr;
  multiboot_uint32_t ನಮೂದು_addr;

  /* These are only valid if MULTIBOOT_VIDEO_MODE is set.  */
  multiboot_uint32_t ಬಹುಲಕ_type;
  multiboot_uint32_t ಅಗಲ;
  multiboot_uint32_t ಎತ್ತರ;
  multiboot_uint32_t ಆಳ;
};

/* The symbol table for a.out.  */
ರಚನೆ multiboot_aout_ಚಿಹ್ನೆ_ಕೋಷ್ಟಕ
{
  multiboot_uint32_t tabsize;
  multiboot_uint32_t strsize;
  multiboot_uint32_t addr;
  multiboot_uint32_t reserved;
};
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ multiboot_aout_ಚಿಹ್ನೆ_ಕೋಷ್ಟಕ multiboot_aout_ಚಿಹ್ನೆ_ಕೋಷ್ಟಕ_t;

/* The section header table for ELF.  */
ರಚನೆ multiboot_elf_section_ಶೀರ್ಷಿಕೆ_ಕೋಷ್ಟಕ
{
  multiboot_uint32_t ಅಂಕೆ;
  multiboot_uint32_t ಗಾತ್ರ_2;
  multiboot_uint32_t addr;
  multiboot_uint32_t shndx;
};
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ multiboot_elf_section_ಶೀರ್ಷಿಕೆ_ಕೋಷ್ಟಕ multiboot_elf_section_ಶೀರ್ಷಿಕೆ_ಕೋಷ್ಟಕ_t;

ರಚನೆ multiboot_info
{
  /* Multiboot info version number */
  multiboot_uint32_t flags;

  /* Available memory from BIOS */
  multiboot_uint32_t mem_ಅಧೋ;
  multiboot_uint32_t mem_ಊರ್ಧ್ವ;

  /* "root" partition */
  multiboot_uint32_t boot_device;

  /* Kernel command line */
  multiboot_uint32_t cmdline;

  /* Boot-Module list */
  multiboot_uint32_t mods_ಎಣಿಕೆ;
  multiboot_uint32_t mods_addr;

  ಒಕ್ಕೂಟ
  {
    multiboot_aout_ಚಿಹ್ನೆ_ಕೋಷ್ಟಕ_t aout_sym;
    multiboot_elf_section_ಶೀರ್ಷಿಕೆ_ಕೋಷ್ಟಕ_t elf_sec;
  } u;

  /* Memory Mapping buffer */
  multiboot_uint32_t mmap_ಉದ್ದಳತೆ;
  multiboot_uint32_t mmap_addr;

  /* Drive Info buffer */
  multiboot_uint32_t drives_ಉದ್ದಳತೆ;
  multiboot_uint32_t drives_addr;

  /* ROM configuration table */
  multiboot_uint32_t config_ಕೋಷ್ಟಕ;

  /* Boot Loader Name */
  multiboot_uint32_t boot_loader_name;

  /* APM table */
  multiboot_uint32_t apm_ಕೋಷ್ಟಕ;

  /* Video */
  multiboot_uint32_t vbe_control_info;
  multiboot_uint32_t vbe_ಬಹುಲಕ_info;
  multiboot_uint16_t vbe_ಬಹುಲಕ;
  multiboot_uint16_t vbe_interface_seg;
  multiboot_uint16_t vbe_interface_off;
  multiboot_uint16_t vbe_interface_len;

  multiboot_uint64_t framebuffer_addr;
  multiboot_uint32_t framebuffer_pitch;
  multiboot_uint32_t framebuffer_ಅಗಲ;
  multiboot_uint32_t framebuffer_ಎತ್ತರ;
  multiboot_uint8_t framebuffer_bpp;
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_FRAMEBUFFER_TYPE_ಸೂಚ್ಯಂಕಿತ 0
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_FRAMEBUFFER_TYPE_EGA_ಪಠ್ಯ	2
  multiboot_uint8_t framebuffer_type;
  ಒಕ್ಕೂಟ
  {
    ರಚನೆ
    {
      multiboot_uint32_t framebuffer_palette_addr;
      multiboot_uint16_t framebuffer_palette_ಅಂಕೆ_colors;
    };
    ರಚನೆ
    {
      multiboot_uint8_t framebuffer_red_ಕ್ಷೇತ್ರ_position;
      multiboot_uint8_t framebuffer_red_ಮುಖವಾಡ_ಗಾತ್ರ;
      multiboot_uint8_t framebuffer_green_ಕ್ಷೇತ್ರ_position;
      multiboot_uint8_t framebuffer_green_ಮುಖವಾಡ_ಗಾತ್ರ;
      multiboot_uint8_t framebuffer_blue_ಕ್ಷೇತ್ರ_position;
      multiboot_uint8_t framebuffer_blue_ಮುಖವಾಡ_ಗಾತ್ರ;
    };
  };
};
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ multiboot_info multiboot_info_t;

ರಚನೆ multiboot_color
{
  multiboot_uint8_t red;
  multiboot_uint8_t green;
  multiboot_uint8_t blue;
};

ರಚನೆ multiboot_mmap_ನಮೂದು
{
  multiboot_uint32_t ಗಾತ್ರ_2;
  multiboot_uint64_t addr;
  multiboot_uint64_t len;
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_ಸ್ಮರಣೆ_ಲಭ್ಯ		1
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_ಸ್ಮರಣೆ_RESERVED		2
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_ಸ್ಮರಣೆ_ACPI_RECLAIMABLE       3
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_ಸ್ಮರಣೆ_NVS                    4
#ವ್ಯಾಖ್ಯಾನಿಸು MULTIBOOT_ಸ್ಮರಣೆ_BADRAM                 5
  multiboot_uint32_t type;
} __attribute__((packed));
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ multiboot_mmap_ನಮೂದು multiboot_ಸ್ಮರಣೆ_map_t;

ರಚನೆ multiboot_mod_list
{
  /* the memory used goes from bytes 'mod_start' to 'mod_end-1' inclusive */
  multiboot_uint32_t mod_ಆರಂಭ;
  multiboot_uint32_t mod_ಕೊನೆ;

  /* Module command line */
  multiboot_uint32_t cmdline;

  /* padding to take it to 16 bytes (must be zero) */
  multiboot_uint32_t pad;
};
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ multiboot_mod_list multiboot_module_t;

#ಆದರೆ_ಅಂತ್ಯ /* ! ASM_FILE */

#ಆದರೆ_ಅಂತ್ಯ /* ! MULTIBOOT_HEADER */
