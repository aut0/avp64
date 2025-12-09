/******************************************************************************
 *                                                                            *
 * Copyright 2025 Nils Bosbach                                                *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#ifndef AVP64_PSP_SYSTEMC_H
#define AVP64_PSP_SYSTEMC_H

#include "avp64/common.h"

namespace avp64 {
namespace psp {

sc_core::sc_time time_from_ps(vcml::u64 time_ps);

} // namespace psp
} // namespace avp64

#endif
