/******************************************************************************
 *                                                                            *
 * Copyright 2020 Lukas Jünger                                                *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#ifndef AVP64_SYSTEM_H
#define AVP64_SYSTEM_H

#include "vcml.h"
#include "avp64/arm64_cpu.h"

namespace avp64 {

    class system: public vcml::system {
    public:
        // properties
        vcml::property<std::string> type;

        vcml::property<sc_core::sc_time> duration;
        vcml::property<sc_core::sc_time> quantum;

        vcml::property<unsigned short> session;
        vcml::property<bool> vspdebug;
        vcml::property<unsigned int> nrcpu;
        vcml::property<unsigned int> clock;

        vcml::property<vcml::range> addr_ram;
        vcml::property<vcml::range> addr_gic_cpuif;
        vcml::property<vcml::range> addr_gic_distif;
        vcml::property<vcml::range> addr_gic_vifctrl;
        vcml::property<vcml::range> addr_gic_vcpuif;
        vcml::property<vcml::range> addr_uart0;
        vcml::property<vcml::range> addr_uart1;
        vcml::property<vcml::range> addr_uart2;
        vcml::property<vcml::range> addr_uart3;
        vcml::property<vcml::range> addr_ethoc;
        vcml::property<vcml::range> addr_sdhci;
        vcml::property<vcml::range> addr_simdev;
        vcml::property<vcml::range> addr_hwrng;

        vcml::property<int> irq_uart0;
        vcml::property<int> irq_uart1;
        vcml::property<int> irq_uart2;
        vcml::property<int> irq_uart3;
        vcml::property<int> irq_ethoc;
        vcml::property<int> irq_sdhci;
        vcml::property<int> irq_gt_hyp;
        vcml::property<int> irq_gt_virt;
        vcml::property<int> irq_gt_ns;
        vcml::property<int> irq_gt_s;

        system(const sc_core::sc_module_name &name);
        system() = delete;
        system(const system &) = delete;
        virtual ~system() = default;

        int run();

        virtual void end_of_elaboration() override;

    private:
        std::vector<std::shared_ptr<arm64_cpu>> m_cpus;

        vcml::generic::clock    m_clock;
        vcml::generic::reset    m_reset;

        vcml::generic::bus      m_bus;
        vcml::generic::memory   m_ram;
        vcml::arm::gic400       m_gic;
        vcml::arm::pl011uart    m_uart0;
        vcml::arm::pl011uart    m_uart1;
        vcml::arm::pl011uart    m_uart2;
        vcml::arm::pl011uart    m_uart3;
        vcml::opencores::ethoc  m_ethoc;
        vcml::generic::sdcard   m_sdcard;
        vcml::generic::sdhci    m_sdhci;
        vcml::meta::simdev      m_simdev;
        vcml::generic::hwrng    m_hwrng;

        void construct_system_arm64();
    };
    
} //namespace avp64

#endif //AVP64_SYSTEM_H
