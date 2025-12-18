/******************************************************************************
 *                                                                            *
 * Copyright 2025 Nils Bosbach                                                *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#include "avp64/version.h"
#include "avp64/psp/cpu.h"

#include <vcml.h>

namespace avp64 {

enum : mwr::u64 {
    RAM_LO = 0x00000000,
    RAM_HI = RAM_LO + 0x10000000 - 1,

    HWRNG_LO = 0x10007000,
    HWRNG_HI = HWRNG_LO + 0x1000 - 1,

    SIMDEV_LO = 0x10008000,
    SIMDEV_HI = SIMDEV_LO + 0x1000 - 1,

    UART0_LO = 0x10009000,
    UART0_HI = UART0_LO + 0x1000 - 1,

    SDHCI_LO = 0x1000d000,
    SDHCI_HI = SDHCI_LO + 0x1000 - 1,

    RTC_LO = 0x1000e000,
    RTC_HI = RTC_LO + 0x1000 - 1,

    LAN0_LO = 0x10010000,
    LAN0_HI = LAN0_LO + 0x10000 - 1,
};

enum : mwr::u64 {
    SPI_UART0 = 5,
    SPI_LAN0 = 9,
    SPI_SDHCI = 10,
};

class system : public vcml::system
{
public:
    // properties
    vcml::property<vcml::range> addr_ram;
    vcml::property<vcml::range> addr_uart0;
    vcml::property<vcml::range> addr_rtc;
    vcml::property<vcml::range> addr_lan0;
    vcml::property<vcml::range> addr_sdhci;
    vcml::property<vcml::range> addr_simdev;
    vcml::property<vcml::range> addr_hwrng;

    vcml::property<int> irq_uart0;
    vcml::property<int> irq_lan0;
    vcml::property<int> irq_sdhci;

    system(const sc_core::sc_module_name& name);
    system() = delete;
    system(const system&) = delete;
    virtual ~system() = default;

    int run() override;

    virtual void end_of_elaboration() override;

    const char* version() const override;
    virtual const char* kind() const override { return "avp64::system"; }

private:
    vcml::generic::clock m_clock_cpu;
    vcml::generic::reset m_reset;
    vcml::meta::throttle m_throttle;

    vcml::generic::bus m_bus;
    vcml::generic::memory m_ram;
    vcml::serial::pl011 m_uart0;
    vcml::serial::terminal m_term0;
    vcml::ethernet::lan9118 m_lan0;
    vcml::ethernet::network m_net;
    vcml::ethernet::bridge m_bridge;
    vcml::sd::card m_sdcard;
    vcml::sd::sdhci m_sdhci;
    vcml::meta::simdev m_simdev;
    vcml::generic::hwrng m_hwrng;
    vcml::timers::rtc1742 m_rtc;

    psp::cpu m_cpu;
};

system::system(const sc_core::sc_module_name& nm):
    vcml::system(nm),
    addr_ram("addr_ram", { RAM_LO, RAM_HI }),
    addr_uart0("addr_uart0", { UART0_LO, UART0_HI }),
    addr_rtc("addr_rtc", { RTC_LO, RTC_HI }),
    addr_lan0("addr_lan0", { LAN0_LO, LAN0_HI }),
    addr_sdhci("addr_sdhci", { SDHCI_LO, SDHCI_HI }),
    addr_simdev("addr_simdev", { SIMDEV_LO, SIMDEV_HI }),
    addr_hwrng("addr_hwrng", { HWRNG_LO, HWRNG_HI }),
    irq_uart0("irq_uart0", SPI_UART0),
    irq_lan0("irq_lan0", SPI_LAN0),
    irq_sdhci("irq_sdhci", SPI_SDHCI),
    m_clock_cpu("clock_cpu", 1 * mwr::GHz),
    m_reset("reset"),
    m_throttle("throttle"),
    m_bus("bus"),
    m_ram("ram", addr_ram.get().length()),
    m_uart0("uart0"),
    m_term0("term0"),
    m_lan0("lan0"),
    m_net("net"),
    m_bridge("bridge"),
    m_sdcard("sdcard"),
    m_sdhci("sdhci"),
    m_simdev("simdev"),
    m_hwrng("hwrng"),
    m_rtc("rtc"),
    m_cpu("cpu") {
    clk_bind(m_clock_cpu, "clk", m_bus, "clk");
    clk_bind(m_clock_cpu, "clk", m_ram, "clk");
    clk_bind(m_clock_cpu, "clk", m_uart0, "clk");
    clk_bind(m_clock_cpu, "clk", m_lan0, "clk");
    clk_bind(m_clock_cpu, "clk", m_sdhci, "clk");
    clk_bind(m_clock_cpu, "clk", m_sdcard, "clk");
    clk_bind(m_clock_cpu, "clk", m_simdev, "clk");
    clk_bind(m_clock_cpu, "clk", m_hwrng, "clk");
    clk_bind(m_clock_cpu, "clk", m_rtc, "clk");
    clk_bind(m_clock_cpu, "clk", m_cpu, "clk");

    gpio_bind(m_reset, "rst", m_bus, "rst");
    gpio_bind(m_reset, "rst", m_ram, "rst");
    gpio_bind(m_reset, "rst", m_uart0, "rst");
    gpio_bind(m_reset, "rst", m_lan0, "rst");
    gpio_bind(m_reset, "rst", m_sdhci, "rst");
    gpio_bind(m_reset, "rst", m_sdcard, "rst");
    gpio_bind(m_reset, "rst", m_simdev, "rst");
    gpio_bind(m_reset, "rst", m_hwrng, "rst");
    gpio_bind(m_reset, "rst", m_rtc, "rst");
    gpio_bind(m_reset, "rst", m_cpu, "rst");

    tlm_bind(m_bus, m_cpu, "bus");
    tlm_bind(m_bus, m_ram, "in", addr_ram);
    tlm_bind(m_bus, m_uart0, "in", addr_uart0);
    tlm_bind(m_bus, m_lan0, "in", addr_lan0);
    tlm_bind(m_bus, m_sdhci, "in", addr_sdhci);
    tlm_bind(m_bus, m_sdhci, "out");
    tlm_bind(m_bus, m_simdev, "in", addr_simdev);
    tlm_bind(m_bus, m_hwrng, "in", addr_hwrng);
    tlm_bind(m_bus, m_rtc, "in", addr_rtc);

    // Connect network to eth
    eth_bind(m_net, "eth_tx", 0, m_lan0, "eth_rx");
    eth_bind(m_net, "eth_rx", 0, m_lan0, "eth_tx");
    eth_bind(m_net, "eth_tx", 1, m_bridge, "eth_rx");
    eth_bind(m_net, "eth_rx", 1, m_bridge, "eth_tx");

    // Connect terminals to uarts
    serial_bind(m_term0, "serial_tx", m_uart0, "serial_rx");
    serial_bind(m_term0, "serial_rx", m_uart0, "serial_tx");

    // Connect SD card to SDHCI
    sd_bind(m_sdhci, "sd_out", m_sdcard, "sd_in");

    // IRQs
    gpio_bind(m_uart0, "irq", m_cpu, "spi", irq_uart0);
    gpio_bind(m_lan0, "irq", m_cpu, "spi", irq_lan0);
    gpio_bind(m_sdhci, "irq", m_cpu, "spi", irq_sdhci);
}

int system::run() {
    double simstart = mwr::timestamp();
    int result = vcml::system::run();
    double realtime = mwr::timestamp() - simstart;
    double duration = sc_core::sc_time_stamp().to_seconds();
    vcml::u64 ninsn = m_cpu.cycle_count();

    double mips = realtime == 0.0 ? 0.0 : ninsn / realtime / 1e6;
    vcml::log_info("total");
    vcml::log_info("  duration       : %.9fs", duration);
    vcml::log_info("  runtime        : %.4fs", realtime);
    vcml::log_info("  instructions   : %llu", ninsn);
    vcml::log_info("  sim speed      : %.1f MIPS", mips);
    vcml::log_info("  realtime ratio : %.2f / 1s",
                   realtime == 0.0 ? 0.0 : realtime / duration);

    return result;
}

void system::end_of_elaboration() {
    vcml::system::end_of_elaboration();

    std::stringstream ss;
    m_bus.execute("show", ss);
    log_debug("%s", ss.str().c_str());
}

const char* system::version() const {
    return AVP64_VERSION_STRING;
}

} // namespace avp64

extern "C" int sc_main(int argc, char** argv) {
    avp64::system system("system");
    return system.run();
}
