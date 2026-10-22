#اگر_تعریف_نشده __XEN_H
#تعریف_کن __XEN_H

فراغ xen_preinit(فراغ);
فراغ xen_ramsize_preinit(فراغ);
فراغ xen_hypercall_راهاندازی(فراغ);
فراغ xen_biostable_راهاندازی(فراغ);

خارجي بیعلامت دراز xen_hypercall_صفحه;

#تعریف_کن _hypercall0(type, name)                                         \
({                                                                      \
    بیعلامت دراز __hentry = xen_hypercall_صفحه+__HYPERVISOR_##name*32; \
    دراز __res;                                                         \
    asm متطاير (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res)                                                  \
        : "0" (__hentry)                                                \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#تعریف_کن _hypercall1(type, name, a1)                                     \
({                                                                      \
    بیعلامت دراز __hentry = xen_hypercall_صفحه+__HYPERVISOR_##name*32; \
    دراز __res, __ign1;                                                 \
    asm متطاير (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1)                                   \
        : "0" (__hentry), "1" ((دراز)(a1))                              \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#تعریف_کن _hypercall2(type, name, a1, a2)                                 \
({                                                                      \
    بیعلامت دراز __hentry = xen_hypercall_صفحه+__HYPERVISOR_##name*32; \
    دراز __res, __ign1, __ign2;                                         \
    asm متطاير (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2)                    \
        : "0" (__hentry), "1" ((دراز)(a1)), "2" ((دراز)(a2))            \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#تعریف_کن _hypercall3(type, name, a1, a2, a3)                             \
({                                                                      \
    بیعلامت دراز __hentry = xen_hypercall_صفحه+__HYPERVISOR_##name*32; \
    دراز __res, __ign1, __ign2, __ign3;                                 \
    asm متطاير (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3)                                                 \
        : "0" (__hentry), "1" ((دراز)(a1)), "2" ((دراز)(a2)),           \
          "3" ((دراز)(a3))                                              \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#تعریف_کن _hypercall4(type, name, a1, a2, a3, a4)                         \
({                                                                      \
    بیعلامت دراز __hentry = xen_hypercall_صفحه+__HYPERVISOR_##name*32; \
    دراز __res, __ign1, __ign2, __ign3, __ign4;                         \
    asm متطاير (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3), "=S" (__ign4)                                  \
        : "0" (__hentry), "1" ((دراز)(a1)), "2" ((دراز)(a2)),           \
          "3" ((دراز)(a3)), "4" ((دراز)(a4))                            \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#تعریف_کن _hypercall5(type, name, a1, a2, a3, a4, a5)                     \
({                                                                      \
    بیعلامت دراز __hentry = xen_hypercall_صفحه+__HYPERVISOR_##name*32; \
    دراز __res, __ign1, __ign2, __ign3, __ign4, __ign5;                 \
    asm متطاير (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3), "=S" (__ign4), "=D" (__ign5)                   \
        : "0" (__hentry), "1" ((دراز)(a1)), "2" ((دراز)(a2)),           \
          "3" ((دراز)(a3)), "4" ((دراز)(a4)),                           \
          "5" ((دراز)(a5))                                              \
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

#تعریف_کن __HYPERVISOR_xen_version          17

/* version.h */

/* arg == xen_extraversion_t. */
#تعریف_کن XENVER_extraversion 1
تعريفنوع حرف xen_extraversion_t[16];
#تعریف_کن XEN_EXTRAVERSION_LEN (حجم(xen_extraversion_t))

#پایان_اگر
