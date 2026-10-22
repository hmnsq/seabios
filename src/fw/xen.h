#جيڪڏهن_وصف_نه_ٿيل __XEN_H
#وصف_ڪريو __XEN_H

فراغ xen_preinit(فراغ);
فراغ xen_ramsize_preinit(فراغ);
فراغ xen_hypercall_تياري(فراغ);
فراغ xen_biostable_تياري(فراغ);

خارجي بنانشان ڊگهو xen_hypercall_صفحو;

#وصف_ڪريو _hypercall0(type, name)                                         \
({                                                                      \
    بنانشان ڊگهو __hentry = xen_hypercall_صفحو+__HYPERVISOR_##name*32; \
    ڊگهو __res;                                                         \
    asm متطاير (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res)                                                  \
        : "0" (__hentry)                                                \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#وصف_ڪريو _hypercall1(type, name, a1)                                     \
({                                                                      \
    بنانشان ڊگهو __hentry = xen_hypercall_صفحو+__HYPERVISOR_##name*32; \
    ڊگهو __res, __ign1;                                                 \
    asm متطاير (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1)                                   \
        : "0" (__hentry), "1" ((ڊگهو)(a1))                              \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#وصف_ڪريو _hypercall2(type, name, a1, a2)                                 \
({                                                                      \
    بنانشان ڊگهو __hentry = xen_hypercall_صفحو+__HYPERVISOR_##name*32; \
    ڊگهو __res, __ign1, __ign2;                                         \
    asm متطاير (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2)                    \
        : "0" (__hentry), "1" ((ڊگهو)(a1)), "2" ((ڊگهو)(a2))            \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#وصف_ڪريو _hypercall3(type, name, a1, a2, a3)                             \
({                                                                      \
    بنانشان ڊگهو __hentry = xen_hypercall_صفحو+__HYPERVISOR_##name*32; \
    ڊگهو __res, __ign1, __ign2, __ign3;                                 \
    asm متطاير (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3)                                                 \
        : "0" (__hentry), "1" ((ڊگهو)(a1)), "2" ((ڊگهو)(a2)),           \
          "3" ((ڊگهو)(a3))                                              \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#وصف_ڪريو _hypercall4(type, name, a1, a2, a3, a4)                         \
({                                                                      \
    بنانشان ڊگهو __hentry = xen_hypercall_صفحو+__HYPERVISOR_##name*32; \
    ڊگهو __res, __ign1, __ign2, __ign3, __ign4;                         \
    asm متطاير (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3), "=S" (__ign4)                                  \
        : "0" (__hentry), "1" ((ڊگهو)(a1)), "2" ((ڊگهو)(a2)),           \
          "3" ((ڊگهو)(a3)), "4" ((ڊگهو)(a4))                            \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#وصف_ڪريو _hypercall5(type, name, a1, a2, a3, a4, a5)                     \
({                                                                      \
    بنانشان ڊگهو __hentry = xen_hypercall_صفحو+__HYPERVISOR_##name*32; \
    ڊگهو __res, __ign1, __ign2, __ign3, __ign4, __ign5;                 \
    asm متطاير (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3), "=S" (__ign4), "=D" (__ign5)                   \
        : "0" (__hentry), "1" ((ڊگهو)(a1)), "2" ((ڊگهو)(a2)),           \
          "3" ((ڊگهو)(a3)), "4" ((ڊگهو)(a4)),                           \
          "5" ((ڊگهو)(a5))                                              \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

/******************************************************************************
 *
 * The following interface definitions are taken from Xen and have the
 * following license:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* xen.h */

#وصف_ڪريو __HYPERVISOR_xen_version          17

/* version.h */

/* arg == xen_extraversion_t. */
#وصف_ڪريو XENVER_extraversion 1
تعريفنوع حرف xen_extraversion_t[16];
#وصف_ڪريو XEN_EXTRAVERSION_LEN (حجم(xen_extraversion_t))

#جيڪڏهن_پورو
