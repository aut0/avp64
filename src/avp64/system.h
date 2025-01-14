/******************************************************************************
 *                                                                            *
 * Copyright 2024 Lukas Jünger, Nils Bosbach                                  *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#ifndef AVP64_SYSTEM_H
#define AVP64_SYSTEM_H

#include "vcml.h"
#include "avp64.h"

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

    UART1_LO = 0x1000a000,
    UART1_HI = UART1_LO + 0x1000 - 1,

    UART2_LO = 0x1000b000,
    UART2_HI = UART2_LO + 0x1000 - 1,

    UART3_LO = 0x1000c000,
    UART3_HI = UART3_LO + 0x1000 - 1,

    SDHCI_LO = 0x1000d000,
    SDHCI_HI = SDHCI_LO + 0x1000 - 1,

    RTC_LO = 0x1000e000,
    RTC_HI = RTC_LO + 0x1000 - 1,

    GPIO_LO = 0x1000f000,
    GPIO_HI = GPIO_LO + 0x1000 - 1,

    LAN0_LO = 0x10010000,
    LAN0_HI = LAN0_LO + 0x10000 - 1,

    SPI_LO = 0x10020000,
    SPI_HI = SPI_LO + 0x1000 - 1,

    CAN_LO = 0x10021000,
    CAN_HI = CAN_LO + 0x1000 - 1,

    CAN_MSGRAM_LO = 0x10022000,
    CAN_MSGRAM_HI = CAN_MSGRAM_LO + 0x4000 - 1,

    FB0MEM_LO = 0x20000000,
    FB0MEM_HI = FB0MEM_LO + 0x400000 - 1,
};

enum : mwr::u64 {
    SPI_UART0 = 5,
    SPI_UART1 = 6,
    SPI_UART2 = 7,
    SPI_UART3 = 8,
    SPI_LAN0 = 9,
    SPI_SDHCI = 10,
    SPI_SPI = 11,
    CAN_0 = 14,
    CAN_1 = 15,
};

class system : public vcml::system
{
public:
    // properties
    vcml::property<vcml::range> addr_ram;
    vcml::property<vcml::range> addr_fb0mem;
    vcml::property<vcml::range> addr_uart0;
    vcml::property<vcml::range> addr_uart1;
    vcml::property<vcml::range> addr_uart2;
    vcml::property<vcml::range> addr_uart3;
    vcml::property<vcml::range> addr_rtc;
    vcml::property<vcml::range> addr_lan0;
    vcml::property<vcml::range> addr_sdhci;
    vcml::property<vcml::range> addr_simdev;
    vcml::property<vcml::range> addr_hwrng;
    vcml::property<vcml::range> addr_spi;
    vcml::property<vcml::range> addr_gpio;
    vcml::property<vcml::range> addr_can;
    vcml::property<vcml::range> addr_can_msgram;

    vcml::property<int> irq_uart0;
    vcml::property<int> irq_uart1;
    vcml::property<int> irq_uart2;
    vcml::property<int> irq_uart3;
    vcml::property<int> irq_lan0;
    vcml::property<int> irq_sdhci;
    vcml::property<int> irq_spi;
    vcml::property<int> irq_can0;
    vcml::property<int> irq_can1;

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
    vcml::generic::clock m_fb0fps;
    vcml::generic::reset m_reset;
    vcml::meta::throttle m_throttle;

    vcml::generic::bus m_bus;
    vcml::generic::memory m_ram;
    vcml::generic::fbdev m_fb0;
    vcml::generic::memory m_fb0mem;
    vcml::serial::pl011 m_uart0;
    vcml::serial::pl011 m_uart1;
    vcml::serial::pl011 m_uart2;
    vcml::serial::pl011 m_uart3;
    vcml::serial::terminal m_term0;
    vcml::serial::terminal m_term1;
    vcml::serial::terminal m_term2;
    vcml::serial::terminal m_term3;
    vcml::ethernet::lan9118 m_lan0;
    vcml::ethernet::network m_net;
    vcml::ethernet::bridge m_bridge;
    vcml::sd::card m_sdcard;
    vcml::sd::sdhci m_sdhci;
    vcml::meta::simdev m_simdev;
    vcml::generic::hwrng m_hwrng;
    vcml::spi::ocspi m_spi;
    vcml::gpio::mmgpio m_gpio;
    vcml::spi::max31855 m_max31855;
    vcml::timers::rtc1742 m_rtc;
    vcml::can::bus m_canbus;
    vcml::generic::memory m_can_msgram;
    vcml::can::m_can m_can;
    vcml::can::bridge m_canbridge;

    cpu m_cpu;

    void construct_system_arm64();
};

} // namespace avp64

#endif // AVP64_SYSTEM_H
