// Initialize the VGA console and possibly show a boot splash image.
//
// Copyright (C) 2009-2010  coresystems GmbH
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "bregs.h" // struct bregs
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "farptr.h" // FLATPTR_TO_SEG
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "romfile.h" // romfile_loadfile
#अन्तर्भाव "stacks.h" // call16_int
#अन्तर्भाव "std/vbe.h" // struct vbe_info
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // enable_bootsplash


/****************************************************************
 * Helper functions
 ****************************************************************/

// Call int10 vga handler.
स्थिर शून्य
call16_int10(संरचना bregs *br)
{
    br->flags = F_IF;
    start_preempt();
    call16_int(0x10, br);
    finish_preempt();
}


/****************************************************************
 * VGA text / graphics console
 ****************************************************************/

शून्य
enable_vga_console(शून्य)
{
    dprintf(1, "Turning on vga text mode console\n");
    संरचना bregs br;

    /* Enable VGA text mode */
    memset(&br, 0, परिमाण(br));
    br.ax = 0x0003;
    call16_int10(&br);

    // Write to screen.
    printf("SeaBIOS (version %s)\n", VERSION);
    display_uuid();
}

स्थिर पूर्णाङ्क
find_videomode(संरचना vbe_info *vesa_info, संरचना vbe_mode_info *mode_info
               , पूर्णाङ्क width, पूर्णाङ्क height, पूर्णाङ्क bpp_req)
{
    dprintf(3, "Finding vesa mode with dimensions %d/%d\n", width, height);
    u16 *videomodes = SEGOFF_TO_FLATPTR(vesa_info->video_mode);
    प्रति (;; videomodes++) {
        u16 videomode = *videomodes;
        जइ (videomode == 0xffff) {
            dprintf(1, "Unable to find vesa video mode dimensions %d/%d\n"
                    , width, height);
            प्रत्यागम -1;
        }
        संरचना bregs br;
        memset(&br, 0, परिमाण(br));
        br.ax = 0x4f01;
        br.cx = videomode;
        br.di = FLATPTR_TO_OFFSET(mode_info);
        br.es = FLATPTR_TO_SEG(mode_info);
        call16_int10(&br);
        जइ (br.ax != 0x4f) {
            dprintf(1, "get_mode failed.\n");
            अनुवर्त;
        }
        जइ (mode_info->xres != width
            || mode_info->yres != height)
            अनुवर्त;
        u8 depth = mode_info->bits_per_pixel;
        जइ (bpp_req == 0) {
            जइ ((depth != 16 && depth != 24 && depth != 32)
                || mode_info->green_size == 5)
                अनुवर्त;
        } अण्णहा {
            जइ (depth != bpp_req)
                अनुवर्त;
        }
        प्रत्यागम videomode;
    }
}

स्थिर पूर्णाङ्क BootsplashActive;

शून्य
enable_bootsplash(शून्य)
{
    जइ (!CONFIG_BOOTSPLASH)
        प्रत्यागम;
    /* splash picture can be bmp or jpeg file */
    dprintf(3, "Checking for bootsplash\n");
    u8 type = 0; /* 0 means jpg, 1 means bmp, default is 0=jpg */
    पूर्णाङ्क filesize;
    u8 *filedata = romfile_loadfile("bootsplash.jpg", &filesize);
    जइ (!filedata) {
        filedata = romfile_loadfile("bootsplash.bmp", &filesize);
        जइ (!filedata)
            प्रत्यागम;
        type = 1;
    }
    dprintf(3, "start showing bootsplash\n");

    u8 *picture = रिक्त; /* data buff used to be flushed to the video buf */
    संरचना jpeg_decdata *jpeg = रिक्त;
    संरचना bmp_decdata *bmp = रिक्त;
    संरचना vbe_info *vesa_info = malloc_tmplow(परिमाण(*vesa_info));
    संरचना vbe_mode_info *mode_info = malloc_tmplow(परिमाण(*mode_info));
    जइ (!vesa_info || !mode_info) {
        warn_noalloc();
        गच्छ done;
    }

    /* Check whether we have a VESA 2.0 compliant BIOS */
    memset(vesa_info, 0, परिमाण(संरचना vbe_info));
    vesa_info->signature = VBE2_SIGNATURE;
    संरचना bregs br;
    memset(&br, 0, परिमाण(br));
    br.ax = 0x4f00;
    br.di = FLATPTR_TO_OFFSET(vesa_info);
    br.es = FLATPTR_TO_SEG(vesa_info);
    call16_int10(&br);
    जइ (vesa_info->signature != VESA_SIGNATURE) {
        dprintf(1,"No VBE2 found.\n");
        गच्छ done;
    }

    /* Print some debugging information about our card. */
    वर्ण *vendor = SEGOFF_TO_FLATPTR(vesa_info->oem_vendor_string);
    वर्ण *product = SEGOFF_TO_FLATPTR(vesa_info->oem_product_string);
    dprintf(3, "VESA %d.%d\nVENDOR: %s\nPRODUCT: %s\n",
            vesa_info->version>>8, vesa_info->version&0xff,
            vendor, product);

    पूर्णाङ्क ret, width, height;
    पूर्णाङ्क bpp_require = 0;
    जइ (type == 0) {
        jpeg = jpeg_alloc();
        जइ (!jpeg) {
            warn_noalloc();
            गच्छ done;
        }
        /* Parse jpeg and get image size. */
        dprintf(5, "Decoding bootsplash.jpg\n");
        ret = jpeg_decode(jpeg, filedata);
        जइ (ret) {
            dprintf(1, "jpeg_decode failed with return code %d...\n", ret);
            गच्छ done;
        }
        jpeg_get_size(jpeg, &width, &height);
    } अण्णहा {
        bmp = bmp_alloc();
        जइ (!bmp) {
            warn_noalloc();
            गच्छ done;
        }
        /* Parse bmp and get image size. */
        dprintf(5, "Decoding bootsplash.bmp\n");
        ret = bmp_decode(bmp, filedata, filesize);
        जइ (ret) {
            dprintf(1, "bmp_decode failed with return code %d...\n", ret);
            गच्छ done;
        }
        bmp_get_info(bmp, &width, &height, &bpp_require);
    }

    // jpeg would use 16 or 24 bpp video mode, BMP uses 16/24/32 bpp mode.

    // Try to find a graphics mode with the corresponding dimensions.
    पूर्णाङ्क videomode = find_videomode(vesa_info, mode_info, width, height,
                                       bpp_require);
    जइ (videomode < 0) {
        dprintf(1, "failed to find a videomode with %dx%d %dbpp (0=any).\n",
                    width, height, bpp_require);
        गच्छ done;
    }
    शून्य *framebuffer = (शून्य *)mode_info->phys_base;
    पूर्णाङ्क depth = mode_info->bits_per_pixel;
    dprintf(3, "mode: %04x\n", videomode);
    dprintf(3, "framebuffer: %p\n", framebuffer);
    dprintf(3, "bytes per scanline: %d\n", mode_info->bytes_per_scanline);
    dprintf(3, "bits per pixel: %d\n", depth);

    // Allocate space for image and decompress it.
    पूर्णाङ्क imagesize = height * mode_info->bytes_per_scanline;
    picture = malloc_tmphigh(imagesize);
    जइ (!picture) {
        warn_noalloc();
        गच्छ done;
    }

    जइ (type == 0) {
        dprintf(5, "Decompressing bootsplash.jpg\n");
        ret = jpeg_show(jpeg, picture, width, height, depth,
                            mode_info->bytes_per_scanline);
        जइ (ret) {
            dprintf(1, "jpeg_show failed with return code %d...\n", ret);
            गच्छ done;
        }
    } अण्णहा {
        dprintf(5, "Decompressing bootsplash.bmp\n");
        ret = bmp_show(bmp, picture, width, height, depth,
                           mode_info->bytes_per_scanline);
        जइ (ret) {
            dprintf(1, "bmp_show failed with return code %d...\n", ret);
            गच्छ done;
        }
    }

    /* Switch to graphics mode */
    dprintf(5, "Switching to graphics mode\n");
    memset(&br, 0, परिमाण(br));
    br.ax = 0x4f02;
    br.bx = videomode | VBE_MODE_LINEAR_FRAME_BUFFER;
    call16_int10(&br);
    जइ (br.ax != 0x4f) {
        dprintf(1, "set_mode failed.\n");
        गच्छ done;
    }

    /* Show the picture */
    dprintf(5, "Showing bootsplash picture\n");
    iomemcpy(framebuffer, picture, imagesize);
    dprintf(5, "Bootsplash copy complete\n");
    BootsplashActive = 1;

done:
    मुक्त(filedata);
    मुक्त(picture);
    मुक्त(vesa_info);
    मुक्त(mode_info);
    मुक्त(jpeg);
    मुक्त(bmp);
    प्रत्यागम;
}

शून्य
disable_bootsplash(शून्य)
{
    जइ (!CONFIG_BOOTSPLASH || !BootsplashActive)
        प्रत्यागम;
    BootsplashActive = 0;
    enable_vga_console();
}
