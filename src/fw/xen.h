#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __XEN_H
#ವ್ಯಾಖ್ಯಾನಿಸು __XEN_H

ಶೂನ್ಯ xen_preinit(ಶೂನ್ಯ);
ಶೂನ್ಯ xen_ramsize_preinit(ಶೂನ್ಯ);
ಶೂನ್ಯ xen_hypercall_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಶೂನ್ಯ xen_biostable_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);

ಬಾಹ್ಯ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ xen_hypercall_ಪುಟ;

#ವ್ಯಾಖ್ಯಾನಿಸು _hypercall0(type, name)                                         \
({                                                                      \
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ __hentry = xen_hypercall_ಪುಟ+__HYPERVISOR_##name*32; \
    ದೀರ್ಘ __res;                                                         \
    asm ಅಸ್ಥಿರ (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res)                                                  \
        : "0" (__hentry)                                                \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#ವ್ಯಾಖ್ಯಾನಿಸು _hypercall1(type, name, a1)                                     \
({                                                                      \
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ __hentry = xen_hypercall_ಪುಟ+__HYPERVISOR_##name*32; \
    ದೀರ್ಘ __res, __ign1;                                                 \
    asm ಅಸ್ಥಿರ (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1)                                   \
        : "0" (__hentry), "1" ((ದೀರ್ಘ)(a1))                              \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#ವ್ಯಾಖ್ಯಾನಿಸು _hypercall2(type, name, a1, a2)                                 \
({                                                                      \
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ __hentry = xen_hypercall_ಪುಟ+__HYPERVISOR_##name*32; \
    ದೀರ್ಘ __res, __ign1, __ign2;                                         \
    asm ಅಸ್ಥಿರ (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2)                    \
        : "0" (__hentry), "1" ((ದೀರ್ಘ)(a1)), "2" ((ದೀರ್ಘ)(a2))            \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#ವ್ಯಾಖ್ಯಾನಿಸು _hypercall3(type, name, a1, a2, a3)                             \
({                                                                      \
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ __hentry = xen_hypercall_ಪುಟ+__HYPERVISOR_##name*32; \
    ದೀರ್ಘ __res, __ign1, __ign2, __ign3;                                 \
    asm ಅಸ್ಥಿರ (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3)                                                 \
        : "0" (__hentry), "1" ((ದೀರ್ಘ)(a1)), "2" ((ದೀರ್ಘ)(a2)),           \
          "3" ((ದೀರ್ಘ)(a3))                                              \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#ವ್ಯಾಖ್ಯಾನಿಸು _hypercall4(type, name, a1, a2, a3, a4)                         \
({                                                                      \
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ __hentry = xen_hypercall_ಪುಟ+__HYPERVISOR_##name*32; \
    ದೀರ್ಘ __res, __ign1, __ign2, __ign3, __ign4;                         \
    asm ಅಸ್ಥಿರ (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3), "=S" (__ign4)                                  \
        : "0" (__hentry), "1" ((ದೀರ್ಘ)(a1)), "2" ((ದೀರ್ಘ)(a2)),           \
          "3" ((ದೀರ್ಘ)(a3)), "4" ((ದೀರ್ಘ)(a4))                            \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#ವ್ಯಾಖ್ಯಾನಿಸು _hypercall5(type, name, a1, a2, a3, a4, a5)                     \
({                                                                      \
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ __hentry = xen_hypercall_ಪುಟ+__HYPERVISOR_##name*32; \
    ದೀರ್ಘ __res, __ign1, __ign2, __ign3, __ign4, __ign5;                 \
    asm ಅಸ್ಥಿರ (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3), "=S" (__ign4), "=D" (__ign5)                   \
        : "0" (__hentry), "1" ((ದೀರ್ಘ)(a1)), "2" ((ದೀರ್ಘ)(a2)),           \
          "3" ((ದೀರ್ಘ)(a3)), "4" ((ದೀರ್ಘ)(a4)),                           \
          "5" ((ದೀರ್ಘ)(a5))                                              \
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

#ವ್ಯಾಖ್ಯಾನಿಸು __HYPERVISOR_xen_version          17

/* version.h */

/* arg == xen_extraversion_t. */
#ವ್ಯಾಖ್ಯಾನಿಸು XENVER_extraversion 1
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಅಕ್ಷರ xen_extraversion_t[16];
#ವ್ಯಾಖ್ಯಾನಿಸು XEN_EXTRAVERSION_LEN (ಗಾತ್ರ(xen_extraversion_t))

#ಆದರೆ_ಅಂತ್ಯ
