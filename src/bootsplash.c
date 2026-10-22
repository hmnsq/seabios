// Initialize the VGA console and possibly show a boot splash image.
//
// Copyright (C) 2009-2010  coresystems GmbH
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "bregs.h" // struct bregs
#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "farptr.h" // FLATPTR_TO_SEG
#அந்தர்பாவ "malloc.h" // free
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "romfile.h" // romfile_loadfile
#அந்தர்பாவ "stacks.h" // call16_int
#அந்தர்பாவ "std/vbe.h" // struct vbe_info
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // enable_bootsplash


/****************************************************************
 * Helper functions
 ****************************************************************/

// Call int10 vga handler.
நிலையான வெறுமை
call16_int10(கட்டமைப்பு bregs *br)
{
    br->flags = F_IF;
    start_preempt();
    call16_int(0x10, br);
    finish_preempt();
}


/****************************************************************
 * VGA text / graphics console
 ****************************************************************/

வெறுமை
enable_vga_console(வெறுமை)
{
    dprintf(1, "Turning on vga text mode console\n");
    கட்டமைப்பு bregs br;

    /* Enable VGA text mode */
    memset(&br, 0, அளவு(br));
    br.ax = 0x0003;
    call16_int10(&br);

    // Write to screen.
    printf("SeaBIOS (version %s)\n", VERSION);
    display_uuid();
}

நிலையான முழு
find_videomode(கட்டமைப்பு vbe_info *vesa_info, கட்டமைப்பு vbe_mode_info *mode_info
               , முழு width, முழு height, முழு bpp_req)
{
    dprintf(3, "Finding vesa mode with dimensions %d/%d\n", width, height);
    u16 *videomodes = SEGOFF_TO_FLATPTR(vesa_info->video_mode);
    க்கு (;; videomodes++) {
        u16 videomode = *videomodes;
        எனில் (videomode == 0xffff) {
            dprintf(1, "Unable to find vesa video mode dimensions %d/%d\n"
                    , width, height);
            திரும்பு -1;
        }
        கட்டமைப்பு bregs br;
        memset(&br, 0, அளவு(br));
        br.ax = 0x4f01;
        br.cx = videomode;
        br.di = FLATPTR_TO_OFFSET(mode_info);
        br.es = FLATPTR_TO_SEG(mode_info);
        call16_int10(&br);
        எனில் (br.ax != 0x4f) {
            dprintf(1, "get_mode failed.\n");
            தொடர்;
        }
        எனில் (mode_info->xres != width
            || mode_info->yres != height)
            தொடர்;
        u8 depth = mode_info->bits_per_pixel;
        எனில் (bpp_req == 0) {
            எனில் ((depth != 16 && depth != 24 && depth != 32)
                || mode_info->green_size == 5)
                தொடர்;
        } இல்லையெனில் {
            எனில் (depth != bpp_req)
                தொடர்;
        }
        திரும்பு videomode;
    }
}

நிலையான முழு BootsplashActive;

வெறுமை
enable_bootsplash(வெறுமை)
{
    எனில் (!CONFIG_BOOTSPLASH)
        திரும்பு;
    /* splash picture can be bmp or jpeg file */
    dprintf(3, "Checking for bootsplash\n");
    u8 type = 0; /* 0 means jpg, 1 means bmp, default is 0=jpg */
    முழு filesize;
    u8 *filedata = romfile_loadfile("bootsplash.jpg", &filesize);
    எனில் (!filedata) {
        filedata = romfile_loadfile("bootsplash.bmp", &filesize);
        எனில் (!filedata)
            திரும்பு;
        type = 1;
    }
    dprintf(3, "start showing bootsplash\n");

    u8 *picture = வெற்று; /* data buff used to be flushed to the video buf */
    கட்டமைப்பு jpeg_decdata *jpeg = வெற்று;
    கட்டமைப்பு bmp_decdata *bmp = வெற்று;
    கட்டமைப்பு vbe_info *vesa_info = malloc_tmplow(அளவு(*vesa_info));
    கட்டமைப்பு vbe_mode_info *mode_info = malloc_tmplow(அளவு(*mode_info));
    எனில் (!vesa_info || !mode_info) {
        warn_noalloc();
        செல் done;
    }

    /* Check whether we have a VESA 2.0 compliant BIOS */
    memset(vesa_info, 0, அளவு(கட்டமைப்பு vbe_info));
    vesa_info->signature = VBE2_SIGNATURE;
    கட்டமைப்பு bregs br;
    memset(&br, 0, அளவு(br));
    br.ax = 0x4f00;
    br.di = FLATPTR_TO_OFFSET(vesa_info);
    br.es = FLATPTR_TO_SEG(vesa_info);
    call16_int10(&br);
    எனில் (vesa_info->signature != VESA_SIGNATURE) {
        dprintf(1,"No VBE2 found.\n");
        செல் done;
    }

    /* Print some debugging information about our card. */
    எழுத்து *vendor = SEGOFF_TO_FLATPTR(vesa_info->oem_vendor_string);
    எழுத்து *product = SEGOFF_TO_FLATPTR(vesa_info->oem_product_string);
    dprintf(3, "VESA %d.%d\nVENDOR: %s\nPRODUCT: %s\n",
            vesa_info->version>>8, vesa_info->version&0xff,
            vendor, product);

    முழு ret, width, height;
    முழு bpp_require = 0;
    எனில் (type == 0) {
        jpeg = jpeg_alloc();
        எனில் (!jpeg) {
            warn_noalloc();
            செல் done;
        }
        /* Parse jpeg and get image size. */
        dprintf(5, "Decoding bootsplash.jpg\n");
        ret = jpeg_decode(jpeg, filedata);
        எனில் (ret) {
            dprintf(1, "jpeg_decode failed with return code %d...\n", ret);
            செல் done;
        }
        jpeg_get_size(jpeg, &width, &height);
    } இல்லையெனில் {
        bmp = bmp_alloc();
        எனில் (!bmp) {
            warn_noalloc();
            செல் done;
        }
        /* Parse bmp and get image size. */
        dprintf(5, "Decoding bootsplash.bmp\n");
        ret = bmp_decode(bmp, filedata, filesize);
        எனில் (ret) {
            dprintf(1, "bmp_decode failed with return code %d...\n", ret);
            செல் done;
        }
        bmp_get_info(bmp, &width, &height, &bpp_require);
    }

    // jpeg would use 16 or 24 bpp video mode, BMP uses 16/24/32 bpp mode.

    // Try to find a graphics mode with the corresponding dimensions.
    முழு videomode = find_videomode(vesa_info, mode_info, width, height,
                                       bpp_require);
    எனில் (videomode < 0) {
        dprintf(1, "failed to find a videomode with %dx%d %dbpp (0=any).\n",
                    width, height, bpp_require);
        செல் done;
    }
    வெறுமை *framebuffer = (வெறுமை *)mode_info->phys_base;
    முழு depth = mode_info->bits_per_pixel;
    dprintf(3, "mode: %04x\n", videomode);
    dprintf(3, "framebuffer: %p\n", framebuffer);
    dprintf(3, "bytes per scanline: %d\n", mode_info->bytes_per_scanline);
    dprintf(3, "bits per pixel: %d\n", depth);

    // Allocate space for image and decompress it.
    முழு imagesize = height * mode_info->bytes_per_scanline;
    picture = malloc_tmphigh(imagesize);
    எனில் (!picture) {
        warn_noalloc();
        செல் done;
    }

    எனில் (type == 0) {
        dprintf(5, "Decompressing bootsplash.jpg\n");
        ret = jpeg_show(jpeg, picture, width, height, depth,
                            mode_info->bytes_per_scanline);
        எனில் (ret) {
            dprintf(1, "jpeg_show failed with return code %d...\n", ret);
            செல் done;
        }
    } இல்லையெனில் {
        dprintf(5, "Decompressing bootsplash.bmp\n");
        ret = bmp_show(bmp, picture, width, height, depth,
                           mode_info->bytes_per_scanline);
        எனில் (ret) {
            dprintf(1, "bmp_show failed with return code %d...\n", ret);
            செல் done;
        }
    }

    /* Switch to graphics mode */
    dprintf(5, "Switching to graphics mode\n");
    memset(&br, 0, அளவு(br));
    br.ax = 0x4f02;
    br.bx = videomode | VBE_MODE_LINEAR_FRAME_BUFFER;
    call16_int10(&br);
    எனில் (br.ax != 0x4f) {
        dprintf(1, "set_mode failed.\n");
        செல் done;
    }

    /* Show the picture */
    dprintf(5, "Showing bootsplash picture\n");
    iomemcpy(framebuffer, picture, imagesize);
    dprintf(5, "Bootsplash copy complete\n");
    BootsplashActive = 1;

done:
    விடுவி(filedata);
    விடுவி(picture);
    விடுவி(vesa_info);
    விடுவி(mode_info);
    விடுவி(jpeg);
    விடுவி(bmp);
    திரும்பு;
}

வெறுமை
disable_bootsplash(வெறுமை)
{
    எனில் (!CONFIG_BOOTSPLASH || !BootsplashActive)
        திரும்பு;
    BootsplashActive = 0;
    enable_vga_console();
}
