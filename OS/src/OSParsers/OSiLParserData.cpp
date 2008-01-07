/** @file OSiLParserData.cpp
 * 
 * @author  Robert Fourer, Gus Gassmann, Jun Ma, Kipp Martin, 
 * @version 2.0, 12/21/2007
 * @since   OS1.0
 *
 * \remarks
 * Copyright (C) 2005-2007, Robert Fourer, Gus Gassmann, Jun Ma, Kipp Martin,
 * Northwestern University, Dalhousie University and the University of Chicago.
 * All Rights Reserved.
 * This software is licensed under the Common Public License. 
 * Please see the accompanying LICENSE file in root directory for terms.
 * 
 */
 #include "OSiLParserData.h"
 OSiLParserData::~OSiLParserData(){
 	// clear the vectors of pointers
	nlNodeVec.clear();
	sumVec.clear();
	maxVec.clear();
	productVec.clear();
 }//~OSiLParserData
 

 OSiLParserData::OSiLParserData() :
 	osillineno(0),
 	qtermcount(0),
  	qtermidxOneattON(false),
	qtermidxTwoattON(false),
	qtermidxattON(false),
	qtermidattON(false),
	qtermcoefattON(false),
	nlnodecount(0),
	tmpnlcount(0),	
 	numbertypeattON(false),
 	numbervalueattON(false),
 	numberidattON(false),
 	variableidxattON(false),
 	variablecoefattON(false),
	timeDomainStages(false),
	timeDomainInterval(false),
	stagename(""),
	stagecount(0),
	intervalhorizon(0.0),
	intervalstart(0.0),
	stagenameON(false),
	stageVariablesON(false),
	stageConstraintsON(false),
	stageObjectivesON(false),
	stageVariablesOrdered(false),
	stageConstraintsOrdered(false),
	stageObjectivesOrdered(false),
	stageVariableStartIdx(0),
	stageConstraintStartIdx(0),
	stageObjectiveStartIdx(0),
	stagevarcount(0),
	stageconcount(0),
	stageobjcount(0),
	intervalhorizonON(false),
	intervalstartON(false)
 {

 }//OSiLParserData
