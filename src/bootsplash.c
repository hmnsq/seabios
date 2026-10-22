// Initialize the VGA console and possibly show a boot splash image.
//
// Copyright (C) 2009-2010  coresystems GmbH
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "bregs.h" // struct bregs
#ضمن "config.h" // CONFIG_*
#ضمن "farptr.h" // FLATPTR_TO_SEG
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "romfile.h" // romfile_loadfile
#ضمن "stacks.h" // call16_int
#ضمن "std/vbe.h" // struct vbe_info
#ضمن "string.h" // memset
#ضمن "util.h" // enable_bootsplash


/****************************************************************
 * Helper functions
 ****************************************************************/

// Call int10 vga handler.
ساكن فراغ
call16_int10(بنية bregs *br)
{
    br->flags = F_IF;
    start_preempt();
    call16_int(0x10, br);
    finish_preempt();
}


/****************************************************************
 * VGA text / graphics console
 ****************************************************************/

فراغ
enable_vga_console(فراغ)
{
    dprintf(1, "Turning on vga text mode console\n");
    بنية bregs br;

    /* Enable VGA text mode */
    memset(&br, 0, حجم(br));
    br.ax = 0x0003;
    call16_int10(&br);

    // Write to screen.
    printf("SeaBIOS (version %s)\n", VERSION);
    display_uuid();
}

ساكن صحيح
find_videomode(بنية vbe_info *vesa_info, بنية vbe_mode_info *mode_info
               , صحيح width, صحيح height, صحيح bpp_req)
{
    dprintf(3, "Finding vesa mode with dimensions %d/%d\n", width, height);
    u16 *videomodes = SEGOFF_TO_FLATPTR(vesa_info->video_mode);
    لكل (;; videomodes++) {
        u16 videomode = *videomodes;
        إذا (videomode == 0xffff) {
            dprintf(1, "Unable to find vesa video mode dimensions %d/%d\n"
                    , width, height);
            إرجاع -1;
        }
        بنية bregs br;
        memset(&br, 0, حجم(br));
        br.ax = 0x4f01;
        br.cx = videomode;
        br.di = FLATPTR_TO_OFFSET(mode_info);
        br.es = FLATPTR_TO_SEG(mode_info);
        call16_int10(&br);
        إذا (br.ax != 0x4f) {
            dprintf(1, "get_mode failed.\n");
            استمر;
        }
        إذا (mode_info->xres != width
            || mode_info->yres != height)
            استمر;
        u8 depth = mode_info->bits_per_pixel;
        إذا (bpp_req == 0) {
            إذا ((depth != 16 && depth != 24 && depth != 32)
                || mode_info->green_size == 5)
                استمر;
        } وإلا {
            إذا (depth != bpp_req)
                استمر;
        }
        إرجاع videomode;
    }
}

ساكن صحيح BootsplashActive;

فراغ
enable_bootsplash(فراغ)
{
    إذا (!CONFIG_BOOTSPLASH)
        إرجاع;
    /* splash picture can be bmp or jpeg file */
    dprintf(3, "Checking for bootsplash\n");
    u8 type = 0; /* 0 means jpg, 1 means bmp, default is 0=jpg */
    صحيح filesize;
    u8 *filedata = romfile_loadfile("bootsplash.jpg", &filesize);
    إذا (!filedata) {
        filedata = romfile_loadfile("bootsplash.bmp", &filesize);
        إذا (!filedata)
            إرجاع;
        type = 1;
    }
    dprintf(3, "start showing bootsplash\n");

    u8 *picture = عدم; /* data buff used to be flushed to the video buf */
    بنية jpeg_decdata *jpeg = عدم;
    بنية bmp_decdata *bmp = عدم;
    بنية vbe_info *vesa_info = malloc_tmplow(حجم(*vesa_info));
    بنية vbe_mode_info *mode_info = malloc_tmplow(حجم(*mode_info));
    إذا (!vesa_info || !mode_info) {
        warn_noalloc();
        اذهب done;
    }

    /* Check whether we have a VESA 2.0 compliant BIOS */
    memset(vesa_info, 0, حجم(بنية vbe_info));
    vesa_info->signature = VBE2_SIGNATURE;
    بنية bregs br;
    memset(&br, 0, حجم(br));
    br.ax = 0x4f00;
    br.di = FLATPTR_TO_OFFSET(vesa_info);
    br.es = FLATPTR_TO_SEG(vesa_info);
    call16_int10(&br);
    إذا (vesa_info->signature != VESA_SIGNATURE) {
        dprintf(1,"No VBE2 found.\n");
        اذهب done;
    }

    /* Print some debugging information about our card. */
    حرف *vendor = SEGOFF_TO_FLATPTR(vesa_info->oem_vendor_string);
    حرف *product = SEGOFF_TO_FLATPTR(vesa_info->oem_product_string);
    dprintf(3, "VESA %d.%d\nVENDOR: %s\nPRODUCT: %s\n",
            vesa_info->version>>8, vesa_info->version&0xff,
            vendor, product);

    صحيح ret, width, height;
    صحيح bpp_require = 0;
    إذا (type == 0) {
        jpeg = jpeg_alloc();
        إذا (!jpeg) {
            warn_noalloc();
            اذهب done;
        }
        /* Parse jpeg and get image size. */
        dprintf(5, "Decoding bootsplash.jpg\n");
        ret = jpeg_decode(jpeg, filedata);
        إذا (ret) {
            dprintf(1, "jpeg_decode failed with return code %d...\n", ret);
            اذهب done;
        }
        jpeg_get_size(jpeg, &width, &height);
    } وإلا {
        bmp = bmp_alloc();
        إذا (!bmp) {
            warn_noalloc();
            اذهب done;
        }
        /* Parse bmp and get image size. */
        dprintf(5, "Decoding bootsplash.bmp\n");
        ret = bmp_decode(bmp, filedata, filesize);
        إذا (ret) {
            dprintf(1, "bmp_decode failed with return code %d...\n", ret);
            اذهب done;
        }
        bmp_get_info(bmp, &width, &height, &bpp_require);
    }

    // jpeg would use 16 or 24 bpp video mode, BMP uses 16/24/32 bpp mode.

    // Try to find a graphics mode with the corresponding dimensions.
    صحيح videomode = find_videomode(vesa_info, mode_info, width, height,
                                       bpp_require);
    إذا (videomode < 0) {
        dprintf(1, "failed to find a videomode with %dx%d %dbpp (0=any).\n",
                    width, height, bpp_require);
        اذهب done;
    }
    فراغ *framebuffer = (فراغ *)mode_info->phys_base;
    صحيح depth = mode_info->bits_per_pixel;
    dprintf(3, "mode: %04x\n", videomode);
    dprintf(3, "framebuffer: %p\n", framebuffer);
    dprintf(3, "bytes per scanline: %d\n", mode_info->bytes_per_scanline);
    dprintf(3, "bits per pixel: %d\n", depth);

    // Allocate space for image and decompress it.
    صحيح imagesize = height * mode_info->bytes_per_scanline;
    picture = malloc_tmphigh(imagesize);
    إذا (!picture) {
        warn_noalloc();
        اذهب done;
    }

    إذا (type == 0) {
        dprintf(5, "Decompressing bootsplash.jpg\n");
        ret = jpeg_show(jpeg, picture, width, height, depth,
                            mode_info->bytes_per_scanline);
        إذا (ret) {
            dprintf(1, "jpeg_show failed with return code %d...\n", ret);
            اذهب done;
        }
    } وإلا {
        dprintf(5, "Decompressing bootsplash.bmp\n");
        ret = bmp_show(bmp, picture, width, height, depth,
                           mode_info->bytes_per_scanline);
        إذا (ret) {
            dprintf(1, "bmp_show failed with return code %d...\n", ret);
            اذهب done;
        }
    }

    /* Switch to graphics mode */
    dprintf(5, "Switching to graphics mode\n");
    memset(&br, 0, حجم(br));
    br.ax = 0x4f02;
    br.bx = videomode | VBE_MODE_LINEAR_FRAME_BUFFER;
    call16_int10(&br);
    إذا (br.ax != 0x4f) {
        dprintf(1, "set_mode failed.\n");
        اذهب done;
    }

    /* Show the picture */
    dprintf(5, "Showing bootsplash picture\n");
    iomemcpy(framebuffer, picture, imagesize);
    dprintf(5, "Bootsplash copy complete\n");
    BootsplashActive = 1;

done:
    حرر(filedata);
    حرر(picture);
    حرر(vesa_info);
    حرر(mode_info);
    حرر(jpeg);
    حرر(bmp);
    إرجاع;
}

فراغ
disable_bootsplash(فراغ)
{
    إذا (!CONFIG_BOOTSPLASH || !BootsplashActive)
        إرجاع;
    BootsplashActive = 0;
    enable_vga_console();
}
