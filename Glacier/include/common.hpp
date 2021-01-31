#pragma once

#ifdef GLACIER_BUILD_DLL
#ifdef GLACIER_BUILD
#define GLACIER_API __declspec(dllexport)
#else
#define GLACIER_API __declspec(dllimport)
#endif // GLACIER_BUILD
#else
#define GLACIER_API
#endif // GLACIER_BUILD_DLL
