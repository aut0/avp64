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

#define SAFE_DELETE(p)       do { if (p) delete    (p); (p) = NULL; } while (0)

#define AVP64_ARM64_CPU_IRQ  (0)
#define AVP64_ARM64_CPU_FIQ  (1)
#define AVP64_ARM64_CPU_VIRQ (2)
#define AVP64_ARM64_CPU_VFIQ (3)

namespace avp64 {

    void system::construct_system_arm64() {
        m_cpus.resize(nrcpu);
        for(unsigned int cpu = 0; cpu < nrcpu; cpu++) {
            std::stringstream ss; ss << "arm" << cpu;
            m_cpus[cpu] = std::make_shared<arm64_cpu>(ss.str().c_str(), 0, cpu);
            std::string irq_name("irq_" + ss.str());
            std::string fiq_name("fiq_" + ss.str());
            std::string virq_name("virq_" + ss.str());
            std::string vfiq_name("vfiq_" + ss.str());
            std::string irq_gt_hyp_name("irq_gt_hyp_" + ss.str());
            std::string irq_gt_virt_name("irq_gt_virt_" + ss.str());
            std::string irq_gt_ns_name("irq_gt_ns_" + ss.str());
            std::string irq_gt_s_name("irq_gt_s_" + ss.str());

            m_cpus[cpu]->IRQ[AVP64_ARM64_CPU_IRQ].bind(m_gic.IRQ_OUT[cpu]);
            m_cpus[cpu]->IRQ[AVP64_ARM64_CPU_FIQ].bind(m_gic.FIQ_OUT[cpu]);
            m_cpus[cpu]->IRQ[AVP64_ARM64_CPU_VIRQ].bind(m_gic.VIRQ_OUT[cpu]);
            m_cpus[cpu]->IRQ[AVP64_ARM64_CPU_VFIQ].bind(m_gic.VFIQ_OUT[cpu]);
            m_cpus[cpu]->TIMER_IRQ_OUT[avp64::ARM_TIMER_PHYS].bind(m_gic.PPI_IN[irq_gt_ns-16+(cpu*16)]);
            m_cpus[cpu]->TIMER_IRQ_OUT[avp64::ARM_TIMER_VIRT].bind(m_gic.PPI_IN[irq_gt_virt-16+(cpu*16)]);
            m_cpus[cpu]->TIMER_IRQ_OUT[avp64::ARM_TIMER_HYP].bind(m_gic.PPI_IN[irq_gt_hyp-16+(cpu*16)]);
            m_cpus[cpu]->TIMER_IRQ_OUT[avp64::ARM_TIMER_SEC].bind(m_gic.PPI_IN[irq_gt_s-16+(cpu*16)]);
        }
        for(unsigned int cpu = 0; cpu < nrcpu; cpu++) {
            for(unsigned int i=0; i < nrcpu; i++) {
                if (i==cpu)
                    continue;
                else
                    m_cpus[i]->add_syscall_subscriber(m_cpus[cpu]);
            }
        }

        // Clock
        for (auto cpu : m_cpus) {
            cpu->CLOCK.bind(m_sig_clock);
        }
        m_clock.CLOCK.bind(m_sig_clock);
        m_bus.CLOCK.bind(m_sig_clock);
        m_ram.CLOCK.bind(m_sig_clock);
        m_gic.CLOCK.bind(m_sig_clock);
        m_uart0.CLOCK.bind(m_sig_clock);
        m_uart1.CLOCK.bind(m_sig_clock);
        m_uart2.CLOCK.bind(m_sig_clock);
        m_uart3.CLOCK.bind(m_sig_clock);
        m_ethoc.CLOCK.bind(m_sig_clock);
        m_sdcard.CLOCK.bind(m_sig_clock);
        m_sdhci.CLOCK.bind(m_sig_clock);
        m_simdev.CLOCK.bind(m_sig_clock);
        m_hwrng.CLOCK.bind(m_sig_clock);

        // Reset
        for (auto cpu : m_cpus) {
            cpu->RESET.bind(m_sig_reset);
        }
        m_reset.RESET.bind(m_sig_reset);
        m_bus.RESET.bind(m_sig_reset);
        m_ram.RESET.bind(m_sig_reset);
        m_gic.RESET.bind(m_sig_reset);
        m_uart0.RESET.bind(m_sig_reset);
        m_uart1.RESET.bind(m_sig_reset);
        m_uart2.RESET.bind(m_sig_reset);
        m_uart3.RESET.bind(m_sig_reset);
        m_ethoc.RESET.bind(m_sig_reset);
        m_sdcard.RESET.bind(m_sig_reset);
        m_sdhci.RESET.bind(m_sig_reset);
        m_simdev.RESET.bind(m_sig_reset);
        m_hwrng.RESET.bind(m_sig_reset);

        // Bus bindings
        for (auto cpu : m_cpus) {
            m_bus.bind(cpu->INSN);
            m_bus.bind(cpu->DATA);
        }
        m_bus.bind(m_ram.IN, addr_ram);
        m_bus.bind(m_uart0.IN, addr_uart0);
        m_bus.bind(m_uart1.IN, addr_uart1);
        m_bus.bind(m_uart2.IN, addr_uart2);
        m_bus.bind(m_uart3.IN, addr_uart3);
        m_bus.bind(m_ethoc.IN, addr_ethoc.get());
        m_bus.bind(m_ethoc.OUT);
        m_bus.bind(m_sdhci.IN, addr_sdhci.get());
        m_bus.bind(m_sdhci.OUT);
        m_bus.bind(m_gic.CPUIF.IN, addr_gic_cpuif);
        m_bus.bind(m_gic.DISTIF.IN, addr_gic_distif);
        m_bus.bind(m_gic.VIFCTRL.IN, addr_gic_vifctrl);
        m_bus.bind(m_gic.VCPUIF.IN, addr_gic_vcpuif);
        m_bus.bind(m_simdev.IN, addr_simdev);
        m_bus.bind(m_hwrng.IN, addr_hwrng);

        // Bind SDHCI and SDCARD
        m_sdhci.SD_OUT.bind(m_sdcard.SD_IN);

        // IRQs
        m_gic.SPI_IN[irq_uart0].bind(m_uart0.IRQ);
        m_gic.SPI_IN[irq_uart1].bind(m_uart1.IRQ);
        m_gic.SPI_IN[irq_uart2].bind(m_uart2.IRQ);
        m_gic.SPI_IN[irq_uart3].bind(m_uart3.IRQ);
        m_gic.SPI_IN[irq_ethoc].bind(m_ethoc.IRQ);
        m_gic.SPI_IN[irq_sdhci].bind(m_sdhci.IRQ);
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
        addr_gic_cpuif("addr_gic_cpuif", vcml::range(AVP64_GIC_CPUIF_ADDR, AVP64_GIC_CPUIF_HIGH)),
        addr_gic_distif("addr_gic_distif", vcml::range(AVP64_GIC_DISTIF_ADDR, AVP64_GIC_DISTIF_HIGH)),
        addr_gic_vifctrl("addr_gic_vifctrl", vcml::range(AVP64_GIC_VIFCTRL_ADDR, AVP64_GIC_VIFCTRL_HIGH)),
        addr_gic_vcpuif("addr_gic_vcpuif", vcml::range(AVP64_GIC_VCPUIF_ADDR, AVP64_GIC_VCPUIF_HIGH)),
        addr_uart0("addr_uart0", vcml::range(AVP64_UART0_ADDR, AVP64_UART0_HIGH)),
        addr_uart1("addr_uart1", vcml::range(AVP64_UART1_ADDR, AVP64_UART1_HIGH)),
        addr_uart2("addr_uart2", vcml::range(AVP64_UART2_ADDR, AVP64_UART2_HIGH)),
        addr_uart3("addr_uart3", vcml::range(AVP64_UART3_ADDR, AVP64_UART3_HIGH)),
        addr_ethoc("addr_ethoc", vcml::range(AVP64_ETHOC_ADDR, AVP64_ETHOC_HIGH)),
        addr_sdhci("addr_sdhci", vcml::range(AVP64_SDHCI_ADDR, AVP64_SDHCI_HIGH)),
        addr_simdev("addr_simdev", vcml::range(AVP64_SIMDEV_ADDR, AVP64_SIMDEV_HIGH)),
        addr_hwrng("addr_hwrng", vcml::range(AVP64_HWRNG_ADDR, AVP64_HWRNG_HIGH)),
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
        m_ethoc("ethoc"),
        m_sdcard("sdcard"),
        m_sdhci("sdhci"),
        m_simdev("simdev"),
        m_hwrng("hwrng"),
        m_sig_clock("sig_clock"),
        m_sig_reset("sig_reset") {
        construct_system_arm64();
   }

    int system::run() {
        double simstart = vcml::realtime();
        int result = vcml::system::run();
        double realtime = vcml::realtime() - simstart;
        double total_runtime = sc_core::sc_time_stamp().to_seconds();

        uint64_t ninsn = 0;
        for (unsigned int cpu = 0; cpu < nrcpu; cpu++) {
            double mhz = m_cpus[cpu]->CLOCK.read() * 1e-6;
            double mips = m_cpus[cpu]->get_cps() * 1e-6; //approximated
            double cpurt = m_cpus[cpu]->get_run_time();
            uint64_t cycles = m_cpus[cpu]->cycle_count();
            ninsn += cycles;

            vcml::log_info("core %d", cpu);
            vcml::log_info("  clock speed  : %.1f MHz", mhz);
            vcml::log_info("  sim speed    : %.1f MIPS", mips);
            vcml::log_info("  run time     : %.1fs", cpurt);
            vcml::log_info("  cycles       : %" PRId64, cycles);

            for (auto irq : m_cpus[cpu]->IRQ) {
                vcml::irq_stats stats;
                if (!m_cpus[cpu]->get_irq_stats(irq.first, stats) ||
                    stats.irq_count == 0)
                    continue;

                std::string s;
                s += vcml::mkstr("  irq %d status :", stats.irq);
                s += vcml::mkstr(" %d#", stats.irq_count);
                s += vcml::mkstr(", avg %.1fus",
                        stats.irq_uptime.to_seconds() / stats.irq_count * 1e6);
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
}
