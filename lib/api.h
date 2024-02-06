// Software Name : cppuprofile
// SPDX-FileCopyrightText: Copyright (c) 2023 Orange
// SPDX-License-Identifier: BSD-3-Clause
//
// This software is distributed under the BSD License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al.

#pragma once

// UPROFAPI is used to export public API functions from the DLL / shared library.
#if defined(_UPROFILE_BUILD_SHARED)
#if defined(_WIN32)
/* Build as a Win32 DLL */
#define UPROFAPI __declspec(dllexport)
#elif defined(__linux__)
/* Build as a shared library */
#define UPROFAPI __attribute__((visibility("default")))
#endif // if defined(_UPROFILE_BUILD_SHARED)

#elif defined(UPROFILE_DLL)
#if defined(_WIN32)
/* Call uprofile as a Win32 DLL */
#define UPROFAPI __declspec(dllimport)
#endif // if defined(_WIN32)
#endif // if defined(UPROFILE_DLL)

#if !defined(UPROFAPI)
#define UPROFAPI
#endif
