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

    const std::vector<vcml::debugging::cpureg> aarch64_regs = {
        /* aarch64 core registers */
        { 0,  "X0",  8, VCML_ACCESS_READ_WRITE },
        { 1,  "X1",  8, VCML_ACCESS_READ_WRITE },
        { 2,  "X2",  8, VCML_ACCESS_READ_WRITE },
        { 3,  "X3",  8, VCML_ACCESS_READ_WRITE },
        { 4,  "X4",  8, VCML_ACCESS_READ_WRITE },
        { 5,  "X5",  8, VCML_ACCESS_READ_WRITE },
        { 6,  "X6",  8, VCML_ACCESS_READ_WRITE },
        { 7,  "X7",  8, VCML_ACCESS_READ_WRITE },
        { 8,  "X8",  8, VCML_ACCESS_READ_WRITE },
        { 9,  "X9",  8, VCML_ACCESS_READ_WRITE },
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
        { 31, "SP",  8, VCML_ACCESS_READ_WRITE },
        { 32, "PC",  8, VCML_ACCESS_READ_WRITE },

        /* aarch64 status register and bitfields */
        { 33, "CPSR", 4, VCML_ACCESS_READ_WRITE },

        /* aarch64 floating point registers */
        { 117, "V0",  8, VCML_ACCESS_READ_WRITE },
        { 118, "V1",  8, VCML_ACCESS_READ_WRITE },
        { 119, "V2",  8, VCML_ACCESS_READ_WRITE },
        { 120, "V3",  8, VCML_ACCESS_READ_WRITE },
        { 121, "V4",  8, VCML_ACCESS_READ_WRITE },
        { 122, "V5",  8, VCML_ACCESS_READ_WRITE },
        { 123, "V6",  8, VCML_ACCESS_READ_WRITE },
        { 124, "V7",  8, VCML_ACCESS_READ_WRITE },
        { 125, "V8",  8, VCML_ACCESS_READ_WRITE },
        { 126, "V9",  8, VCML_ACCESS_READ_WRITE },
        { 127, "V10", 8, VCML_ACCESS_READ_WRITE },
        { 128, "V11", 8, VCML_ACCESS_READ_WRITE },
        { 129, "V12", 8, VCML_ACCESS_READ_WRITE },
        { 130, "V13", 8, VCML_ACCESS_READ_WRITE },
        { 131, "V14", 8, VCML_ACCESS_READ_WRITE },
        { 132, "V15", 8, VCML_ACCESS_READ_WRITE },
        { 133, "V16", 8, VCML_ACCESS_READ_WRITE },
        { 134, "V17", 8, VCML_ACCESS_READ_WRITE },
        { 135, "V18", 8, VCML_ACCESS_READ_WRITE },
        { 136, "V19", 8, VCML_ACCESS_READ_WRITE },
        { 137, "V20", 8, VCML_ACCESS_READ_WRITE },
        { 138, "V21", 8, VCML_ACCESS_READ_WRITE },
        { 139, "V22", 8, VCML_ACCESS_READ_WRITE },
        { 140, "V23", 8, VCML_ACCESS_READ_WRITE },
        { 141, "V24", 8, VCML_ACCESS_READ_WRITE },
        { 142, "V25", 8, VCML_ACCESS_READ_WRITE },
        { 143, "V26", 8, VCML_ACCESS_READ_WRITE },
        { 144, "V27", 8, VCML_ACCESS_READ_WRITE },
        { 145, "V28", 8, VCML_ACCESS_READ_WRITE },
        { 146, "V29", 8, VCML_ACCESS_READ_WRITE },
        { 147, "V30", 8, VCML_ACCESS_READ_WRITE },
        { 148, "V31", 8, VCML_ACCESS_READ_WRITE },

        /* aarch64 floating point status registers */
        { 149, "FPSR", 4, VCML_ACCESS_READ_WRITE },
        { 150, "FPCR", 4, VCML_ACCESS_READ_WRITE },
    };

} // namespcae avp64
#endif // AVP64_AARCH64_REGS_H
