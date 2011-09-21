/* $Id: OSgLParserData.cpp 3710 2010-10-04 03:39:42Z Gassmann $ */
/** @file OSgLParserData.cpp
 *
 * @author  Horand Gassmann, Jun Ma, Kipp Martin,
 *
 * \remarks
 * Copyright (C) 2005-2011, Horand Gassmann, Jun Ma, Kipp Martin,
 * Dalhousie University, Northwestern University, and the University of Chicago.
 * All Rights Reserved.
 * This software is licensed under the Eclipse Public License.
 * Please see the accompanying LICENSE file in root directory for terms.
 *
 */
#include "OSgLParserData.h"
#include <stdio.h>


OSgLParserData::~OSgLParserData()
{
    if (osglIntArray != NULL)
        delete[] osglIntArray;
    osglIntArray = NULL;
}//~OSgLParserData


OSgLParserData::OSgLParserData() :
    osglMultPresent(false),
    osglIncrPresent(false),
    osglNumberOfElPresent(false),
    osglNumberOfEl(-1),
    osglIntArray(NULL),
    osglMult(1),
    osglIncr(1),
    osglSize(0),
    osglCounter(0),
    osglTempint(0),
    fileName(""),
    source(""),
    description(""),
    fileCreator(""),
    licence("")

{

}//OSgLParserData

