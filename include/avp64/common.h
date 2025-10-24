/******************************************************************************
 *                                                                            *
 * Copyright 2025 Nils Bosbach                                                *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#ifndef AVP64_COMMON_H
#define AVP64_COMMON_H

#include <vcml.h>
#include <list>
#include <vector>
#include <memory>
#include <string>

namespace avp64 {

#define AVP64_KIND(name)                        \
    virtual const char* kind() const override { \
        return "avp64::" #name;                 \
    }

using std::string;

using std::list;
using std::vector;
using std::shared_ptr;

using std::pair;

} // namespace avp64

#endif
