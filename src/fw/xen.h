#यदि_अपरिभाषितम् __XEN_H
#परिभाषय __XEN_H

शून्य xen_preinit(शून्य);
शून्य xen_ramsize_preinit(शून्य);
शून्य xen_hypercall_सज्जता(शून्य);
शून्य xen_biostable_सज्जता(शून्य);

बाह्य अचिह्नित दीर्घ xen_hypercall_page;

#परिभाषय _hypercall0(type, name)                                         \
({                                                                      \
    अचिह्नित दीर्घ __hentry = xen_hypercall_page+__HYPERVISOR_##name*32; \
    दीर्घ __res;                                                         \
    asm चञ्चल (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res)                                                  \
        : "0" (__hentry)                                                \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#परिभाषय _hypercall1(type, name, a1)                                     \
({                                                                      \
    अचिह्नित दीर्घ __hentry = xen_hypercall_page+__HYPERVISOR_##name*32; \
    दीर्घ __res, __ign1;                                                 \
    asm चञ्चल (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1)                                   \
        : "0" (__hentry), "1" ((दीर्घ)(a1))                              \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#परिभाषय _hypercall2(type, name, a1, a2)                                 \
({                                                                      \
    अचिह्नित दीर्घ __hentry = xen_hypercall_page+__HYPERVISOR_##name*32; \
    दीर्घ __res, __ign1, __ign2;                                         \
    asm चञ्चल (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2)                    \
        : "0" (__hentry), "1" ((दीर्घ)(a1)), "2" ((दीर्घ)(a2))            \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#परिभाषय _hypercall3(type, name, a1, a2, a3)                             \
({                                                                      \
    अचिह्नित दीर्घ __hentry = xen_hypercall_page+__HYPERVISOR_##name*32; \
    दीर्घ __res, __ign1, __ign2, __ign3;                                 \
    asm चञ्चल (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3)                                                 \
        : "0" (__hentry), "1" ((दीर्घ)(a1)), "2" ((दीर्घ)(a2)),           \
          "3" ((दीर्घ)(a3))                                              \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#परिभाषय _hypercall4(type, name, a1, a2, a3, a4)                         \
({                                                                      \
    अचिह्नित दीर्घ __hentry = xen_hypercall_page+__HYPERVISOR_##name*32; \
    दीर्घ __res, __ign1, __ign2, __ign3, __ign4;                         \
    asm चञ्चल (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3), "=S" (__ign4)                                  \
        : "0" (__hentry), "1" ((दीर्घ)(a1)), "2" ((दीर्घ)(a2)),           \
          "3" ((दीर्घ)(a3)), "4" ((दीर्घ)(a4))                            \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#परिभाषय _hypercall5(type, name, a1, a2, a3, a4, a5)                     \
({                                                                      \
    अचिह्नित दीर्घ __hentry = xen_hypercall_page+__HYPERVISOR_##name*32; \
    दीर्घ __res, __ign1, __ign2, __ign3, __ign4, __ign5;                 \
    asm चञ्चल (                                                      \
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

#परिभाषय __HYPERVISOR_xen_version          17

/* version.h */

/* arg == xen_extraversion_t. */
#परिभाषय XENVER_extraversion 1
प्रकारनिर्देश वर्ण xen_extraversion_t[16];
#परिभाषय XEN_EXTRAVERSION_LEN (परिमाण(xen_extraversion_t))

#यदि_अन्तः
