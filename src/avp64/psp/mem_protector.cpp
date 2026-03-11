/******************************************************************************
 *                                                                            *
 * Copyright 2026 Nils Bosbach                                                *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#include "avp64/psp/mem_protector.h"

#include <sys/mman.h>

namespace avp64 {
namespace psp {

const vcml::u64 mem_protector::HOST_PAGE_BITS = mwr::ctz(mwr::get_page_size());
const vcml::u64 mem_protector::HOST_PAGE_MASK = ~(mwr::get_page_size() - 1);

mem_protector::mem_protector(): m_protected_pages() {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    ::sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segfault_handler;
    VCML_ERROR_ON(::sigaction(SIGSEGV, &sa, &m_sa_orig) != 0, "sigaction: %s",
                  std::strerror(errno));
}

mem_protector::~mem_protector() {
    VCML_ERROR_ON(::sigaction(SIGSEGV, &m_sa_orig, nullptr) != 0,
                  "sigaction: %s", std::strerror(errno));
}

void mem_protector::segfault_handler(int sig, siginfo_t* si, void* arg) {
    VCML_ERROR_ON(sig != SIGSEGV, "unexpected signal");
    instance().segfault_handler_int(sig, si, arg);
}

void mem_protector::segfault_handler_int(int sig, siginfo_t* si, void* arg) {
    if (!notify_page(si->si_addr))
        m_sa_orig.sa_sigaction(sig, si, arg);
}

void mem_protector::register_page(mem_protector_if* core, vcml::u64 page_addr,
                                  void* host_addr) {
    vcml::u64 target_page_size = core->page_size();
    VCML_ERROR_ON(
        mwr::get_page_size() < target_page_size,
        "host page size is smaller than the target one - not implemented");
    VCML_ERROR_ON(target_page_size == 0, "page size is 0");

    void* host_page_addr = reinterpret_cast<void*>(
        reinterpret_cast<vcml::u64>(host_addr) & HOST_PAGE_MASK);

    auto& host_page = m_protected_pages[host_page_addr];
    bool target_page_found = false;

    for (auto& tp : host_page.target_pages) {
        if (tp.host_address == host_addr) {
            VCML_ERROR_ON(tp.page_addr != page_addr,
                          "page_addr do not match! %llu vs. %llu",
                          tp.page_addr, page_addr);
            if (host_page.locked)
                return;

            target_page_found = true;
        }
    }

    if (!target_page_found) {
        host_page.target_pages.emplace_back(
            target_page_data{ core, page_addr, target_page_size, host_addr });
    }

    VCML_ERROR_ON(!protect_page(host_addr), "failed to protect page: %s",
                  std::strerror(errno));
    host_page.locked = true;
}

bool mem_protector::protect_page(void* addr) {
    VCML_ERROR_ON(reinterpret_cast<vcml::u64>(addr) & ~HOST_PAGE_MASK,
                  "invalid page address: %llu",
                  reinterpret_cast<vcml::u64>(addr));
    return ::mprotect(addr, mwr::get_page_size(), PROT_READ) == 0;
}

bool mem_protector::unprotect_page(void* addr) {
    VCML_ERROR_ON(reinterpret_cast<vcml::u64>(addr) & ~HOST_PAGE_MASK,
                  "invalid page address: %llu",
                  reinterpret_cast<vcml::u64>(addr));
    return ::mprotect(addr, mwr::get_page_size(), PROT_READ | PROT_WRITE) == 0;
}

bool mem_protector::notify_page(void* access_addr) {
    void* page_addr = reinterpret_cast<void*>(
        reinterpret_cast<vcml::u64>(access_addr) & HOST_PAGE_MASK);

    if (!m_protected_pages.count(page_addr)) // not a locked page
        return false;

    auto& page = m_protected_pages[page_addr];
    for (const auto& tp : page.target_pages) {
        tp.c->update_page(tp.page_addr);
    }

    if (unprotect_page(reinterpret_cast<void*>(page_addr))) {
        page.locked = false;
        return true;
    }

    return false;
}

void mem_protector::deregister_page(mem_protector_if* cpu,
                                    vcml::u64 page_addr) {
    for (auto host_page_it = m_protected_pages.begin();
         host_page_it != m_protected_pages.end();) {
        auto& target_pages = host_page_it->second.target_pages;

        for (auto target_page_it = target_pages.begin();
             target_page_it != target_pages.end();) {
            if (target_page_it->page_addr == page_addr) {
                target_page_it = target_pages.erase(target_page_it);
                continue;
            }
            ++target_page_it;
        }
        if (target_pages.empty() && host_page_it->second.locked) {
            VCML_ERROR_ON(
                !unprotect_page(reinterpret_cast<void*>(host_page_it->first)),
                "failed to unprotect page: %s", std::strerror(errno));
            host_page_it = m_protected_pages.erase(host_page_it);
            continue;
        }
        ++host_page_it;
    }
}

void mem_protector::deregister_pages(mem_protector_if* cpu, vcml::u64 start,
                                     vcml::u64 end) {
    for (auto host_page_it = m_protected_pages.begin();
         host_page_it != m_protected_pages.end();) {
        auto& target_pages = host_page_it->second.target_pages;

        for (auto target_page_it = target_pages.begin();
             target_page_it != target_pages.end();) {
            vcml::u64 target_page_start = target_page_it->page_addr;
            vcml::u64 target_page_end = target_page_start +
                                        target_page_it->page_size - 1;

            if (target_page_start >= start && target_page_end <= end) {
                target_page_it = target_pages.erase(target_page_it);
                continue;
            }
            ++target_page_it;
        }
        if (target_pages.empty() && host_page_it->second.locked) {
            VCML_ERROR_ON(
                !unprotect_page(reinterpret_cast<void*>(host_page_it->first)),
                "failed to unprotect page: %s", std::strerror(errno));
            host_page_it = m_protected_pages.erase(host_page_it);
            continue;
        }
        ++host_page_it;
    }
}

mem_protector& mem_protector::instance() {
    static mem_protector inst;
    return inst;
}

} // namespace psp
} // namespace avp64
