#pragma once

// Pragmas we want to keep
// 4127 is constant conditional (assert)
#pragma warning(disable: 4127)

// Pragmas we want to fix
// 4100 is unreferenced formal parameters,
// 4201 nonstandard extension used : nameless struct/union (happens a lot in Windows include headers)
// 4996 Disable _strcmp warnings
// 4244 '=' : conversion from '__w64 int' to 'int', possible loss of data
// 4267 '=' : conversion from 'size_t' to 'int', possible loss of data
// 4311 'type cast' : pointer truncation from 'LPSTR' to 'int'
// 4995 'strcat': name was marked as #pragma deprecated
// 4312 conversion from 'x' to 'y' of greater size
// 4702 unreachable code
#pragma warning(disable: 4100 4201 4996 4244 4267 4311 4995 4312 4702)

// Pragmas for warnings that should be error
// 4114 same type qualifier used more than once
// 4239 nonstandard extension used : 'token' : conversion from 'type' to 'type'
//      inclusive of initial value of reference to non-const must be an lvalue
//      May appear as an error when compiling PS3 code, but only a level 4 warning for PC
#pragma warning(error: 4114 4239)