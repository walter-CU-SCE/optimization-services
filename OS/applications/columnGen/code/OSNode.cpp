/* $Id: OSNode.cpp 3186 2010-02-06 23:38:35Z kmartin $ */
/** @file OSNode.cpp
 * 
 * \remarks
 * Copyright (C) 2005-2010, Horand Gassmann, Jun Ma, Kipp Martin,
 * Dalhousie University, Northwestern University, and the University of Chicago.
 * All Rights Reserved.
 * This software is licensed under the Eclipse Public License. 
 * Please see the accompanying LICENSE file in root directory for terms.
 * 
 */

#include "OSNode.h"
#include <iostream>



	OSNode::OSNode(){
		
		rowIdxNumNonz = 0;
		nodeID = -99999;
		
	}//end default constructor
	
	
	OSNode::OSNode(int rowIdxNumNonz_,  int thetaNumNonz_){
		
		rowIdxNumNonz = rowIdxNumNonz_;
		rowIdx = new int[ rowIdxNumNonz] ;
		rowUB = new double[ rowIdxNumNonz] ;
		rowLB = new double[ rowIdxNumNonz] ;
		
		thetaNumNonz = thetaNumNonz_;
		thetaIdx = new int[ thetaNumNonz] ;
		theta = new double[ thetaNumNonz] ;
		
		nodeID = -99999;
		

		
		
	}//end constructor for allocating arrays
	
	

	OSNode::~OSNode(){
		
		//garbage collection
		
		delete[] rowIdx;
		rowIdx = NULL;
		
		delete[] rowUB;
		rowUB = NULL;
		
		delete[] rowLB;
		rowLB = NULL;
		
		delete[] thetaIdx;
		thetaIdx = NULL;
		
		delete[] theta;
		theta = NULL;
		
		
		std::cout << "I AM DELETING A NODE IN OSNODE DESTRUCTOR" << std::endl;
	}//end default destructor
	
	