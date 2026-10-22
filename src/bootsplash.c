// Initialize the VGA console and possibly show a boot splash image.
//
// Copyright (C) 2009-2010  coresystems GmbH
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "bregs.h" // struct bregs
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "farptr.h" // FLATPTR_TO_SEG
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "romfile.h" // romfile_loadfile
#అన్తర్భావ "stacks.h" // call16_int
#అన్తర్భావ "std/vbe.h" // struct vbe_info
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // enable_bootsplash


/****************************************************************
 * Helper functions
 ****************************************************************/

// Call int10 vga handler.
స్థిర శూన్య
call16_int10(నిర్మాణం bregs *br)
{
    br->flags = F_IF;
    ప్రారంభం_preempt();
    call16_int(0x10, br);
    ముగింపు_preempt();
}


/****************************************************************
 * VGA text / graphics console
 ****************************************************************/

శూన్య
enable_vga_console(శూన్య)
{
    dprintf(1, "Turning on vga text mode console\n");
    నిర్మాణం bregs br;

    /* Enable VGA text mode */
    memset(&br, 0, పరిమాణం(br));
    br.ax = 0x0003;
    call16_int10(&br);

    // Write to screen.
    రాయి("SeaBIOS (version %s)\n", సంస్కరణ);
    display_uuid();
}

స్థిర పూర్ణాంకం
కనుగొను_videomode(నిర్మాణం vbe_info *vesa_info, నిర్మాణం vbe_బహుళకం_info *బహుళకం_info
               , పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు, పూర్ణాంకం bpp_req)
{
    dprintf(3, "Finding vesa mode with dimensions %d/%d\n", వెడల్పు, ఎత్తు);
    u16 *videomodes = SEGOFF_గమ్యం_FLATPTR(vesa_info->video_బహుళకం);
    కొరకు (;; videomodes++) {
        u16 videomode = *videomodes;
        ఒకవేళ (videomode == 0xffff) {
            dprintf(1, "Unable to find vesa video mode dimensions %d/%d\n"
                    , వెడల్పు, ఎత్తు);
            తిరిగి -1;
        }
        నిర్మాణం bregs br;
        memset(&br, 0, పరిమాణం(br));
        br.ax = 0x4f01;
        br.cx = videomode;
        br.di = FLATPTR_గమ్యం_స్థానభ్రంశం(బహుళకం_info);
        br.es = FLATPTR_గమ్యం_SEG(బహుళకం_info);
        call16_int10(&br);
        ఒకవేళ (br.ax != 0x4f) {
            dprintf(1, "get_mode failed.\n");
            కొనసాగు;
        }
        ఒకవేళ (బహుళకం_info->xres != వెడల్పు
            || బహుళకం_info->yres != ఎత్తు)
            కొనసాగు;
        u8 లోతు = బహుళకం_info->బిట్లు_చొప్పున_pixel;
        ఒకవేళ (bpp_req == 0) {
            ఒకవేళ ((లోతు != 16 && లోతు != 24 && లోతు != 32)
                || బహుళకం_info->green_కొలత == 5)
                కొనసాగు;
        } లేకపోతే {
            ఒకవేళ (లోతు != bpp_req)
                కొనసాగు;
        }
        తిరిగి videomode;
    }
}

స్థిర పూర్ణాంకం Bootsplash_చురుకైన;

శూన్య
enable_bootsplash(శూన్య)
{
    ఒకవేళ (!CONFIG_BOOTSPLASH)
        తిరిగి;
    /* splash picture can be bmp or jpeg file */
    dprintf(3, "Checking for bootsplash\n");
    u8 type = 0; /* 0 means jpg, 1 means bmp, default is 0=jpg */
    పూర్ణాంకం filesize;
    u8 *filedata = romfile_loadfile("bootsplash.jpg", &filesize);
    ఒకవేళ (!filedata) {
        filedata = romfile_loadfile("bootsplash.bmp", &filesize);
        ఒకవేళ (!filedata)
            తిరిగి;
        type = 1;
    }
    dprintf(3, "start showing bootsplash\n");

    u8 *picture = ఖాళీ; /* data buff used to be flushed to the video buf */
    నిర్మాణం jpeg_decdata *jpeg = ఖాళీ;
    నిర్మాణం bmp_decdata *bmp = ఖాళీ;
    నిర్మాణం vbe_info *vesa_info = malloc_tmplow(పరిమాణం(*vesa_info));
    నిర్మాణం vbe_బహుళకం_info *బహుళకం_info = malloc_tmplow(పరిమాణం(*బహుళకం_info));
    ఒకవేళ (!vesa_info || !బహుళకం_info) {
        warn_noalloc();
        వెళ్ళు పూర్తి;
    }

    /* Check whether we have a VESA 2.0 compliant BIOS */
    memset(vesa_info, 0, పరిమాణం(నిర్మాణం vbe_info));
    vesa_info->signature = VBE2_SIGNATURE;
    నిర్మాణం bregs br;
    memset(&br, 0, పరిమాణం(br));
    br.ax = 0x4f00;
    br.di = FLATPTR_గమ్యం_స్థానభ్రంశం(vesa_info);
    br.es = FLATPTR_గమ్యం_SEG(vesa_info);
    call16_int10(&br);
    ఒకవేళ (vesa_info->signature != VESA_SIGNATURE) {
        dprintf(1,"No VBE2 found.\n");
        వెళ్ళు పూర్తి;
    }

    /* Print some debugging information about our card. */
    అక్షరం *vendor = SEGOFF_గమ్యం_FLATPTR(vesa_info->oem_vendor_string);
    అక్షరం *లబ్ధం = SEGOFF_గమ్యం_FLATPTR(vesa_info->oem_లబ్ధం_string);
    dprintf(3, "VESA %d.%d\nVENDOR: %s\nPRODUCT: %s\n",
            vesa_info->సంస్కరణ_2>>8, vesa_info->సంస్కరణ_2&0xff,
            vendor, లబ్ధం);

    పూర్ణాంకం ret, వెడల్పు, ఎత్తు;
    పూర్ణాంకం bpp_require = 0;
    ఒకవేళ (type == 0) {
        jpeg = jpeg_alloc();
        ఒకవేళ (!jpeg) {
            warn_noalloc();
            వెళ్ళు పూర్తి;
        }
        /* Parse jpeg and get image size. */
        dprintf(5, "Decoding bootsplash.jpg\n");
        ret = jpeg_decode(jpeg, filedata);
        ఒకవేళ (ret) {
            dprintf(1, "jpeg_decode failed with return code %d...\n", ret);
            వెళ్ళు పూర్తి;
        }
        jpeg_get_కొలత(jpeg, &వెడల్పు, &ఎత్తు);
    } లేకపోతే {
        bmp = bmp_alloc();
        ఒకవేళ (!bmp) {
            warn_noalloc();
            వెళ్ళు పూర్తి;
        }
        /* Parse bmp and get image size. */
        dprintf(5, "Decoding bootsplash.bmp\n");
        ret = bmp_decode(bmp, filedata, filesize);
        ఒకవేళ (ret) {
            dprintf(1, "bmp_decode failed with return code %d...\n", ret);
            వెళ్ళు పూర్తి;
        }
        bmp_get_info(bmp, &వెడల్పు, &ఎత్తు, &bpp_require);
    }

    // jpeg would use 16 or 24 bpp video mode, BMP uses 16/24/32 bpp mode.

    // Try to find a graphics mode with the corresponding dimensions.
    పూర్ణాంకం videomode = కనుగొను_videomode(vesa_info, బహుళకం_info, వెడల్పు, ఎత్తు,
                                       bpp_require);
    ఒకవేళ (videomode < 0) {
        dprintf(1, "failed to find a videomode with %dx%d %dbpp (0=any).\n",
                    వెడల్పు, ఎత్తు, bpp_require);
        వెళ్ళు పూర్తి;
    }
    శూన్య *framebuffer = (శూన్య *)బహుళకం_info->phys_పునాది;
    పూర్ణాంకం లోతు = బహుళకం_info->బిట్లు_చొప్పున_pixel;
    dprintf(3, "mode: %04x\n", videomode);
    dprintf(3, "framebuffer: %p\n", framebuffer);
    dprintf(3, "bytes per scanline: %d\n", బహుళకం_info->బైట్లు_చొప్పున_scanline);
    dprintf(3, "bits per pixel: %d\n", లోతు);

    // Allocate space for image and decompress it.
    పూర్ణాంకం imagesize = ఎత్తు * బహుళకం_info->బైట్లు_చొప్పున_scanline;
    picture = malloc_tmphigh(imagesize);
    ఒకవేళ (!picture) {
        warn_noalloc();
        వెళ్ళు పూర్తి;
    }

    ఒకవేళ (type == 0) {
        dprintf(5, "Decompressing bootsplash.jpg\n");
        ret = jpeg_show(jpeg, picture, వెడల్పు, ఎత్తు, లోతు,
                            బహుళకం_info->బైట్లు_చొప్పున_scanline);
        ఒకవేళ (ret) {
            dprintf(1, "jpeg_show failed with return code %d...\n", ret);
            వెళ్ళు పూర్తి;
        }
    } లేకపోతే {
        dprintf(5, "Decompressing bootsplash.bmp\n");
        ret = bmp_show(bmp, picture, వెడల్పు, ఎత్తు, లోతు,
                           బహుళకం_info->బైట్లు_చొప్పున_scanline);
        ఒకవేళ (ret) {
            dprintf(1, "bmp_show failed with return code %d...\n", ret);
            వెళ్ళు పూర్తి;
        }
    }

    /* Switch to graphics mode */
    dprintf(5, "Switching to graphics mode\n");
    memset(&br, 0, పరిమాణం(br));
    br.ax = 0x4f02;
    br.bx = videomode | VBE_బహుళకం_రేఖీయ_చట్రం_బఫరు;
    call16_int10(&br);
    ఒకవేళ (br.ax != 0x4f) {
        dprintf(1, "set_mode failed.\n");
        వెళ్ళు పూర్తి;
    }

    /* Show the picture */
    dprintf(5, "Showing bootsplash picture\n");
    iomemcpy(framebuffer, picture, imagesize);
    dprintf(5, "Bootsplash copy complete\n");
    Bootsplash_చురుకైన = 1;

పూర్తి:
    విడుదల(filedata);
    విడుదల(picture);
    విడుదల(vesa_info);
    విడుదల(బహుళకం_info);
    విడుదల(jpeg);
    విడుదల(bmp);
    తిరిగి;
}

శూన్య
disable_bootsplash(శూన్య)
{
    ఒకవేళ (!CONFIG_BOOTSPLASH || !Bootsplash_చురుకైన)
        తిరిగి;
    Bootsplash_చురుకైన = 0;
    enable_vga_console();
}
