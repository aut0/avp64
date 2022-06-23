/******************************************************************************
 *                                                                            *
 * Copyright 2020 Lukas Jünger                                                *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#ifndef AVP64_CONFIG_H
#define AVP64_CONFIG_H

// memory map
#define AVP64_RAM_ADDR (0x00000000ull)
#define AVP64_RAM_SIZE (0x10000000ull)
#define AVP64_RAM_HIGH (AVP64_RAM_ADDR + AVP64_RAM_SIZE - 1)

#define AVP64_GIC_DISTIF_ADDR (0x10140000ull)
#define AVP64_GIC_DISTIF_SIZE (0x00001000ull)
#define AVP64_GIC_DISTIF_HIGH \
    (AVP64_GIC_DISTIF_ADDR + AVP64_GIC_DISTIF_SIZE - 1)

#define AVP64_GIC_CPUIF_ADDR (0x10141000ull)
#define AVP64_GIC_CPUIF_SIZE (0x00002000ull)
#define AVP64_GIC_CPUIF_HIGH (AVP64_GIC_CPUIF_ADDR + AVP64_GIC_CPUIF_SIZE - 1)

#define AVP64_GIC_VIFCTRL_ADDR (0x10143000ull)
#define AVP64_GIC_VIFCTRL_SIZE (0x00002000ull)
#define AVP64_GIC_VIFCTRL_HIGH \
    (AVP64_GIC_VIFCTRL_ADDR + AVP64_GIC_VIFCTRL_SIZE - 1)

#define AVP64_GIC_VCPUIF_ADDR (0x10145000ull)
#define AVP64_GIC_VCPUIF_SIZE (0x00002000ull)
#define AVP64_GIC_VCPUIF_HIGH \
    (AVP64_GIC_VCPUIF_ADDR + AVP64_GIC_VCPUIF_SIZE - 1)

#define AVP64_HWRNG_ADDR (0x10007000ull)
#define AVP64_HWRNG_SIZE (0x00001000ull)
#define AVP64_HWRNG_HIGH (AVP64_HWRNG_ADDR + AVP64_HWRNG_SIZE - 1)

#define AVP64_SIMDEV_ADDR (0x10008000ull)
#define AVP64_SIMDEV_SIZE (0x00001000ull)
#define AVP64_SIMDEV_HIGH (AVP64_SIMDEV_ADDR + AVP64_SIMDEV_SIZE - 1)

#define AVP64_UART0_ADDR (0x10009000ull)
#define AVP64_UART0_SIZE (0x00001000ull)
#define AVP64_UART0_HIGH (AVP64_UART0_ADDR + AVP64_UART0_SIZE - 1)

#define AVP64_UART1_ADDR (0x1000a000ull)
#define AVP64_UART1_SIZE (0x00001000ull)
#define AVP64_UART1_HIGH (AVP64_UART1_ADDR + AVP64_UART1_SIZE - 1)

#define AVP64_UART2_ADDR (0x1000b000ull)
#define AVP64_UART2_SIZE (0x00001000ull)
#define AVP64_UART2_HIGH (AVP64_UART2_ADDR + AVP64_UART2_SIZE - 1)

#define AVP64_UART3_ADDR (0x1000c000ull)
#define AVP64_UART3_SIZE (0x00001000ull)
#define AVP64_UART3_HIGH (AVP64_UART3_ADDR + AVP64_UART3_SIZE - 1)

#define AVP64_ETHOC_ADDR (0x1000d000ull)
#define AVP64_ETHOC_SIZE (0x00002000ull)
#define AVP64_ETHOC_HIGH (AVP64_ETHOC_ADDR + AVP64_ETHOC_SIZE - 1)

#define AVP64_SDHCI_ADDR (0x1000f000ull)
#define AVP64_SDHCI_SIZE (0x00001000ull)
#define AVP64_SDHCI_HIGH (AVP64_SDHCI_ADDR + AVP64_SDHCI_SIZE - 1)

#define AVP64_SPI_ADDR (0x10147000ull)
#define AVP64_SPI_SIZE (0x00002000ull)
#define AVP64_SPI_HIGH (AVP64_SPI_ADDR + AVP64_SPI_SIZE - 1)

#define AVP64_GPIO_ADDR (0x10149000ull)
#define AVP64_GPIO_SIZE (0x00001000ull)
#define AVP64_GPIO_HIGH (AVP64_GPIO_ADDR + AVP64_GPIO_SIZE - 1)

// interrupt map
#define AVP64_IRQ_UART0   (5)
#define AVP64_IRQ_UART1   (6)
#define AVP64_IRQ_UART2   (7)
#define AVP64_IRQ_UART3   (8)
#define AVP64_IRQ_ETHOC   (9)
#define AVP64_IRQ_SDHCI   (10)
#define AVP64_IRQ_GT_NS   (30)
#define AVP64_IRQ_GT_S    (29)
#define AVP64_IRQ_GT_VIRT (27)
#define AVP64_IRQ_GT_HYP  (26)
#define AVP64_IRQ_SPI     (11)

#define AVP64_CPU_DEFCLK (100 * vcml::MHz)

#endif // AVP64_CONFIG_H
