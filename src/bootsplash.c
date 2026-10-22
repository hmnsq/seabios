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
    स्मृतिं_पूरय(&br, 0, परिमाण(br));
    br.ax = 0x0003;
    call16_int10(&br);

    // Write to screen.
    प्रारूप_लिख("SeaBIOS (version %s)\n", संस्करणम्);
    display_uuid();
}

स्थिर पूर्णाङ्क
find_videomode(संरचना vbe_info *vesa_info, संरचना vbe_बहुलकम्_info *बहुलकम्_info
               , पूर्णाङ्क विस्तारः, पूर्णाङ्क height, पूर्णाङ्क bpp_req)
{
    dprintf(3, "Finding vesa mode with dimensions %d/%d\n", विस्तारः, height);
    u16 *videomodes = SEGOFF_गन्तव्यम्_FLATPTR(vesa_info->video_बहुलकम्);
    प्रति (;; videomodes++) {
        u16 videomode = *videomodes;
        यदि (videomode == 0xffff) {
            dprintf(1, "Unable to find vesa video mode dimensions %d/%d\n"
                    , विस्तारः, height);
            प्रत्यागम -1;
        }
        संरचना bregs br;
        स्मृतिं_पूरय(&br, 0, परिमाण(br));
        br.ax = 0x4f01;
        br.cx = videomode;
        br.di = FLATPTR_गन्तव्यम्_विस्थापनम्(बहुलकम्_info);
        br.es = FLATPTR_गन्तव्यम्_SEG(बहुलकम्_info);
        call16_int10(&br);
        यदि (br.ax != 0x4f) {
            dprintf(1, "get_mode failed.\n");
            अनुवर्त;
        }
        यदि (बहुलकम्_info->xres != विस्तारः
            || बहुलकम्_info->yres != height)
            अनुवर्त;
        u8 गभीरता = बहुलकम्_info->द्वयङ्काः_प्रत्येकम्_pixel;
        यदि (bpp_req == 0) {
            यदि ((गभीरता != 16 && गभीरता != 24 && गभीरता != 32)
                || बहुलकम्_info->green_आकार == 5)
                अनुवर्त;
        } अन्यथा {
            यदि (गभीरता != bpp_req)
                अनुवर्त;
        }
        प्रत्यागम videomode;
    }
}

स्थिर पूर्णाङ्क Bootsplash_सक्रिय;

शून्य
enable_bootsplash(शून्य)
{
    यदि (!CONFIG_BOOTSPLASH)
        प्रत्यागम;
    /* splash picture can be bmp or jpeg file */
    dprintf(3, "Checking for bootsplash\n");
    u8 type = 0; /* 0 means jpg, 1 means bmp, default is 0=jpg */
    पूर्णाङ्क filesize;
    u8 *filedata = romfile_loadfile("bootsplash.jpg", &filesize);
    यदि (!filedata) {
        filedata = romfile_loadfile("bootsplash.bmp", &filesize);
        यदि (!filedata)
            प्रत्यागम;
        type = 1;
    }
    dprintf(3, "start showing bootsplash\n");

    u8 *picture = रिक्त; /* data buff used to be flushed to the video buf */
    संरचना jpeg_decdata *jpeg = रिक्त;
    संरचना bmp_decdata *bmp = रिक्त;
    संरचना vbe_info *vesa_info = malloc_tmplow(परिमाण(*vesa_info));
    संरचना vbe_बहुलकम्_info *बहुलकम्_info = malloc_tmplow(परिमाण(*बहुलकम्_info));
    यदि (!vesa_info || !बहुलकम्_info) {
        warn_noalloc();
        गच्छ समाप्तम्;
    }

    /* Check whether we have a VESA 2.0 compliant BIOS */
    स्मृतिं_पूरय(vesa_info, 0, परिमाण(संरचना vbe_info));
    vesa_info->signature = VBE2_SIGNATURE;
    संरचना bregs br;
    स्मृतिं_पूरय(&br, 0, परिमाण(br));
    br.ax = 0x4f00;
    br.di = FLATPTR_गन्तव्यम्_विस्थापनम्(vesa_info);
    br.es = FLATPTR_गन्तव्यम्_SEG(vesa_info);
    call16_int10(&br);
    यदि (vesa_info->signature != VESA_SIGNATURE) {
        dprintf(1,"No VBE2 found.\n");
        गच्छ समाप्तम्;
    }

    /* Print some debugging information about our card. */
    वर्ण *vendor = SEGOFF_गन्तव्यम्_FLATPTR(vesa_info->oem_vendor_string);
    वर्ण *गुणनफलम् = SEGOFF_गन्तव्यम्_FLATPTR(vesa_info->oem_गुणनफलम्_string);
    dprintf(3, "VESA %d.%d\nVENDOR: %s\nPRODUCT: %s\n",
            vesa_info->संस्करणम्_2>>8, vesa_info->संस्करणम्_2&0xff,
            vendor, गुणनफलम्);

    पूर्णाङ्क ret, विस्तारः, height;
    पूर्णाङ्क bpp_require = 0;
    यदि (type == 0) {
        jpeg = jpeg_alloc();
        यदि (!jpeg) {
            warn_noalloc();
            गच्छ समाप्तम्;
        }
        /* Parse jpeg and get image size. */
        dprintf(5, "Decoding bootsplash.jpg\n");
        ret = jpeg_decode(jpeg, filedata);
        यदि (ret) {
            dprintf(1, "jpeg_decode failed with return code %d...\n", ret);
            गच्छ समाप्तम्;
        }
        jpeg_get_आकार(jpeg, &विस्तारः, &height);
    } अन्यथा {
        bmp = bmp_alloc();
        यदि (!bmp) {
            warn_noalloc();
            गच्छ समाप्तम्;
        }
        /* Parse bmp and get image size. */
        dprintf(5, "Decoding bootsplash.bmp\n");
        ret = bmp_decode(bmp, filedata, filesize);
        यदि (ret) {
            dprintf(1, "bmp_decode failed with return code %d...\n", ret);
            गच्छ समाप्तम्;
        }
        bmp_get_info(bmp, &विस्तारः, &height, &bpp_require);
    }

    // jpeg would use 16 or 24 bpp video mode, BMP uses 16/24/32 bpp mode.

    // Try to find a graphics mode with the corresponding dimensions.
    पूर्णाङ्क videomode = find_videomode(vesa_info, बहुलकम्_info, विस्तारः, height,
                                       bpp_require);
    यदि (videomode < 0) {
        dprintf(1, "failed to find a videomode with %dx%d %dbpp (0=any).\n",
                    विस्तारः, height, bpp_require);
        गच्छ समाप्तम्;
    }
    शून्य *framebuffer = (शून्य *)बहुलकम्_info->phys_आधारः;
    पूर्णाङ्क गभीरता = बहुलकम्_info->द्वयङ्काः_प्रत्येकम्_pixel;
    dprintf(3, "mode: %04x\n", videomode);
    dprintf(3, "framebuffer: %p\n", framebuffer);
    dprintf(3, "bytes per scanline: %d\n", बहुलकम्_info->अष्टकानि_प्रत्येकम्_scanline);
    dprintf(3, "bits per pixel: %d\n", गभीरता);

    // Allocate space for image and decompress it.
    पूर्णाङ्क imagesize = height * बहुलकम्_info->अष्टकानि_प्रत्येकम्_scanline;
    picture = malloc_tmphigh(imagesize);
    यदि (!picture) {
        warn_noalloc();
        गच्छ समाप्तम्;
    }

    यदि (type == 0) {
        dprintf(5, "Decompressing bootsplash.jpg\n");
        ret = jpeg_show(jpeg, picture, विस्तारः, height, गभीरता,
                            बहुलकम्_info->अष्टकानि_प्रत्येकम्_scanline);
        यदि (ret) {
            dprintf(1, "jpeg_show failed with return code %d...\n", ret);
            गच्छ समाप्तम्;
        }
    } अन्यथा {
        dprintf(5, "Decompressing bootsplash.bmp\n");
        ret = bmp_show(bmp, picture, विस्तारः, height, गभीरता,
                           बहुलकम्_info->अष्टकानि_प्रत्येकम्_scanline);
        यदि (ret) {
            dprintf(1, "bmp_show failed with return code %d...\n", ret);
            गच्छ समाप्तम्;
        }
    }

    /* Switch to graphics mode */
    dprintf(5, "Switching to graphics mode\n");
    स्मृतिं_पूरय(&br, 0, परिमाण(br));
    br.ax = 0x4f02;
    br.bx = videomode | VBE_बहुलकम्_रैखिकम्_FRAME_बफर;
    call16_int10(&br);
    यदि (br.ax != 0x4f) {
        dprintf(1, "set_mode failed.\n");
        गच्छ समाप्तम्;
    }

    /* Show the picture */
    dprintf(5, "Showing bootsplash picture\n");
    iomemcpy(framebuffer, picture, imagesize);
    dprintf(5, "Bootsplash copy complete\n");
    Bootsplash_सक्रिय = 1;

समाप्तम्:
    मुक्त(filedata);
    मुक्त(picture);
    मुक्त(vesa_info);
    मुक्त(बहुलकम्_info);
    मुक्त(jpeg);
    मुक्त(bmp);
    प्रत्यागम;
}

शून्य
disable_bootsplash(शून्य)
{
    यदि (!CONFIG_BOOTSPLASH || !Bootsplash_सक्रिय)
        प्रत्यागम;
    Bootsplash_सक्रिय = 0;
    enable_vga_console();
}
