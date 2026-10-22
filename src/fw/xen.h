#ifndef __XEN_H
#define __XEN_H

शून्य xen_preinit(शून्य);
शून्य xen_ramsize_preinit(शून्य);
शून्य xen_hypercall_setup(शून्य);
शून्य xen_biostable_setup(शून्य);

बाह्य अचिन्हित दीर्घ xen_hypercall_page;

#define _hypercall0(type, name)                                         \
({                                                                      \
    अचिन्हित दीर्घ __hentry = xen_hypercall_page+__HYPERVISOR_##name*32; \
    दीर्घ __res;                                                         \
    यंत्र अस्थिर (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res)                                                  \
        : "0" (__hentry)                                                \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#define _hypercall1(type, name, a1)                                     \
({                                                                      \
    अचिन्हित दीर्घ __hentry = xen_hypercall_page+__HYPERVISOR_##name*32; \
    दीर्घ __res, __ign1;                                                 \
    यंत्र अस्थिर (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1)                                   \
        : "0" (__hentry), "1" ((दीर्घ)(a1))                              \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#define _hypercall2(type, name, a1, a2)                                 \
({                                                                      \
    अचिन्हित दीर्घ __hentry = xen_hypercall_page+__HYPERVISOR_##name*32; \
    दीर्घ __res, __ign1, __ign2;                                         \
    यंत्र अस्थिर (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2)                    \
        : "0" (__hentry), "1" ((दीर्घ)(a1)), "2" ((दीर्घ)(a2))            \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#define _hypercall3(type, name, a1, a2, a3)                             \
({                                                                      \
    अचिन्हित दीर्घ __hentry = xen_hypercall_page+__HYPERVISOR_##name*32; \
    दीर्घ __res, __ign1, __ign2, __ign3;                                 \
    यंत्र अस्थिर (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3)                                                 \
        : "0" (__hentry), "1" ((दीर्घ)(a1)), "2" ((दीर्घ)(a2)),           \
          "3" ((दीर्घ)(a3))                                              \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#define _hypercall4(type, name, a1, a2, a3, a4)                         \
({                                                                      \
    अचिन्हित दीर्घ __hentry = xen_hypercall_page+__HYPERVISOR_##name*32; \
    दीर्घ __res, __ign1, __ign2, __ign3, __ign4;                         \
    यंत्र अस्थिर (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3), "=S" (__ign4)                                  \
        : "0" (__hentry), "1" ((दीर्घ)(a1)), "2" ((दीर्घ)(a2)),           \
          "3" ((दीर्घ)(a3)), "4" ((दीर्घ)(a4))                            \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#define _hypercall5(type, name, a1, a2, a3, a4, a5)                     \
({                                                                      \
    अचिन्हित दीर्घ __hentry = xen_hypercall_page+__HYPERVISOR_##name*32; \
    दीर्घ __res, __ign1, __ign2, __ign3, __ign4, __ign5;                 \
    यंत्र अस्थिर (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3), "=S" (__ign4), "=D" (__ign5)                   \
        : "0" (__hentry), "1" ((दीर्घ)(a1)), "2" ((दीर्घ)(a2)),           \
          "3" ((दीर्घ)(a3)), "4" ((दीर्घ)(a4)),                           \
          "5" ((दीर्घ)(a5))                                              \
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

#define __HYPERVISOR_xen_version          17

/* version.h */

/* arg == xen_extraversion_t. */
#define XENVER_extraversion 1
प्रकारव्याख्या अक्षर xen_extraversion_t[16];
#define XEN_EXTRAVERSION_LEN (आकार(xen_extraversion_t))

#endif
