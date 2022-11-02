// Software Name : uprofile
// SPDX-FileCopyrightText: Copyright (c) 2022 Orange
// SPDX-License-Identifier: BSD
//
// This software is distributed under the BSD License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al.

namespace uprofile
{
    void getSystemMemory(int& totalMem, int& availableMem, int& freeMem);
    void getProcessMemory(int& rss, int& shared);
}
