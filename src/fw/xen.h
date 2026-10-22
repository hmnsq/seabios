#که_نه_تعریف_شوی __XEN_H
#تعریف_کړه __XEN_H

فراغ xen_preinit(فراغ);
فراغ xen_ramsize_preinit(فراغ);
فراغ xen_hypercall_تیاری(فراغ);
فراغ xen_biostable_تیاری(فراغ);

خارجي بینښه اوږد xen_hypercall_پاڼه;

#تعریف_کړه _hypercall0(type, name)                                         \
({                                                                      \
    بینښه اوږد __hentry = xen_hypercall_پاڼه+__HYPERVISOR_##name*32; \
    اوږد __res;                                                         \
    asm متطاير (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res)                                                  \
        : "0" (__hentry)                                                \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#تعریف_کړه _hypercall1(type, name, a1)                                     \
({                                                                      \
    بینښه اوږد __hentry = xen_hypercall_پاڼه+__HYPERVISOR_##name*32; \
    اوږد __res, __ign1;                                                 \
    asm متطاير (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1)                                   \
        : "0" (__hentry), "1" ((اوږد)(a1))                              \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#تعریف_کړه _hypercall2(type, name, a1, a2)                                 \
({                                                                      \
    بینښه اوږد __hentry = xen_hypercall_پاڼه+__HYPERVISOR_##name*32; \
    اوږد __res, __ign1, __ign2;                                         \
    asm متطاير (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2)                    \
        : "0" (__hentry), "1" ((اوږد)(a1)), "2" ((اوږد)(a2))            \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#تعریف_کړه _hypercall3(type, name, a1, a2, a3)                             \
({                                                                      \
    بینښه اوږد __hentry = xen_hypercall_پاڼه+__HYPERVISOR_##name*32; \
    اوږد __res, __ign1, __ign2, __ign3;                                 \
    asm متطاير (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3)                                                 \
        : "0" (__hentry), "1" ((اوږد)(a1)), "2" ((اوږد)(a2)),           \
          "3" ((اوږد)(a3))                                              \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#تعریف_کړه _hypercall4(type, name, a1, a2, a3, a4)                         \
({                                                                      \
    بینښه اوږد __hentry = xen_hypercall_پاڼه+__HYPERVISOR_##name*32; \
    اوږد __res, __ign1, __ign2, __ign3, __ign4;                         \
    asm متطاير (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3), "=S" (__ign4)                                  \
        : "0" (__hentry), "1" ((اوږد)(a1)), "2" ((اوږد)(a2)),           \
          "3" ((اوږد)(a3)), "4" ((اوږد)(a4))                            \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#تعریف_کړه _hypercall5(type, name, a1, a2, a3, a4, a5)                     \
({                                                                      \
    بینښه اوږد __hentry = xen_hypercall_پاڼه+__HYPERVISOR_##name*32; \
    اوږد __res, __ign1, __ign2, __ign3, __ign4, __ign5;                 \
    asm متطاير (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3), "=S" (__ign4), "=D" (__ign5)                   \
        : "0" (__hentry), "1" ((اوږد)(a1)), "2" ((اوږد)(a2)),           \
          "3" ((اوږد)(a3)), "4" ((اوږد)(a4)),                           \
          "5" ((اوږد)(a5))                                              \
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

#تعریف_کړه __HYPERVISOR_xen_version          17

/* version.h */

/* arg == xen_extraversion_t. */
#تعریف_کړه XENVER_extraversion 1
تعريفنوع حرف xen_extraversion_t[16];
#تعریف_کړه XEN_EXTRAVERSION_LEN (حجم(xen_extraversion_t))

#د_که_پای
