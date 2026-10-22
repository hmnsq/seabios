#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __XEN_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __XEN_H

ਖਾਲੀ xen_preinit(ਖਾਲੀ);
ਖਾਲੀ xen_ramsize_preinit(ਖਾਲੀ);
ਖਾਲੀ xen_hypercall_ਤਿਆਰੀ(ਖਾਲੀ);
ਖਾਲੀ xen_biostable_ਤਿਆਰੀ(ਖਾਲੀ);

ਬਾਹਰੀ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ xen_hypercall_ਪੰਨਾ;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _hypercall0(type, name)                                         \
({                                                                      \
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ __hentry = xen_hypercall_ਪੰਨਾ+__HYPERVISOR_##name*32; \
    ਲੰਮਾ __res;                                                         \
    asm ਅਸਥਿਰ (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res)                                                  \
        : "0" (__hentry)                                                \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _hypercall1(type, name, a1)                                     \
({                                                                      \
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ __hentry = xen_hypercall_ਪੰਨਾ+__HYPERVISOR_##name*32; \
    ਲੰਮਾ __res, __ign1;                                                 \
    asm ਅਸਥਿਰ (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1)                                   \
        : "0" (__hentry), "1" ((ਲੰਮਾ)(a1))                              \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _hypercall2(type, name, a1, a2)                                 \
({                                                                      \
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ __hentry = xen_hypercall_ਪੰਨਾ+__HYPERVISOR_##name*32; \
    ਲੰਮਾ __res, __ign1, __ign2;                                         \
    asm ਅਸਥਿਰ (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2)                    \
        : "0" (__hentry), "1" ((ਲੰਮਾ)(a1)), "2" ((ਲੰਮਾ)(a2))            \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _hypercall3(type, name, a1, a2, a3)                             \
({                                                                      \
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ __hentry = xen_hypercall_ਪੰਨਾ+__HYPERVISOR_##name*32; \
    ਲੰਮਾ __res, __ign1, __ign2, __ign3;                                 \
    asm ਅਸਥਿਰ (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3)                                                 \
        : "0" (__hentry), "1" ((ਲੰਮਾ)(a1)), "2" ((ਲੰਮਾ)(a2)),           \
          "3" ((ਲੰਮਾ)(a3))                                              \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _hypercall4(type, name, a1, a2, a3, a4)                         \
({                                                                      \
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ __hentry = xen_hypercall_ਪੰਨਾ+__HYPERVISOR_##name*32; \
    ਲੰਮਾ __res, __ign1, __ign2, __ign3, __ign4;                         \
    asm ਅਸਥਿਰ (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3), "=S" (__ign4)                                  \
        : "0" (__hentry), "1" ((ਲੰਮਾ)(a1)), "2" ((ਲੰਮਾ)(a2)),           \
          "3" ((ਲੰਮਾ)(a3)), "4" ((ਲੰਮਾ)(a4))                            \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _hypercall5(type, name, a1, a2, a3, a4, a5)                     \
({                                                                      \
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ __hentry = xen_hypercall_ਪੰਨਾ+__HYPERVISOR_##name*32; \
    ਲੰਮਾ __res, __ign1, __ign2, __ign3, __ign4, __ign5;                 \
    asm ਅਸਥਿਰ (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3), "=S" (__ign4), "=D" (__ign5)                   \
        : "0" (__hentry), "1" ((ਲੰਮਾ)(a1)), "2" ((ਲੰਮਾ)(a2)),           \
          "3" ((ਲੰਮਾ)(a3)), "4" ((ਲੰਮਾ)(a4)),                           \
          "5" ((ਲੰਮਾ)(a5))                                              \
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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __HYPERVISOR_xen_version          17

/* version.h */

/* arg == xen_extraversion_t. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XENVER_extraversion 1
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਅੱਖਰ xen_extraversion_t[16];
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XEN_EXTRAVERSION_LEN (ਆਕਾਰ(xen_extraversion_t))

#ਜੇ_ਅੰਤ
