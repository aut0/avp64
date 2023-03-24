/******************************************************************************
 *                                                                            *
 * Copyright 2020 Lukas Jünger                                                *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#include "avp64/arm64_cpu.h"
#include "avp64/aarch64_regs.h"

#include <dlfcn.h>
#include <sys/mman.h>
#include <stdexcept>

#define CPU_ARCH "aarch64"

namespace avp64 {

memory_protector::memory_protector(): m_protected_pages() {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    ::sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segfault_handler;
    VCML_ERROR_ON(::sigaction(SIGSEGV, &sa, NULL) != 0, "sigaction: %s",
                  std::strerror(errno));
}

void memory_protector::segfault_handler(int sig, siginfo_t* si, void* unused) {
    VCML_ERROR_ON(sig != SIGSEGV, "unexpected signal");
    memory_protector::get_instance().notify_page(si->si_addr);
}

void memory_protector::register_page(arm64_cpu* cpu, vcml::u64 page_addr,
                                     void* host_addr) {
    VCML_ERROR_ON(::mprotect(host_addr, 4096, PROT_READ) != 0, "mprotect: %s",
                  std::strerror(errno));
    m_protected_pages.insert(
        { reinterpret_cast<vcml::u64>(host_addr), { cpu, page_addr } });
}

void memory_protector::notify_page(void* access_addr) {
    vcml::u64 host_page_addr = reinterpret_cast<vcml::u64>(access_addr) &
                               (~0xfffull);
    ::mprotect(reinterpret_cast<void*>(host_page_addr), 4096,
               PROT_READ | PROT_WRITE);
    try {
        auto p = m_protected_pages.at(host_page_addr);
        p.first->memory_protector_update(p.second);
    } catch (std::out_of_range& e) {
        VCML_ERROR("Page to notify not found");
    }
}

ocx::u8* arm64_cpu::get_page_ptr_r(ocx::u64 page_paddr) {
    tlm::tlm_dmi dmi;
    vcml::u64 page_size = get_page_size();
    if (insn.dmi_cache().lookup(page_paddr, page_size, tlm::TLM_READ_COMMAND,
                                dmi)) {
        return dmi.get_dmi_ptr() + page_paddr - dmi.get_start_address();
    } else {
        tlm::tlm_generic_payload tx;
        tx.set_address(page_paddr);
        tx.set_streaming_width(page_size);
        tx.set_data_length(page_size);
        tx.set_read();
        if (insn->get_direct_mem_ptr(tx, dmi)) {
            insn.map_dmi(dmi);
            return dmi.get_dmi_ptr() + page_paddr - dmi.get_start_address();
        } else {
            return nullptr;
        }
    }
}

ocx::u8* arm64_cpu::get_page_ptr_w(ocx::u64 page_paddr) {
    tlm::tlm_dmi dmi;
    vcml::u64 page_size = get_page_size();
    if (insn.dmi_cache().lookup(page_paddr, page_size, tlm::TLM_WRITE_COMMAND,
                                dmi)) {
        return dmi.get_dmi_ptr() + page_paddr - dmi.get_start_address();
    } else {
        tlm::tlm_generic_payload tx;
        tx.set_address(page_paddr);
        tx.set_streaming_width(page_size);
        tx.set_data_length(page_size);
        tx.set_write();
        if (insn->get_direct_mem_ptr(tx, dmi)) {
            insn.map_dmi(dmi);
            return dmi.get_dmi_ptr() + page_paddr - dmi.get_start_address();
        } else {
            return nullptr;
        }
    }
}

void arm64_cpu::protect_page(ocx::u8* page_ptr, ocx::u64 page_addr) {
    memory_protector::get_instance().register_page(this, page_addr, page_ptr);
}

ocx::response arm64_cpu::transport(const ocx::transaction& tx) {
    vcml::tlm_sbi info = vcml::SBI_NONE;
    if (tx.is_debug)
        info |= vcml::SBI_DEBUG;
    if (tx.is_excl)
        info |= vcml::SBI_EXCL;
    info.cpuid = get_core_id();
    tlm::tlm_response_status resp = tlm::TLM_GENERIC_ERROR_RESPONSE;
    if (tx.is_read)
        resp = data.read(tx.addr, tx.data, tx.size, info);
    else
        resp = data.write(tx.addr, tx.data, tx.size, info);
    switch (resp) {
    case tlm::TLM_OK_RESPONSE:
        return ocx::RESP_OK;
    case tlm::TLM_ADDRESS_ERROR_RESPONSE:
        return ocx::RESP_ADDRESS_ERROR;
    case tlm::TLM_COMMAND_ERROR_RESPONSE:
        return ocx::RESP_COMMAND_ERROR;
    case tlm::TLM_INCOMPLETE_RESPONSE:
        return ocx::RESP_FAILED;
    case tlm::TLM_GENERIC_ERROR_RESPONSE:
    case tlm::TLM_BURST_ERROR_RESPONSE:
    case tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE:
    default:
        vcml::log_info("Unexpected TLM transport response.");
        return ocx::RESP_FAILED;
    }
}

void arm64_cpu::signal(ocx::u64 sigid, bool set) {
    timer_irq_out[sigid] = set;
}

void arm64_cpu::broadcast_syscall(int callno, std::shared_ptr<void> arg,
                                  bool async) {
    handle_syscall(callno, arg);
    for (auto const& cpu : m_syscall_subscriber) {
        cpu->handle_syscall(callno, arg);
    }
}

ocx::u64 arm64_cpu::get_time_ps() {
    return sc_core::sc_time_stamp().value() /
           sc_core::sc_time(1, sc_core::SC_PS).value();
}

const char* arm64_cpu::get_param(const char* name) {
    if (strcmp("gicv3", name) == 0)
        return "false";
    else if (strcmp("tbsize", name) == 0)
        return "8MB";
    else
        VCML_ERROR("Unimplemented parameter requested");
}

void arm64_cpu::notify(ocx::u64 eventid, ocx::u64 time_ps) {
    sc_core::sc_time notify_time(time_ps, sc_core::SC_PS);
    sc_core::sc_time delta = notify_time - sc_core::sc_time_stamp();
    timer_events[eventid]->notify(delta);
}

void arm64_cpu::cancel(ocx::u64 eventid) {
    timer_events[eventid]->cancel();
}

void arm64_cpu::hint(ocx::hint_kind kind) {
    switch (kind) {
    case ocx::HINT_WFI: {
        sync();
        sc_core::sc_event_or_list list;
        for (auto it : irq) {
            list |= it.second->default_event();
            // Treat WFI as NOP if IRQ is pending
            if (it.second->read())
                return;
        }
        const sc_core::sc_time before_wait = sc_core::sc_time_stamp();
        sc_core::wait(list);
        VCML_ERROR_ON(local_time_stamp() != sc_core::sc_time_stamp(),
                      "core not synchronized");
        const vcml::u64 cycles = (sc_core::sc_time_stamp() - before_wait) /
                                 clock_cycle();
        m_sleep_cycles += cycles;
        m_total_cycles += cycles;
        m_core->stop();
        break;
    }
    default:
        throw std::logic_error("Unimplemented hint");
    }
}

void arm64_cpu::handle_begin_basic_block(ocx::u64 vaddr) {
    throw std::logic_error("Not implemented");
}

bool arm64_cpu::handle_breakpoint(ocx::u64 vaddr) {
    notify_breakpoint_hit(vaddr);
    return true;
}

bool arm64_cpu::handle_watchpoint(ocx::u64 vaddr, ocx::u64 size, ocx::u64 data,
                                  bool iswr) {
    const vcml::range range(vaddr, vaddr + size);

    if (iswr)
        notify_watchpoint_write(range, data);
    else
        notify_watchpoint_read(range);
    return true;
}

void arm64_cpu::add_syscall_subscriber(const std::shared_ptr<arm64_cpu>& cpu) {
    m_syscall_subscriber.push_back(cpu);
}

void arm64_cpu::memory_protector_update(vcml::u64 page_addr) {
    m_core->tb_flush_page(page_addr, page_addr + 4095);
    m_core->invalidate_page_ptr(page_addr);
    for (auto const& cpu : m_syscall_subscriber) {
        cpu->m_core->tb_flush_page(page_addr, page_addr + 4095);
        cpu->m_core->invalidate_page_ptr(page_addr);
    }
}

void arm64_cpu::timer_irq_trigger(int timer_id) {
    m_core->notified(timer_id);
}

void arm64_cpu::interrupt(unsigned int irq, bool set) {
    m_core->interrupt(irq, set);
}

void arm64_cpu::simulate(unsigned int cycles) {
    // insn_count() is only reset at the beginning of step(), but not at the
    // end, so the number of cycles can only be summed up in the following
    // quantum
    m_run_cycles += m_core->insn_count();
    m_core->step(cycles);
}

bool arm64_cpu::read_reg_dbg(vcml::u64 idx, vcml::u64& val) {
    val = 0x0000000000000000; // init value because vcml doesn't -> necessary,
                              // if reg is smaller than 64 bit
    return m_core->read_reg(idx, &val);
}

bool arm64_cpu::write_reg_dbg(vcml::u64 idx, vcml::u64 val) {
    return m_core->write_reg(idx, &val);
}

bool arm64_cpu::page_size(vcml::u64& size) {
    size = m_core->page_size();
    return true;
}

bool arm64_cpu::virt_to_phys(vcml::u64 vaddr, vcml::u64& paddr) {
    ocx::u64 paddr_ocx = paddr;
    bool ret_val = m_core->virt_to_phys(vaddr, paddr_ocx);
    paddr = paddr_ocx;
    return ret_val;
}

bool arm64_cpu::insert_breakpoint(vcml::u64 addr) {
    return m_core->add_breakpoint(addr);
}

bool arm64_cpu::remove_breakpoint(vcml::u64 addr) {
    return m_core->remove_breakpoint(addr);
}

bool arm64_cpu::insert_watchpoint(const vcml::range& mem,
                                  vcml::vcml_access acs) {
    switch (acs) {
    case vcml::vcml_access::VCML_ACCESS_READ:
        return m_core->add_watchpoint(mem.start, mem.length(), false);
    case vcml::vcml_access::VCML_ACCESS_WRITE:
        return m_core->add_watchpoint(mem.start, mem.length(), true);
    case vcml::vcml_access::VCML_ACCESS_READ_WRITE:
        return m_core->add_watchpoint(mem.start, mem.length(), true) &&
               m_core->add_watchpoint(mem.start, mem.length(), false);
    default:
        vcml::log_error("Unsupported watchpoint");
        return false;
    }
}

bool arm64_cpu::remove_watchpoint(const vcml::range& mem,
                                  vcml::vcml_access acs) {
    switch (acs) {
    case vcml::vcml_access::VCML_ACCESS_READ:
        return m_core->remove_watchpoint(mem.start, mem.length(), false);
    case vcml::vcml_access::VCML_ACCESS_WRITE:
        return m_core->remove_watchpoint(mem.start, mem.length(), true);
    case vcml::vcml_access::VCML_ACCESS_READ_WRITE:
        return m_core->remove_watchpoint(mem.start, mem.length(), true) &&
               m_core->remove_watchpoint(mem.start, mem.length(), false);
    default:
        vcml::log_error("Unsupported watchpoint");
        return false;
    }
}

vcml::u64 arm64_cpu::cycle_count() const {
    return m_run_cycles + m_core->insn_count();
}

void arm64_cpu::update_local_time(sc_core::sc_time& local_time,
                                  sc_core::sc_process_b* proc) {
    if (is_local_process(proc)) {
        vcml::u64 cycles = cycle_count() + m_sleep_cycles;
        VCML_ERROR_ON(cycles < m_total_cycles, "cycle count goes down");
        local_time += clock_cycles(cycles - m_total_cycles);
        m_total_cycles = cycles;
    }
}

bool arm64_cpu::disassemble(vcml::u8* ibuf, vcml::u64& addr,
                            std::string& code) {
    size_t bufsz = 100;
    char buf[bufsz];

    vcml::u64 len = m_core->disassemble(addr, buf, bufsz);
    if (len == 0) {
        return false;
    }

    code = std::string(buf);
    addr += len;
    return true;
}

vcml::u64 arm64_cpu::program_counter() {
    ocx::u64 pc_regid = m_core->pc_regid();
    vcml::u64 pc = 0;
    if (m_core->read_reg(pc_regid, &pc)) {
        return pc;
    } else {
        VCML_ERROR("Could not read program counter");
    }
}

vcml::u64 arm64_cpu::stack_pointer() {
    ocx::u64 sp_regid = m_core->sp_regid();
    vcml::u64 sp = 0;
    if (m_core->read_reg(sp_regid, &sp)) {
        return sp;
    } else {
        VCML_ERROR("Could not read stack pointer");
    }
}

vcml::u64 arm64_cpu::get_core_id() {
    return m_core_id;
}

void arm64_cpu::handle_syscall(int callno, std::shared_ptr<void> arg) {
    m_core->handle_syscall(callno, std::move(arg));
}

vcml::u64 arm64_cpu::get_page_size() {
    return m_core->page_size();
}

void arm64_cpu::end_of_elaboration() {
    for (auto it = timer_events.begin(); it != timer_events.end(); ++it) {
        std::unique_ptr<sc_core::sc_spawn_options> opts(
            new sc_core::sc_spawn_options());
        opts->spawn_method();
        opts->set_sensitivity((*it).get());
        opts->dont_initialize();
        std::stringstream ss;
        ss << (*it)->basename() << "_trigger";
        int index = std::distance(timer_events.begin(), it);
        sc_core::sc_spawn(sc_bind(&arm64_cpu::timer_irq_trigger, this, index),
                          sc_core::sc_gen_unique_name(ss.str().c_str()),
                          opts.get());
    }
    vcml::processor::end_of_elaboration();
}

arm64_cpu::arm64_cpu(const sc_core::sc_module_name& nm, vcml::u64 procid,
                     vcml::u64 coreid):
    vcml::processor(nm, CPU_ARCH),
    m_core(nullptr),
    m_core_id(coreid),
    m_run_cycles(0),
    m_sleep_cycles(0),
    m_total_cycles(0),
    m_syscall_subscriber(),
    timer_irq_out("TIMER_IRQ_OUT"),
    timer_events(4) {
    m_ocx_handle = dlopen("libocx-qemu-arm.so", RTLD_LAZY);
    if (!m_ocx_handle)
        VCML_ERROR("Could not load libocx-qemu-arm.so: %s", dlerror());
    m_create_instance_func = (create_instance_t)dlsym(
        m_ocx_handle, "_ZN3ocx15create_instanceEmRNS_3envEPKc");
    const char* dlsym_err = dlerror();
    if (dlsym_err)
        VCML_ERROR("Could not load symbol create_instance: %s", dlsym_err);

    m_core = m_create_instance_func(20201012ull, *this, "Cortex-A72");
    if (!m_core)
        VCML_ERROR("Could not create ocx::core instance");

    set_little_endian();
    define_cpuregs(AARCH64_REGS);

    m_core->set_id(procid, coreid);
    data.set_cpuid(m_core_id);
    insn.set_cpuid(m_core_id);
    timer_events[ARM_TIMER_PHYS] = std::shared_ptr<sc_core::sc_event>(
        new sc_core::sc_event("arm_timer_ns"));
    timer_events[ARM_TIMER_VIRT] = std::shared_ptr<sc_core::sc_event>(
        new sc_core::sc_event("arm_timer_virt"));
    timer_events[ARM_TIMER_HYP] = std::shared_ptr<sc_core::sc_event>(
        new sc_core::sc_event("arm_timer_hyp"));
    timer_events[ARM_TIMER_SEC] = std::shared_ptr<sc_core::sc_event>(
        new sc_core::sc_event("arm_timer_s"));
}

arm64_cpu::~arm64_cpu() {
    dlclose(m_ocx_handle);
}

} // namespace avp64
