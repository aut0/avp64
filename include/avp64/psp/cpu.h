/******************************************************************************
 *                                                                            *
 * Copyright 2024 Lukas Jünger, Nils Bosbach                                  *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#ifndef AVP64_PSP_CPU_H
#define AVP64_PSP_CPU_H

#include "avp64/common.h"
#include "avp64/psp/core.h"

namespace avp64 {
namespace psp {

class cpu : public vcml::component
{
public:
    const property<size_t> ncores;
    const property<size_t> clusterid;
    const property<vector<string>> symbols;
    property<bool> async;
    property<unsigned int> async_rate;

    const property<range> gic_cpuif;
    const property<range> gic_distif;
    const property<range> gic_vifctrl;
    const property<range> gic_vcpuif;

    const property<int> irq_gt_hyp;
    const property<int> irq_gt_virt;
    const property<int> irq_gt_ns;
    const property<int> irq_gt_s;

    const property<bool> gdb_wait;
    const property<bool> gdb_echo;
    property<int> gdb_port;

    vcml::tlm_base_initiator_socket bus;
    vcml::gpio_base_target_array<vcml::arm::gic400::NSPI> spi;

    explicit cpu(const sc_core::sc_module_name& nm);
    cpu() = delete;
    cpu(const cpu&) = delete;
    cpu& operator=(const cpu&) = delete;
    virtual ~cpu() = default;
    AVP64_KIND(psp::cpu);

    u64 cycle_count() const;

    virtual const char* version() const override;

protected:
    virtual void end_of_elaboration() override;
    virtual void end_of_simulation() override;

private:
    vector<shared_ptr<core>> m_cores;

    vcml::arm::gic400 m_gic;
    vcml::generic::bus m_corebus;

    unique_ptr<vcml::debugging::gdbserver> m_gdb;
};

} // namespace psp
} // namespace avp64

#endif
