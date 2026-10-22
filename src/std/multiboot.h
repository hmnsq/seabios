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

#यदि_अपरिभाषितम् MULTIBOOT_शीर्षकम्
#परिभाषय MULTIBOOT_शीर्षकम् 1

/* How many bytes from the start of the file we search for the header.  */
#परिभाषय MULTIBOOT_अन्वेषणम्			8192
#परिभाषय MULTIBOOT_शीर्षकम्_ALIGN			4

/* The magic field should contain this.  */
#परिभाषय MULTIBOOT_शीर्षकम्_MAGIC			0x1BADB002

/* This should be in %eax.  */
#परिभाषय MULTIBOOT_BOOTLOADER_MAGIC		0x2BADB002

/* Alignment of multiboot modules.  */
#परिभाषय MULTIBOOT_MOD_ALIGN			0x00001000

/* Alignment of the multiboot info structure.  */
#परिभाषय MULTIBOOT_INFO_ALIGN			0x00000004

/* Flags set in the 'flags' member of the multiboot header.  */

/* Align all boot modules on i386 page (4KB) boundaries.  */
#परिभाषय MULTIBOOT_PAGE_ALIGN			0x00000001

/* Must pass memory information to OS.  */
#परिभाषय MULTIBOOT_स्मृतिः_INFO			0x00000002

/* Must pass video information to OS.  */
#परिभाषय MULTIBOOT_VIDEO_बहुलकम्			0x00000004

/* This flag indicates the use of the address fields in the header.  */
#परिभाषय MULTIBOOT_AOUT_KLUDGE			0x00010000

/* Flags to be set in the 'flags' member of the multiboot info structure.  */

/* is there basic lower/upper memory information? */
#परिभाषय MULTIBOOT_INFO_स्मृतिः			0x00000001
/* is there a boot device set? */
#परिभाषय MULTIBOOT_INFO_BOOTDEV			0x00000002
/* is the command-line defined? */
#परिभाषय MULTIBOOT_INFO_CMDLINE			0x00000004
/* are there modules to do something with? */
#परिभाषय MULTIBOOT_INFO_MODS			0x00000008

/* These next two are mutually exclusive */

/* is there a symbol table loaded? */
#परिभाषय MULTIBOOT_INFO_AOUT_SYMS		0x00000010
/* is there an ELF section header table? */
#परिभाषय MULTIBOOT_INFO_ELF_SHDR			0X00000020

/* is there a full memory map? */
#परिभाषय MULTIBOOT_INFO_MEM_MAP			0x00000040

/* Is there drive info?  */
#परिभाषय MULTIBOOT_INFO_DRIVE_INFO		0x00000080

/* Is there a config table?  */
#परिभाषय MULTIBOOT_INFO_CONFIG_सारणी		0x00000100

/* Is there a boot loader name?  */
#परिभाषय MULTIBOOT_INFO_BOOT_LOADER_NAME		0x00000200

/* Is there a APM table?  */
#परिभाषय MULTIBOOT_INFO_APM_सारणी		0x00000400

/* Is there video information?  */
#परिभाषय MULTIBOOT_INFO_VBE_INFO		        0x00000800
#परिभाषय MULTIBOOT_INFO_FRAMEBUFFER_INFO	        0x00001000

#यदि_अपरिभाषितम् ASM_FILE

प्रकारनिर्देश अचिह्नित वर्ण		multiboot_uint8_t;
प्रकारनिर्देश अचिह्नित ह्रस्व		multiboot_uint16_t;
प्रकारनिर्देश अचिह्नित पूर्णाङ्क		multiboot_uint32_t;
प्रकारनिर्देश अचिह्नित दीर्घ दीर्घ	multiboot_uint64_t;

संरचना multiboot_शीर्षकम्
{
  /* Must be MULTIBOOT_MAGIC - see above.  */
  multiboot_uint32_t magic;

  /* Feature flags.  */
  multiboot_uint32_t flags;

  /* The above fields plus this one must equal 0 mod 2^32. */
  multiboot_uint32_t checksum;

  /* These are only valid if MULTIBOOT_AOUT_KLUDGE is set.  */
  multiboot_uint32_t शीर्षकम्_addr;
  multiboot_uint32_t आरोपय_addr;
  multiboot_uint32_t आरोपय_अन्तः_addr;
  multiboot_uint32_t bss_अन्तः_addr;
  multiboot_uint32_t प्रविष्टिः_addr;

  /* These are only valid if MULTIBOOT_VIDEO_MODE is set.  */
  multiboot_uint32_t बहुलकम्_type;
  multiboot_uint32_t विस्तारः;
  multiboot_uint32_t height;
  multiboot_uint32_t गभीरता;
};

/* The symbol table for a.out.  */
संरचना multiboot_aout_चिह्नम्_सारणी
{
  multiboot_uint32_t tabsize;
  multiboot_uint32_t strsize;
  multiboot_uint32_t addr;
  multiboot_uint32_t reserved;
};
प्रकारनिर्देश संरचना multiboot_aout_चिह्नम्_सारणी multiboot_aout_चिह्नम्_सारणी_t;

/* The section header table for ELF.  */
संरचना multiboot_elf_section_शीर्षकम्_सारणी
{
  multiboot_uint32_t अङ्कः;
  multiboot_uint32_t आकार;
  multiboot_uint32_t addr;
  multiboot_uint32_t shndx;
};
प्रकारनिर्देश संरचना multiboot_elf_section_शीर्षकम्_सारणी multiboot_elf_section_शीर्षकम्_सारणी_t;

संरचना multiboot_info
{
  /* Multiboot info version number */
  multiboot_uint32_t flags;

  /* Available memory from BIOS */
  multiboot_uint32_t mem_अधस्तनम्;
  multiboot_uint32_t mem_ऊर्ध्वस्थम्;

  /* "root" partition */
  multiboot_uint32_t boot_device;

  /* Kernel command line */
  multiboot_uint32_t cmdline;

  /* Boot-Module list */
  multiboot_uint32_t mods_गणनम्;
  multiboot_uint32_t mods_addr;

  सङ्घ
  {
    multiboot_aout_चिह्नम्_सारणी_t aout_sym;
    multiboot_elf_section_शीर्षकम्_सारणी_t elf_sec;
  } u;

  /* Memory Mapping buffer */
  multiboot_uint32_t mmap_दीर्घता;
  multiboot_uint32_t mmap_addr;

  /* Drive Info buffer */
  multiboot_uint32_t drives_दीर्घता;
  multiboot_uint32_t drives_addr;

  /* ROM configuration table */
  multiboot_uint32_t config_सारणी;

  /* Boot Loader Name */
  multiboot_uint32_t boot_loader_name;

  /* APM table */
  multiboot_uint32_t apm_सारणी;

  /* Video */
  multiboot_uint32_t vbe_control_info;
  multiboot_uint32_t vbe_बहुलकम्_info;
  multiboot_uint16_t vbe_बहुलकम्;
  multiboot_uint16_t vbe_interface_seg;
  multiboot_uint16_t vbe_interface_off;
  multiboot_uint16_t vbe_interface_len;

  multiboot_uint64_t framebuffer_addr;
  multiboot_uint32_t framebuffer_pitch;
  multiboot_uint32_t framebuffer_विस्तारः;
  multiboot_uint32_t framebuffer_height;
  multiboot_uint8_t framebuffer_bpp;
#परिभाषय MULTIBOOT_FRAMEBUFFER_TYPE_अनुक्रमितम् 0
#परिभाषय MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
#परिभाषय MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT	2
  multiboot_uint8_t framebuffer_type;
  सङ्घ
  {
    संरचना
    {
      multiboot_uint32_t framebuffer_palette_addr;
      multiboot_uint16_t framebuffer_palette_अङ्कः_colors;
    };
    संरचना
    {
      multiboot_uint8_t framebuffer_red_क्षेत्रम्_position;
      multiboot_uint8_t framebuffer_red_आवरणम्_आकार;
      multiboot_uint8_t framebuffer_green_क्षेत्रम्_position;
      multiboot_uint8_t framebuffer_green_आवरणम्_आकार;
      multiboot_uint8_t framebuffer_blue_क्षेत्रम्_position;
      multiboot_uint8_t framebuffer_blue_आवरणम्_आकार;
    };
  };
};
प्रकारनिर्देश संरचना multiboot_info multiboot_info_t;

संरचना multiboot_color
{
  multiboot_uint8_t red;
  multiboot_uint8_t green;
  multiboot_uint8_t blue;
};

संरचना multiboot_mmap_प्रविष्टिः
{
  multiboot_uint32_t आकार;
  multiboot_uint64_t addr;
  multiboot_uint64_t len;
#परिभाषय MULTIBOOT_स्मृतिः_उपलब्धम्		1
#परिभाषय MULTIBOOT_स्मृतिः_RESERVED		2
#परिभाषय MULTIBOOT_स्मृतिः_ACPI_RECLAIMABLE       3
#परिभाषय MULTIBOOT_स्मृतिः_NVS                    4
#परिभाषय MULTIBOOT_स्मृतिः_BADRAM                 5
  multiboot_uint32_t type;
} __attribute__((packed));
प्रकारनिर्देश संरचना multiboot_mmap_प्रविष्टिः multiboot_स्मृतिः_map_t;

संरचना multiboot_mod_list
{
  /* the memory used goes from bytes 'mod_start' to 'mod_end-1' inclusive */
  multiboot_uint32_t mod_start;
  multiboot_uint32_t mod_अन्तः;

  /* Module command line */
  multiboot_uint32_t cmdline;

  /* padding to take it to 16 bytes (must be zero) */
  multiboot_uint32_t pad;
};
प्रकारनिर्देश संरचना multiboot_mod_list multiboot_module_t;

#यदि_अन्तः /* ! ASM_FILE */

#यदि_अन्तः /* ! MULTIBOOT_HEADER */
