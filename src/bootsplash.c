// Initialize the VGA console and possibly show a boot splash image.
//
// Copyright (C) 2009-2010  coresystems GmbH
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "bregs.h" // struct bregs
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "farptr.h" // FLATPTR_TO_SEG
#അന്തര്ഭാവ "malloc.h" // free
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "romfile.h" // romfile_loadfile
#അന്തര്ഭാവ "stacks.h" // call16_int
#അന്തര്ഭാവ "std/vbe.h" // struct vbe_info
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // enable_bootsplash


/****************************************************************
 * Helper functions
 ****************************************************************/

// Call int10 vga handler.
സ്ഥിതം ശൂന്യം
call16_int10(ഘടന bregs *br)
{
    br->flags = F_IF;
    start_preempt();
    call16_int(0x10, br);
    finish_preempt();
}


/****************************************************************
 * VGA text / graphics console
 ****************************************************************/

ശൂന്യം
enable_vga_console(ശൂന്യം)
{
    dprintf(1, "Turning on vga text mode console\n");
    ഘടന bregs br;

    /* Enable VGA text mode */
    memset(&br, 0, വലുപ്പം(br));
    br.ax = 0x0003;
    call16_int10(&br);

    // Write to screen.
    printf("SeaBIOS (version %s)\n", VERSION);
    display_uuid();
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
find_videomode(ഘടന vbe_info *vesa_info, ഘടന vbe_mode_info *mode_info
               , പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ height, പൂർണ്ണസംഖ്യ bpp_req)
{
    dprintf(3, "Finding vesa mode with dimensions %d/%d\n", width, height);
    u16 *videomodes = SEGOFF_TO_FLATPTR(vesa_info->video_mode);
    വേണ്ടി (;; videomodes++) {
        u16 videomode = *videomodes;
        എങ്കിൽ (videomode == 0xffff) {
            dprintf(1, "Unable to find vesa video mode dimensions %d/%d\n"
                    , width, height);
            തിരികെ -1;
        }
        ഘടന bregs br;
        memset(&br, 0, വലുപ്പം(br));
        br.ax = 0x4f01;
        br.cx = videomode;
        br.di = FLATPTR_TO_OFFSET(mode_info);
        br.es = FLATPTR_TO_SEG(mode_info);
        call16_int10(&br);
        എങ്കിൽ (br.ax != 0x4f) {
            dprintf(1, "get_mode failed.\n");
            തുടരുക;
        }
        എങ്കിൽ (mode_info->xres != width
            || mode_info->yres != height)
            തുടരുക;
        u8 depth = mode_info->bits_per_pixel;
        എങ്കിൽ (bpp_req == 0) {
            എങ്കിൽ ((depth != 16 && depth != 24 && depth != 32)
                || mode_info->green_size == 5)
                തുടരുക;
        } അല്ലെങ്കിൽ {
            എങ്കിൽ (depth != bpp_req)
                തുടരുക;
        }
        തിരികെ videomode;
    }
}

സ്ഥിതം പൂർണ്ണസംഖ്യ BootsplashActive;

ശൂന്യം
enable_bootsplash(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_BOOTSPLASH)
        തിരികെ;
    /* splash picture can be bmp or jpeg file */
    dprintf(3, "Checking for bootsplash\n");
    u8 type = 0; /* 0 means jpg, 1 means bmp, default is 0=jpg */
    പൂർണ്ണസംഖ്യ filesize;
    u8 *filedata = romfile_loadfile("bootsplash.jpg", &filesize);
    എങ്കിൽ (!filedata) {
        filedata = romfile_loadfile("bootsplash.bmp", &filesize);
        എങ്കിൽ (!filedata)
            തിരികെ;
        type = 1;
    }
    dprintf(3, "start showing bootsplash\n");

    u8 *picture = ശൂന്യ; /* data buff used to be flushed to the video buf */
    ഘടന jpeg_decdata *jpeg = ശൂന്യ;
    ഘടന bmp_decdata *bmp = ശൂന്യ;
    ഘടന vbe_info *vesa_info = malloc_tmplow(വലുപ്പം(*vesa_info));
    ഘടന vbe_mode_info *mode_info = malloc_tmplow(വലുപ്പം(*mode_info));
    എങ്കിൽ (!vesa_info || !mode_info) {
        warn_noalloc();
        പോകുക done;
    }

    /* Check whether we have a VESA 2.0 compliant BIOS */
    memset(vesa_info, 0, വലുപ്പം(ഘടന vbe_info));
    vesa_info->signature = VBE2_SIGNATURE;
    ഘടന bregs br;
    memset(&br, 0, വലുപ്പം(br));
    br.ax = 0x4f00;
    br.di = FLATPTR_TO_OFFSET(vesa_info);
    br.es = FLATPTR_TO_SEG(vesa_info);
    call16_int10(&br);
    എങ്കിൽ (vesa_info->signature != VESA_SIGNATURE) {
        dprintf(1,"No VBE2 found.\n");
        പോകുക done;
    }

    /* Print some debugging information about our card. */
    അക്ഷരം *vendor = SEGOFF_TO_FLATPTR(vesa_info->oem_vendor_string);
    അക്ഷരം *product = SEGOFF_TO_FLATPTR(vesa_info->oem_product_string);
    dprintf(3, "VESA %d.%d\nVENDOR: %s\nPRODUCT: %s\n",
            vesa_info->version>>8, vesa_info->version&0xff,
            vendor, product);

    പൂർണ്ണസംഖ്യ ret, width, height;
    പൂർണ്ണസംഖ്യ bpp_require = 0;
    എങ്കിൽ (type == 0) {
        jpeg = jpeg_alloc();
        എങ്കിൽ (!jpeg) {
            warn_noalloc();
            പോകുക done;
        }
        /* Parse jpeg and get image size. */
        dprintf(5, "Decoding bootsplash.jpg\n");
        ret = jpeg_decode(jpeg, filedata);
        എങ്കിൽ (ret) {
            dprintf(1, "jpeg_decode failed with return code %d...\n", ret);
            പോകുക done;
        }
        jpeg_get_size(jpeg, &width, &height);
    } അല്ലെങ്കിൽ {
        bmp = bmp_alloc();
        എങ്കിൽ (!bmp) {
            warn_noalloc();
            പോകുക done;
        }
        /* Parse bmp and get image size. */
        dprintf(5, "Decoding bootsplash.bmp\n");
        ret = bmp_decode(bmp, filedata, filesize);
        എങ്കിൽ (ret) {
            dprintf(1, "bmp_decode failed with return code %d...\n", ret);
            പോകുക done;
        }
        bmp_get_info(bmp, &width, &height, &bpp_require);
    }

    // jpeg would use 16 or 24 bpp video mode, BMP uses 16/24/32 bpp mode.

    // Try to find a graphics mode with the corresponding dimensions.
    പൂർണ്ണസംഖ്യ videomode = find_videomode(vesa_info, mode_info, width, height,
                                       bpp_require);
    എങ്കിൽ (videomode < 0) {
        dprintf(1, "failed to find a videomode with %dx%d %dbpp (0=any).\n",
                    width, height, bpp_require);
        പോകുക done;
    }
    ശൂന്യം *framebuffer = (ശൂന്യം *)mode_info->phys_base;
    പൂർണ്ണസംഖ്യ depth = mode_info->bits_per_pixel;
    dprintf(3, "mode: %04x\n", videomode);
    dprintf(3, "framebuffer: %p\n", framebuffer);
    dprintf(3, "bytes per scanline: %d\n", mode_info->bytes_per_scanline);
    dprintf(3, "bits per pixel: %d\n", depth);

    // Allocate space for image and decompress it.
    പൂർണ്ണസംഖ്യ imagesize = height * mode_info->bytes_per_scanline;
    picture = malloc_tmphigh(imagesize);
    എങ്കിൽ (!picture) {
        warn_noalloc();
        പോകുക done;
    }

    എങ്കിൽ (type == 0) {
        dprintf(5, "Decompressing bootsplash.jpg\n");
        ret = jpeg_show(jpeg, picture, width, height, depth,
                            mode_info->bytes_per_scanline);
        എങ്കിൽ (ret) {
            dprintf(1, "jpeg_show failed with return code %d...\n", ret);
            പോകുക done;
        }
    } അല്ലെങ്കിൽ {
        dprintf(5, "Decompressing bootsplash.bmp\n");
        ret = bmp_show(bmp, picture, width, height, depth,
                           mode_info->bytes_per_scanline);
        എങ്കിൽ (ret) {
            dprintf(1, "bmp_show failed with return code %d...\n", ret);
            പോകുക done;
        }
    }

    /* Switch to graphics mode */
    dprintf(5, "Switching to graphics mode\n");
    memset(&br, 0, വലുപ്പം(br));
    br.ax = 0x4f02;
    br.bx = videomode | VBE_MODE_LINEAR_FRAME_BUFFER;
    call16_int10(&br);
    എങ്കിൽ (br.ax != 0x4f) {
        dprintf(1, "set_mode failed.\n");
        പോകുക done;
    }

    /* Show the picture */
    dprintf(5, "Showing bootsplash picture\n");
    iomemcpy(framebuffer, picture, imagesize);
    dprintf(5, "Bootsplash copy complete\n");
    BootsplashActive = 1;

done:
    സ്വതന്ത്രം(filedata);
    സ്വതന്ത്രം(picture);
    സ്വതന്ത്രം(vesa_info);
    സ്വതന്ത്രം(mode_info);
    സ്വതന്ത്രം(jpeg);
    സ്വതന്ത്രം(bmp);
    തിരികെ;
}

ശൂന്യം
disable_bootsplash(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_BOOTSPLASH || !BootsplashActive)
        തിരികെ;
    BootsplashActive = 0;
    enable_vga_console();
}
