// Initialize the VGA console and possibly show a boot splash image.
//
// Copyright (C) 2009-2010  coresystems GmbH
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "bregs.h" // struct bregs
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "farptr.h" // FLATPTR_TO_SEG
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // romfile_loadfile
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // call16_int
#ਅਨ੍ਤਰ੍ਭਾਵ "std/vbe.h" // struct vbe_info
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // enable_bootsplash


/****************************************************************
 * Helper functions
 ****************************************************************/

// Call int10 vga handler.
ਸਥਿਰ ਖਾਲੀ
call16_int10(ਬਣਤਰ bregs *br)
{
    br->flags = F_IF;
    ਸ਼ੁਰੂ_preempt();
    call16_int(0x10, br);
    ਅੰਤ_preempt();
}


/****************************************************************
 * VGA text / graphics console
 ****************************************************************/

ਖਾਲੀ
enable_vga_console(ਖਾਲੀ)
{
    dprintf(1, "Turning on vga text mode console\n");
    ਬਣਤਰ bregs br;

    /* Enable VGA text mode */
    memset(&br, 0, ਆਕਾਰ(br));
    br.ax = 0x0003;
    call16_int10(&br);

    // Write to screen.
    ਲਿਖੋ("SeaBIOS (version %s)\n", ਸੰਸਕਰਣ);
    display_uuid();
}

ਸਥਿਰ ਪੂਰਨ
ਲੱਭੋ_videomode(ਬਣਤਰ vbe_info *vesa_info, ਬਣਤਰ vbe_ਬਹੁਲਕ_info *ਬਹੁਲਕ_info
               , ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ, ਪੂਰਨ bpp_req)
{
    dprintf(3, "Finding vesa mode with dimensions %d/%d\n", ਚੌੜਾਈ, ਉਚਾਈ);
    u16 *videomodes = SEGOFF_ਮੰਜ਼ਿਲ_FLATPTR(vesa_info->video_ਬਹੁਲਕ);
    ਲਈ (;; videomodes++) {
        u16 videomode = *videomodes;
        ਜੇ (videomode == 0xffff) {
            dprintf(1, "Unable to find vesa video mode dimensions %d/%d\n"
                    , ਚੌੜਾਈ, ਉਚਾਈ);
            ਵਾਪਸ -1;
        }
        ਬਣਤਰ bregs br;
        memset(&br, 0, ਆਕਾਰ(br));
        br.ax = 0x4f01;
        br.cx = videomode;
        br.di = FLATPTR_ਮੰਜ਼ਿਲ_ਵਿੱਥ(ਬਹੁਲਕ_info);
        br.es = FLATPTR_ਮੰਜ਼ਿਲ_SEG(ਬਹੁਲਕ_info);
        call16_int10(&br);
        ਜੇ (br.ax != 0x4f) {
            dprintf(1, "get_mode failed.\n");
            ਜਾਰੀ;
        }
        ਜੇ (ਬਹੁਲਕ_info->xres != ਚੌੜਾਈ
            || ਬਹੁਲਕ_info->yres != ਉਚਾਈ)
            ਜਾਰੀ;
        u8 ਡੂੰਘਾਈ = ਬਹੁਲਕ_info->ਬਿੱਟ_ਪ੍ਰਤੀ_pixel;
        ਜੇ (bpp_req == 0) {
            ਜੇ ((ਡੂੰਘਾਈ != 16 && ਡੂੰਘਾਈ != 24 && ਡੂੰਘਾਈ != 32)
                || ਬਹੁਲਕ_info->green_ਮਾਪ == 5)
                ਜਾਰੀ;
        } ਨਹੀਂਤਾਂ {
            ਜੇ (ਡੂੰਘਾਈ != bpp_req)
                ਜਾਰੀ;
        }
        ਵਾਪਸ videomode;
    }
}

ਸਥਿਰ ਪੂਰਨ Bootsplash_ਸਰਗਰਮ;

ਖਾਲੀ
enable_bootsplash(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_BOOTSPLASH)
        ਵਾਪਸ;
    /* splash picture can be bmp or jpeg file */
    dprintf(3, "Checking for bootsplash\n");
    u8 type = 0; /* 0 means jpg, 1 means bmp, default is 0=jpg */
    ਪੂਰਨ filesize;
    u8 *filedata = romfile_loadfile("bootsplash.jpg", &filesize);
    ਜੇ (!filedata) {
        filedata = romfile_loadfile("bootsplash.bmp", &filesize);
        ਜੇ (!filedata)
            ਵਾਪਸ;
        type = 1;
    }
    dprintf(3, "start showing bootsplash\n");

    u8 *picture = ਨਿਰਾ; /* data buff used to be flushed to the video buf */
    ਬਣਤਰ jpeg_decdata *jpeg = ਨਿਰਾ;
    ਬਣਤਰ bmp_decdata *bmp = ਨਿਰਾ;
    ਬਣਤਰ vbe_info *vesa_info = malloc_tmplow(ਆਕਾਰ(*vesa_info));
    ਬਣਤਰ vbe_ਬਹੁਲਕ_info *ਬਹੁਲਕ_info = malloc_tmplow(ਆਕਾਰ(*ਬਹੁਲਕ_info));
    ਜੇ (!vesa_info || !ਬਹੁਲਕ_info) {
        warn_noalloc();
        ਜਾਓ ਪੂਰਾ;
    }

    /* Check whether we have a VESA 2.0 compliant BIOS */
    memset(vesa_info, 0, ਆਕਾਰ(ਬਣਤਰ vbe_info));
    vesa_info->signature = VBE2_SIGNATURE;
    ਬਣਤਰ bregs br;
    memset(&br, 0, ਆਕਾਰ(br));
    br.ax = 0x4f00;
    br.di = FLATPTR_ਮੰਜ਼ਿਲ_ਵਿੱਥ(vesa_info);
    br.es = FLATPTR_ਮੰਜ਼ਿਲ_SEG(vesa_info);
    call16_int10(&br);
    ਜੇ (vesa_info->signature != VESA_SIGNATURE) {
        dprintf(1,"No VBE2 found.\n");
        ਜਾਓ ਪੂਰਾ;
    }

    /* Print some debugging information about our card. */
    ਅੱਖਰ *vendor = SEGOFF_ਮੰਜ਼ਿਲ_FLATPTR(vesa_info->oem_vendor_string);
    ਅੱਖਰ *ਗੁਣਨਫਲ = SEGOFF_ਮੰਜ਼ਿਲ_FLATPTR(vesa_info->oem_ਗੁਣਨਫਲ_string);
    dprintf(3, "VESA %d.%d\nVENDOR: %s\nPRODUCT: %s\n",
            vesa_info->ਸੰਸਕਰਣ_2>>8, vesa_info->ਸੰਸਕਰਣ_2&0xff,
            vendor, ਗੁਣਨਫਲ);

    ਪੂਰਨ ret, ਚੌੜਾਈ, ਉਚਾਈ;
    ਪੂਰਨ bpp_require = 0;
    ਜੇ (type == 0) {
        jpeg = jpeg_alloc();
        ਜੇ (!jpeg) {
            warn_noalloc();
            ਜਾਓ ਪੂਰਾ;
        }
        /* Parse jpeg and get image size. */
        dprintf(5, "Decoding bootsplash.jpg\n");
        ret = jpeg_decode(jpeg, filedata);
        ਜੇ (ret) {
            dprintf(1, "jpeg_decode failed with return code %d...\n", ret);
            ਜਾਓ ਪੂਰਾ;
        }
        jpeg_get_ਮਾਪ(jpeg, &ਚੌੜਾਈ, &ਉਚਾਈ);
    } ਨਹੀਂਤਾਂ {
        bmp = bmp_alloc();
        ਜੇ (!bmp) {
            warn_noalloc();
            ਜਾਓ ਪੂਰਾ;
        }
        /* Parse bmp and get image size. */
        dprintf(5, "Decoding bootsplash.bmp\n");
        ret = bmp_decode(bmp, filedata, filesize);
        ਜੇ (ret) {
            dprintf(1, "bmp_decode failed with return code %d...\n", ret);
            ਜਾਓ ਪੂਰਾ;
        }
        bmp_get_info(bmp, &ਚੌੜਾਈ, &ਉਚਾਈ, &bpp_require);
    }

    // jpeg would use 16 or 24 bpp video mode, BMP uses 16/24/32 bpp mode.

    // Try to find a graphics mode with the corresponding dimensions.
    ਪੂਰਨ videomode = ਲੱਭੋ_videomode(vesa_info, ਬਹੁਲਕ_info, ਚੌੜਾਈ, ਉਚਾਈ,
                                       bpp_require);
    ਜੇ (videomode < 0) {
        dprintf(1, "failed to find a videomode with %dx%d %dbpp (0=any).\n",
                    ਚੌੜਾਈ, ਉਚਾਈ, bpp_require);
        ਜਾਓ ਪੂਰਾ;
    }
    ਖਾਲੀ *framebuffer = (ਖਾਲੀ *)ਬਹੁਲਕ_info->phys_ਅਧਾਰ;
    ਪੂਰਨ ਡੂੰਘਾਈ = ਬਹੁਲਕ_info->ਬਿੱਟ_ਪ੍ਰਤੀ_pixel;
    dprintf(3, "mode: %04x\n", videomode);
    dprintf(3, "framebuffer: %p\n", framebuffer);
    dprintf(3, "bytes per scanline: %d\n", ਬਹੁਲਕ_info->ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_scanline);
    dprintf(3, "bits per pixel: %d\n", ਡੂੰਘਾਈ);

    // Allocate space for image and decompress it.
    ਪੂਰਨ imagesize = ਉਚਾਈ * ਬਹੁਲਕ_info->ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_scanline;
    picture = malloc_tmphigh(imagesize);
    ਜੇ (!picture) {
        warn_noalloc();
        ਜਾਓ ਪੂਰਾ;
    }

    ਜੇ (type == 0) {
        dprintf(5, "Decompressing bootsplash.jpg\n");
        ret = jpeg_show(jpeg, picture, ਚੌੜਾਈ, ਉਚਾਈ, ਡੂੰਘਾਈ,
                            ਬਹੁਲਕ_info->ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_scanline);
        ਜੇ (ret) {
            dprintf(1, "jpeg_show failed with return code %d...\n", ret);
            ਜਾਓ ਪੂਰਾ;
        }
    } ਨਹੀਂਤਾਂ {
        dprintf(5, "Decompressing bootsplash.bmp\n");
        ret = bmp_show(bmp, picture, ਚੌੜਾਈ, ਉਚਾਈ, ਡੂੰਘਾਈ,
                           ਬਹੁਲਕ_info->ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_scanline);
        ਜੇ (ret) {
            dprintf(1, "bmp_show failed with return code %d...\n", ret);
            ਜਾਓ ਪੂਰਾ;
        }
    }

    /* Switch to graphics mode */
    dprintf(5, "Switching to graphics mode\n");
    memset(&br, 0, ਆਕਾਰ(br));
    br.ax = 0x4f02;
    br.bx = videomode | VBE_ਬਹੁਲਕ_ਰੇਖਿਕ_ਚੌਖਟਾ_ਬਫਰ;
    call16_int10(&br);
    ਜੇ (br.ax != 0x4f) {
        dprintf(1, "set_mode failed.\n");
        ਜਾਓ ਪੂਰਾ;
    }

    /* Show the picture */
    dprintf(5, "Showing bootsplash picture\n");
    iomemcpy(framebuffer, picture, imagesize);
    dprintf(5, "Bootsplash copy complete\n");
    Bootsplash_ਸਰਗਰਮ = 1;

ਪੂਰਾ:
    ਮੁਕਤ(filedata);
    ਮੁਕਤ(picture);
    ਮੁਕਤ(vesa_info);
    ਮੁਕਤ(ਬਹੁਲਕ_info);
    ਮੁਕਤ(jpeg);
    ਮੁਕਤ(bmp);
    ਵਾਪਸ;
}

ਖਾਲੀ
disable_bootsplash(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_BOOTSPLASH || !Bootsplash_ਸਰਗਰਮ)
        ਵਾਪਸ;
    Bootsplash_ਸਰਗਰਮ = 0;
    enable_vga_console();
}
