// Initialize the VGA console and possibly show a boot splash image.
//
// Copyright (C) 2009-2010  coresystems GmbH
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "bregs.h" // struct bregs
#شامل "config.h" // CONFIG_*
#شامل "farptr.h" // FLATPTR_TO_SEG
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "romfile.h" // romfile_loadfile
#شامل "stacks.h" // call16_int
#شامل "std/vbe.h" // struct vbe_info
#شامل "string.h" // memset
#شامل "util.h" // enable_bootsplash


/****************************************************************
 * Helper functions
 ****************************************************************/

// Call int10 vga handler.
ایستا تهی
call16_int10(ساختار bregs *br)
{
    br->flags = F_IF;
    start_preempt();
    call16_int(0x10, br);
    finish_preempt();
}


/****************************************************************
 * VGA text / graphics console
 ****************************************************************/

تهی
enable_vga_console(تهی)
{
    dprintf(1, "Turning on vga text mode console\n");
    ساختار bregs br;

    /* Enable VGA text mode */
    memset(&br, 0, اندازه(br));
    br.ax = 0x0003;
    call16_int10(&br);

    // Write to screen.
    printf("SeaBIOS (version %s)\n", VERSION);
    display_uuid();
}

ایستا صحیح
find_videomode(ساختار vbe_info *vesa_info, ساختار vbe_mode_info *mode_info
               , صحیح width, صحیح height, صحیح bpp_req)
{
    dprintf(3, "Finding vesa mode with dimensions %d/%d\n", width, height);
    u16 *videomodes = SEGOFF_TO_FLATPTR(vesa_info->video_mode);
    برای (;; videomodes++) {
        u16 videomode = *videomodes;
        اگر (videomode == 0xffff) {
            dprintf(1, "Unable to find vesa video mode dimensions %d/%d\n"
                    , width, height);
            بازگشت -1;
        }
        ساختار bregs br;
        memset(&br, 0, اندازه(br));
        br.ax = 0x4f01;
        br.cx = videomode;
        br.di = FLATPTR_TO_OFFSET(mode_info);
        br.es = FLATPTR_TO_SEG(mode_info);
        call16_int10(&br);
        اگر (br.ax != 0x4f) {
            dprintf(1, "get_mode failed.\n");
            ادامه;
        }
        اگر (mode_info->xres != width
            || mode_info->yres != height)
            ادامه;
        u8 depth = mode_info->bits_per_pixel;
        اگر (bpp_req == 0) {
            اگر ((depth != 16 && depth != 24 && depth != 32)
                || mode_info->green_size == 5)
                ادامه;
        } وگرنه {
            اگر (depth != bpp_req)
                ادامه;
        }
        بازگشت videomode;
    }
}

ایستا صحیح BootsplashActive;

تهی
enable_bootsplash(تهی)
{
    اگر (!CONFIG_BOOTSPLASH)
        بازگشت;
    /* splash picture can be bmp or jpeg file */
    dprintf(3, "Checking for bootsplash\n");
    u8 type = 0; /* 0 means jpg, 1 means bmp, default is 0=jpg */
    صحیح filesize;
    u8 *filedata = romfile_loadfile("bootsplash.jpg", &filesize);
    اگر (!filedata) {
        filedata = romfile_loadfile("bootsplash.bmp", &filesize);
        اگر (!filedata)
            بازگشت;
        type = 1;
    }
    dprintf(3, "start showing bootsplash\n");

    u8 *picture = پوچ; /* data buff used to be flushed to the video buf */
    ساختار jpeg_decdata *jpeg = پوچ;
    ساختار bmp_decdata *bmp = پوچ;
    ساختار vbe_info *vesa_info = malloc_tmplow(اندازه(*vesa_info));
    ساختار vbe_mode_info *mode_info = malloc_tmplow(اندازه(*mode_info));
    اگر (!vesa_info || !mode_info) {
        warn_noalloc();
        برو done;
    }

    /* Check whether we have a VESA 2.0 compliant BIOS */
    memset(vesa_info, 0, اندازه(ساختار vbe_info));
    vesa_info->signature = VBE2_SIGNATURE;
    ساختار bregs br;
    memset(&br, 0, اندازه(br));
    br.ax = 0x4f00;
    br.di = FLATPTR_TO_OFFSET(vesa_info);
    br.es = FLATPTR_TO_SEG(vesa_info);
    call16_int10(&br);
    اگر (vesa_info->signature != VESA_SIGNATURE) {
        dprintf(1,"No VBE2 found.\n");
        برو done;
    }

    /* Print some debugging information about our card. */
    نویسه *vendor = SEGOFF_TO_FLATPTR(vesa_info->oem_vendor_string);
    نویسه *product = SEGOFF_TO_FLATPTR(vesa_info->oem_product_string);
    dprintf(3, "VESA %d.%d\nVENDOR: %s\nPRODUCT: %s\n",
            vesa_info->version>>8, vesa_info->version&0xff,
            vendor, product);

    صحیح ret, width, height;
    صحیح bpp_require = 0;
    اگر (type == 0) {
        jpeg = jpeg_alloc();
        اگر (!jpeg) {
            warn_noalloc();
            برو done;
        }
        /* Parse jpeg and get image size. */
        dprintf(5, "Decoding bootsplash.jpg\n");
        ret = jpeg_decode(jpeg, filedata);
        اگر (ret) {
            dprintf(1, "jpeg_decode failed with return code %d...\n", ret);
            برو done;
        }
        jpeg_get_size(jpeg, &width, &height);
    } وگرنه {
        bmp = bmp_alloc();
        اگر (!bmp) {
            warn_noalloc();
            برو done;
        }
        /* Parse bmp and get image size. */
        dprintf(5, "Decoding bootsplash.bmp\n");
        ret = bmp_decode(bmp, filedata, filesize);
        اگر (ret) {
            dprintf(1, "bmp_decode failed with return code %d...\n", ret);
            برو done;
        }
        bmp_get_info(bmp, &width, &height, &bpp_require);
    }

    // jpeg would use 16 or 24 bpp video mode, BMP uses 16/24/32 bpp mode.

    // Try to find a graphics mode with the corresponding dimensions.
    صحیح videomode = find_videomode(vesa_info, mode_info, width, height,
                                       bpp_require);
    اگر (videomode < 0) {
        dprintf(1, "failed to find a videomode with %dx%d %dbpp (0=any).\n",
                    width, height, bpp_require);
        برو done;
    }
    تهی *framebuffer = (تهی *)mode_info->phys_base;
    صحیح depth = mode_info->bits_per_pixel;
    dprintf(3, "mode: %04x\n", videomode);
    dprintf(3, "framebuffer: %p\n", framebuffer);
    dprintf(3, "bytes per scanline: %d\n", mode_info->bytes_per_scanline);
    dprintf(3, "bits per pixel: %d\n", depth);

    // Allocate space for image and decompress it.
    صحیح imagesize = height * mode_info->bytes_per_scanline;
    picture = malloc_tmphigh(imagesize);
    اگر (!picture) {
        warn_noalloc();
        برو done;
    }

    اگر (type == 0) {
        dprintf(5, "Decompressing bootsplash.jpg\n");
        ret = jpeg_show(jpeg, picture, width, height, depth,
                            mode_info->bytes_per_scanline);
        اگر (ret) {
            dprintf(1, "jpeg_show failed with return code %d...\n", ret);
            برو done;
        }
    } وگرنه {
        dprintf(5, "Decompressing bootsplash.bmp\n");
        ret = bmp_show(bmp, picture, width, height, depth,
                           mode_info->bytes_per_scanline);
        اگر (ret) {
            dprintf(1, "bmp_show failed with return code %d...\n", ret);
            برو done;
        }
    }

    /* Switch to graphics mode */
    dprintf(5, "Switching to graphics mode\n");
    memset(&br, 0, اندازه(br));
    br.ax = 0x4f02;
    br.bx = videomode | VBE_MODE_LINEAR_FRAME_BUFFER;
    call16_int10(&br);
    اگر (br.ax != 0x4f) {
        dprintf(1, "set_mode failed.\n");
        برو done;
    }

    /* Show the picture */
    dprintf(5, "Showing bootsplash picture\n");
    iomemcpy(framebuffer, picture, imagesize);
    dprintf(5, "Bootsplash copy complete\n");
    BootsplashActive = 1;

done:
    آزاد(filedata);
    آزاد(picture);
    آزاد(vesa_info);
    آزاد(mode_info);
    آزاد(jpeg);
    آزاد(bmp);
    بازگشت;
}

تهی
disable_bootsplash(تهی)
{
    اگر (!CONFIG_BOOTSPLASH || !BootsplashActive)
        بازگشت;
    BootsplashActive = 0;
    enable_vga_console();
}
