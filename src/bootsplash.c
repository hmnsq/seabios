// Initialize the VGA console and possibly show a boot splash image.
//
// Copyright (C) 2009-2010  coresystems GmbH
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "bregs.h" // struct bregs
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "farptr.h" // FLATPTR_TO_SEG
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "romfile.h" // romfile_loadfile
#অন্তর্ভাব "stacks.h" // call16_int
#অন্তর্ভাব "std/vbe.h" // struct vbe_info
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // enable_bootsplash


/****************************************************************
 * Helper functions
 ****************************************************************/

// Call int10 vga handler.
স্থির শূন্য
call16_int10(সংরচনা bregs *br)
{
    br->flags = F_IF;
    start_preempt();
    call16_int(0x10, br);
    finish_preempt();
}


/****************************************************************
 * VGA text / graphics console
 ****************************************************************/

শূন্য
enable_vga_console(শূন্য)
{
    dprintf(1, "Turning on vga text mode console\n");
    সংরচনা bregs br;

    /* Enable VGA text mode */
    memset(&br, 0, পরিমাণ(br));
    br.ax = 0x0003;
    call16_int10(&br);

    // Write to screen.
    printf("SeaBIOS (version %s)\n", VERSION);
    display_uuid();
}

স্থির পূর্ণাঙ্ক
find_videomode(সংরচনা vbe_info *vesa_info, সংরচনা vbe_mode_info *mode_info
               , পূর্ণাঙ্ক width, পূর্ণাঙ্ক height, পূর্ণাঙ্ক bpp_req)
{
    dprintf(3, "Finding vesa mode with dimensions %d/%d\n", width, height);
    u16 *videomodes = SEGOFF_TO_FLATPTR(vesa_info->video_mode);
    প্রতি (;; videomodes++) {
        u16 videomode = *videomodes;
        যদি (videomode == 0xffff) {
            dprintf(1, "Unable to find vesa video mode dimensions %d/%d\n"
                    , width, height);
            প্রত্যাগম -1;
        }
        সংরচনা bregs br;
        memset(&br, 0, পরিমাণ(br));
        br.ax = 0x4f01;
        br.cx = videomode;
        br.di = FLATPTR_TO_OFFSET(mode_info);
        br.es = FLATPTR_TO_SEG(mode_info);
        call16_int10(&br);
        যদি (br.ax != 0x4f) {
            dprintf(1, "get_mode failed.\n");
            অনুবর্ত;
        }
        যদি (mode_info->xres != width
            || mode_info->yres != height)
            অনুবর্ত;
        u8 depth = mode_info->bits_per_pixel;
        যদি (bpp_req == 0) {
            যদি ((depth != 16 && depth != 24 && depth != 32)
                || mode_info->green_size == 5)
                অনুবর্ত;
        } অন্যথা {
            যদি (depth != bpp_req)
                অনুবর্ত;
        }
        প্রত্যাগম videomode;
    }
}

স্থির পূর্ণাঙ্ক BootsplashActive;

শূন্য
enable_bootsplash(শূন্য)
{
    যদি (!CONFIG_BOOTSPLASH)
        প্রত্যাগম;
    /* splash picture can be bmp or jpeg file */
    dprintf(3, "Checking for bootsplash\n");
    u8 type = 0; /* 0 means jpg, 1 means bmp, default is 0=jpg */
    পূর্ণাঙ্ক filesize;
    u8 *filedata = romfile_loadfile("bootsplash.jpg", &filesize);
    যদি (!filedata) {
        filedata = romfile_loadfile("bootsplash.bmp", &filesize);
        যদি (!filedata)
            প্রত্যাগম;
        type = 1;
    }
    dprintf(3, "start showing bootsplash\n");

    u8 *picture = রিক্ত; /* data buff used to be flushed to the video buf */
    সংরচনা jpeg_decdata *jpeg = রিক্ত;
    সংরচনা bmp_decdata *bmp = রিক্ত;
    সংরচনা vbe_info *vesa_info = malloc_tmplow(পরিমাণ(*vesa_info));
    সংরচনা vbe_mode_info *mode_info = malloc_tmplow(পরিমাণ(*mode_info));
    যদি (!vesa_info || !mode_info) {
        warn_noalloc();
        গচ্ছ done;
    }

    /* Check whether we have a VESA 2.0 compliant BIOS */
    memset(vesa_info, 0, পরিমাণ(সংরচনা vbe_info));
    vesa_info->signature = VBE2_SIGNATURE;
    সংরচনা bregs br;
    memset(&br, 0, পরিমাণ(br));
    br.ax = 0x4f00;
    br.di = FLATPTR_TO_OFFSET(vesa_info);
    br.es = FLATPTR_TO_SEG(vesa_info);
    call16_int10(&br);
    যদি (vesa_info->signature != VESA_SIGNATURE) {
        dprintf(1,"No VBE2 found.\n");
        গচ্ছ done;
    }

    /* Print some debugging information about our card. */
    বর্ণ *vendor = SEGOFF_TO_FLATPTR(vesa_info->oem_vendor_string);
    বর্ণ *product = SEGOFF_TO_FLATPTR(vesa_info->oem_product_string);
    dprintf(3, "VESA %d.%d\nVENDOR: %s\nPRODUCT: %s\n",
            vesa_info->version>>8, vesa_info->version&0xff,
            vendor, product);

    পূর্ণাঙ্ক ret, width, height;
    পূর্ণাঙ্ক bpp_require = 0;
    যদি (type == 0) {
        jpeg = jpeg_alloc();
        যদি (!jpeg) {
            warn_noalloc();
            গচ্ছ done;
        }
        /* Parse jpeg and get image size. */
        dprintf(5, "Decoding bootsplash.jpg\n");
        ret = jpeg_decode(jpeg, filedata);
        যদি (ret) {
            dprintf(1, "jpeg_decode failed with return code %d...\n", ret);
            গচ্ছ done;
        }
        jpeg_get_size(jpeg, &width, &height);
    } অন্যথা {
        bmp = bmp_alloc();
        যদি (!bmp) {
            warn_noalloc();
            গচ্ছ done;
        }
        /* Parse bmp and get image size. */
        dprintf(5, "Decoding bootsplash.bmp\n");
        ret = bmp_decode(bmp, filedata, filesize);
        যদি (ret) {
            dprintf(1, "bmp_decode failed with return code %d...\n", ret);
            গচ্ছ done;
        }
        bmp_get_info(bmp, &width, &height, &bpp_require);
    }

    // jpeg would use 16 or 24 bpp video mode, BMP uses 16/24/32 bpp mode.

    // Try to find a graphics mode with the corresponding dimensions.
    পূর্ণাঙ্ক videomode = find_videomode(vesa_info, mode_info, width, height,
                                       bpp_require);
    যদি (videomode < 0) {
        dprintf(1, "failed to find a videomode with %dx%d %dbpp (0=any).\n",
                    width, height, bpp_require);
        গচ্ছ done;
    }
    শূন্য *framebuffer = (শূন্য *)mode_info->phys_base;
    পূর্ণাঙ্ক depth = mode_info->bits_per_pixel;
    dprintf(3, "mode: %04x\n", videomode);
    dprintf(3, "framebuffer: %p\n", framebuffer);
    dprintf(3, "bytes per scanline: %d\n", mode_info->bytes_per_scanline);
    dprintf(3, "bits per pixel: %d\n", depth);

    // Allocate space for image and decompress it.
    পূর্ণাঙ্ক imagesize = height * mode_info->bytes_per_scanline;
    picture = malloc_tmphigh(imagesize);
    যদি (!picture) {
        warn_noalloc();
        গচ্ছ done;
    }

    যদি (type == 0) {
        dprintf(5, "Decompressing bootsplash.jpg\n");
        ret = jpeg_show(jpeg, picture, width, height, depth,
                            mode_info->bytes_per_scanline);
        যদি (ret) {
            dprintf(1, "jpeg_show failed with return code %d...\n", ret);
            গচ্ছ done;
        }
    } অন্যথা {
        dprintf(5, "Decompressing bootsplash.bmp\n");
        ret = bmp_show(bmp, picture, width, height, depth,
                           mode_info->bytes_per_scanline);
        যদি (ret) {
            dprintf(1, "bmp_show failed with return code %d...\n", ret);
            গচ্ছ done;
        }
    }

    /* Switch to graphics mode */
    dprintf(5, "Switching to graphics mode\n");
    memset(&br, 0, পরিমাণ(br));
    br.ax = 0x4f02;
    br.bx = videomode | VBE_MODE_LINEAR_FRAME_BUFFER;
    call16_int10(&br);
    যদি (br.ax != 0x4f) {
        dprintf(1, "set_mode failed.\n");
        গচ্ছ done;
    }

    /* Show the picture */
    dprintf(5, "Showing bootsplash picture\n");
    iomemcpy(framebuffer, picture, imagesize);
    dprintf(5, "Bootsplash copy complete\n");
    BootsplashActive = 1;

done:
    মুক্ত(filedata);
    মুক্ত(picture);
    মুক্ত(vesa_info);
    মুক্ত(mode_info);
    মুক্ত(jpeg);
    মুক্ত(bmp);
    প্রত্যাগম;
}

শূন্য
disable_bootsplash(শূন্য)
{
    যদি (!CONFIG_BOOTSPLASH || !BootsplashActive)
        প্রত্যাগম;
    BootsplashActive = 0;
    enable_vga_console();
}
