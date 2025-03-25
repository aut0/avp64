/******************************************************************************
 *                                                                            *
 * Copyright 2024 Lukas Jünger, Nils Bosbach                                  *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#ifndef AVP64_CPU_H
#define AVP64_CPU_H

#include "vcml.h"
#include "avp64/core.h"
#include "vcml/models/arm/gic400.h"

#include <memory>
#include <vector>

namespace avp64 {

enum : mwr::u64 {
    GIC_DISTIF_LO = 0x10140000,
    GIC_DISTIF_HI = GIC_DISTIF_LO + 0x1000 - 1,
    GIC_CPUIF_LO = 0x10141000,
    GIC_CPUIF_HI = GIC_CPUIF_LO + 0x2000 - 1,
    GIC_VIFCTRL_LO = 0x10143000,
    GIC_VIFCTRL_HI = GIC_VIFCTRL_LO + 0x2000 - 1,
    GIC_VCPUIF_LO = 0x10145000,
    GIC_VCPUIF_HI = GIC_VCPUIF_LO + 0x2000 - 1,
};

enum : mwr::u64 {
    PPI_GT_NS = 14,
    PPI_GT_S = 13,
    PPI_GT_VIRT = 11,
    PPI_GT_HYP = 10,
};

class cpu : public vcml::component
{
public:
    vcml::property<size_t> ncores;
    vcml::property<size_t> clusterid;
    vcml::property<std::vector<std::string>> symbols;
    vcml::property<bool> async;
    vcml::property<unsigned int> async_rate;

    vcml::property<vcml::range> gic_cpuif;
    vcml::property<vcml::range> gic_distif;
    vcml::property<vcml::range> gic_vifctrl;
    vcml::property<vcml::range> gic_vcpuif;

    vcml::property<int> irq_gt_hyp;
    vcml::property<int> irq_gt_virt;
    vcml::property<int> irq_gt_ns;
    vcml::property<int> irq_gt_s;

    vcml::property<bool> gdb_wait;
    vcml::property<bool> gdb_echo;
    vcml::property<int> gdb_port;

    tlm::tlm_initiator_socket<> bus;
    vcml::gpio_target_array<vcml::arm::gic400::NSPI> spi;

    cpu(const sc_core::sc_module_name& nm);
    cpu() = delete;
    cpu(const cpu&) = delete;
    virtual ~cpu() = default;
    virtual const char* kind() const override { return "avp64::cpu"; }

    vcml::u64 cycle_count() const;

    virtual const char* version() const override;

protected:
    virtual void end_of_elaboration() override;
    virtual void before_end_of_elaboration() override;
    virtual void end_of_simulation() override;

private:
    std::vector<std::shared_ptr<core>> m_cores;

    vcml::arm::gic400 m_gic;
    vcml::generic::bus m_corebus;

    std::shared_ptr<vcml::debugging::gdbserver> m_gdb;
};
} // namespace avp64
#endif
