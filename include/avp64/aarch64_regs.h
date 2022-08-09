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

    /* aarch64 floating point registers */
    { 443, "D0", 8, VCML_ACCESS_READ_WRITE },
    { 444, "D1", 8, VCML_ACCESS_READ_WRITE },
    { 445, "D2", 8, VCML_ACCESS_READ_WRITE },
    { 446, "D3", 8, VCML_ACCESS_READ_WRITE },
    { 447, "D4", 8, VCML_ACCESS_READ_WRITE },
    { 448, "D5", 8, VCML_ACCESS_READ_WRITE },
    { 449, "D6", 8, VCML_ACCESS_READ_WRITE },
    { 450, "D7", 8, VCML_ACCESS_READ_WRITE },
    { 451, "D8", 8, VCML_ACCESS_READ_WRITE },
    { 452, "D9", 8, VCML_ACCESS_READ_WRITE },
    { 453, "D10", 8, VCML_ACCESS_READ_WRITE },
    { 454, "D11", 8, VCML_ACCESS_READ_WRITE },
    { 455, "D12", 8, VCML_ACCESS_READ_WRITE },
    { 456, "D13", 8, VCML_ACCESS_READ_WRITE },
    { 457, "D14", 8, VCML_ACCESS_READ_WRITE },
    { 458, "D15", 8, VCML_ACCESS_READ_WRITE },
    { 459, "D16", 8, VCML_ACCESS_READ_WRITE },
    { 460, "D17", 8, VCML_ACCESS_READ_WRITE },
    { 461, "D18", 8, VCML_ACCESS_READ_WRITE },
    { 462, "D19", 8, VCML_ACCESS_READ_WRITE },
    { 463, "D20", 8, VCML_ACCESS_READ_WRITE },
    { 464, "D21", 8, VCML_ACCESS_READ_WRITE },
    { 465, "D22", 8, VCML_ACCESS_READ_WRITE },
    { 466, "D23", 8, VCML_ACCESS_READ_WRITE },
    { 467, "D24", 8, VCML_ACCESS_READ_WRITE },
    { 468, "D25", 8, VCML_ACCESS_READ_WRITE },
    { 469, "D26", 8, VCML_ACCESS_READ_WRITE },
    { 470, "D27", 8, VCML_ACCESS_READ_WRITE },
    { 471, "D28", 8, VCML_ACCESS_READ_WRITE },
    { 472, "D29", 8, VCML_ACCESS_READ_WRITE },
    { 473, "D30", 8, VCML_ACCESS_READ_WRITE },
    { 474, "D31", 8, VCML_ACCESS_READ_WRITE },

    /* aarch64 floating point status registers */
    { 187, "FPSR", 4, VCML_ACCESS_READ_WRITE },
    { 188, "FPCR", 4, VCML_ACCESS_READ_WRITE },
};

} // namespace avp64
#endif // AVP64_AARCH64_REGS_H
