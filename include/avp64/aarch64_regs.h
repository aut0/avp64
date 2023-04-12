/******************************************************************************
 *                                                                            *
 * Copyright 2020 Lukas Jünger                                                *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#ifndef AVP64_AARCH64_REGS_H
#define AVP64_AARCH64_REGS_H

#include "vcml.h"
#include <string.h>

namespace avp64 {

using vcml::VCML_ACCESS_READ;
using vcml::VCML_ACCESS_WRITE;
using vcml::VCML_ACCESS_READ_WRITE;

const std::vector<vcml::debugging::cpureg> AARCH64_REGS = {
    /* aarch64 core registers */
    { 0, "X0", 8, VCML_ACCESS_READ_WRITE },
    { 1, "X1", 8, VCML_ACCESS_READ_WRITE },
    { 2, "X2", 8, VCML_ACCESS_READ_WRITE },
    { 3, "X3", 8, VCML_ACCESS_READ_WRITE },
    { 4, "X4", 8, VCML_ACCESS_READ_WRITE },
    { 5, "X5", 8, VCML_ACCESS_READ_WRITE },
    { 6, "X6", 8, VCML_ACCESS_READ_WRITE },
    { 7, "X7", 8, VCML_ACCESS_READ_WRITE },
    { 8, "X8", 8, VCML_ACCESS_READ_WRITE },
    { 9, "X9", 8, VCML_ACCESS_READ_WRITE },
    { 10, "X10", 8, VCML_ACCESS_READ_WRITE },
    { 11, "X11", 8, VCML_ACCESS_READ_WRITE },
    { 12, "X12", 8, VCML_ACCESS_READ_WRITE },
    { 13, "X13", 8, VCML_ACCESS_READ_WRITE },
    { 14, "X14", 8, VCML_ACCESS_READ_WRITE },
    { 15, "X15", 8, VCML_ACCESS_READ_WRITE },
    { 16, "X16", 8, VCML_ACCESS_READ_WRITE },
    { 17, "X17", 8, VCML_ACCESS_READ_WRITE },
    { 18, "X18", 8, VCML_ACCESS_READ_WRITE },
    { 19, "X19", 8, VCML_ACCESS_READ_WRITE },
    { 20, "X20", 8, VCML_ACCESS_READ_WRITE },
    { 21, "X21", 8, VCML_ACCESS_READ_WRITE },
    { 22, "X22", 8, VCML_ACCESS_READ_WRITE },
    { 23, "X23", 8, VCML_ACCESS_READ_WRITE },
    { 24, "X24", 8, VCML_ACCESS_READ_WRITE },
    { 25, "X25", 8, VCML_ACCESS_READ_WRITE },
    { 26, "X26", 8, VCML_ACCESS_READ_WRITE },
    { 27, "X27", 8, VCML_ACCESS_READ_WRITE },
    { 28, "X28", 8, VCML_ACCESS_READ_WRITE },
    { 29, "X29", 8, VCML_ACCESS_READ_WRITE },
    { 30, "X30", 8, VCML_ACCESS_READ_WRITE },
    { 31, "SP", 8, VCML_ACCESS_READ_WRITE },
    { 32, "PC", 8, VCML_ACCESS_READ_WRITE },

    /* aarch64 status register and bitfields */
    { 33, "CPSR", 4, VCML_ACCESS_READ_WRITE },
    { 50, "SPSR_EL1", 4, VCML_ACCESS_READ_WRITE },
    { 64, "SPSR_EL2", 4, VCML_ACCESS_READ_WRITE },
    { 78, "SPSR_EL3", 4, VCML_ACCESS_READ_WRITE },
    { 92, "SP_EL0", 8, VCML_ACCESS_READ_WRITE },
    { 93, "SP_EL1", 8, VCML_ACCESS_READ_WRITE },
    { 94, "SP_EL2", 8, VCML_ACCESS_READ_WRITE },
    { 95, "SP_EL3", 8, VCML_ACCESS_READ_WRITE },
    { 96, "ELR_EL0", 8, VCML_ACCESS_READ_WRITE },
    { 97, "ELR_EL1", 8, VCML_ACCESS_READ_WRITE },
    { 98, "ELR_EL2", 8, VCML_ACCESS_READ_WRITE },
    { 99, "ELR_EL3", 8, VCML_ACCESS_READ_WRITE },

    /* aarch64 floating point registers */
    { 449, "D0", 8, VCML_ACCESS_READ_WRITE },
    { 450, "D1", 8, VCML_ACCESS_READ_WRITE },
    { 451, "D2", 8, VCML_ACCESS_READ_WRITE },
    { 452, "D3", 8, VCML_ACCESS_READ_WRITE },
    { 453, "D4", 8, VCML_ACCESS_READ_WRITE },
    { 454, "D5", 8, VCML_ACCESS_READ_WRITE },
    { 455, "D6", 8, VCML_ACCESS_READ_WRITE },
    { 456, "D7", 8, VCML_ACCESS_READ_WRITE },
    { 457, "D8", 8, VCML_ACCESS_READ_WRITE },
    { 458, "D9", 8, VCML_ACCESS_READ_WRITE },
    { 459, "D10", 8, VCML_ACCESS_READ_WRITE },
    { 460, "D11", 8, VCML_ACCESS_READ_WRITE },
    { 461, "D12", 8, VCML_ACCESS_READ_WRITE },
    { 462, "D13", 8, VCML_ACCESS_READ_WRITE },
    { 463, "D14", 8, VCML_ACCESS_READ_WRITE },
    { 464, "D15", 8, VCML_ACCESS_READ_WRITE },
    { 465, "D16", 8, VCML_ACCESS_READ_WRITE },
    { 466, "D17", 8, VCML_ACCESS_READ_WRITE },
    { 467, "D18", 8, VCML_ACCESS_READ_WRITE },
    { 468, "D19", 8, VCML_ACCESS_READ_WRITE },
    { 469, "D20", 8, VCML_ACCESS_READ_WRITE },
    { 470, "D21", 8, VCML_ACCESS_READ_WRITE },
    { 471, "D22", 8, VCML_ACCESS_READ_WRITE },
    { 472, "D23", 8, VCML_ACCESS_READ_WRITE },
    { 473, "D24", 8, VCML_ACCESS_READ_WRITE },
    { 474, "D25", 8, VCML_ACCESS_READ_WRITE },
    { 475, "D26", 8, VCML_ACCESS_READ_WRITE },
    { 476, "D27", 8, VCML_ACCESS_READ_WRITE },
    { 477, "D28", 8, VCML_ACCESS_READ_WRITE },
    { 478, "D29", 8, VCML_ACCESS_READ_WRITE },
    { 479, "D30", 8, VCML_ACCESS_READ_WRITE },
    { 480, "D31", 8, VCML_ACCESS_READ_WRITE },

    /* aarch64 floating point status registers */
    { 192, "FPSR", 4, VCML_ACCESS_READ_WRITE },
    { 193, "FPCR", 4, VCML_ACCESS_READ_WRITE },
};

} // namespace avp64
#endif // AVP64_AARCH64_REGS_H
