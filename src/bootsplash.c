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
    شروع_preempt();
    call16_int(0x10, br);
    پایان_preempt();
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
    چاپ_کن("SeaBIOS (version %s)\n", نسخه);
    display_uuid();
}

ساكن صحيح
پیدا_کن_videomode(بنية vbe_info *vesa_info, بنية vbe_نما_info *نما_info
               , صحيح عرض, صحيح ارتفاع, صحيح bpp_req)
{
    dprintf(3, "Finding vesa mode with dimensions %d/%d\n", عرض, ارتفاع);
    u16 *videomodes = SEGOFF_مقصد_FLATPTR(vesa_info->video_نما);
    لكل (;; videomodes++) {
        u16 videomode = *videomodes;
        إذا (videomode == 0xffff) {
            dprintf(1, "Unable to find vesa video mode dimensions %d/%d\n"
                    , عرض, ارتفاع);
            أرجع -1;
        }
        بنية bregs br;
        memset(&br, 0, حجم(br));
        br.ax = 0x4f01;
        br.cx = videomode;
        br.di = FLATPTR_مقصد_جابه_جایی(نما_info);
        br.es = FLATPTR_مقصد_SEG(نما_info);
        call16_int10(&br);
        إذا (br.ax != 0x4f) {
            dprintf(1, "get_mode failed.\n");
            استمر;
        }
        إذا (نما_info->xres != عرض
            || نما_info->yres != ارتفاع)
            استمر;
        u8 عمق = نما_info->بیتها_در_هر_pixel;
        إذا (bpp_req == 0) {
            إذا ((عمق != 16 && عمق != 24 && عمق != 32)
                || نما_info->green_اندازه == 5)
                استمر;
        } وإلا {
            إذا (عمق != bpp_req)
                استمر;
        }
        أرجع videomode;
    }
}

ساكن صحيح Bootsplash_فعال;

فراغ
enable_bootsplash(فراغ)
{
    إذا (!CONFIG_BOOTSPLASH)
        أرجع;
    /* splash picture can be bmp or jpeg file */
    dprintf(3, "Checking for bootsplash\n");
    u8 type = 0; /* 0 means jpg, 1 means bmp, default is 0=jpg */
    صحيح filesize;
    u8 *filedata = romfile_loadfile("bootsplash.jpg", &filesize);
    إذا (!filedata) {
        filedata = romfile_loadfile("bootsplash.bmp", &filesize);
        إذا (!filedata)
            أرجع;
        type = 1;
    }
    dprintf(3, "start showing bootsplash\n");

    u8 *picture = عدم; /* data buff used to be flushed to the video buf */
    بنية jpeg_decdata *jpeg = عدم;
    بنية bmp_decdata *bmp = عدم;
    بنية vbe_info *vesa_info = malloc_tmplow(حجم(*vesa_info));
    بنية vbe_نما_info *نما_info = malloc_tmplow(حجم(*نما_info));
    إذا (!vesa_info || !نما_info) {
        warn_noalloc();
        اذهب انجامشده;
    }

    /* Check whether we have a VESA 2.0 compliant BIOS */
    memset(vesa_info, 0, حجم(بنية vbe_info));
    vesa_info->signature = VBE2_SIGNATURE;
    بنية bregs br;
    memset(&br, 0, حجم(br));
    br.ax = 0x4f00;
    br.di = FLATPTR_مقصد_جابه_جایی(vesa_info);
    br.es = FLATPTR_مقصد_SEG(vesa_info);
    call16_int10(&br);
    إذا (vesa_info->signature != VESA_SIGNATURE) {
        dprintf(1,"No VBE2 found.\n");
        اذهب انجامشده;
    }

    /* Print some debugging information about our card. */
    حرف *vendor = SEGOFF_مقصد_FLATPTR(vesa_info->oem_vendor_string);
    حرف *حاصلضرب = SEGOFF_مقصد_FLATPTR(vesa_info->oem_حاصلضرب_string);
    dprintf(3, "VESA %d.%d\nVENDOR: %s\nPRODUCT: %s\n",
            vesa_info->نسخه_2>>8, vesa_info->نسخه_2&0xff,
            vendor, حاصلضرب);

    صحيح ret, عرض, ارتفاع;
    صحيح bpp_require = 0;
    إذا (type == 0) {
        jpeg = jpeg_alloc();
        إذا (!jpeg) {
            warn_noalloc();
            اذهب انجامشده;
        }
        /* Parse jpeg and get image size. */
        dprintf(5, "Decoding bootsplash.jpg\n");
        ret = jpeg_decode(jpeg, filedata);
        إذا (ret) {
            dprintf(1, "jpeg_decode failed with return code %d...\n", ret);
            اذهب انجامشده;
        }
        jpeg_get_اندازه(jpeg, &عرض, &ارتفاع);
    } وإلا {
        bmp = bmp_alloc();
        إذا (!bmp) {
            warn_noalloc();
            اذهب انجامشده;
        }
        /* Parse bmp and get image size. */
        dprintf(5, "Decoding bootsplash.bmp\n");
        ret = bmp_decode(bmp, filedata, filesize);
        إذا (ret) {
            dprintf(1, "bmp_decode failed with return code %d...\n", ret);
            اذهب انجامشده;
        }
        bmp_get_info(bmp, &عرض, &ارتفاع, &bpp_require);
    }

    // jpeg would use 16 or 24 bpp video mode, BMP uses 16/24/32 bpp mode.

    // Try to find a graphics mode with the corresponding dimensions.
    صحيح videomode = پیدا_کن_videomode(vesa_info, نما_info, عرض, ارتفاع,
                                       bpp_require);
    إذا (videomode < 0) {
        dprintf(1, "failed to find a videomode with %dx%d %dbpp (0=any).\n",
                    عرض, ارتفاع, bpp_require);
        اذهب انجامشده;
    }
    فراغ *framebuffer = (فراغ *)نما_info->phys_پایه;
    صحيح عمق = نما_info->بیتها_در_هر_pixel;
    dprintf(3, "mode: %04x\n", videomode);
    dprintf(3, "framebuffer: %p\n", framebuffer);
    dprintf(3, "bytes per scanline: %d\n", نما_info->بایتها_در_هر_scanline);
    dprintf(3, "bits per pixel: %d\n", عمق);

    // Allocate space for image and decompress it.
    صحيح imagesize = ارتفاع * نما_info->بایتها_در_هر_scanline;
    picture = malloc_tmphigh(imagesize);
    إذا (!picture) {
        warn_noalloc();
        اذهب انجامشده;
    }

    إذا (type == 0) {
        dprintf(5, "Decompressing bootsplash.jpg\n");
        ret = jpeg_show(jpeg, picture, عرض, ارتفاع, عمق,
                            نما_info->بایتها_در_هر_scanline);
        إذا (ret) {
            dprintf(1, "jpeg_show failed with return code %d...\n", ret);
            اذهب انجامشده;
        }
    } وإلا {
        dprintf(5, "Decompressing bootsplash.bmp\n");
        ret = bmp_show(bmp, picture, عرض, ارتفاع, عمق,
                           نما_info->بایتها_در_هر_scanline);
        إذا (ret) {
            dprintf(1, "bmp_show failed with return code %d...\n", ret);
            اذهب انجامشده;
        }
    }

    /* Switch to graphics mode */
    dprintf(5, "Switching to graphics mode\n");
    memset(&br, 0, حجم(br));
    br.ax = 0x4f02;
    br.bx = videomode | VBE_نما_خطی_قاب_میانگیر;
    call16_int10(&br);
    إذا (br.ax != 0x4f) {
        dprintf(1, "set_mode failed.\n");
        اذهب انجامشده;
    }

    /* Show the picture */
    dprintf(5, "Showing bootsplash picture\n");
    iomemcpy(framebuffer, picture, imagesize);
    dprintf(5, "Bootsplash copy complete\n");
    Bootsplash_فعال = 1;

انجامشده:
    حرر(filedata);
    حرر(picture);
    حرر(vesa_info);
    حرر(نما_info);
    حرر(jpeg);
    حرر(bmp);
    أرجع;
}

فراغ
disable_bootsplash(فراغ)
{
    إذا (!CONFIG_BOOTSPLASH || !Bootsplash_فعال)
        أرجع;
    Bootsplash_فعال = 0;
    enable_vga_console();
}
