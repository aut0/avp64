/******************************************************************************
 *                                                                            *
 * Copyright 2024 Lukas Jünger, Nils Bosbach                                  *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#include "avp64/core.h"

#include <dlfcn.h>
#include <sys/mman.h>
#include <stdexcept>

#define CPU_ARCH "aarch64"

namespace avp64 {

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
    get_instance().segfault_handler_int(sig, si, arg);
}

void mem_protector::segfault_handler_int(int sig, siginfo_t* si, void* arg) {
    if (!mem_protector::get_instance().notify_page(si->si_addr)) {
        m_sa_orig.sa_sigaction(sig, si, arg);
    }
}

void mem_protector::register_page(core* core, vcml::u64 page_addr,
                                  void* host_addr) {
    VCML_ERROR_ON(
        mwr::get_page_size() < core->get_page_size(),
        "host page size is smaller than the target one - not implemented");

    vcml::u64 target_page_bits = mwr::ctz(core->get_page_size());
    vcml::u64 page_number = mwr::extract(page_addr, target_page_bits,
                                         HOST_PAGE_BITS - target_page_bits);

    host_addr = reinterpret_cast<void*>(
        reinterpret_cast<vcml::u64>(host_addr) & HOST_PAGE_MASK);
    page_addr &= HOST_PAGE_MASK;

    auto it = m_protected_pages.find(reinterpret_cast<vcml::u64>(host_addr));
    if (it != m_protected_pages.end()) {
        it->second.page_number |= (1 << page_number);

        if (it->second.locked) {
            if (it->second.page_addr != page_addr) {
                vcml::log_error("page_addr do not match! %llu vs. %llu",
                                it->second.page_addr, page_addr);
            }
            return;
        }
        it->second.locked = true;
    } else {
        struct page_data&
            pd = m_protected_pages[reinterpret_cast<vcml::u64>(host_addr)];
        pd.c = core;
        pd.page_addr = page_addr;
        pd.page_number = 1 << page_number;
        pd.page_size = mwr::get_page_size();
        pd.locked = true;
    }
    VCML_ERROR_ON(::mprotect(host_addr, mwr::get_page_size(), PROT_READ) != 0,
                  "mprotect: %s", std::strerror(errno));
}

bool mem_protector::notify_page(void* access_addr) {
    vcml::u64 page_addr = reinterpret_cast<vcml::u64>(access_addr) &
                          HOST_PAGE_MASK;

    auto it = m_protected_pages.find(page_addr);
    if (it == m_protected_pages.end()) { // not a locked page
        return false;
    }

    vcml::u64 page_number = it->second.page_number;
    int i = 0;
    while (page_number != 0) {
        if (page_number & 1) {
            it->second.c->memory_protector_update(
                it->second.page_addr | (i << it->second.c->get_page_size()));
        }

        ++i;
        page_number >>= 1;
    }

    it->second.locked = false;
    return ::mprotect(reinterpret_cast<void*>(page_addr), it->second.page_size,
                      PROT_READ | PROT_WRITE) == 0;
}

ocx::u8* core::get_page_ptr_r(ocx::u64 page_paddr) {
    tlm::tlm_dmi dmi;
    vcml::u64 page_size = get_page_size();
    if (insn.dmi_cache().lookup(page_paddr, page_size, tlm::TLM_READ_COMMAND,
                                dmi)) {
        return dmi.get_dmi_ptr() + page_paddr - dmi.get_start_address();
    }

    tlm::tlm_generic_payload tx;
    tx.set_address(page_paddr);
    tx.set_streaming_width(page_size);
    tx.set_data_length(page_size);
    tx.set_read();
    if (insn->get_direct_mem_ptr(tx, dmi)) {
        insn.map_dmi(dmi);
        return dmi.get_dmi_ptr() + page_paddr - dmi.get_start_address();
    }
    return nullptr;
}

ocx::u8* core::get_page_ptr_w(ocx::u64 page_paddr) {
    tlm::tlm_dmi dmi;
    vcml::u64 page_size = get_page_size();
    if (insn.dmi_cache().lookup(page_paddr, page_size, tlm::TLM_WRITE_COMMAND,
                                dmi)) {
        return dmi.get_dmi_ptr() + page_paddr - dmi.get_start_address();
    }

    tlm::tlm_generic_payload tx;
    tx.set_address(page_paddr);
    tx.set_streaming_width(page_size);
    tx.set_data_length(page_size);
    tx.set_write();
    if (insn->get_direct_mem_ptr(tx, dmi)) {
        insn.map_dmi(dmi);
        return dmi.get_dmi_ptr() + page_paddr - dmi.get_start_address();
    }
    return nullptr;
}

void core::protect_page(ocx::u8* page_ptr, ocx::u64 page_addr) {
    mem_protector::get_instance().register_page(this, page_addr, page_ptr);
}

ocx::response core::transport(const ocx::transaction& tx) {
    m_transport = true;
    vcml::tlm_sbi info = vcml::SBI_NONE;
    if (tx.is_debug)
        info |= vcml::SBI_DEBUG;
    if (tx.is_excl)
        info |= vcml::SBI_EXCL;
    if (tx.is_secure)
        info |= vcml::SBI_SECURE;
    info.cpuid = core_id();
    tlm::tlm_response_status resp = tlm::TLM_GENERIC_ERROR_RESPONSE;

    resp = tx.is_read ? data.read(tx.addr, tx.data, tx.size, info)
                      : data.write(tx.addr, tx.data, tx.size, info);
    m_transport = false;

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
        log_info("Unexpected TLM transport response.");
        return ocx::RESP_FAILED;
    }
}

void core::log_timing_info() const {
    log_info("core %llu", m_core_id);
    log_info("  clock speed  : %.1f MHz", clk.read() / 1e6);
    log_info("  sim speed    : %.1f MIPS", get_cps() / 1e6);
    log_info("  run time     : %.1f s", get_run_time());
    log_info("  cycles       : %llu", cycle_count());
    log_info("  sleep cycles : %llu (%.1f %%)", m_sleep_cycles,
             static_cast<double>(m_sleep_cycles) * 100.0 /
                 static_cast<double>(m_sleep_cycles + cycle_count()));

    for (auto i : irq) {
        vcml::irq_stats stats;
        if (!get_irq_stats(i.first, stats) || stats.irq_count == 0)
            continue;

        std::string s;
        s += vcml::mkstr("  irq %lu status :", stats.irq);
        s += vcml::mkstr(" %lu #", stats.irq_count);
        s += vcml::mkstr(", avg %.1f us", stats.irq_uptime.to_seconds() /
                                              stats.irq_count * 1e6);
        s += vcml::mkstr(", max %.1f us",
                         stats.irq_longest.to_seconds() * 1e6);
        log_info("%s", s.c_str());
    }
}

void core::signal(ocx::u64 sigid, bool set) {
    if (timer_irq_out[sigid] != set) {
        timer_irq_out[sigid] = set;
    }
}

void core::broadcast_syscall(int callno, std::shared_ptr<void> arg,
                             bool async) {
    handle_syscall(callno, arg);
    for (auto const& cpu : m_syscall_subscriber) {
        cpu->handle_syscall(callno, arg);
    }
}

ocx::u64 core::get_time_ps() {
    return sc_core::sc_time_stamp().value() /
           sc_core::sc_time(1, sc_core::SC_PS).value();
}

const char* core::get_param(const char* name) {
    if (strcmp("gicv3", name) == 0)
        return "false";
    if (strcmp("tbsize", name) == 0)
        return "8MB";
    VCML_ERROR("Unimplemented parameter requested");
}

void core::notify(ocx::u64 eventid, ocx::u64 time_ps) {
    sc_core::sc_time notify_time = vcml::time_from_value(time_ps);
    sc_core::sc_time delta = notify_time - sc_core::sc_time_stamp();
    timer_events[eventid]->notify(delta);
}

void core::cancel(ocx::u64 eventid) {
    timer_events[eventid]->cancel();
}

void core::hint(ocx::hint_kind kind) {
    switch (kind) {
    case ocx::HINT_WFI: {
        sync();
        for (auto it : irq) {
            if (it.second->read())
                return;
        }
        const sc_core::sc_time before_wait = sc_core::sc_time_stamp();
        wait_for_interrupt(m_irqev);
        VCML_ERROR_ON(local_time() != sc_core::SC_ZERO_TIME,
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

void core::handle_begin_basic_block(ocx::u64 vaddr) {
    notify_basic_block(vaddr, 0, 0);
}

bool core::handle_breakpoint(ocx::u64 vaddr) {
    notify_breakpoint_hit(vaddr);
    return true;
}

bool core::handle_watchpoint(ocx::u64 vaddr, ocx::u64 size, ocx::u64 data,
                             bool iswr) {
    const vcml::range range(vaddr, vaddr + size);

    if (iswr)
        notify_watchpoint_write(range, data);
    else
        notify_watchpoint_read(range);
    return true;
}

void core::add_syscall_subscriber(const std::shared_ptr<core>& cpu) {
    m_syscall_subscriber.push_back(cpu);
}

void core::memory_protector_update(vcml::u64 page_addr) {
    m_core->tb_flush_page(page_addr, page_addr + get_page_size() - 1);
    m_core->invalidate_page_ptr(page_addr);
    for (auto const& cpu : m_syscall_subscriber) {
        cpu->m_core->tb_flush_page(page_addr, page_addr + get_page_size() - 1);
        cpu->m_core->invalidate_page_ptr(page_addr);
    }
}

void core::timer_irq_trigger(int timer_id) {
    m_core->notified(timer_id);
}

void core::interrupt(size_t irq, bool set) {
    m_core->interrupt(irq, set);
    m_irqev.notify();
}

void core::simulate(size_t cycles) {
    if (m_bb_trace != is_tracing_basic_blocks()) {
        m_bb_trace = is_tracing_basic_blocks();
        m_core->trace_basic_blocks(m_bb_trace);
    }

    // insn_count() is only reset at the beginning of step(), but not at
    // the end, so the number of cycles can only be summed up in the
    // following quantum
    m_run_cycles += m_core->insn_count();
    m_core->step(cycles);
}

bool core::read_reg_dbg(size_t regno, void* buf, size_t len) {
    return m_core->read_reg(regno, buf);
}

bool core::write_reg_dbg(size_t regno, const void* buf, size_t len) {
    if (regno == m_core->pc_regid()) {
        if (len != 8)
            return false;

        if (*reinterpret_cast<const vcml::u64*>(buf) == program_counter())
            return true;

        // the pc cannot be changed during an ongoing b_transport
        if (m_transport)
            return false;
    }
    return m_core->write_reg(regno, buf);
}

bool core::page_size(vcml::u64& size) {
    size = m_core->page_size();
    return true;
}

bool core::virt_to_phys(vcml::u64 vaddr, vcml::u64& paddr) {
    ocx::u64 paddr_ocx = paddr;
    bool ret_val = m_core->virt_to_phys(vaddr, paddr_ocx);
    paddr = paddr_ocx;
    return ret_val;
}

bool core::insert_breakpoint(vcml::u64 addr) {
    return m_core->add_breakpoint(addr);
}

bool core::remove_breakpoint(vcml::u64 addr) {
    return m_core->remove_breakpoint(addr);
}

bool core::insert_watchpoint(const vcml::range& mem, vcml::vcml_access acs) {
    switch (acs) {
    case vcml::vcml_access::VCML_ACCESS_READ:
        return m_core->add_watchpoint(mem.start, mem.length(), false);
    case vcml::vcml_access::VCML_ACCESS_WRITE:
        return m_core->add_watchpoint(mem.start, mem.length(), true);
    case vcml::vcml_access::VCML_ACCESS_READ_WRITE:
        return m_core->add_watchpoint(mem.start, mem.length(), true) &&
               m_core->add_watchpoint(mem.start, mem.length(), false);
    default:
        log_error("Unsupported watchpoint");
        return false;
    }
}

bool core::remove_watchpoint(const vcml::range& mem, vcml::vcml_access acs) {
    switch (acs) {
    case vcml::vcml_access::VCML_ACCESS_READ:
        return m_core->remove_watchpoint(mem.start, mem.length(), false);
    case vcml::vcml_access::VCML_ACCESS_WRITE:
        return m_core->remove_watchpoint(mem.start, mem.length(), true);
    case vcml::vcml_access::VCML_ACCESS_READ_WRITE:
        return m_core->remove_watchpoint(mem.start, mem.length(), true) &&
               m_core->remove_watchpoint(mem.start, mem.length(), false);
    default:
        log_error("Unsupported watchpoint");
        return false;
    }
}

vcml::u64 core::cycle_count() const {
    return m_run_cycles + m_core->insn_count();
}

void core::update_local_time(sc_core::sc_time& local_time,
                             sc_core::sc_process_b* proc) {
    if (is_local_process(proc)) {
        vcml::u64 cycles = cycle_count() + m_sleep_cycles;
        VCML_ERROR_ON(cycles < m_total_cycles, "cycle count goes down");
        local_time += clock_cycles(cycles - m_total_cycles);
        m_total_cycles = cycles;
    }
}

bool core::disassemble(vcml::u8* ibuf, vcml::u64& addr, std::string& code) {
    const size_t bufsz = 100;
    char buf[bufsz];
    vcml::u64 len;
    len = m_core->disassemble(addr, buf, bufsz);

    if (len == 0) {
        return false;
    }

    code = std::string(buf);
    addr += len;
    return true;
}

vcml::u64 core::program_counter() {
    ocx::u64 pc_regid = m_core->pc_regid();
    vcml::u64 pc = 0;
    VCML_ERROR_ON(!m_core->read_reg(pc_regid, &pc),
                  "Could not read program counter");
    return pc;
}

vcml::u64 core::stack_pointer() {
    ocx::u64 sp_regid = m_core->sp_regid();
    vcml::u64 sp = 0;
    VCML_ERROR_ON(!m_core->read_reg(sp_regid, &sp),
                  "Could not read stack pointer");
    return sp;
}

vcml::u64 core::core_id() {
    return m_core_id;
}

void core::handle_syscall(int callno, std::shared_ptr<void> arg) {
    m_core->handle_syscall(callno, std::move(arg));
}

vcml::u64 core::get_page_size() {
    return m_core->page_size();
}

void core::end_of_elaboration() {
    for (auto it = timer_events.begin(); it != timer_events.end(); ++it) {
        std::unique_ptr<sc_core::sc_spawn_options> opts(
            new sc_core::sc_spawn_options());
        opts->spawn_method();
        opts->set_sensitivity((*it).get());
        opts->dont_initialize();
        std::stringstream ss;
        ss << (*it)->basename() << "_trigger";
        int index = std::distance(timer_events.begin(), it);
        sc_core::sc_spawn(sc_bind(&core::timer_irq_trigger, this, index),
                          sc_core::sc_gen_unique_name(ss.str().c_str()),
                          opts.get());
    }
    vcml::processor::end_of_elaboration();
}

void core::load_symbols() {
    for (const std::string& s : symbols) {
        std::string symfile = mwr::trim(s);
        if (symfile.empty())
            continue;

        if (!mwr::file_exists(symfile)) {
            log_warn("cannot open file '%s'", symfile.c_str());
            continue;
        }

        vcml::u64 n = load_symbols_from_elf(symfile);
        log_debug("loaded %llu symbols from '%s'", n, symfile.c_str());
    }
}

core::core(const sc_core::sc_module_name& nm, vcml::u64 procid,
           vcml::u64 coreid):
    vcml::processor(nm, CPU_ARCH),
    m_core(nullptr),
    m_irqev("irqev"),
    m_core_id(coreid),
    m_run_cycles(0),
    m_sleep_cycles(0),
    m_total_cycles(0),
    m_transport(false),
    m_syscall_subscriber(),
    m_update_mem(),
    m_syscalls(),
    m_bb_trace(false),
    timer_irq_out("TIMER_IRQ_OUT"),
    timer_events(4) {
    symbols.inherit_default();
    async.inherit_default();
    async_rate.inherit_default();

    if (symbols.is_default() && !symbols.get().empty())
        load_symbols();

    m_ocx_handle = dlopen("libocx-qemu-arm.so", RTLD_LAZY);
    VCML_ERROR_ON(!m_ocx_handle, "Could not load libocx-qemu-arm.so: %s",
                  dlerror());

    create_instance_t create_instance_func = (create_instance_t)dlsym(
        m_ocx_handle, "_ZN3ocx15create_instanceEmRNS_3envEPKc");

    const char* dlsym_err = dlerror();
    VCML_ERROR_ON(dlsym_err, "Could not load symbol create_instance: %s",
                  dlsym_err);

    m_core = create_instance_func(20201012ull, *this, "Cortex-A72");
    VCML_ERROR_ON(!m_core, "Could not create ocx::core instance");

    set_little_endian();

    // X0 - X30
    for (id_t i = 0; i < 31; ++i)
        define_cpureg_rw(i, mwr::mkstr("X%u", i), 8);

    define_cpureg_rw(31, "SP", 8);
    define_cpureg_rw(32, "PC", 8);

    // aarch64 status register and bitfields
    define_cpureg_rw(33, "CPSR", 4);
    define_cpureg_rw(50, "SPSR_EL1", 4);
    define_cpureg_rw(64, "SPSR_EL2", 4);
    define_cpureg_rw(78, "SPSR_EL3", 4);
    define_cpureg_rw(92, "SP_EL0", 8);
    define_cpureg_rw(93, "SP_EL1", 8);
    define_cpureg_rw(94, "SP_EL2", 8);
    define_cpureg_rw(95, "SP_EL3", 8);
    define_cpureg_rw(96, "ELR_EL0", 8);
    define_cpureg_rw(97, "ELR_EL1", 8);
    define_cpureg_rw(98, "ELR_EL2", 8);
    define_cpureg_rw(99, "ELR_EL3", 8);

    // aarch64 floating point registers
    for (id_t i = 0; i < 32; ++i)
        define_cpureg_rw(i + 449, mwr::mkstr("D%u", i), 8);

    // aarch64 floating point status registers
    define_cpureg_rw(192, "FPSR", 4);
    define_cpureg_rw(193, "FPCR", 4);

    m_core->set_id(procid, coreid);
    data.set_cpuid(m_core_id);
    insn.set_cpuid(m_core_id);
    timer_events[ARM_TIMER_PHYS] = std::make_shared<sc_core::sc_event>(
        "arm_timer_ns");
    timer_events[ARM_TIMER_VIRT] = std::make_shared<sc_core::sc_event>(
        "arm_timer_virt");
    timer_events[ARM_TIMER_HYP] = std::make_shared<sc_core::sc_event>(
        "arm_timer_hyp");
    timer_events[ARM_TIMER_SEC] = std::make_shared<sc_core::sc_event>(
        "arm_timer_s");
}

core::~core() {
    dlclose(m_ocx_handle);
}

} // namespace avp64
