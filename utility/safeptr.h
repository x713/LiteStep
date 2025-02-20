
#ifndef __SAFEPTR_H
#define __SAFEPTR_H

#include "common.h"
#include "windows.h"

//
// Valid String Pointer Functions
//

//
// Unicode
//
BOOL IsValidStringPtrW(LPCWSTR lpcwz);


BOOL IsValidStringPtrW(LPWSTR lpwz);
BOOL IsValidStringPtrW(LPWSTR lpwz, UINT_PTR ucchMax);
//BOOL IsValidStringPtrW(LPCWSTR lpwz, UINT_PTR ucchMax);

//
// Ansi
//
BOOL IsValidStringPtrA(LPCSTR lpsz);
#ifdef __cplusplus
BOOL IsValidStringPtrA(LPCSTR lpsz, UINT_PTR ucchMax);
#else // __cplusplus
BOOL IsValidStringPtrCchA(LPCSTR lpsz, UINT_PTR ucchMax);
#endif // __cplusplus
#ifdef __cplusplus
BOOL IsValidStringPtrA(LPSTR lpsz);
BOOL IsValidStringPtrA(LPSTR lpsz, UINT_PTR ucchMax);
#else // __cplusplus
BOOL IsValidStringReadPtrA(LPSTR lpsz);
BOOL IsValidStringReadPtrCchA(LPSTR lpsz, UINT_PTR ucchMax);
#endif // __cplusplus

//
// TCHAR defines
//
#ifdef _UNICODE
#ifdef __cplusplus
#define IsValidStringPtr IsValidStringPtrW
#else // __cplusplus
#define IsValidStringPtr(lpwz) IsValidStringPtrW(lpwz)
#define IsValidStringPtrCch(lpwz, ucchMax) IsValidStringPtrCchW(lpwz, ucchMax)
#define IsValidStringReadPtr(lpwz) IsValidStringReadPtrW(lpwz)
#define IsValidStringReadPtrCch(lpwz, ucchMax) IsValidStringReadPtrCchW(lpwz, ucchMax)
#endif // _cplusplus
#else // _UNICODE
#ifdef __cplusplus
#define IsValidStringPtr IsValidStringPtrA
#else // __cplusplus
#define IsValidStringPtr(lpsz) IsValidStringPtrA(lpsz)
#define IsValidStringPtrCch(lpsz, ucchMax) IsValidStringPtrCchA(lpsz, ucchMax)
#define IsValidStringReadPtr(lpsz) IsValidStringReadPtrA(lpsz)
#define IsValidStringReadPtrCch(lpsz, ucchMax) IsValidStringReadPtrCchA(lpsz, ucchMax)
#endif // _cplusplus
#endif // _UNICODE

//
// Valid Pointer Functions
//
BOOL IsValidReadPtr(CONST VOID *lp);

#ifdef __cplusplus
BOOL IsValidReadPtr(CONST VOID *lp, UINT_PTR ucb);
#else // __cplusplus
BOOL IsValidReadPtrUcb(CONST VOID *lp, UINT_PTR ucb);
#endif // __cplusplus


BOOL IsValidWritePtr(LPVOID lp);

#ifdef __cplusplus
BOOL IsValidWritePtr(LPVOID lp, UINT_PTR ucb);
#else // __cplusplus
BOOL IsValidWritePtrUcb(LPVOID lp, UINT_PTR ucb);
#endif // __cplusplus
BOOL IsValidCodePtr(FARPROC lfpn);

#endif // _SAFEPTR_H
