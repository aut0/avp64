/******************************************************************************
 *                                                                            *
 * Copyright 2024 Lukas Jünger, Nils Bosbach                                  *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#ifndef AVP64_CORE_H
#define AVP64_CORE_H

#include "vcml.h"
#include "ocx/ocx.h"

#include <signal.h>

namespace avp64 {

class core;

class memory_protector
{
private:
    struct page_data {
        core* c;
        vcml::u64 page_addr;
        vcml::u64 page_size;
        bool locked;
    };
    std::map<vcml::u64, struct page_data> m_protected_pages;
    struct sigaction m_sa_orig;

    memory_protector();
    void segfault_handler_int(int sig, siginfo_t* si, void*);

public:
    static memory_protector& get_instance() {
        static memory_protector inst;
        return inst;
    }

    memory_protector(const memory_protector&) = delete;
    void operator=(memory_protector const&) = delete;
    virtual ~memory_protector();

    static void segfault_handler(int sig, siginfo_t* si, void*);
    void register_page(core* cpu, vcml::u64 page_addr, void* host_addr);
    bool notify_page(void* access_addr);
};

enum : size_t {
    INTERRUPT_IRQ = 0,
    INTERRUPT_FIQ = 1,
    INTERRUPT_VIRQ = 2,
    INTERRUPT_VFIQ = 3,
};

enum arm_generic_timer_type {
    ARM_TIMER_PHYS = 0,
    ARM_TIMER_VIRT = 1,
    ARM_TIMER_HYP = 2,
    ARM_TIMER_SEC = 3,
};

typedef ocx::core* (*create_instance_t)(ocx::u64, ocx::env&, const char*);

class core : public vcml::processor, private ocx::env
{
private:
    ocx::core* m_core;
    sc_core::sc_event m_irqev;
    vcml::u64 m_core_id;
    vcml::u64 m_run_cycles;
    vcml::u64 m_sleep_cycles;
    vcml::u64 m_total_cycles;
    bool m_transport;
    void* m_ocx_handle;
    create_instance_t m_create_instance_func;
    std::vector<std::shared_ptr<core>> m_syscall_subscriber;
    std::unordered_set<vcml::u64> m_update_mem;
    std::list<std::pair<int, std::shared_ptr<void>>> m_syscalls;
    bool m_bb_trace;

    void timer_irq_trigger(int timer_id);
    static void segfault_handler(int sig, siginfo_t* si, void* unused);

protected:
    virtual void interrupt(size_t irq, bool set) override;
    virtual void simulate(size_t cycles) override;
    virtual void end_of_elaboration() override;

    virtual bool read_reg_dbg(size_t regno, void* buf, size_t len) override;
    virtual bool write_reg_dbg(size_t regno, const void* buf,
                               size_t len) override;
    virtual bool page_size(vcml::u64& size) override;
    virtual bool virt_to_phys(vcml::u64 vaddr, vcml::u64& paddr) override;
    virtual bool insert_breakpoint(vcml::u64 addr) override;
    virtual bool remove_breakpoint(vcml::u64 addr) override;
    virtual bool insert_watchpoint(const vcml::range& mem,
                                   vcml::vcml_access acs) override;
    virtual bool remove_watchpoint(const vcml::range& mem,
                                   vcml::vcml_access acs) override;

public:
    using vcml::component::transport; // needed to not hide vcml transport
                                      // function by ocx transport

    vcml::gpio_initiator_array timer_irq_out;
    std::vector<std::shared_ptr<sc_core::sc_event>> timer_events;

    void log_timing_info() const;

    virtual ocx::u8* get_page_ptr_r(ocx::u64 page_paddr) override;
    virtual ocx::u8* get_page_ptr_w(ocx::u64 page_paddr) override;

    virtual void protect_page(ocx::u8* page_ptr, ocx::u64 page_addr) override;
    void memory_protector_update(vcml::u64 page_addr);

    virtual ocx::response transport(const ocx::transaction& tx) override;
    virtual void signal(ocx::u64 sigid, bool set) override;

    virtual void broadcast_syscall(int callno, std::shared_ptr<void> arg,
                                   bool async) override;

    virtual ocx::u64 get_time_ps() override;
    virtual const char* get_param(const char* name) override;

    virtual void notify(ocx::u64 eventid, ocx::u64 time_ps) override;
    virtual void cancel(ocx::u64 eventid) override;

    virtual void hint(ocx::hint_kind kind) override;

    virtual void handle_begin_basic_block(ocx::u64 vaddr) override;
    virtual bool handle_breakpoint(ocx::u64 vaddr) override;
    virtual bool handle_watchpoint(ocx::u64 vaddr, ocx::u64 size,
                                   ocx::u64 data, bool iswr) override;

    void inject_cpu(core* cpu);

    virtual vcml::u64 cycle_count() const override;
    virtual void update_local_time(sc_core::sc_time& local_time,
                                   sc_core::sc_process_b* proc) override;
    virtual bool disassemble(vcml::u8* ibuf, vcml::u64& addr,
                             std::string& code) override;
    virtual vcml::u64 program_counter() override;
    virtual vcml::u64 stack_pointer() override;
    virtual vcml::u64 core_id() override;

    void handle_syscall(int callno, std::shared_ptr<void> arg);
    void add_syscall_subscriber(const std::shared_ptr<core>& cpu);
    vcml::u64 get_page_size();

    core() = delete;
    core(const core&) = delete;
    explicit core(const sc_core::sc_module_name& name, vcml::u64 procid,
                  vcml::u64 coreid);
    virtual ~core() override;
    virtual const char* kind() const override { return "avp64::core"; }
};

} // namespace avp64
#endif
