/******************************************************************************
 *                                                                            *
 * Copyright 2020 Lukas Jünger                                                *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#ifndef AVP64_COMMON_H
#define AVP64_COMMON_H

#include <systemc>
#include <vcml.h>

#define SAFE_DELETE(p)       do { if (p) delete    (p); (p) = NULL; } while (0)
#define SAFE_DELETE_ARRAY(p) do { if (p) delete [] (p); (p) = NULL; } while (0)

#endif //AVP64_COMMON_H
