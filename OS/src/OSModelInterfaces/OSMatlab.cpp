/** @file TestOS.cpp
 * 
 * @author  Robert Fourer,  Jun Ma, Kipp Martin, 
 * @version 1.0, 10/05/2005
 * @since   OS1.0
 *
 * \remarks
 * Copyright (C) 2005, Robert Fourer, Jun Ma, Kipp Martin, Huanyuan Sheng
 * Northwestern University, and the University of Chicago.
 * All Rights Reserved.
 * This software is licensed under the Common Public License. 
 * Please see the accompanying LICENSE file in root directory for terms.
 * 
 */


#include <iostream>
#include <sstream>
#include <string>
#include<stdlib.h>
#include "OSMatlab.h"

  

using std::cout;
using std::endl;
using std::ostringstream; 


 

OSMatlab::OSMatlab() {
	sparseMat = NULL;
	bu = NULL;
  	bl = NULL;
  	obj = NULL;
  	vu = NULL;
  	vl = NULL;
  	vu = NULL;
  	objType = 0;
  	osinstance = NULL;
  	varType = NULL;
  	numVar = 0;
  	numCon = 0;
}//end OSMatlab


OSMatlab::~OSMatlab() {
	
}//end ~OSMatlab

std::string OSMatlab::display() {
	ostringstream outStr;
	int i;
	if(osinstance != NULL){
		OSiLWriter *osilwriter;
		osilwriter = new OSiLWriter();
		//for(i = 0; i < (sparseMat->valueSize) - 1 ; i++){
		//	outStr << sparseMat->values[ i] << endl;
		//}
		outStr << osilwriter->writeOSiL( osinstance);
		outStr << endl;
		outStr << "Now Solve with LINDO" << endl;
		outStr << "create a new LINDO Solver for OSiL string solution" << endl;
		CoinSolver *m_Solver;	
		m_Solver = new CoinSolver();
		m_Solver->m_sSolverName = "cbc";
		m_Solver->osinstance = osinstance;
		outStr << "call the LINDO Solver" << endl;
		m_Solver->solve();
		outStr << "Here is the LINDO solver solution" << endl;
		outStr << m_Solver->osrl << endl;
		// do the garbage collection 
		//lindo->osinstance = NULL;
		//delete lindo;
		return outStr.str();
		//return  osilwriter->writeOSiL( osinstance);
	}else
	return "there was no instance";

  //using CppAD::NearEqual;
  /*
  FileUtil *fileUtil = NULL;  
	DefaultSolver *m_Solver  = NULL;
	std::string cbcFileName;
	std::string osil;
    std::string dataDir;
  	std::string osol = "<osol></osoL>";
  	// get the input files
     const char dirsep =  CoinFindDirSeparator();
  	// Set directory containing mps data files.
    dataDir = dirsep == '/' ? "../data/" : "..\\data\\";
  	try{
        std::string test = "";
		cbcFileName = dataDir + "parincLinear.osil";
		osil = fileUtil->getFileAsString( &cbcFileName[0]);
		m_Solver = new CoinSolver();
		m_Solver->m_sSolverName = "cbc";
		m_Solver->osil = osil;
		m_Solver->osol = osol;  
		m_Solver->osinstance = NULL;
		m_Solver->solve();
        test = m_Solver->osrl;
       // delete m_Solver;
		m_Solver = NULL;
		return test;
	}
	catch(const ErrorClass& eclass){
		//cout << "OSrL =  " <<  m_Solver->osrl <<  endl;
		return  m_Solver->osrl;
		*/
	/*
    	try{
       lindoFileName = dataDir + "parincLinear.osil"; 
		osil = fileUtil->getFileAsString( &lindoFileName[0]);
		mexPrintf("Got file lindoapiaddins:\n");
		m_Solver = new LindoSolver();	
		m_Solver->osil = osil;
		m_Solver->osol = osol;
		m_Solver->osinstance = NULL;
		mexPrintf("Call the Lindo Solver:\n");
		m_Solver->solve();
        mexPrintf("Done solving in LIndo\n");
        std::string test = m_Solver->osrl;
        mexPrintf( &test[0]);
		m_Solver->osinstance = NULL;
		//delete m_Solver;
		m_Solver = NULL;
  
	}
	catch(const ErrorClass& eclass){
	}
	*/
  //end OS stuff
}//end display

void OSMatlab::createOSInstance(){
	int i;
	std::string varname = "";
	std::string conname = "";
	osinstance = new OSInstance();
	//
	// put in some of the OSInstance <instanceHeader> information
	//osinstance->setInstanceSource("An example from the LINDO API samples directory");
	osinstance->setInstanceDescription("A MATLAB Created Problem");
	//
	// now put in the OSInstance <instanceData> information
	// 
	// first the variables
	osinstance->setVariableNumber( numVar);   
	//addVariable(int index, string name, double lowerBound, double upperBound, char type, double init, string initString);
	// we could use setVariables() and add all the variable with one method call -- below is easier
	for(i = 0; i < numVar; i++){
		varname = "x" ;
		osinstance->addVariable(i, "" , vl[ i], vu[ i], varType[ i], OSNAN, "");
	}
	//
	// now add the objective function
	osinstance->setObjectiveNumber( 1);
	// now the coefficient
	SparseVector *objcoeff;
	objcoeff = new SparseVector( numVar);   
	objcoeff->indexes = new int[ numVar];
	objcoeff->values = new double[ numVar];
	for(i = 0; i < numVar; i++){
		objcoeff->indexes[ i] = i;
		objcoeff->values[ i] = obj[ i];
	}
	//bool addObjective(int index, string name, string maxOrMin, double constant, double weight, SparseVector* objectiveCoefficients);
	osinstance->addObjective(-1, "objfunction", "max", 0.0, 1.0, objcoeff);
	//
	// now the constraints
	osinstance->setConstraintNumber( numCon); 
	//bool addConstraint(int index, string name, double lowerBound, double upperBound, double constant);
	// note: we could use setConstraints() and add all the constraints with one method call -- below is easier
	for(i = 0; i < numCon; i++){
		osinstance->addConstraint(i, "", bl[ i], bu[ i], 0);
	}
	//
	// now add the <linearConstraintCoefficients>
	//bool setLinearConstraintCoefficients(int numberOfValues, bool isColumnMajor, 
	//double* values, int valuesBegin, int valuesEnd, 
	//int* indexes, int indexesBegin, int indexesEnd,   			
	//int* starts, int startsBegin, int startsEnd);	
	osinstance->setLinearConstraintCoefficients(sparseMat->valueSize, true, sparseMat->values, 0, sparseMat->valueSize - 1, 
		sparseMat->indexes, 0, sparseMat->valueSize - 1, sparseMat->starts, 0, sparseMat->startSize - 1);	
	display();
	return;
}// end createOSInstance
