/******************************************************************************
 *                                                                            *
 * Copyright 2020 Lukas Jünger                                                *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#include "avp64/arm64_cpu.h"
#include <dlfcn.h>

namespace avp64 {
    ocx::u8 *arm64_cpu_env::get_page_ptr_r(ocx::u64 page_paddr) {
        tlm::tlm_dmi dmi;
        vcml::u64 page_size = m_cpu->get_page_size();
        if (m_cpu->INSN.dmi().lookup(page_paddr, page_size, tlm::TLM_READ_COMMAND, dmi)) {
            return dmi.get_dmi_ptr() + page_paddr - dmi.get_start_address();
        } else {
            tlm::tlm_generic_payload tx;
            tx.set_address(page_paddr);
            tx.set_read();
            if (m_cpu->INSN->get_direct_mem_ptr(tx, dmi)) {
                m_cpu->INSN.map_dmi(dmi);
                return dmi.get_dmi_ptr() + page_paddr - dmi.get_start_address();
            } else {
                return nullptr;
            }
        }
    }

    ocx::u8 *arm64_cpu_env::get_page_ptr_w(ocx::u64 page_paddr) {
        tlm::tlm_dmi dmi;
        vcml::u64 page_size = m_cpu->get_page_size();
        if (m_cpu->INSN.dmi().lookup(page_paddr, page_size, tlm::TLM_WRITE_COMMAND, dmi)) {
            return dmi.get_dmi_ptr() + page_paddr - dmi.get_start_address();
        } else {
            tlm::tlm_generic_payload tx;
            tx.set_address(page_paddr);
            tx.set_write();
            if (m_cpu->INSN->get_direct_mem_ptr(tx, dmi)) {
                m_cpu->INSN.map_dmi(dmi);
                return dmi.get_dmi_ptr() + page_paddr - dmi.get_start_address();
            } else {
                return nullptr;
            }
        }
    }

    ocx::response arm64_cpu_env::transport(const ocx::transaction& tx) {
        vcml::sideband info = vcml::SBI_NONE;
        if (tx.is_debug)
            info |= vcml::SBI_DEBUG;
        if (tx.is_excl)
            info |= vcml::SBI_EXCL;
        info.cpuid = m_cpu->get_core_id();
        tlm::tlm_response_status resp = tlm::TLM_GENERIC_ERROR_RESPONSE;
        if (tx.is_read)
            resp = m_cpu->DATA.read(tx.addr, tx.data, tx.size, info);
        else
            resp = m_cpu->DATA.write(tx.addr, tx.data, tx.size, info);
        // FIXME this is an ugly hack to sync
        if (!tx.is_debug && vcml::is_thread()) {
            sc_core::wait(sc_core::SC_ZERO_TIME);
            sc_core::wait(sc_core::SC_ZERO_TIME);
        }
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

    void arm64_cpu_env::signal(ocx::u64 sigid, bool set) {
        m_cpu->TIMER_IRQ_OUT[sigid] = set;
    }

    void arm64_cpu_env::broadcast_syscall(int callno, void* arg) {
        m_cpu->handle_syscall(callno, arg);
        for (auto const& cpu: m_syscall_subscriber) {
            cpu->handle_syscall(callno, arg);
        }
    }

    ocx::u64 arm64_cpu_env::get_time_ps() {
        sc_core::sc_time tmp = sc_core::sc_time_stamp();
        uint64_t val_ps = (tmp.to_seconds()+0.5)*1e12;
        return val_ps;
    }

    const char* arm64_cpu_env::get_param(const char* name) {
        if (strcmp("gicv3", name) == 0)
            return "false";
        else
            VCML_ERROR("Unimplemented parameter requested");
    }

    void arm64_cpu_env::notify(ocx::u64 eventid, ocx::u64 time_ps) {
        sc_core::sc_time notify_time(time_ps, sc_core::SC_PS);
        sc_core::sc_time delta = notify_time-sc_core::sc_time_stamp();
        m_cpu->timer_events[eventid]->notify(delta);
    }

    void arm64_cpu_env::cancel(ocx::u64 eventid) {
        m_cpu->timer_events[eventid]->cancel();
    }

    void arm64_cpu_env::hint(ocx::hint_kind kind) {
        switch (kind) {
            case ocx::HINT_WFI: {
                sc_core::sc_event_or_list list;
                for (auto it : m_cpu->IRQ) {
                    list |= it.second->posedge_event();
                    // Treat WFI as NOP if IRQ is pending
                    if (it.second->read())
                        return;
                }
                vcml::log_debug("waiting for interrupt");
                sc_core::wait(list);
                break;
            }
            default:
                throw std::logic_error("Unimplemented hint");
        }
    }

    void arm64_cpu_env::handle_begin_basic_block(ocx::u64 vaddr) {
        throw std::logic_error("Not implemented");
    }

    bool arm64_cpu_env::handle_breakpoint(ocx::u64 vaddr) {
        m_cpu->gdb_notify(vcml::debugging::GDBSIG_TRAP);
        return true;
    }

    bool arm64_cpu_env::handle_watchpoint(ocx::u64 vaddr, ocx::u64 size, ocx::u64 data, bool iswr) {
        throw std::logic_error("Not implemented");
        return true;
    }

    void arm64_cpu_env::inject_cpu(arm64_cpu *cpu) {
        m_cpu = cpu;
    }

    void arm64_cpu_env::add_syscall_subscriber(std::shared_ptr<arm64_cpu> cpu) {
        m_syscall_subscriber.push_back(cpu);
    }

    arm64_cpu_env::arm64_cpu_env() :
        m_cpu(nullptr) {
        // Nothing to do
    }

   arm64_cpu_env::~arm64_cpu_env() {

    }

    void arm64_cpu::timer_irq_trigger(int timer_id) {
        m_core->notified(timer_id);
    }

    void arm64_cpu::interrupt(unsigned int irq, bool set) {
        m_core->interrupt(irq, set);
    }

    void arm64_cpu::simulate(unsigned int cycles) {
        ocx::u64 ran_cycles = m_core->step(cycles);
        m_cycle_count += ran_cycles;
    }

    vcml::u64 arm64_cpu::gdb_num_registers() {
        return 34;
    }

    vcml::u64 arm64_cpu::gdb_register_width(vcml::u64 idx) {
        return m_core->reg_size(idx);
    }

    bool arm64_cpu::gdb_read_reg(vcml::u64 idx, void* p, vcml::u64 size) {
        return m_core->read_reg(idx, p);
    }

    bool arm64_cpu::gdb_write_reg(vcml::u64 idx, const void* p, vcml::u64 sz) {
        return m_core->write_reg(idx, p);
    }

    bool arm64_cpu::gdb_page_size(vcml::u64& size) {
        size = m_core->page_size();
        return true;
    }
    bool arm64_cpu::gdb_virt_to_phys(vcml::u64 vaddr, vcml::u64& paddr) {
        return m_core->virt_to_phys(vaddr, paddr);
    }

    bool arm64_cpu::gdb_insert_breakpoint(vcml::u64 addr) {
        return m_core->add_breakpoint(addr);
    }

    bool arm64_cpu::gdb_remove_breakpoint(vcml::u64 addr) {
        return m_core->remove_breakpoint(addr);
    }

    bool arm64_cpu::gdb_insert_watchpoint(const vcml::range& mem, vcml::vcml_access acs) {
        if (acs==vcml::vcml_access::VCML_ACCESS_WRITE || acs==vcml::vcml_access::VCML_ACCESS_READ){
            bool isrw = (acs==vcml::vcml_access::VCML_ACCESS_WRITE) ? true : false;
            return  m_core->add_watchpoint(mem.start, mem.length(), isrw);
        } else{
            vcml::log_error("Unsupported watchpoint");
            return false;
        }
    }

    bool arm64_cpu::gdb_remove_watchpoint(const vcml::range& mem, vcml::vcml_access acs) {
        if (acs==vcml::vcml_access::VCML_ACCESS_WRITE || acs==vcml::vcml_access::VCML_ACCESS_READ){
            bool isrw = (acs==vcml::vcml_access::VCML_ACCESS_WRITE) ? true : false;
            return  m_core->remove_watchpoint(mem.start, mem.length(), isrw);
        } else{
            vcml::log_error("Unsupported remove watchpoint");
            return false;
        }
    }

    vcml::u64 arm64_cpu::cycle_count() const {
        return m_cycle_count;
    }
    std::string arm64_cpu::disassemble(vcml::u64&, unsigned char*) {
        //FIXME
        throw std::logic_error("Not implemented");
        return "blub";
    }
    vcml::u64 arm64_cpu::get_program_counter() {
        ocx::u64 pc_regid = m_core->pc_regid();
        vcml::u64 pc = 0;
        if(m_core->read_reg(pc_regid, &pc)) {
            return pc;
        } else {
            VCML_ERROR("Could not read program counter");
        }
    }
    vcml::u64 arm64_cpu::get_stack_pointer() {
        ocx::u64 sp_regid = m_core->sp_regid();
        vcml::u64 sp = 0;
        if(m_core->read_reg(sp_regid, &sp)) {
            return sp;
        } else {
            VCML_ERROR("Could not read stack pointer");
        }
    }

    vcml::u64 arm64_cpu::get_core_id() {
        return m_core_id;
    }

    void arm64_cpu::gdb_notify(int signal) {
        vcml::processor::gdb_notify(signal);
    }

    void arm64_cpu::handle_syscall(int callno, void *arg) {
        m_core->handle_syscall(callno, arg);
    }

    void arm64_cpu::add_syscall_subscriber(std::shared_ptr<arm64_cpu> cpu) {
        m_env.add_syscall_subscriber(cpu);
    }

    vcml::u64 arm64_cpu::get_page_size() {
        return m_core->page_size();
    }

    void arm64_cpu::end_of_elaboration() {
        for (auto it = timer_events.begin(); it != timer_events.end(); ++it) {
            std::unique_ptr<sc_core::sc_spawn_options> opts(new sc_core::sc_spawn_options());
            opts->spawn_method();
            opts->set_sensitivity((*it).get());
            opts->dont_initialize();
            std::stringstream ss;
            ss << (*it)->basename() << "_trigger";
            int index = std::distance(timer_events.begin(), it);
            sc_core::sc_spawn(
                    sc_bind(&arm64_cpu::timer_irq_trigger, this, index),
                    sc_core::sc_gen_unique_name(ss.str().c_str()),
                    opts.get()
            );
        }
        vcml::processor::end_of_elaboration();
    }

    arm64_cpu::arm64_cpu(const sc_core::sc_module_name &nm, vcml::u64 procid, vcml::u64 coreid) :
            vcml::processor(nm),
            m_core(nullptr),
            m_core_id(coreid),
            m_env(),
            m_cycle_count(0),
            TIMER_IRQ_OUT("TIMER_IRQ_OUT"),
            timer_events(4) {
        m_ocx_handle = dlopen("libocx-qemu-arm.so", RTLD_LAZY);
        if (!m_ocx_handle)
            VCML_ERROR("Could not load libocx-qemu-arm.so: %s", dlerror());
        m_create_instance_func = (create_instance_t) dlsym(m_ocx_handle, "_ZN3ocx15create_instanceEmRNS_3envEPKc");
        const char *dlsym_err = dlerror();
        if (dlsym_err)
            VCML_ERROR("Could not load symbol create_instance: %s", dlsym_err);

        m_core = m_create_instance_func(20191113ull, m_env, "Cortex-A72");
        if (!m_core)
            VCML_ERROR("Could not create ocx::core instance");
        m_env.inject_cpu(this);

        m_core->set_id(procid, coreid);
        DATA.set_cpuid(m_core_id);
        INSN.set_cpuid(m_core_id);
        timer_events[ARM_TIMER_PHYS] = std::shared_ptr<sc_core::sc_event>(new sc_core::sc_event("arm_timer_ns"));
        timer_events[ARM_TIMER_VIRT] = std::shared_ptr<sc_core::sc_event>(new sc_core::sc_event("arm_timer_virt"));
        timer_events[ARM_TIMER_HYP] = std::shared_ptr<sc_core::sc_event>(new sc_core::sc_event("arm_timer_hyp"));
        timer_events[ARM_TIMER_SEC] = std::shared_ptr<sc_core::sc_event>(new sc_core::sc_event("arm_timer_s"));
    }

    arm64_cpu::~arm64_cpu() {
        dlclose(m_ocx_handle);
    }

}
