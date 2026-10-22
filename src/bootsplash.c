// Initialize the VGA console and possibly show a boot splash image.
//
// Copyright (C) 2009-2010  coresystems GmbH
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "bregs.h" // struct bregs
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "farptr.h" // FLATPTR_TO_SEG
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_loadfile
#ಅನ್ತರ್ಭಾವ "stacks.h" // call16_int
#ಅನ್ತರ್ಭಾವ "std/vbe.h" // struct vbe_info
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // enable_bootsplash


/****************************************************************
 * Helper functions
 ****************************************************************/

// Call int10 vga handler.
ಸ್ಥಿರ ಶೂನ್ಯ
call16_int10(ಸಂರಚನಾ bregs *br)
{
    br->flags = F_IF;
    start_preempt();
    call16_int(0x10, br);
    finish_preempt();
}


/****************************************************************
 * VGA text / graphics console
 ****************************************************************/

ಶೂನ್ಯ
enable_vga_console(ಶೂನ್ಯ)
{
    dprintf(1, "Turning on vga text mode console\n");
    ಸಂರಚನಾ bregs br;

    /* Enable VGA text mode */
    memset(&br, 0, ಪರಿಮಾಣ(br));
    br.ax = 0x0003;
    call16_int10(&br);

    // Write to screen.
    printf("SeaBIOS (version %s)\n", VERSION);
    display_uuid();
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
find_videomode(ಸಂರಚನಾ vbe_info *vesa_info, ಸಂರಚನಾ vbe_mode_info *mode_info
               , ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ height, ಪೂರ್ಣಾಙ್ಕ bpp_req)
{
    dprintf(3, "Finding vesa mode with dimensions %d/%d\n", width, height);
    u16 *videomodes = SEGOFF_TO_FLATPTR(vesa_info->video_mode);
    ಪ್ರತಿ (;; videomodes++) {
        u16 videomode = *videomodes;
        ಯದಿ (videomode == 0xffff) {
            dprintf(1, "Unable to find vesa video mode dimensions %d/%d\n"
                    , width, height);
            ಪ್ರತ್ಯಾಗಮ -1;
        }
        ಸಂರಚನಾ bregs br;
        memset(&br, 0, ಪರಿಮಾಣ(br));
        br.ax = 0x4f01;
        br.cx = videomode;
        br.di = FLATPTR_TO_OFFSET(mode_info);
        br.es = FLATPTR_TO_SEG(mode_info);
        call16_int10(&br);
        ಯದಿ (br.ax != 0x4f) {
            dprintf(1, "get_mode failed.\n");
            ಅನುವರ್ತ;
        }
        ಯದಿ (mode_info->xres != width
            || mode_info->yres != height)
            ಅನುವರ್ತ;
        u8 depth = mode_info->bits_per_pixel;
        ಯದಿ (bpp_req == 0) {
            ಯದಿ ((depth != 16 && depth != 24 && depth != 32)
                || mode_info->green_size == 5)
                ಅನುವರ್ತ;
        } ಅನ್ಯಥಾ {
            ಯದಿ (depth != bpp_req)
                ಅನುವರ್ತ;
        }
        ಪ್ರತ್ಯಾಗಮ videomode;
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ BootsplashActive;

ಶೂನ್ಯ
enable_bootsplash(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_BOOTSPLASH)
        ಪ್ರತ್ಯಾಗಮ;
    /* splash picture can be bmp or jpeg file */
    dprintf(3, "Checking for bootsplash\n");
    u8 type = 0; /* 0 means jpg, 1 means bmp, default is 0=jpg */
    ಪೂರ್ಣಾಙ್ಕ filesize;
    u8 *filedata = romfile_loadfile("bootsplash.jpg", &filesize);
    ಯದಿ (!filedata) {
        filedata = romfile_loadfile("bootsplash.bmp", &filesize);
        ಯದಿ (!filedata)
            ಪ್ರತ್ಯಾಗಮ;
        type = 1;
    }
    dprintf(3, "start showing bootsplash\n");

    u8 *picture = ರಿಕ್ತ; /* data buff used to be flushed to the video buf */
    ಸಂರಚನಾ jpeg_decdata *jpeg = ರಿಕ್ತ;
    ಸಂರಚನಾ bmp_decdata *bmp = ರಿಕ್ತ;
    ಸಂರಚನಾ vbe_info *vesa_info = malloc_tmplow(ಪರಿಮಾಣ(*vesa_info));
    ಸಂರಚನಾ vbe_mode_info *mode_info = malloc_tmplow(ಪರಿಮಾಣ(*mode_info));
    ಯದಿ (!vesa_info || !mode_info) {
        warn_noalloc();
        ಗಚ್ಛ done;
    }

    /* Check whether we have a VESA 2.0 compliant BIOS */
    memset(vesa_info, 0, ಪರಿಮಾಣ(ಸಂರಚನಾ vbe_info));
    vesa_info->signature = VBE2_SIGNATURE;
    ಸಂರಚನಾ bregs br;
    memset(&br, 0, ಪರಿಮಾಣ(br));
    br.ax = 0x4f00;
    br.di = FLATPTR_TO_OFFSET(vesa_info);
    br.es = FLATPTR_TO_SEG(vesa_info);
    call16_int10(&br);
    ಯದಿ (vesa_info->signature != VESA_SIGNATURE) {
        dprintf(1,"No VBE2 found.\n");
        ಗಚ್ಛ done;
    }

    /* Print some debugging information about our card. */
    ವರ್ಣ *vendor = SEGOFF_TO_FLATPTR(vesa_info->oem_vendor_string);
    ವರ್ಣ *product = SEGOFF_TO_FLATPTR(vesa_info->oem_product_string);
    dprintf(3, "VESA %d.%d\nVENDOR: %s\nPRODUCT: %s\n",
            vesa_info->version>>8, vesa_info->version&0xff,
            vendor, product);

    ಪೂರ್ಣಾಙ್ಕ ret, width, height;
    ಪೂರ್ಣಾಙ್ಕ bpp_require = 0;
    ಯದಿ (type == 0) {
        jpeg = jpeg_alloc();
        ಯದಿ (!jpeg) {
            warn_noalloc();
            ಗಚ್ಛ done;
        }
        /* Parse jpeg and get image size. */
        dprintf(5, "Decoding bootsplash.jpg\n");
        ret = jpeg_decode(jpeg, filedata);
        ಯದಿ (ret) {
            dprintf(1, "jpeg_decode failed with return code %d...\n", ret);
            ಗಚ್ಛ done;
        }
        jpeg_get_size(jpeg, &width, &height);
    } ಅನ್ಯಥಾ {
        bmp = bmp_alloc();
        ಯದಿ (!bmp) {
            warn_noalloc();
            ಗಚ್ಛ done;
        }
        /* Parse bmp and get image size. */
        dprintf(5, "Decoding bootsplash.bmp\n");
        ret = bmp_decode(bmp, filedata, filesize);
        ಯದಿ (ret) {
            dprintf(1, "bmp_decode failed with return code %d...\n", ret);
            ಗಚ್ಛ done;
        }
        bmp_get_info(bmp, &width, &height, &bpp_require);
    }

    // jpeg would use 16 or 24 bpp video mode, BMP uses 16/24/32 bpp mode.

    // Try to find a graphics mode with the corresponding dimensions.
    ಪೂರ್ಣಾಙ್ಕ videomode = find_videomode(vesa_info, mode_info, width, height,
                                       bpp_require);
    ಯದಿ (videomode < 0) {
        dprintf(1, "failed to find a videomode with %dx%d %dbpp (0=any).\n",
                    width, height, bpp_require);
        ಗಚ್ಛ done;
    }
    ಶೂನ್ಯ *framebuffer = (ಶೂನ್ಯ *)mode_info->phys_base;
    ಪೂರ್ಣಾಙ್ಕ depth = mode_info->bits_per_pixel;
    dprintf(3, "mode: %04x\n", videomode);
    dprintf(3, "framebuffer: %p\n", framebuffer);
    dprintf(3, "bytes per scanline: %d\n", mode_info->bytes_per_scanline);
    dprintf(3, "bits per pixel: %d\n", depth);

    // Allocate space for image and decompress it.
    ಪೂರ್ಣಾಙ್ಕ imagesize = height * mode_info->bytes_per_scanline;
    picture = malloc_tmphigh(imagesize);
    ಯದಿ (!picture) {
        warn_noalloc();
        ಗಚ್ಛ done;
    }

    ಯದಿ (type == 0) {
        dprintf(5, "Decompressing bootsplash.jpg\n");
        ret = jpeg_show(jpeg, picture, width, height, depth,
                            mode_info->bytes_per_scanline);
        ಯದಿ (ret) {
            dprintf(1, "jpeg_show failed with return code %d...\n", ret);
            ಗಚ್ಛ done;
        }
    } ಅನ್ಯಥಾ {
        dprintf(5, "Decompressing bootsplash.bmp\n");
        ret = bmp_show(bmp, picture, width, height, depth,
                           mode_info->bytes_per_scanline);
        ಯದಿ (ret) {
            dprintf(1, "bmp_show failed with return code %d...\n", ret);
            ಗಚ್ಛ done;
        }
    }

    /* Switch to graphics mode */
    dprintf(5, "Switching to graphics mode\n");
    memset(&br, 0, ಪರಿಮಾಣ(br));
    br.ax = 0x4f02;
    br.bx = videomode | VBE_MODE_LINEAR_FRAME_BUFFER;
    call16_int10(&br);
    ಯದಿ (br.ax != 0x4f) {
        dprintf(1, "set_mode failed.\n");
        ಗಚ್ಛ done;
    }

    /* Show the picture */
    dprintf(5, "Showing bootsplash picture\n");
    iomemcpy(framebuffer, picture, imagesize);
    dprintf(5, "Bootsplash copy complete\n");
    BootsplashActive = 1;

done:
    ಮುಕ್ತ(filedata);
    ಮುಕ್ತ(picture);
    ಮುಕ್ತ(vesa_info);
    ಮುಕ್ತ(mode_info);
    ಮುಕ್ತ(jpeg);
    ಮುಕ್ತ(bmp);
    ಪ್ರತ್ಯಾಗಮ;
}

ಶೂನ್ಯ
disable_bootsplash(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_BOOTSPLASH || !BootsplashActive)
        ಪ್ರತ್ಯಾಗಮ;
    BootsplashActive = 0;
    enable_vga_console();
}
