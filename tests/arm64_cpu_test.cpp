/******************************************************************************
 *                                                                            *
 * Copyright 2020 Lukas Jünger                                                *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#include <gtest/gtest.h>
#include "avp64/arm64_cpu.h"

class arm64_cpu_test : public avp64::arm64_cpu {
public:
    arm64_cpu_test() : avp64::arm64_cpu("test_cpu", 0,1) {
    };
    bool read_reg(vcml::u64 idx, void *buf, vcml::u64 size) {
        return gdb_read_reg(idx, buf, size);
    }
    bool write_reg(vcml::u64 idx, const void *buf, vcml::u64 size) {
        return gdb_write_reg(idx, buf, size);
    }
};

TEST(Arm64CpuTest, simple) {
    arm64_cpu_test test_cpu;

    sc_core::sc_signal<clock_t> clk("clk");
    sc_core::sc_signal<bool> rst("rst");
    sc_core::sc_signal<bool> irq0("irq0");
    sc_core::sc_signal<bool> irq1("irq1");

    vcml::generic::memory imem("imem", 0x1000);
    vcml::generic::memory dmem("dmem", 0x10000);
    vcml::range r(0x0, 0xf);
    vcml::sideband s;

    test_cpu.CLOCK.bind(clk);
    imem.CLOCK.bind(clk);
    dmem.CLOCK.bind(clk);
    test_cpu.RESET.bind(rst);
    imem.RESET.bind(rst);
    dmem.RESET.bind(rst);
    // Sockets are bound, but only DATA is used for MMIO
    test_cpu.INSN.bind(imem.IN);
    test_cpu.DATA.bind(dmem.IN);

    vcml::clock_t defclk = 1* vcml::kHz;
    clk.write(defclk);
    rst.write(false);

    vcml::u64 pc, x0,x1;
    sc_core::sc_time quantum(1.0, sc_core::SC_SEC);
    tlm::tlm_global_quantum::instance().set(quantum);

    vcml::u32 insn_mmio[4] = {0xd2995fc0,    // 0x0: mov x0, #0xcafe
                              0xf9400001,    // 0x4: ldr x1, [x0]
                              0x14000000,    // 0x8: b 0x8
                              0x00000000};

    imem.write(r, &insn_mmio, s);

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
    EXPECT_EQ(pc, 0x8);
    EXPECT_EQ(x0, 0xcafe);
    EXPECT_EQ(x1, 0x0);
}


