// Software Name : cppuprofile
// SPDX-FileCopyrightText: Copyright (c) 2022 Orange
// SPDX-License-Identifier: BSD-3-Clause
//
// This software is distributed under the BSD License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al.

#ifndef TIMESTAMP_UNIT_H_
#define TIMESTAMP_UNIT_H_

namespace uprofile
{

enum class TimestampUnit {
    EPOCH_TIME, // Time since epoch
    UPTIME      // Time since boot
};

}

#endif /* TIMESTAMP_UNIT_H_ */
