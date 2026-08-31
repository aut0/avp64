/******************************************************************************
 *                                                                            *
 * Copyright 2026 Nils Bosbach                                                *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#ifndef AVP64_PSP_MEM_PROTECTOR_H
#define AVP64_PSP_MEM_PROTECTOR_H

#include "avp64/common.h"

#include <csignal>

namespace avp64 {
namespace psp {

class mem_protector_if
{
public:
    virtual ~mem_protector_if() = default;

    virtual u64 page_size() = 0;
    virtual void update_page(u64 page_addr) = 0;
};

class mem_protector
{
private:
    struct target_page_data {
        mem_protector_if* c;
        u64 page_addr;
        u64 page_size;
        void* host_address;
    };
    struct host_page_data {
        std::list<target_page_data> target_pages;
        bool locked;

        host_page_data(): target_pages(), locked(false) {}
    };

    static const u64 HOST_PAGE_BITS;
    static const u64 HOST_PAGE_MASK;

    unordered_map<void*, struct host_page_data> m_protected_pages;
    struct sigaction m_sa_orig;

    mem_protector();
    void segfault_handler_int(int sig, siginfo_t* si, void*);
    bool protect_page(void* addr);
    bool unprotect_page(void* addr);

public:
    static mem_protector& instance();

    mem_protector(const mem_protector&) = delete;
    void operator=(mem_protector const&) = delete;
    virtual ~mem_protector();

    static void segfault_handler(int sig, siginfo_t* si, void*);
    void register_page(mem_protector_if* cpu, u64 page_addr, void* host_addr);
    void deregister_pages(mem_protector_if* cpu, u64 start, u64 end);
    void deregister_page(mem_protector_if* cpu, u64 page_addr);
    bool notify_page(void* access_addr);
};

} // namespace psp
} // namespace avp64

#endif
