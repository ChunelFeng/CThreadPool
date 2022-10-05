/***************************
@Author: Chunel
@Contact: chunel@foxmail.com
@File: CValTypes.h
@Time: 2022/2/3 12:58 下午
@Desc: 
***************************/

#ifndef CGRAPH_CVALTYPE_H
#define CGRAPH_CVALTYPE_H

#include "CStatus.h"
#include "CException.h"

using CChar = CTP::CCHAR;
using CUint = CTP::CUINT;
using CSec = CTP::CUINT;            // 表示秒信息, for second
using CMSec = CTP::CUINT;           // 表示毫秒信息, for millisecond
using CSize = CTP::CSIZE;
using CVoid = CTP::CVOID;
using CVoidPtr = CTP::CVOID *;
using CInt = CTP::CINT;
using CLevel = CTP::CINT;
using CLong = CTP::CLONG;
using CULong = CTP::CULONG;
using CBool = CTP::CBOOL;
using CIndex = CTP::CINT;            // 表示标识信息，可以为负数
using CFloat = CTP::CFLOAT;
using CDouble = CTP::CDOUBLE;
using CConStr = CTP::CCONSTR;    // 表示 const char*
using CBigBool = CTP::CBIGBOOL;

using CStatus = CTP::CSTATUS;
using CException = CTP::CEXCEPTION;

#endif //CGRAPH_CVALTYPE_H
