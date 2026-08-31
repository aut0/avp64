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
using std::array;
using std::unordered_map;
using std::unordered_set;
using std::shared_ptr;
using std::weak_ptr;
using std::unique_ptr;

using std::function;

using std::pair;
using vcml::u64;
using vcml::u32;
using vcml::u16;
using vcml::u8;

using vcml::i64;
using vcml::i32;
using vcml::i16;
using vcml::i8;

using vcml::range;
using vcml::vcml_access;

using vcml::property;

} // namespace avp64

#endif
