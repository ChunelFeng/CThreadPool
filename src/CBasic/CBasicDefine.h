/***************************
@Author: Chunel
@Contact: chunel@foxmail.com
@File: CBasicDefine.h
@Time: 2021/4/26 8:15 下午
@Desc:
***************************/

#ifndef CGRAPH_CBASICDEFINE_H
#define CGRAPH_CBASICDEFINE_H

#include <cstddef>

#define CGRAPH_NAMESPACE_BEGIN                                          \
namespace CTP {                                                         \

#define CGRAPH_NAMESPACE_END                                            \
} /* end of namespace CTP */                                            \

CGRAPH_NAMESPACE_BEGIN

using CCHAR = char;
using CUINT = unsigned int;
using CVOID = void;
using CINT = int;
using CLONG = long;
using CULONG = unsigned long;
using CBOOL = bool;
using CBIGBOOL = int;
using CFLOAT = float;
using CDOUBLE = double;
using CCONSTR = const char*;
using CSIZE = size_t;

CGRAPH_NAMESPACE_END

#endif //CGRAPH_CBASICDEFINE_H
