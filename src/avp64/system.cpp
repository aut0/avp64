/******************************************************************************
 *                                                                            *
 * Copyright 2024 Lukas Jünger, Nils Bosbach                                  *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#include "avp64/system.h"
#include "avp64/version.h"

namespace avp64 {

void system::construct_system_arm64() {
    clk_bind(m_clock_cpu, "clk", m_bus, "clk");
    clk_bind(m_clock_cpu, "clk", m_ram, "clk");
    clk_bind(m_clock_cpu, "clk", m_uart0, "clk");
    clk_bind(m_clock_cpu, "clk", m_uart1, "clk");
    clk_bind(m_clock_cpu, "clk", m_uart2, "clk");
    clk_bind(m_clock_cpu, "clk", m_uart3, "clk");
    clk_bind(m_clock_cpu, "clk", m_lan0, "clk");
    clk_bind(m_clock_cpu, "clk", m_sdhci, "clk");
    clk_bind(m_clock_cpu, "clk", m_sdcard, "clk");
    clk_bind(m_clock_cpu, "clk", m_simdev, "clk");
    clk_bind(m_clock_cpu, "clk", m_hwrng, "clk");
    clk_bind(m_clock_cpu, "clk", m_spi, "clk");
    clk_bind(m_clock_cpu, "clk", m_gpio, "clk");
    clk_bind(m_clock_cpu, "clk", m_rtc, "clk");
    clk_bind(m_clock_cpu, "clk", m_cpu, "clk");

    gpio_bind(m_reset, "rst", m_bus, "rst");
    gpio_bind(m_reset, "rst", m_ram, "rst");
    gpio_bind(m_reset, "rst", m_uart0, "rst");
    gpio_bind(m_reset, "rst", m_uart1, "rst");
    gpio_bind(m_reset, "rst", m_uart2, "rst");
    gpio_bind(m_reset, "rst", m_uart3, "rst");
    gpio_bind(m_reset, "rst", m_lan0, "rst");
    gpio_bind(m_reset, "rst", m_sdhci, "rst");
    gpio_bind(m_reset, "rst", m_sdcard, "rst");
    gpio_bind(m_reset, "rst", m_simdev, "rst");
    gpio_bind(m_reset, "rst", m_hwrng, "rst");
    gpio_bind(m_reset, "rst", m_spi, "rst");
    gpio_bind(m_reset, "rst", m_gpio, "rst");
    gpio_bind(m_reset, "rst", m_rtc, "rst");
    gpio_bind(m_reset, "rst", m_cpu, "rst");

    tlm_bind(m_bus, m_cpu, "bus");
    tlm_bind(m_bus, m_ram, "in", addr_ram);
    tlm_bind(m_bus, m_uart0, "in", addr_uart0);
    tlm_bind(m_bus, m_uart1, "in", addr_uart1);
    tlm_bind(m_bus, m_uart2, "in", addr_uart2);
    tlm_bind(m_bus, m_uart3, "in", addr_uart3);
    tlm_bind(m_bus, m_lan0, "in", addr_lan0);
    tlm_bind(m_bus, m_sdhci, "in", addr_sdhci);
    tlm_bind(m_bus, m_sdhci, "out");
    tlm_bind(m_bus, m_simdev, "in", addr_simdev);
    tlm_bind(m_bus, m_hwrng, "in", addr_hwrng);
    tlm_bind(m_bus, m_spi, "in", addr_spi);
    tlm_bind(m_bus, m_gpio, "in", addr_gpio);
    tlm_bind(m_bus, m_rtc, "in", addr_rtc);

    // Connect network to eth
    eth_bind(m_net, "eth_tx", 0, m_lan0, "eth_rx");
    eth_bind(m_net, "eth_rx", 0, m_lan0, "eth_tx");
    eth_bind(m_net, "eth_tx", 1, m_bridge, "eth_rx");
    eth_bind(m_net, "eth_rx", 1, m_bridge, "eth_tx");

    // Connect terminals to uarts
    serial_bind(m_term0, "serial_tx", m_uart0, "serial_rx");
    serial_bind(m_term0, "serial_rx", m_uart0, "serial_tx");
    serial_bind(m_term1, "serial_tx", m_uart1, "serial_rx");
    serial_bind(m_term1, "serial_rx", m_uart1, "serial_tx");
    serial_bind(m_term2, "serial_tx", m_uart2, "serial_rx");
    serial_bind(m_term2, "serial_rx", m_uart2, "serial_tx");
    serial_bind(m_term3, "serial_tx", m_uart3, "serial_rx");
    serial_bind(m_term3, "serial_rx", m_uart3, "serial_tx");

    // Connect SD card to SDHCI
    sd_bind(m_sdhci, "sd_out", m_sdcard, "sd_in");

    // Connect SPI devices to SPI controller
    spi_bind(m_spi, "spi_out", m_max31855, "spi_in");
    m_max31855.bind(m_gpio.gpio_out[0],
                    false); // active low CS forced by Linux kernel

    // IRQs
    gpio_bind(m_uart0, "irq", m_cpu, "spi", irq_uart0);
    gpio_bind(m_uart1, "irq", m_cpu, "spi", irq_uart1);
    gpio_bind(m_uart2, "irq", m_cpu, "spi", irq_uart2);
    gpio_bind(m_uart3, "irq", m_cpu, "spi", irq_uart3);
    gpio_bind(m_lan0, "irq", m_cpu, "spi", irq_lan0);
    gpio_bind(m_sdhci, "irq", m_cpu, "spi", irq_sdhci);
    gpio_bind(m_spi, "irq", m_cpu, "spi", irq_spi);
}

system::system(const sc_core::sc_module_name& nm):
    vcml::system(nm),
    addr_ram("addr_ram", { RAM_LO, RAM_HI }),
    addr_uart0("addr_uart0", { UART0_LO, UART0_HI }),
    addr_uart1("addr_uart1", { UART1_LO, UART1_HI }),
    addr_uart2("addr_uart2", { UART2_LO, UART2_HI }),
    addr_uart3("addr_uart3", { UART3_LO, UART3_HI }),
    addr_rtc("addr_rtc", { RTC_LO, RTC_HI }),
    addr_lan0("addr_lan0", { LAN0_LO, LAN0_HI }),
    addr_sdhci("addr_sdhci", { SDHCI_LO, SDHCI_HI }),
    addr_simdev("addr_simdev", { SIMDEV_LO, SIMDEV_HI }),
    addr_hwrng("addr_hwrng", { HWRNG_LO, HWRNG_HI }),
    addr_spi("addr_spi", { SPI_LO, SPI_HI }),
    addr_gpio("addr_gpio", { GPIO_LO, GPIO_HI }),
    irq_uart0("irq_uart0", SPI_UART0),
    irq_uart1("irq_uart1", SPI_UART1),
    irq_uart2("irq_uart2", SPI_UART2),
    irq_uart3("irq_uart3", SPI_UART3),
    irq_lan0("irq_lan0", SPI_LAN0),
    irq_sdhci("irq_sdhci", SPI_SDHCI),
    irq_spi("irq_spi", SPI_SPI),
    m_clock_cpu("clock_cpu", 1 * mwr::GHz),
    m_reset("reset"),
    m_throttle("throttle"),
    m_bus("bus"),
    m_ram("ram", addr_ram.get().length()),
    m_uart0("uart0"),
    m_uart1("uart1"),
    m_uart2("uart2"),
    m_uart3("uart3"),
    m_term0("term0"),
    m_term1("term1"),
    m_term2("term2"),
    m_term3("term3"),
    m_lan0("lan0"),
    m_net("net"),
    m_bridge("bridge"),
    m_sdcard("sdcard"),
    m_sdhci("sdhci"),
    m_simdev("simdev"),
    m_hwrng("hwrng"),
    m_spi("spi"),
    m_gpio("gpio"),
    m_max31855("max31855"),
    m_rtc("rtc"),
    m_cpu("cpu") {
    construct_system_arm64();
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
