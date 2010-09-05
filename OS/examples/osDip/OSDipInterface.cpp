/* $Id: OSDipInterface.cpp 3038 2009-11-07 11:43:44Z Gassmann $ */
/** @file OSDipInterface.cpp
 * 
 *
 * @author  Gus Gassmann, Jun Ma, Kipp Martin, 
 * @version 1.0, 21/July/2008
 * @since   OS1.1
 *
 * \remarks
 * Copyright (C) 2005-2008, Gus Gassmann, Jun Ma, Kipp Martin,
 * Dalhousie University,  Northwestern University, and the University of Chicago.
 * All Rights Reserved.
 * This software is licensed under the Common Public License. 
 * Please see the accompanying LICENSE file in root directory for terms.
 * 
 */

//===========================================================================//




#include "OSDipInterface.h"
#include "UtilMacrosDecomp.h"
 


//===========================================================================//
void OS_DipInterface::readOSiL(string & fileName){
	
	FileUtil *fileUtil = NULL; 
	fileUtil = new FileUtil();


	
	try{


		std::string osil;
		osil = fileUtil->getFileAsString( fileName.c_str() );
		m_osilreader = new OSiLReader();
		m_osinstance = m_osilreader->readOSiL( osil);
		
		
		
		
	bool columnMajor = false;
	double maxGap = 0;

	
	m_coinpm = new CoinPackedMatrix(
			columnMajor, //Column or Row Major
			columnMajor? m_osinstance->getConstraintNumber() : m_osinstance->getVariableNumber(), //Minor Dimension
			columnMajor? m_osinstance->getVariableNumber() : m_osinstance->getConstraintNumber(), //Major Dimension
			m_osinstance->getLinearConstraintCoefficientNumber(), //Number of nonzeroes
			columnMajor? m_osinstance->getLinearConstraintCoefficientsInColumnMajor()->values : 
				m_osinstance->getLinearConstraintCoefficientsInRowMajor()->values, //Pointer to matrix nonzeroes
			columnMajor? m_osinstance->getLinearConstraintCoefficientsInColumnMajor()->indexes : 
				m_osinstance->getLinearConstraintCoefficientsInRowMajor()->indexes, //Pointer to start of minor dimension indexes -- change to allow for row storage
			columnMajor? m_osinstance->getLinearConstraintCoefficientsInColumnMajor()->starts : 
			m_osinstance->getLinearConstraintCoefficientsInRowMajor()->starts, //Pointers to start of columns.
			0,   0, maxGap );
				
		
		
	
		delete fileUtil;
		fileUtil  = NULL;
		//kipp  -- worry about garbage collection here -- do I delete m_osinstance
		//delete  osilreader;
		//osilreader = NULL;
		
		
	}
	catch(const ErrorClass& eclass){
		std::cout << "PROBLEM READING OSiL File" <<std::endl;
		delete fileUtil;
		std::cout << eclass.errormsg <<  std::endl;
		fileUtil  = NULL;
		throw ErrorClass( eclass.errormsg);
		
	} 
	
	
}//end readOSiL


//===========================================================================//
void OS_DipInterface::readOSoL(string & fileName){
	
	FileUtil *fileUtil = NULL; 
	fileUtil = new FileUtil();	
	try{


		std::string osol;
		osol = fileUtil->getFileAsString( fileName.c_str() );
		m_osolreader = new OSoLReader();
		m_osoption = m_osolreader->readOSoL( osol);
	
		delete fileUtil;
		fileUtil  = NULL;
		//kipp  -- worry about garbage collection here -- do I delete m_osinstance
		//delete  osilreader;
		//osilreader = NULL;
		
		
	}
	catch(const ErrorClass& eclass){
		std::cout << "Problem reading OSoL File" <<std::endl;
		delete fileUtil;
		std::cout << eclass.errormsg <<  std::endl;
		fileUtil  = NULL;
		throw ErrorClass( eclass.errormsg);
		
	} 
	
	
}// end readOSoL

CoinPackedMatrix * OS_DipInterface::getCoinPackedMatrix( ){
	

	
	return m_coinpm;
}//end getCoinPackedMatrix

CoinPackedVector * OS_DipInterface::getRow( int i){
	
	 
	 
	 m_row =  new CoinPackedVector();
	 

	 int k;
	 
	 SparseMatrix* sm;
	 
	 sm = m_osinstance->getLinearConstraintCoefficientsInRowMajor();
	 
	 for(k = sm->starts[i];  k < sm->starts[i + 1];  k++){
		 
		 m_row->insert(sm->indexes[k], sm->values[k]);
		 
	 }
	 

	 
	 return m_row;
}//end getRow

const char* OS_DipInterface::getIntegerColumns(){
	
	int numVars = getVariableNumber();
	m_integerVars = new char[ numVars];
	int i;
	
	for(i  = 0; i < numVars;  i++){
		
		if( m_osinstance->getVariableTypes()[ i] == 'B' || m_osinstance->getVariableTypes()[i] == 'I') m_integerVars[i] = '1';
		else m_integerVars[i] = '0';
		
		
	}
	return m_integerVars;
	
}// end getIntegerColumns


double OS_DipInterface::getObjectiveOffset(){
	
	double offset = 0;
	//assume only one objective function for now
	
	if(getObjectiveConstants() != NULL){
		offset = getObjectiveConstants()[0];
	}
	
	return offset;
}// end getObjectiveOffset




/*
std::vector<std::set<int> > OS_DipInterface::getBlockVarIndexes(int whichBlock) {
	
	//get the variable indexes for each block in the model
	std::set<int> varSet; //variables indexes in the specific block whichBlock
	std::vector<std::set<int> > blockVars;
	
	try {
		if (m_osoption == NULL)
			throw ErrorClass("we have a null osoption");
	
		int numVar;
		int i;
		std::vector<OtherVariableOption*> otherVariableOptions;
		std::vector<OtherVariableOption*>::iterator vit;

		
		if ( m_osoption != NULL && m_osoption->getNumberOfOtherVariableOptions() > 0) {

			otherVariableOptions = m_osoption->getOtherVariableOptions("Dip");
			//iterate over the vector of contraint options
			for (vit = otherVariableOptions.begin(); vit
					!= otherVariableOptions.end(); vit++) {

				// see if we have a set of block variables
				// if so we insert into our vector of sets
				varSet.clear();
				// right now we assume blocks are ordered  -- we ignor value
				if ( (*vit)->name.compare("variableBlockSet") == 0  ) {

					
					numVar =  (*vit)->numberOfVar;
					
					for (i = 0; i < numVar; i++) {
						
						if ((*vit)->var[i]->idx >= m_osinstance->getVariableNumber() )
							throw ErrorClass(
									"found an invalid varaible index in OSoL file");

						
						varSet.insert( (*vit)->var[i]->idx);



					}//end for on variables in this block
				}// end of if on whichBlock
				
				blockVars.push_back( varSet);
			}//end for over constraint options
		}// if on ospton null

	} //end try
	
	catch (const ErrorClass& eclass) {

		std::cout << eclass.errormsg << std::endl;
		throw ErrorClass(eclass.errormsg);

	}
	return blockVars;
}//end getBlockVarIndexes
*/


double* OS_DipInterface::getObjectiveFunctionCoeff(){
	
	//Dip has only one objective function 
	
	return  m_osinstance->getDenseObjectiveCoefficients()[ 0];
}// end getObjectiveFunctionCoeff()



	void OS_DipInterface::initMembers(){
		m_isProvenOptimal =  false;
		m_bestKnownLB     = -1.e20;
		m_bestKnownUB     =  1.e20;
		m_coinpm = NULL;
		

		
	}



	/** Default constructor. */
	OS_DipInterface::OS_DipInterface(){
		initMembers();
	}

	/** Default constructor. Takes an instance of UtilParameters */
	OS_DipInterface::OS_DipInterface(string & fileName) {
		initMembers();
		readOSiL(fileName);
	}

	OS_DipInterface::~OS_DipInterface() {
		std::cout << "INSIDE OS DIP INTERFACE DESTRUCTOR" << std::endl;
		if(m_osilreader != NULL) delete m_osilreader;
		if(m_osolreader != NULL) delete m_osolreader;
		delete m_coinpm;
	}