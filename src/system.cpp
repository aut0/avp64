/******************************************************************************
 *                                                                            *
 * Copyright 2020 Lukas Jünger                                                *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#include <inttypes.h>
#include "avp64/system.h"
#include "avp64/config.h"

#define SAFE_DELETE(p)  \
    do {                \
        if (p)          \
            delete (p); \
        (p) = NULL;     \
    } while (0)

#define AVP64_ARM64_CPU_IRQ  (0)
#define AVP64_ARM64_CPU_FIQ  (1)
#define AVP64_ARM64_CPU_VIRQ (2)
#define AVP64_ARM64_CPU_VFIQ (3)

namespace avp64 {

void system::construct_system_arm64() {
    m_cpus.resize(nrcpu);
    for (unsigned int cpu = 0; cpu < nrcpu; cpu++) {
        std::stringstream ss;
        ss << "arm" << cpu;
        m_cpus[cpu] = std::make_shared<arm64_cpu>(ss.str().c_str(), 0, cpu);
        std::string irq_name("irq_" + ss.str());
        std::string fiq_name("fiq_" + ss.str());
        std::string virq_name("virq_" + ss.str());
        std::string vfiq_name("vfiq_" + ss.str());
        std::string irq_gt_hyp_name("irq_gt_hyp_" + ss.str());
        std::string irq_gt_virt_name("irq_gt_virt_" + ss.str());
        std::string irq_gt_ns_name("irq_gt_ns_" + ss.str());
        std::string irq_gt_s_name("irq_gt_s_" + ss.str());

        m_cpus[cpu]->irq[AVP64_ARM64_CPU_IRQ].bind(m_gic.irq_out[cpu]);
        m_cpus[cpu]->irq[AVP64_ARM64_CPU_FIQ].bind(m_gic.fiq_out[cpu]);
        m_cpus[cpu]->irq[AVP64_ARM64_CPU_VIRQ].bind(m_gic.virq_out[cpu]);
        m_cpus[cpu]->irq[AVP64_ARM64_CPU_VFIQ].bind(m_gic.vfiq_out[cpu]);
        m_cpus[cpu]->timer_irq_out[avp64::ARM_TIMER_PHYS].bind(
            m_gic.ppi_in[irq_gt_ns - 16 + (cpu * 16)]);
        m_cpus[cpu]->timer_irq_out[avp64::ARM_TIMER_VIRT].bind(
            m_gic.ppi_in[irq_gt_virt - 16 + (cpu * 16)]);
        m_cpus[cpu]->timer_irq_out[avp64::ARM_TIMER_HYP].bind(
            m_gic.ppi_in[irq_gt_hyp - 16 + (cpu * 16)]);
        m_cpus[cpu]->timer_irq_out[avp64::ARM_TIMER_SEC].bind(
            m_gic.ppi_in[irq_gt_s - 16 + (cpu * 16)]);
    }
    for (unsigned int cpu = 0; cpu < nrcpu; cpu++) {
        for (unsigned int i = 0; i < nrcpu; i++) {
            if (i == cpu)
                continue;
            else
                m_cpus[i]->add_syscall_subscriber(m_cpus[cpu]);
        }
    }

    // Clock
    for (const auto& cpu : m_cpus) {
        m_clock.clk.bind(cpu->clk);
    }
    m_clock.clk.bind(m_bus.clk);
    m_clock.clk.bind(m_ram.clk);
    m_clock.clk.bind(m_gic.clk);
    m_clock.clk.bind(m_uart0.clk);
    m_clock.clk.bind(m_uart1.clk);
    m_clock.clk.bind(m_uart2.clk);
    m_clock.clk.bind(m_uart3.clk);
    m_clock.clk.bind(m_ethoc.clk);
    m_clock.clk.bind(m_sdcard.clk);
    m_clock.clk.bind(m_sdhci.clk);
    m_clock.clk.bind(m_simdev.clk);
    m_clock.clk.bind(m_hwrng.clk);
    m_clock.clk.bind(m_spibus.clk);
    m_clock.clk.bind(m_spi.clk);
    m_clock.clk.bind(m_gpio.clk);

    // Reset
    for (const auto& cpu : m_cpus) {
        m_reset.rst.bind(cpu->rst);
    }
    m_reset.rst.bind(m_bus.rst);
    m_reset.rst.bind(m_ram.rst);
    m_reset.rst.bind(m_gic.rst);
    m_reset.rst.bind(m_uart0.rst);
    m_reset.rst.bind(m_uart1.rst);
    m_reset.rst.bind(m_uart2.rst);
    m_reset.rst.bind(m_uart3.rst);
    m_reset.rst.bind(m_ethoc.rst);
    m_reset.rst.bind(m_sdcard.rst);
    m_reset.rst.bind(m_sdhci.rst);
    m_reset.rst.bind(m_simdev.rst);
    m_reset.rst.bind(m_hwrng.rst);
    m_reset.rst.bind(m_spibus.rst);
    m_reset.rst.bind(m_spi.rst);
    m_reset.rst.bind(m_gpio.rst);

    // Bus bindings
    for (const auto& cpu : m_cpus) {
        m_bus.bind(cpu->insn);
        m_bus.bind(cpu->data);
    }
    m_bus.bind(m_ram.in, addr_ram);
    m_bus.bind(m_uart0.in, addr_uart0);
    m_bus.bind(m_uart1.in, addr_uart1);
    m_bus.bind(m_uart2.in, addr_uart2);
    m_bus.bind(m_uart3.in, addr_uart3);
    m_bus.bind(m_ethoc.in, addr_ethoc.get());
    m_bus.bind(m_ethoc.out);
    m_bus.bind(m_sdhci.in, addr_sdhci.get());
    m_bus.bind(m_sdhci.out);
    m_bus.bind(m_gic.cpuif.in, addr_gic_cpuif);
    m_bus.bind(m_gic.distif.in, addr_gic_distif);
    m_bus.bind(m_gic.vifctrl.in, addr_gic_vifctrl);
    m_bus.bind(m_gic.vcpuif.in, addr_gic_vcpuif);
    m_bus.bind(m_simdev.in, addr_simdev);
    m_bus.bind(m_hwrng.in, addr_hwrng);
    m_bus.bind(m_spi.in, addr_spi);
    m_bus.bind(m_gpio.in, addr_gpio);

    // Connect network to eth
    m_net.connect(m_ethoc);
    m_net.connect(m_bridge);

    // Connect terminals to uarts
    m_term0.connect(m_uart0);
    m_term1.connect(m_uart1);
    m_term2.connect(m_uart2);
    m_term3.connect(m_uart3);

    // Bind SDHCI and SDCARD
    m_sdhci.sd_out.bind(m_sdcard.sd_in);

    m_spi.spi_out.bind(m_spibus.spi_in);
    m_spibus.bind(m_max31855.spi_in, m_gpio.gpio_out[0],
                  false);                       // CS_ACTIVE_LOW
    m_max31855.bind(m_gpio.gpio_out[0], false); // CS_ACTIVE_LOW

    // IRQs
    m_gic.spi_in[irq_uart0].bind(m_uart0.irq);
    m_gic.spi_in[irq_uart1].bind(m_uart1.irq);
    m_gic.spi_in[irq_uart2].bind(m_uart2.irq);
    m_gic.spi_in[irq_uart3].bind(m_uart3.irq);
    m_gic.spi_in[irq_ethoc].bind(m_ethoc.irq);
    m_gic.spi_in[irq_sdhci].bind(m_sdhci.irq);
    m_gic.spi_in[irq_spi].bind(m_spi.irq);
}

system::system(const sc_core::sc_module_name& nm):
    vcml::system(nm),
    type("type", "unknown"),
    duration("duration", sc_core::SC_ZERO_TIME),
    quantum("quantum", sc_core::sc_time(10.0, sc_core::SC_US)),
    session("session", 0),
    vspdebug("vspdebug", false),
    nrcpu("nrcpu", 1),
    clock("clock", AVP64_CPU_DEFCLK),
    addr_ram("addr_ram", vcml::range(AVP64_RAM_ADDR, AVP64_RAM_HIGH)),
    addr_gic_cpuif("addr_gic_cpuif",
                   vcml::range(AVP64_GIC_CPUIF_ADDR, AVP64_GIC_CPUIF_HIGH)),
    addr_gic_distif("addr_gic_distif",
                    vcml::range(AVP64_GIC_DISTIF_ADDR, AVP64_GIC_DISTIF_HIGH)),
    addr_gic_vifctrl("addr_gic_vifctrl", vcml::range(AVP64_GIC_VIFCTRL_ADDR,
                                                     AVP64_GIC_VIFCTRL_HIGH)),
    addr_gic_vcpuif("addr_gic_vcpuif",
                    vcml::range(AVP64_GIC_VCPUIF_ADDR, AVP64_GIC_VCPUIF_HIGH)),
    addr_uart0("addr_uart0", vcml::range(AVP64_UART0_ADDR, AVP64_UART0_HIGH)),
    addr_uart1("addr_uart1", vcml::range(AVP64_UART1_ADDR, AVP64_UART1_HIGH)),
    addr_uart2("addr_uart2", vcml::range(AVP64_UART2_ADDR, AVP64_UART2_HIGH)),
    addr_uart3("addr_uart3", vcml::range(AVP64_UART3_ADDR, AVP64_UART3_HIGH)),
    addr_ethoc("addr_ethoc", vcml::range(AVP64_ETHOC_ADDR, AVP64_ETHOC_HIGH)),
    addr_sdhci("addr_sdhci", vcml::range(AVP64_SDHCI_ADDR, AVP64_SDHCI_HIGH)),
    addr_simdev("addr_simdev",
                vcml::range(AVP64_SIMDEV_ADDR, AVP64_SIMDEV_HIGH)),
    addr_hwrng("addr_hwrng", vcml::range(AVP64_HWRNG_ADDR, AVP64_HWRNG_HIGH)),
    addr_spi("addr_spi", vcml::range(AVP64_SPI_ADDR, AVP64_SPI_HIGH)),
    addr_gpio("addr_gpio", vcml::range(AVP64_GPIO_ADDR, AVP64_GPIO_HIGH)),
    irq_uart0("irq_uart0", AVP64_IRQ_UART0),
    irq_uart1("irq_uart1", AVP64_IRQ_UART1),
    irq_uart2("irq_uart2", AVP64_IRQ_UART2),
    irq_uart3("irq_uart3", AVP64_IRQ_UART3),
    irq_ethoc("irq_ethoc", AVP64_IRQ_ETHOC),
    irq_sdhci("irq_sdhci", AVP64_IRQ_SDHCI),
    irq_gt_hyp("irq_gt_hyp", AVP64_IRQ_GT_HYP),
    irq_gt_virt("irq_gt_virt", AVP64_IRQ_GT_VIRT),
    irq_gt_ns("irq_gt_ns", AVP64_IRQ_GT_NS),
    irq_gt_s("irq_gt_s", AVP64_IRQ_GT_S),
    irq_spi("irq_spi", AVP64_IRQ_SPI),
    m_cpus(),
    m_clock("clock", clock),
    m_reset("reset"),
    m_bus("bus"),
    m_ram("ram", addr_ram.get().length()),
    m_gic("gic"),
    m_uart0("uart0"),
    m_uart1("uart1"),
    m_uart2("uart2"),
    m_uart3("uart3"),
    m_term0("term0"),
    m_term1("term1"),
    m_term2("term2"),
    m_term3("term3"),
    m_ethoc("ethoc"),
    m_net("net"),
    m_bridge("bridge"),
    m_sdcard("sdcard"),
    m_sdhci("sdhci"),
    m_simdev("simdev"),
    m_hwrng("hwrng"),
    m_spibus("spibus"),
    m_spi("spi"),
    m_gpio("gpio"),
    m_max31855("max31855") {
    construct_system_arm64();
}

int system::run() {
    double simstart = vcml::realtime();
    int result = vcml::system::run();
    double realtime = vcml::realtime() - simstart;
    double total_runtime = sc_core::sc_time_stamp().to_seconds();

    uint64_t ninsn = 0;
    for (unsigned int cpu = 0; cpu < nrcpu; cpu++) {
        double mhz = m_cpus[cpu]->clk.read() * 1e-6;
        double mips = m_cpus[cpu]->get_cps() * 1e-6; // approximated
        double cpurt = m_cpus[cpu]->get_run_time();
        uint64_t cycles = m_cpus[cpu]->cycle_count();
        ninsn += cycles;

        vcml::log_info("core %d", cpu);
        vcml::log_info("  clock speed  : %.1f MHz", mhz);
        vcml::log_info("  sim speed    : %.1f MIPS", mips);
        vcml::log_info("  run time     : %.1fs", cpurt);
        vcml::log_info("  cycles       : %" PRId64, cycles);

        for (auto irq : m_cpus[cpu]->irq) {
            vcml::irq_stats stats;
            if (!m_cpus[cpu]->get_irq_stats(irq.first, stats) ||
                stats.irq_count == 0)
                continue;

            std::string s;
            s += vcml::mkstr("  irq %d status :", stats.irq);
            s += vcml::mkstr(" %d#", stats.irq_count);
            s += vcml::mkstr(", avg %.1fus", stats.irq_uptime.to_seconds() /
                                                 stats.irq_count * 1e6);
            s += vcml::mkstr(", max %.1fus",
                             stats.irq_longest.to_seconds() * 1e6);
            vcml::log_info("%s", s.c_str());
        }
    }

    double mips = realtime == 0.0 ? 0.0 : ninsn / realtime / 1e6;
    vcml::log_info("total");
    vcml::log_info("  duration     : %.9fs", total_runtime);
    vcml::log_info("  run time     : %.4fs", realtime);
    vcml::log_info("  instructions : %" PRId64, ninsn);
    vcml::log_info("  sim speed    : %.1f MIPS", mips);

    return result;
}

void system::end_of_elaboration() {
    std::stringstream ss;
    m_bus.execute("show", VCML_NO_ARGS, ss);
    log_debug("%s", ss.str().c_str());

    ss.str("");
    ss.clear();

    m_gic.execute("show", VCML_NO_ARGS, ss);
    log_debug("%s", ss.str().c_str());
}
} // namespace avp64
