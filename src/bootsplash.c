// Initialize the VGA console and possibly show a boot splash image.
//
// Copyright (C) 2009-2010  coresystems GmbH
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "bregs.h" // struct bregs
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "farptr.h" // FLATPTR_TO_SEG
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "romfile.h" // romfile_loadfile
#ଅନ୍ତର୍ଭାଵ "stacks.h" // call16_int
#ଅନ୍ତର୍ଭାଵ "std/vbe.h" // struct vbe_info
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // enable_bootsplash


/****************************************************************
 * Helper functions
 ****************************************************************/

// Call int10 vga handler.
ସ୍ଥିର ଶୂନ୍ୟ
call16_int10(ସଂରଚନା bregs *br)
{
    br->flags = F_IF;
    start_preempt();
    call16_int(0x10, br);
    finish_preempt();
}


/****************************************************************
 * VGA text / graphics console
 ****************************************************************/

ଶୂନ୍ୟ
enable_vga_console(ଶୂନ୍ୟ)
{
    dprintf(1, "Turning on vga text mode console\n");
    ସଂରଚନା bregs br;

    /* Enable VGA text mode */
    memset(&br, 0, ଆକାର(br));
    br.ax = 0x0003;
    call16_int10(&br);

    // Write to screen.
    printf("SeaBIOS (version %s)\n", VERSION);
    display_uuid();
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
find_videomode(ସଂରଚନା vbe_info *vesa_info, ସଂରଚନା vbe_mode_info *mode_info
               , ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height, ପୂର୍ଣ୍ଣସଂଖ୍ୟା bpp_req)
{
    dprintf(3, "Finding vesa mode with dimensions %d/%d\n", width, height);
    u16 *videomodes = SEGOFF_TO_FLATPTR(vesa_info->video_mode);
    ପାଇଁ (;; videomodes++) {
        u16 videomode = *videomodes;
        ଯଦି (videomode == 0xffff) {
            dprintf(1, "Unable to find vesa video mode dimensions %d/%d\n"
                    , width, height);
            ଫେରନ୍ତୁ -1;
        }
        ସଂରଚନା bregs br;
        memset(&br, 0, ଆକାର(br));
        br.ax = 0x4f01;
        br.cx = videomode;
        br.di = FLATPTR_TO_OFFSET(mode_info);
        br.es = FLATPTR_TO_SEG(mode_info);
        call16_int10(&br);
        ଯଦି (br.ax != 0x4f) {
            dprintf(1, "get_mode failed.\n");
            ଜାରି;
        }
        ଯଦି (mode_info->xres != width
            || mode_info->yres != height)
            ଜାରି;
        u8 depth = mode_info->bits_per_pixel;
        ଯଦି (bpp_req == 0) {
            ଯଦି ((depth != 16 && depth != 24 && depth != 32)
                || mode_info->green_size == 5)
                ଜାରି;
        } ଅନ୍ୟଥା {
            ଯଦି (depth != bpp_req)
                ଜାରି;
        }
        ଫେରନ୍ତୁ videomode;
    }
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା BootsplashActive;

ଶୂନ୍ୟ
enable_bootsplash(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_BOOTSPLASH)
        ଫେରନ୍ତୁ;
    /* splash picture can be bmp or jpeg file */
    dprintf(3, "Checking for bootsplash\n");
    u8 type = 0; /* 0 means jpg, 1 means bmp, default is 0=jpg */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା filesize;
    u8 *filedata = romfile_loadfile("bootsplash.jpg", &filesize);
    ଯଦି (!filedata) {
        filedata = romfile_loadfile("bootsplash.bmp", &filesize);
        ଯଦି (!filedata)
            ଫେରନ୍ତୁ;
        type = 1;
    }
    dprintf(3, "start showing bootsplash\n");

    u8 *picture = ଖାଲି; /* data buff used to be flushed to the video buf */
    ସଂରଚନା jpeg_decdata *jpeg = ଖାଲି;
    ସଂରଚନା bmp_decdata *bmp = ଖାଲି;
    ସଂରଚନା vbe_info *vesa_info = malloc_tmplow(ଆକାର(*vesa_info));
    ସଂରଚନା vbe_mode_info *mode_info = malloc_tmplow(ଆକାର(*mode_info));
    ଯଦି (!vesa_info || !mode_info) {
        warn_noalloc();
        ଯାଅ done;
    }

    /* Check whether we have a VESA 2.0 compliant BIOS */
    memset(vesa_info, 0, ଆକାର(ସଂରଚନା vbe_info));
    vesa_info->signature = VBE2_SIGNATURE;
    ସଂରଚନା bregs br;
    memset(&br, 0, ଆକାର(br));
    br.ax = 0x4f00;
    br.di = FLATPTR_TO_OFFSET(vesa_info);
    br.es = FLATPTR_TO_SEG(vesa_info);
    call16_int10(&br);
    ଯଦି (vesa_info->signature != VESA_SIGNATURE) {
        dprintf(1,"No VBE2 found.\n");
        ଯାଅ done;
    }

    /* Print some debugging information about our card. */
    ଅକ୍ଷର *vendor = SEGOFF_TO_FLATPTR(vesa_info->oem_vendor_string);
    ଅକ୍ଷର *product = SEGOFF_TO_FLATPTR(vesa_info->oem_product_string);
    dprintf(3, "VESA %d.%d\nVENDOR: %s\nPRODUCT: %s\n",
            vesa_info->version>>8, vesa_info->version&0xff,
            vendor, product);

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret, width, height;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା bpp_require = 0;
    ଯଦି (type == 0) {
        jpeg = jpeg_alloc();
        ଯଦି (!jpeg) {
            warn_noalloc();
            ଯାଅ done;
        }
        /* Parse jpeg and get image size. */
        dprintf(5, "Decoding bootsplash.jpg\n");
        ret = jpeg_decode(jpeg, filedata);
        ଯଦି (ret) {
            dprintf(1, "jpeg_decode failed with return code %d...\n", ret);
            ଯାଅ done;
        }
        jpeg_get_size(jpeg, &width, &height);
    } ଅନ୍ୟଥା {
        bmp = bmp_alloc();
        ଯଦି (!bmp) {
            warn_noalloc();
            ଯାଅ done;
        }
        /* Parse bmp and get image size. */
        dprintf(5, "Decoding bootsplash.bmp\n");
        ret = bmp_decode(bmp, filedata, filesize);
        ଯଦି (ret) {
            dprintf(1, "bmp_decode failed with return code %d...\n", ret);
            ଯାଅ done;
        }
        bmp_get_info(bmp, &width, &height, &bpp_require);
    }

    // jpeg would use 16 or 24 bpp video mode, BMP uses 16/24/32 bpp mode.

    // Try to find a graphics mode with the corresponding dimensions.
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା videomode = find_videomode(vesa_info, mode_info, width, height,
                                       bpp_require);
    ଯଦି (videomode < 0) {
        dprintf(1, "failed to find a videomode with %dx%d %dbpp (0=any).\n",
                    width, height, bpp_require);
        ଯାଅ done;
    }
    ଶୂନ୍ୟ *framebuffer = (ଶୂନ୍ୟ *)mode_info->phys_base;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା depth = mode_info->bits_per_pixel;
    dprintf(3, "mode: %04x\n", videomode);
    dprintf(3, "framebuffer: %p\n", framebuffer);
    dprintf(3, "bytes per scanline: %d\n", mode_info->bytes_per_scanline);
    dprintf(3, "bits per pixel: %d\n", depth);

    // Allocate space for image and decompress it.
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା imagesize = height * mode_info->bytes_per_scanline;
    picture = malloc_tmphigh(imagesize);
    ଯଦି (!picture) {
        warn_noalloc();
        ଯାଅ done;
    }

    ଯଦି (type == 0) {
        dprintf(5, "Decompressing bootsplash.jpg\n");
        ret = jpeg_show(jpeg, picture, width, height, depth,
                            mode_info->bytes_per_scanline);
        ଯଦି (ret) {
            dprintf(1, "jpeg_show failed with return code %d...\n", ret);
            ଯାଅ done;
        }
    } ଅନ୍ୟଥା {
        dprintf(5, "Decompressing bootsplash.bmp\n");
        ret = bmp_show(bmp, picture, width, height, depth,
                           mode_info->bytes_per_scanline);
        ଯଦି (ret) {
            dprintf(1, "bmp_show failed with return code %d...\n", ret);
            ଯାଅ done;
        }
    }

    /* Switch to graphics mode */
    dprintf(5, "Switching to graphics mode\n");
    memset(&br, 0, ଆକାର(br));
    br.ax = 0x4f02;
    br.bx = videomode | VBE_MODE_LINEAR_FRAME_BUFFER;
    call16_int10(&br);
    ଯଦି (br.ax != 0x4f) {
        dprintf(1, "set_mode failed.\n");
        ଯାଅ done;
    }

    /* Show the picture */
    dprintf(5, "Showing bootsplash picture\n");
    iomemcpy(framebuffer, picture, imagesize);
    dprintf(5, "Bootsplash copy complete\n");
    BootsplashActive = 1;

done:
    ମୁକ୍ତ(filedata);
    ମୁକ୍ତ(picture);
    ମୁକ୍ତ(vesa_info);
    ମୁକ୍ତ(mode_info);
    ମୁକ୍ତ(jpeg);
    ମୁକ୍ତ(bmp);
    ଫେରନ୍ତୁ;
}

ଶୂନ୍ୟ
disable_bootsplash(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_BOOTSPLASH || !BootsplashActive)
        ଫେରନ୍ତୁ;
    BootsplashActive = 0;
    enable_vga_console();
}
