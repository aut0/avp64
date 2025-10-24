/******************************************************************************
 *                                                                            *
 * Copyright 2024 Lukas Jünger, Nils Bosbach                                  *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#include "avp64/cpu.h"
#include "avp64/version.h"

namespace avp64 {

cpu::cpu(const sc_core::sc_module_name& nm):
    vcml::component(nm),
    ncores("ncores", 1),
    clusterid("clusterid", 0),
    symbols("symbols"),
    async("async", false),
    async_rate("async_rate", 10),
    gic_cpuif("addr_gic_cpuif", { GIC_CPUIF_LO, GIC_CPUIF_HI }),
    gic_distif("addr_gic_distif", { GIC_DISTIF_LO, GIC_DISTIF_HI }),
    gic_vifctrl("addr_gic_vifctrl", { GIC_VIFCTRL_LO, GIC_VIFCTRL_HI }),
    gic_vcpuif("addr_gic_vcpuif", { GIC_VCPUIF_LO, GIC_VCPUIF_HI }),
    irq_gt_hyp("irq_gt_hyp", PPI_GT_HYP),
    irq_gt_virt("irq_gt_virt", PPI_GT_VIRT),
    irq_gt_ns("irq_gt_ns", PPI_GT_NS),
    irq_gt_s("irq_gt_s", PPI_GT_S),
    gdb_wait("gdb_wait", false),
    gdb_echo("gdb_echo", false),
    gdb_port("gdb_port", gdb_wait ? 0 : -1),
    bus("bus"),
    spi("spi"),
    m_cores(),
    m_gic("gic"),
    m_corebus("corebus"),
    m_gdb(nullptr) {
    m_cores.resize(ncores);

    // initialize cores and bind interrupts
    for (size_t id = 0; id < ncores; ++id) {
        auto nm = mwr::mkstr("arm%zu", id);
        m_cores[id] = std::make_shared<core>(nm.c_str(), clusterid, id);

        m_cores[id]->irq[core::INTERRUPT_IRQ].bind(m_gic.irq_out[id]);
        m_cores[id]->irq[core::INTERRUPT_FIQ].bind(m_gic.fiq_out[id]);
        m_cores[id]->irq[core::INTERRUPT_VIRQ].bind(m_gic.virq_out[id]);
        m_cores[id]->irq[core::INTERRUPT_VFIQ].bind(m_gic.vfiq_out[id]);

        m_cores[id]->timer_irq_out[core::ARM_TIMER_PHYS].bind(
            m_gic.ppi(id, irq_gt_ns));
        m_cores[id]->timer_irq_out[core::ARM_TIMER_VIRT].bind(
            m_gic.ppi(id, irq_gt_virt));
        m_cores[id]->timer_irq_out[core::ARM_TIMER_HYP].bind(
            m_gic.ppi(id, irq_gt_hyp));
        m_cores[id]->timer_irq_out[core::ARM_TIMER_SEC].bind(
            m_gic.ppi(id, irq_gt_s));

        for (size_t core = 0; core < id; ++core) {
            m_cores[core]->add_syscall_subscriber(m_cores[id]);
            m_cores[id]->add_syscall_subscriber(m_cores[core]);
        }

        m_corebus.bind(m_cores[id]->data);
        m_corebus.bind(m_cores[id]->insn);

        clk.bind(m_cores[id]->clk);
        rst.bind(m_cores[id]->rst);
    }

    m_corebus.bind(m_gic.cpuif.in, gic_cpuif);
    m_corebus.bind(m_gic.distif.in, gic_distif);
    m_corebus.bind(m_gic.vifctrl.in, gic_vifctrl);
    m_corebus.bind(m_gic.vcpuif.in, gic_vcpuif);

    m_corebus.bind_default(bus);

    clk.bind(m_corebus.clk);
    clk.bind(m_gic.clk);

    rst.bind(m_corebus.rst);
    rst.bind(m_gic.rst);
}

void cpu::before_end_of_elaboration() {
    vcml::component::before_end_of_elaboration();

    for (auto [idx, port] : spi) {
        log_debug("binding spi %zu to gic", idx);
        port->bind(m_gic.spi_in[idx]);
    }
}

void cpu::end_of_elaboration() {
    component::end_of_elaboration();

    if (gdb_port >= 0) {
        auto run = gdb_wait ? vcml::debugging::GDB_STOPPED
                            : vcml::debugging::GDB_RUNNING;
        vector<vcml::debugging::target*> tgts;

        tgts.reserve(m_cores.size());
        for (const auto& c : m_cores)
            tgts.push_back(c.get());

        m_gdb = std::make_unique<vcml::debugging::gdbserver>(
            gdb_port, std::move(tgts), run);
        m_gdb->echo(gdb_echo);

        if (gdb_port == 0)
            gdb_port = m_gdb->port();

        log_info("%s for GDB connection on port %hu",
                 gdb_wait ? "waiting" : "listening", m_gdb->port());
    }
}

void cpu::end_of_simulation() {
    vcml::component::end_of_simulation();

    for (const auto& c : m_cores)
        c->log_timing_info();

    log_info("total - cluster %zu", clusterid.get());
    log_info("  instructions : %llu", cycle_count());
}

vcml::u64 cpu::cycle_count() const {
    vcml::u64 total_insn = 0;

    for (const auto& c : m_cores)
        total_insn += c->cycle_count();

    return total_insn;
}

const char* cpu::version() const {
    return AVP64_VERSION_STRING;
}

VCML_EXPORT_MODEL(avp64::cpu, name, args) {
    return new avp64::cpu(name);
}

} // namespace avp64
