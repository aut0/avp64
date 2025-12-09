/******************************************************************************
 *                                                                            *
 * Copyright 2025 Nils Bosbach                                                *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#include "avp64/psp/systemc.h"

namespace avp64 {
namespace psp {

sc_core::sc_time time_from_ps(vcml::u64 time_ps) {
    return vcml::time_from_value(
        time_ps * sc_core::sc_time(1.0, sc_core::SC_PS).value());
}

} // namespace psp
} // namespace avp64
