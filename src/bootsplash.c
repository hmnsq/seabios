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
    انت_preempt();
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
    لکھو("SeaBIOS (version %s)\n", نسخہ);
    display_uuid();
}

ساكن صحيح
لبھو_videomode(بنية vbe_info *vesa_info, بنية vbe_منوال_info *منوال_info
               , صحيح چوڑائی, صحيح اچائی, صحيح bpp_req)
{
    dprintf(3, "Finding vesa mode with dimensions %d/%d\n", چوڑائی, اچائی);
    u16 *videomodes = SEGOFF_منزل_FLATPTR(vesa_info->video_منوال);
    لكل (;; videomodes++) {
        u16 videomode = *videomodes;
        إذا (videomode == 0xffff) {
            dprintf(1, "Unable to find vesa video mode dimensions %d/%d\n"
                    , چوڑائی, اچائی);
            أرجع -1;
        }
        بنية bregs br;
        memset(&br, 0, حجم(br));
        br.ax = 0x4f01;
        br.cx = videomode;
        br.di = FLATPTR_منزل_وتھ(منوال_info);
        br.es = FLATPTR_منزل_SEG(منوال_info);
        call16_int10(&br);
        إذا (br.ax != 0x4f) {
            dprintf(1, "get_mode failed.\n");
            استمر;
        }
        إذا (منوال_info->xres != چوڑائی
            || منوال_info->yres != اچائی)
            استمر;
        u8 ڈونگھائی = منوال_info->بٹ_فی_pixel;
        إذا (bpp_req == 0) {
            إذا ((ڈونگھائی != 16 && ڈونگھائی != 24 && ڈونگھائی != 32)
                || منوال_info->green_ناپ == 5)
                استمر;
        } وإلا {
            إذا (ڈونگھائی != bpp_req)
                استمر;
        }
        أرجع videomode;
    }
}

ساكن صحيح Bootsplash_سرگرم;

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
    بنية vbe_منوال_info *منوال_info = malloc_tmplow(حجم(*منوال_info));
    إذا (!vesa_info || !منوال_info) {
        warn_noalloc();
        اذهب پورا;
    }

    /* Check whether we have a VESA 2.0 compliant BIOS */
    memset(vesa_info, 0, حجم(بنية vbe_info));
    vesa_info->signature = VBE2_SIGNATURE;
    بنية bregs br;
    memset(&br, 0, حجم(br));
    br.ax = 0x4f00;
    br.di = FLATPTR_منزل_وتھ(vesa_info);
    br.es = FLATPTR_منزل_SEG(vesa_info);
    call16_int10(&br);
    إذا (vesa_info->signature != VESA_SIGNATURE) {
        dprintf(1,"No VBE2 found.\n");
        اذهب پورا;
    }

    /* Print some debugging information about our card. */
    حرف *vendor = SEGOFF_منزل_FLATPTR(vesa_info->oem_vendor_string);
    حرف *ضرب_حاصل = SEGOFF_منزل_FLATPTR(vesa_info->oem_ضرب_حاصل_string);
    dprintf(3, "VESA %d.%d\nVENDOR: %s\nPRODUCT: %s\n",
            vesa_info->نسخہ_2>>8, vesa_info->نسخہ_2&0xff,
            vendor, ضرب_حاصل);

    صحيح ret, چوڑائی, اچائی;
    صحيح bpp_require = 0;
    إذا (type == 0) {
        jpeg = jpeg_alloc();
        إذا (!jpeg) {
            warn_noalloc();
            اذهب پورا;
        }
        /* Parse jpeg and get image size. */
        dprintf(5, "Decoding bootsplash.jpg\n");
        ret = jpeg_decode(jpeg, filedata);
        إذا (ret) {
            dprintf(1, "jpeg_decode failed with return code %d...\n", ret);
            اذهب پورا;
        }
        jpeg_get_ناپ(jpeg, &چوڑائی, &اچائی);
    } وإلا {
        bmp = bmp_alloc();
        إذا (!bmp) {
            warn_noalloc();
            اذهب پورا;
        }
        /* Parse bmp and get image size. */
        dprintf(5, "Decoding bootsplash.bmp\n");
        ret = bmp_decode(bmp, filedata, filesize);
        إذا (ret) {
            dprintf(1, "bmp_decode failed with return code %d...\n", ret);
            اذهب پورا;
        }
        bmp_get_info(bmp, &چوڑائی, &اچائی, &bpp_require);
    }

    // jpeg would use 16 or 24 bpp video mode, BMP uses 16/24/32 bpp mode.

    // Try to find a graphics mode with the corresponding dimensions.
    صحيح videomode = لبھو_videomode(vesa_info, منوال_info, چوڑائی, اچائی,
                                       bpp_require);
    إذا (videomode < 0) {
        dprintf(1, "failed to find a videomode with %dx%d %dbpp (0=any).\n",
                    چوڑائی, اچائی, bpp_require);
        اذهب پورا;
    }
    فراغ *framebuffer = (فراغ *)منوال_info->phys_ادھار;
    صحيح ڈونگھائی = منوال_info->بٹ_فی_pixel;
    dprintf(3, "mode: %04x\n", videomode);
    dprintf(3, "framebuffer: %p\n", framebuffer);
    dprintf(3, "bytes per scanline: %d\n", منوال_info->بائٹاں_فی_scanline);
    dprintf(3, "bits per pixel: %d\n", ڈونگھائی);

    // Allocate space for image and decompress it.
    صحيح imagesize = اچائی * منوال_info->بائٹاں_فی_scanline;
    picture = malloc_tmphigh(imagesize);
    إذا (!picture) {
        warn_noalloc();
        اذهب پورا;
    }

    إذا (type == 0) {
        dprintf(5, "Decompressing bootsplash.jpg\n");
        ret = jpeg_show(jpeg, picture, چوڑائی, اچائی, ڈونگھائی,
                            منوال_info->بائٹاں_فی_scanline);
        إذا (ret) {
            dprintf(1, "jpeg_show failed with return code %d...\n", ret);
            اذهب پورا;
        }
    } وإلا {
        dprintf(5, "Decompressing bootsplash.bmp\n");
        ret = bmp_show(bmp, picture, چوڑائی, اچائی, ڈونگھائی,
                           منوال_info->بائٹاں_فی_scanline);
        إذا (ret) {
            dprintf(1, "bmp_show failed with return code %d...\n", ret);
            اذهب پورا;
        }
    }

    /* Switch to graphics mode */
    dprintf(5, "Switching to graphics mode\n");
    memset(&br, 0, حجم(br));
    br.ax = 0x4f02;
    br.bx = videomode | VBE_منوال_خطی_چوکھٹا_بفر;
    call16_int10(&br);
    إذا (br.ax != 0x4f) {
        dprintf(1, "set_mode failed.\n");
        اذهب پورا;
    }

    /* Show the picture */
    dprintf(5, "Showing bootsplash picture\n");
    iomemcpy(framebuffer, picture, imagesize);
    dprintf(5, "Bootsplash copy complete\n");
    Bootsplash_سرگرم = 1;

پورا:
    حرر(filedata);
    حرر(picture);
    حرر(vesa_info);
    حرر(منوال_info);
    حرر(jpeg);
    حرر(bmp);
    أرجع;
}

فراغ
disable_bootsplash(فراغ)
{
    إذا (!CONFIG_BOOTSPLASH || !Bootsplash_سرگرم)
        أرجع;
    Bootsplash_سرگرم = 0;
    enable_vga_console();
}
