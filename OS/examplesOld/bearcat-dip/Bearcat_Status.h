//===========================================================================//
// This file is part of the Decomp Solver Framework.                         //
//                                                                           //
// Decomp is distributed under the Common Public License as part of the      //
// COIN-OR repository (http://www.coin-or.org).                              //
//                                                                           //
// Author: Matthew Galati, Lehigh University                                 //
//                                                                           //
// Copyright (C) 2002-2009, Lehigh University, Matthew Galati, and Ted Ralphs//
// All Rights Reserved.                                                      //
//===========================================================================//

#ifndef BEARCAT_STATUS_INCLUDED
#define BEARCAT_STATUS_INCLUDED

//===========================================================================//
#include <string>
//===========================================================================//
//---
//--- return codes
//---
//===========================================================================//
enum BearcatStatus {
   BearcatStatusOk = 0,
   BearcatStatusError,      //general error
   BearcatStatusFileIO,     //error in i/o
   BearcatStatusOutOfMemory //out of memory
};
const std::string BearcatStatusStr[4] = {
   "BearcatStatusOk",
   "BearcatStatusError",
   "BearcatStatusFileIO",
   "BearcatStatusOutOfMemory"
};
#endif
