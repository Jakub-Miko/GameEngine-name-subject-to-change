#pragma once

#ifdef WIN32

#ifdef BUILDDLL
#define LIBEXP __declspec(dllexport)
#else
#define LIBEXP __declspec(dllimport)
#endif
#else
#define LIBEXP
#endif


#if defined(WIN32)
#define CDECL_CALL __cdecl
#else
#define CDECL_CALL
#endif