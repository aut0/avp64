/******************************************************************************
 *                                                                            *
 * Copyright 2024 Lukas Jünger, Nils Bosbach                                  *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#include <gtest/gtest.h>
#include "avp64/core.h"

class arm64_core_test : public avp64::core
{
public:
    arm64_core_test(): avp64::core("test_core", 0, 1){};
    bool read_reg(id_t regno, void* buf, size_t len) {
        return read_reg_dbg(regno, buf, len);
    }
    bool write_reg(id_t regno, const void* buf, size_t len) {
        return write_reg_dbg(regno, buf, len);
    }
};

TEST(avp64, simple) {
    arm64_core_test test_cpu;

    mwr::hz_t defclk = 1 * mwr::kHz;
    vcml::generic::clock clock("clk", defclk);
    vcml::generic::reset reset("rst");

    vcml::generic::memory imem("imem", 0x1000);
    vcml::generic::memory dmem("dmem", 0x10000);
    vcml::range r(0x0, 0xf);

    clock.clk.bind(test_cpu.clk);
    clock.clk.bind(imem.clk);
    clock.clk.bind(dmem.clk);
    reset.rst.bind(test_cpu.rst);
    reset.rst.bind(imem.rst);
    reset.rst.bind(dmem.rst);
    // Sockets are bound, but only DATA is used for MMIO
    test_cpu.insn.bind(imem.in);
    test_cpu.data.bind(dmem.in);
    for (size_t i = 0; i < avp64::core::ARM_TIMER_COUNT; ++i)
        test_cpu.timer_irq_out[i].stub();

    vcml::u64 pc, x0, x1;
    sc_core::sc_time quantum(1.0, sc_core::SC_SEC);
    tlm::tlm_global_quantum::instance().set(quantum);

    vcml::u32 insn_mmio[4] = { 0xd2995fc0, // 0x0: mov x0, #0xcafe
                               0xf9400001, // 0x4: ldr x1, [x0]
                               0x14000000, // 0x8: b 0x8
                               0x00000000 };

    vcml::tlm_sbi info = vcml::SBI_NONE;
    imem.write(r, &insn_mmio, info);

    pc = 0;
    EXPECT_TRUE(test_cpu.write_reg(32, &pc, 8));
    EXPECT_TRUE(test_cpu.write_reg(0, &pc, 8));
    EXPECT_TRUE(test_cpu.write_reg(1, &pc, 8));
    EXPECT_TRUE(test_cpu.read_reg(32, &pc, 8));
    EXPECT_TRUE(test_cpu.read_reg(0, &x0, 8));
    EXPECT_TRUE(test_cpu.read_reg(1, &x1, 8));
    EXPECT_EQ(pc, 0x0);
    EXPECT_EQ(x0, 0x0);
    EXPECT_EQ(x1, 0x0);

    sc_core::sc_start(quantum);

    EXPECT_TRUE(test_cpu.read_reg(32, &pc, 8));
    EXPECT_TRUE(test_cpu.read_reg(0, &x0, 8));
    EXPECT_TRUE(test_cpu.read_reg(1, &x1, 8));
    EXPECT_EQ(test_cpu.cycle_count(), quantum.to_seconds() * defclk);
    EXPECT_EQ(pc, 0x8);
    EXPECT_EQ(x0, 0xcafe);
    EXPECT_EQ(x1, 0x0);
}
