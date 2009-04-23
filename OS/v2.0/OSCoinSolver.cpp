/** @file CoinSolver.cpp
 * 
 * \brief This file defines the CoinSolver class.
 * \detail Read an OSInstance object and convert to COIN data structures
 *
 * @author  Robert Fourer,  Jun Ma, Kipp Martin, 
 * @version 1.0, 10/05/2005
 * @since   OS1.0
 *
 * \remarks
 * Copyright (C) 2005, Robert Fourer, Jun Ma, Kipp Martin,
 * Northwestern University, and the University of Chicago.
 * All Rights Reserved.
 * This software is licensed under the Common Public License. 
 * Please see the accompanying LICENSE file in root directory for terms.
 * 
 */


#define DEBUG

#include "OSCoinSolver.h"
#include "OSInstance.h"
#include "OSFileUtil.h"
#include "CoinTime.hpp"
#include "CglPreProcess.hpp"
#include "CglGomory.hpp"
#include "CglSimpleRounding.hpp"
#include "CglMixedIntegerRounding2.hpp"
#include "CglKnapsackCover.hpp"
#include "CglFlowCover.hpp"
#include "CbcModel.hpp"
#include "CbcBranchActual.hpp" //for CbcSOS

#include "OsiClpSolverInterface.hpp"
#include "OsiSymSolverInterface.hpp"
#include "OsiVolSolverInterface.hpp"

#include "OSDataStructures.h"
#include "OSParameters.h" 
#include "OSCommonUtil.h"
#include "OSMathUtil.h"

#include<map>
  
#include <iostream>
#ifdef HAVE_CTIME
# include <ctime>
#else
# ifdef HAVE_TIME_H
#  include <time.h>
# else
#  error "don't have header file for time"
# endif
#endif 
using std::cout; 
using std::endl; 
using std::ostringstream;



CoinSolver::CoinSolver() : 
osiSolver(NULL),
m_osilreader(NULL),
m_osolreader(NULL),
m_CoinPackedMatrix(NULL),
cbc_argv( NULL),
num_cbc_argv( 0),
cpuTime( 0)

{
osrlwriter = new OSrLWriter();
}

CoinSolver::~CoinSolver() {
	#ifdef DEBUG
	cout << "inside CoinSolver destructor" << endl;
	#endif
	if(m_osilreader != NULL) delete m_osilreader;
	m_osilreader = NULL;
	if(m_osolreader != NULL) delete m_osolreader;
	m_osolreader = NULL;
	delete m_CoinPackedMatrix;
	m_CoinPackedMatrix = NULL;
	delete osiSolver;
	if(osiSolver != NULL) osiSolver = NULL;
	delete osrlwriter;
	osrlwriter = NULL;
	delete osresult;
	osresult = NULL;
	if(num_cbc_argv > 0){
		int i;
		for(i = 0; i < num_cbc_argv; i++){
			//delete cbc_argv[ i];
		}
		//delete[] cbc_argv;
		cbc_argv = NULL;
	}
	cout << "leaving CoinSolver destructor" << endl;
}


void CoinSolver::buildSolverInstance() throw (ErrorClass) {
	try{
		osresult = new OSResult();
			if(osil.length() == 0 && osinstance == NULL) throw ErrorClass("there is no instance");
			clock_t start, finish;
			double duration;
			start = clock();
			if(osinstance == NULL){
				m_osilreader = new OSiLReader();
				osinstance = m_osilreader->readOSiL( osil);
			}
			finish = clock();
			duration = (double) (finish - start) / CLOCKS_PER_SEC;
			cout << "Parsing took (seconds): "<< duration << endl;
				cout << "Start Solve with a Coin Solver" << endl;
			// get the type of solver requested from OSoL string
			bool solverIsDefined = false;
			std::cout << "SOLVER NAME =  " << sSolverName << std::endl;
			if( sSolverName.find("clp") != std::string::npos){
				solverIsDefined = true;
				osiSolver = new OsiClpSolverInterface();
			}
			else{
				if( sSolverName.find("vol") != std::string::npos){
	             #ifdef COIN_HAS_VOL
					solverIsDefined = true;
					osiSolver = new OsiVolSolverInterface();
	             #endif
				}
				else{
					if( sSolverName.find( "cplex") != std::string::npos){
						#ifdef COIN_HAS_CPX
						solverIsDefined = true;
						osiSolver = new OsiCpxSolverInterface();
						#endif
					}
					else{
						if(sSolverName.find( "glpk") != std::string::npos){
							#ifdef COIN_HAS_GLPK
							solverIsDefined = true;
							osiSolver = new OsiGlpkSolverInterface();
							#endif
						}
						else{
							if(sSolverName.find( "dylp") != std::string::npos){
								#ifdef COIN_HAS_DYLP
								solverIsDefined = true;
								osiSolver = new OsiDylpSolverInterface();
								#endif
							}
							else{
								if( sSolverName.find( "symphony") != std::string::npos) {
									#ifdef COIN_HAS_SYMPHONY
									solverIsDefined = true;
									osiSolver = new OsiSymSolverInterface();
									#endif
								}
								else{
									// default solver is CBC
									solverIsDefined = true;
									osiSolver = new OsiClpSolverInterface();
								}
							}
						}
					}
				}
			}

			if(solverIsDefined == false) throw ErrorClass("a supported solver was not defined");
			if(osinstance->getConstraintNumber() <= 0)throw ErrorClass("Coin solver Needs Constraints");
			if(osinstance->getVariableNumber() <= 0)throw ErrorClass("Coin solver requires decision variables");
			if(osinstance->getObjectiveNumber() <= 0) throw ErrorClass("Coin solver needs an objective function");
			if(osinstance->getLinearConstraintCoefficientNumber() <= 0) throw ErrorClass("Coin solver needs linear constraints");
			if(!setCoinPackedMatrix() ) throw ErrorClass("Problem generating coin packed matrix");
			osiSolver->loadProblem(*m_CoinPackedMatrix, osinstance->getVariableLowerBounds(), 
				osinstance->getVariableUpperBounds(),  
				osinstance->getDenseObjectiveCoefficients()[0], 
				osinstance->getConstraintLowerBounds(), osinstance->getConstraintUpperBounds()
			);
			//dataEchoCheck();	
			if(osinstance->getObjectiveNumber() == 0) throw ErrorClass("there is no objective function");
			if( osinstance->getObjectiveMaxOrMins()[0] == "min") osiSolver->setObjSense(1.0);
			else osiSolver->setObjSense(-1.0);
			// set the integer variables
			int *intIndex = NULL;
			int i = 0;
			int k = 0;
			char *varType;
			int numOfIntVars = osinstance->getNumberOfIntegerVariables() + osinstance->getNumberOfBinaryVariables();
			if(numOfIntVars > 0) {
				intIndex = new int[ numOfIntVars];
				varType = osinstance->getVariableTypes();
				for(i = 0; i < osinstance->getVariableNumber(); i++){
					if( (varType[i] == 'B') || (varType[i]) == 'I' ) {
						intIndex[k++] = i;
					}
				}
				osiSolver->setInteger( intIndex,  numOfIntVars);
			}
			if(numOfIntVars > 0){ 
				delete[] intIndex;
				intIndex = NULL;
			}
			bCallbuildSolverInstance = true;
	}
	catch(const ErrorClass& eclass){
		std::cout << "THERE IS AN ERROR" << std::endl;
		osresult->setGeneralMessage( eclass.errormsg);
		osresult->setGeneralStatusType( "error");
		osrl = osrlwriter->writeOSrL( osresult);
		throw ErrorClass( osrl) ;
	}				
}//end buildSolverInstance()



void CoinSolver::setSolverOptions() throw (ErrorClass) {

	  

	// the osi maps
	// the OsiHintParamameter Map
	std::map<std::string, OsiHintParam> hintParamMap;
	hintParamMap["OsiDoPresolveInInitial"] = OsiDoPresolveInInitial;
	hintParamMap["OsiDoDualInInitial"] = OsiDoDualInInitial;
	hintParamMap["OsiDoPresolveInResolve"] = OsiDoPresolveInResolve;
	hintParamMap["OsiDoDualInResolve"] = OsiDoDualInResolve;
	hintParamMap["OsiDoScale"] = OsiDoScale;
	hintParamMap["OsiDoCrash"] = OsiDoCrash;
	hintParamMap["OsiDoReducePrint"] = OsiDoReducePrint;
	hintParamMap["OsiDoInBranchAndCut"] =  OsiDoInBranchAndCut;
	hintParamMap["OsiLastHintParam"] = OsiLastHintParam;
	//
	// the OsiHintStrength Map
	std::map<std::string, OsiHintStrength> hintStrengthMap;
	hintStrengthMap["OsiHintIgnore"] = OsiHintIgnore;
	hintStrengthMap["OsiHintTry"] = OsiHintTry;
	hintStrengthMap["OsiHintDo"] = OsiHintDo;
	hintStrengthMap["OsiForceDo"] = OsiForceDo;
	//
	// the OsiStrParam Map
	std::map<std::string, OsiStrParam> strParamMap;
	strParamMap["OsiProbName"] = OsiProbName;
	strParamMap["OsiSolverName"] = OsiSolverName;
	strParamMap["OsiLastStrParam"] = OsiLastStrParam;
	//
	// the OsiDblParam Map
	std::map<std::string, OsiDblParam>  dblParamMap;
	dblParamMap["OsiDualObjectiveLimit"] = OsiDualObjectiveLimit;
	dblParamMap["OsiPrimalObjectiveLimit"] = OsiPrimalObjectiveLimit;
	dblParamMap["OsiDualTolerance"] = OsiDualTolerance;	
	dblParamMap["OsiPrimalTolerance"] = OsiPrimalTolerance;
	dblParamMap["OsiObjOffset"] = OsiObjOffset;
	dblParamMap["OsiLastDblParam"] = OsiLastDblParam;
	//
	//
	// the OsiIntParam Map
	std::map<std::string, OsiIntParam>  intParamMap;
	intParamMap["OsiMaxNumIteration"] = OsiMaxNumIteration;
	intParamMap["OsiMaxNumIterationHotStart"] = OsiMaxNumIterationHotStart;
	intParamMap["OsiNameDiscipline"] = OsiNameDiscipline;	
	intParamMap["OsiLastIntParam"] = OsiLastIntParam;
	//
	//
	// initialize low level of printing
	
	
	/* 
	 * start default settings -- these get set
	 * even when the OSOption object is NULL
	 * 
	 * */
	OsiHintStrength hintStrength = OsiHintTry; //don't want too much output
	osiSolver->setHintParam(OsiDoReducePrint, true, hintStrength);
	osiSolver->setDblParam(OsiObjOffset, osinstance->getObjectiveConstants()[0]);
	/* 
	 * end default settings 
	 * 
	 * */
	
	//
	try{
		if(osoption == NULL && osol.length() > 0)
		{
			m_osolreader = new OSoLReader();
			osoption = m_osolreader->readOSoL( osol);
		}

		if(osoption != NULL){

			std::cout << "number of solver options "  <<  osoption->getNumberOfSolverOptions() << std::endl;
			if( osoption->getNumberOfSolverOptions() <= 0) return;
			this->bSetSolverOptions = true;
			std::vector<SolverOption*> optionsVector;
			//get the osi options
			optionsVector = osoption->getSolverOptions( "osi");
			int num_osi_options = optionsVector.size();
			int i;
			char *pEnd;
			bool yesNo;

			for(i = 0; i < num_osi_options; i++){
				std::cout << "osi solver option  "  << optionsVector[ i]->name << std::endl;
				if (optionsVector[ i]->type == "OsiHintStrength" ){
					if( hintStrengthMap.find( optionsVector[ i]->name ) != hintStrengthMap.end() ){
						hintStrength = hintStrengthMap[ optionsVector[ i]->name] ;
					}
				}
			}
			for(i = 0; i < num_osi_options; i++){
				std::cout << "osi solver option  "  << optionsVector[ i]->name << std::endl;

				if (optionsVector[ i]->type == "OsiHintParam" ){
					
					if( optionsVector[ i]->value == "true" ) {
						yesNo = true;
					}
					else{
						yesNo = false;
					}	
					if( hintParamMap.find( optionsVector[ i]->name ) != hintParamMap.end() ){
						
						osiSolver->setHintParam( hintParamMap[ optionsVector[ i]->name] , yesNo, hintStrength);
					}
					
				}
				else if(optionsVector[ i]->type == "OsiStrParam" ){
					
					if( strParamMap.find( optionsVector[ i]->name ) != strParamMap.end() ){
						
						osiSolver->setStrParam( strParamMap[ optionsVector[ i]->name] , optionsVector[ i]->value);
					}	

				}
				else if(optionsVector[ i]->type == "OsiDblParam" ){
					
					if( dblParamMap.find( optionsVector[ i]->name ) != dblParamMap.end() ){
						
						osiSolver->setDblParam( dblParamMap[ optionsVector[ i]->name] , os_strtod( optionsVector[ i]->value.c_str(), &pEnd ));
					}				
					
				}
				else if(optionsVector[ i]->type == "OsiIntParam" ){
					
					
					if( intParamMap.find( optionsVector[ i]->name ) != intParamMap.end() ){
						
						osiSolver->setIntParam( intParamMap[ optionsVector[ i]->name] , atoi( optionsVector[ i]->value.c_str() ) );
					}						
					
				}
			}
			
			// treat Cbc separately to take advantage of CbcMain1()
			
			



			//if(optionsVector.size() > 0) optionsVector.clear();
//			if( !optionsVector.empty() ) optionsVector.clear();	//HIG: This must eventually come out
			
			if( sSolverName.find( "cbc") != std::string::npos) {	
				// get Cbc options		
				optionsVector = osoption->getSolverOptions( "cbc");
				int num_cbc_options = optionsVector.size();	
				char *cstr;
				std::string cbc_option;
				// we are going to add a log level option -- it can be overridden
				num_cbc_argv = optionsVector.size() + 2;
				cbc_argv = new const char*[ num_cbc_argv];
				
				// the first option
				cbc_option = "OS";
				cstr = new char [cbc_option.size() + 1];
				strcpy (cstr, cbc_option.c_str());
				cbc_argv[ 0] = cstr;
				
				
				for(i = 0; i < num_cbc_options; i++){
					std::cout << "cbc solver option  "  << optionsVector[ i]->name << std::endl;
					std::cout << "cbc solver value  "  << optionsVector[ i]->value << std::endl;
					if(optionsVector[ i]->value.length() > 0){
						cbc_option = "-" + optionsVector[ i]->name +"="+optionsVector[ i]->value;
					}
					else{
						cbc_option = "-" + optionsVector[ i]->name ;
					}
					cstr = new char [cbc_option.size() + 1];
					strcpy (cstr, cbc_option.c_str());
					cbc_argv[i +  1] = cstr;		
				}
				
				// the quit option
				cbc_option = "-quit";
				cstr = new char [cbc_option.size() + 1];
				strcpy (cstr, cbc_option.c_str());
				cbc_argv[ num_cbc_argv - 1] = cstr;

			}//end of cbc if
			
			// also need to treat SYMPHONY differently
			
			
			// set some OSI options
	#ifdef COIN_HAS_SYMPHONY
			//if(optionsVector.size() > 0) optionsVector.clear();
			if( !optionsVector.empty() ) optionsVector.clear();	
			//first the number of processors -- applies only to SYMPHONY
			if( sSolverName.find( "symphony") != std::string::npos) {
				OsiSymSolverInterface * si =
				dynamic_cast<OsiSymSolverInterface *>(osiSolver) ;
				
				optionsVector = osoption->getSolverOptions( "symphony");
				int num_sym_options = optionsVector.size();
				
				
				for(i = 0; i < num_sym_options; i++){
					std::cout << "symphony solver option  "  << optionsVector[ i]->name << std::endl;
					std::cout << "symphony solver value  "  << optionsVector[ i]->value << std::endl;
					if( optionsVector[ i]->name  ==  "max_active_nodes"){
						si->setSymParam("max_active_nodes",   optionsVector[ i]->value);
					}
					else{
						//ignore for now
					}	
				}				
			}
	#endif	   //symphony end		
			
			
			//now set initial values
			int n,m,k;
			if (osoption != NULL)
				m = osoption->getNumberOfInitVarValues();
			else
				m = 0;
#ifdef DEBUG
			cout << "number of variables initialed: " << m << endl;
#endif

			if (m > 0)
			{
#ifdef DEBUG
			cout << "get initial values " << endl;
#endif
				n = osinstance->getVariableNumber();
				double* denseInitVarVector;
				denseInitVarVector = new double[n];
				bool* initialed;
				initialed = new bool[n];

				for(k = 0; k < n; k++)
					initialed[k] = false;

				InitVarValue**  initVarVector = osoption->getInitVarValuesSparse();
#ifdef DEBUG
				cout << "done " << endl;
#endif

				double initval;
				for(k = 0; k < m; k++)
				{	cout << "process component " << k << " -- index " << initVarVector[k]->idx << endl;
					i = initVarVector[k]->idx;
					if (initVarVector[k]->idx > n)
						throw ErrorClass ("Illegal index value in variable initialization");

						initval = initVarVector[k]->value;
						if (osinstance->instanceData->variables->var[k]->ub == OSDBL_MAX)
						{	if (osinstance->instanceData->variables->var[k]->lb > initval)
								throw ErrorClass ("Initial value outside of bounds");
						}
						else
							if (osinstance->instanceData->variables->var[k]->lb == -OSDBL_MAX)
							{	if (osinstance->instanceData->variables->var[k]->ub < initval)
									throw ErrorClass ("Initial value outside of bounds");
							}
							else
							{	if ((osinstance->instanceData->variables->var[k]->lb > initval) ||
									(osinstance->instanceData->variables->var[k]->ub < initval))
									throw ErrorClass ("Initial value outside of bounds");
							}

					denseInitVarVector[initVarVector[k]->idx] = initval;
					initialed[initVarVector[k]->idx] = true;
				}

				double default_initval;
				default_initval = 0.0;

				for(k = 0; k < n; k++)
				{	cout << "verify component " << k << endl;
					if (!initialed[k])
						if (osinstance->instanceData->variables->var[k]->ub == OSDBL_MAX)
							if (osinstance->instanceData->variables->var[k]->lb <= default_initval)
								denseInitVarVector[k] = default_initval;
							else
								denseInitVarVector[k] = osinstance->instanceData->variables->var[k]->lb;
						else
							if (osinstance->instanceData->variables->var[k]->lb == -OSDBL_MAX)
								if (osinstance->instanceData->variables->var[k]->ub >= default_initval)
									denseInitVarVector[k] = default_initval;
								else
									denseInitVarVector[k] = osinstance->instanceData->variables->var[k]->ub;
							else
								if ((osinstance->instanceData->variables->var[k]->lb <= default_initval) && 
									(osinstance->instanceData->variables->var[k]->ub >= default_initval))
									denseInitVarVector[k] = default_initval;
								else
									if (osinstance->instanceData->variables->var[k]->lb > default_initval)
										denseInitVarVector[k] = osinstance->instanceData->variables->var[k]->lb;
									else
										denseInitVarVector[k] = osinstance->instanceData->variables->var[k]->ub;
									denseInitVarVector[k] = default_initval;
									denseInitVarVector[k] = osinstance->instanceData->variables->var[k]->lb;
				}
#ifdef DEBUG
				cout << "set initial values: " << endl;
				for (k=0; k < n; k++)
					cout << "  " << k << ": " << denseInitVarVector[k] << endl;	
#endif
				osiSolver->setColSolution( denseInitVarVector);
				delete[] denseInitVarVector;
				delete[] initialed;
#ifdef DEBUG
			cout << "done " << endl;
#endif

			}  //  end if (m > 0)		
		}// end of osoption if	
		
		
	}//end of try 
	catch(const ErrorClass& eclass){
		std::cout << "THERE IS AN ERROR" << std::endl;
		osresult->setGeneralMessage( eclass.errormsg);
		osresult->setGeneralStatusType( "error");
		osrl = osrlwriter->writeOSrL( osresult);
		throw ErrorClass( osrl) ;
	}				
}//end setSolverOptions() 


bool CoinSolver::setCoinPackedMatrix(){
	bool columnMajor = osinstance->getLinearConstraintCoefficientMajor();
	try{
		int maxGap = 0;
		m_CoinPackedMatrix = new CoinPackedMatrix(
			columnMajor, //Column or Row Major
			columnMajor? osinstance->getConstraintNumber() : osinstance->getVariableNumber(), //Minor Dimension
			columnMajor? osinstance->getVariableNumber() : osinstance->getConstraintNumber(), //Major Dimension
			osinstance->getLinearConstraintCoefficientNumber(), //Number of nonzeroes
			columnMajor? osinstance->getLinearConstraintCoefficientsInColumnMajor()->values : osinstance->getLinearConstraintCoefficientsInRowMajor()->values, //Pointer to matrix nonzeroes
			columnMajor? osinstance->getLinearConstraintCoefficientsInColumnMajor()->indexes : osinstance->getLinearConstraintCoefficientsInRowMajor()->indexes, //Pointer to start of minor dimension indexes -- change to allow for row storage
			columnMajor? osinstance->getLinearConstraintCoefficientsInColumnMajor()->starts : osinstance->getLinearConstraintCoefficientsInRowMajor()->starts, //Pointers to start of columns.
			0,   0, maxGap ); 

		return true;
	}
	catch(const ErrorClass& eclass){
		osresult->setGeneralMessage( eclass.errormsg);
		osresult->setGeneralStatusType( "error");
		osrl = osrlwriter->writeOSrL( osresult);
		throw ;
	}
} // end setCoinPackedMatrix

void CoinSolver::solve() throw (ErrorClass) {
	// make sure the solver instance exists
	if( this->bCallbuildSolverInstance == false) buildSolverInstance();
	if( this->bSetSolverOptions == false) setSolverOptions();
	
	// first check the various solvers and see if they are of the proper problem type
	if( osinstance->getNumberOfIntegerVariables() + osinstance->getNumberOfBinaryVariables() > 0){
		// throw an exception if we have a solver that cannot do integer programming
		if( sSolverName.find("clp") != std::string::npos) throw ErrorClass( "Clp cannot do integer programming");
		if( sSolverName.find("vol") != std::string::npos) throw ErrorClass( "Vol cannot do integer programming");
		if( sSolverName.find("dylp") != std::string::npos) throw ErrorClass( "DyLP cannot do integer programming");
		if( sSolverName.find("ipopt") != std::string::npos) throw ErrorClass( "Ipopt cannot do integer programming");
	}
	if( (osinstance->getNumberOfNonlinearExpressions() > 0)
		|| (osinstance->getNumberOfQuadraticTerms() > 0) ){
		throw ErrorClass( "This COIN-OR Solver is not configured for nonlinear programming");
	}
	// if we are throw an exception if the problem is nonlinear

	// resultHeader information
	if(osresult->setServiceName("Solved with Coin Solver: " + sSolverName) != true)
		throw ErrorClass("OSResult error: setServiceName");
	if(osresult->setInstanceName(  osinstance->getInstanceName()) != true)
		throw ErrorClass("OSResult error: setInstanceName");
	//if(osresult->setJobID( osresultdata->jobID) != true)
	//	throw ErrorClass("OSResult error: setJobID");
	//if(osresult->setGeneralMessage( osresultdata->message) != true)
	//	throw ErrorClass("OSResult error: setGeneralMessage");
	// set basic problem parameters
	if(osresult->setVariableNumber( osinstance->getVariableNumber()) != true)
		throw ErrorClass("OSResult error: setVariableNumer");
	if(osresult->setObjectiveNumber( 1) != true)
		throw ErrorClass("OSResult error: setObjectiveNumber");
	if(osresult->setConstraintNumber( osinstance->getConstraintNumber()) != true)
		throw ErrorClass("OSResult error: setConstraintNumber");
	if(osresult->setSolutionNumber(  1) != true)
		throw ErrorClass("OSResult error: setSolutionNumer");	
	//
	try{
		double start = CoinCpuTime();
		try{
			if( sSolverName.find( "cbc") != std::string::npos){
			//if( osinstance->getNumberOfIntegerVariables() + osinstance->getNumberOfBinaryVariables() > 0){
			// just use simple branch and bound for anything but cbc
				CbcModel model(  *osiSolver);
				CbcMain0(  model);		

				
				// make sure we define cbc_argv if not done already when reading options
				if(num_cbc_argv <= 0){
					char *cstr;
					std::string cbc_option;
					num_cbc_argv = 4;
					cbc_argv = new const char*[ num_cbc_argv];
			
					// the first option
					cbc_option = "OS";
					cstr = new char [cbc_option.size() + 1];
					strcpy (cstr, cbc_option.c_str());
					cbc_argv[ 0] = cstr;
					
					
					// the log option -- by default minimal printing
					cbc_option = "-log=0";
					cstr = new char [cbc_option.size() + 1];
					strcpy (cstr, cbc_option.c_str());
					cbc_argv[ 1] = cstr;
					
					
					// the solve option
					cbc_option = "-solve";
					cstr = new char [cbc_option.size() + 1];
					strcpy (cstr, cbc_option.c_str());
					cbc_argv[ 2] = cstr;
			
					// the quit option
					cbc_option = "-quit";
					cstr = new char [cbc_option.size() + 1];
					strcpy (cstr, cbc_option.c_str());
					cbc_argv[ 3] = cstr;
								
				}
				std::cout << "CALLING THE CBC SOLVER CBCMAIN1()" << std::endl;
				int i;
				for(i = 0; i < num_cbc_argv; i++){
					std::cout << "Cbc Option: "  << cbc_argv[ i]   <<  std::endl;
				}
				CbcMain1( num_cbc_argv, cbc_argv, model);	
				
				//do the garbage collection on cbc_argv
				for(i = 0; i < num_cbc_argv; i++){
					delete[]  cbc_argv[ i];	
					cbc_argv[i] = NULL;
				}
				delete[] cbc_argv;
				cbc_argv = NULL;
				
				
				// create a solver 
				OsiSolverInterface *solver = model.solver();
				cpuTime = CoinCpuTime() - start;

				writeResult( solver);
			}
			else{ // use other solvers
				//if an LP just do initial solve
				if( osinstance->getNumberOfIntegerVariables() + osinstance->getNumberOfBinaryVariables() > 0){
					osiSolver->branchAndBound();
				}
				else{
					osiSolver->initialSolve();
				}
				cpuTime = CoinCpuTime() - start;
			
				writeResult( osiSolver);
			}
			

		}
		catch(CoinError e){
			std::string errmsg;
			errmsg = "Coin Solver Error: " + e.message() + "\n" + " see method "  
				+ e.methodName() + " in class " + e.className();
			throw ErrorClass( errmsg );
		}

	}
	catch(const ErrorClass& eclass){
		osresult->setGeneralMessage( eclass.errormsg);
		osresult->setGeneralStatusType( "error");
		osrl = osrlwriter->writeOSrL( osresult);
		throw ;
	}
} // end solve

std::string CoinSolver::getCoinSolverType(std::string lcl_osol){
// this is deprecated, but keep it around
	try{
		if( lcl_osol.find( "clp") != std::string::npos){
			return "coin_solver_glpk";
		}
		else{
			if( lcl_osol.find( "cbc") != std::string::npos){
				return "coin_solver_cpx";
			}
			else{
				if( lcl_osol.find( "cpx") != std::string::npos){
					return "coin_solver_clp";
				}
				else{
					if(lcl_osol.find( "glpk") != std::string::npos){
						return "";
					}
					else throw ErrorClass("a supported solver was not defined");
				}
			}
		}
	}
	catch(const ErrorClass& eclass){
		osresult->setGeneralMessage( eclass.errormsg);
		osresult->setGeneralStatusType( "error");
		osrl = osrlwriter->writeOSrL( osresult);
		throw ;
	}
} // end getCoinSolverType

void CoinSolver::dataEchoCheck(){
	int i;
	// print out problem parameters
	cout << "This is problem:  " << osinstance->getInstanceName() << endl;
	cout << "The problem source is:  " << osinstance->getInstanceSource() << endl;
	cout << "The problem description is:  " << osinstance->getInstanceDescription() << endl;
	cout << "number of variables = " << osinstance->getVariableNumber() << endl;
	cout << "number of Rows = " << osinstance->getConstraintNumber() << endl;

	// print out the variable information
	if(osinstance->getVariableNumber() > 0){
		for(i = 0; i < osinstance->getVariableNumber(); i++){
			if(osinstance->getVariableNames() != NULL) cout << "variable Names  " << osinstance->getVariableNames()[ i]  << endl;
			if(osinstance->getVariableTypes() != NULL) cout << "variable Types  " << osinstance->getVariableTypes()[ i]  << endl;
			if(osinstance->getVariableLowerBounds() != NULL) cout << "variable Lower Bounds  " << osinstance->getVariableLowerBounds()[ i]  << endl;
			if(osinstance->getVariableUpperBounds() != NULL) cout << "variable Upper Bounds  " <<  osinstance->getVariableUpperBounds()[i] << endl;
		}
	}
	
	// print out objective function information
	if(osinstance->getVariableNumber() > 0 || osinstance->instanceData->objectives->obj != NULL || osinstance->instanceData->objectives->numberOfObjectives > 0){
		if( osinstance->getObjectiveMaxOrMins()[0] == "min")  cout <<  "problem is a minimization" << endl;
		else cout <<  "problem is a maximization" << endl;
		for(i = 0; i < osinstance->getVariableNumber(); i++){
			cout << "OBJ COEFFICIENT =  " <<  osinstance->getDenseObjectiveCoefficients()[0][i] << endl;
		}
	}
	// print out constraint information
	if(osinstance->getConstraintNumber() > 0){
		for(i = 0; i < osinstance->getConstraintNumber(); i++){
			if(osinstance->getConstraintNames() != NULL) cout << "row name = " << osinstance->getConstraintNames()[i] <<  endl;
			if(osinstance->getConstraintLowerBounds() != NULL) cout << "row lower bound = " << osinstance->getConstraintLowerBounds()[i] <<  endl;
			if(osinstance->getConstraintUpperBounds() != NULL) cout << "row upper bound = " << osinstance->getConstraintUpperBounds()[i] <<  endl; 
		}
	}
	
	// print out linear constraint data
	if(m_CoinPackedMatrix != NULL) m_CoinPackedMatrix->dumpMatrix();
} // end dataEchoCheck



void CoinSolver::writeResult(OsiSolverInterface *solver){
	double *x = NULL;
	double *y = NULL;
	double *z = NULL;
	int i = 0;
	std::string *rcost = NULL;
	int solIdx = 0;
	int n, m;
	std::string description = "";
	osresult->setGeneralStatusType("normal");
//	osresult->resultHeader->time = os_dtoa_format(  cpuTime);
	osresult->addTimingInformation("cpuTime","total","second","",cpuTime);
	if (solver->isProvenOptimal() == true){
		osresult->setSolutionStatus(solIdx, "optimal", description);
		/* Retrieve the solution */
		x = new double[osinstance->getVariableNumber() ];
		y = new double[osinstance->getConstraintNumber() ];
		z = new double[1];
		n = osinstance->getVariableNumber();
		m = osinstance->getConstraintNumber();
		rcost = new std::string[ osinstance->getVariableNumber()];
		//
		*(z + 0)  =  solver->getObjValue();
		osresult->setObjectiveValues(solIdx, z, 1);
		for(i=0; i < osinstance->getVariableNumber(); i++){
			*(x + i) = solver->getColSolution()[i];
		}
		osresult->setPrimalVariableValues(solIdx, x, n);
		// Symphony does not get dual prices
		if( sSolverName.find( "symphony") == std::string::npos && osinstance->getNumberOfIntegerVariables() == 0 && osinstance->getNumberOfBinaryVariables() == 0) {
			for(i=0; i <  osinstance->getConstraintNumber(); i++){
				*(y + i) = solver->getRowPrice()[ i];
			}
			osresult->setDualVariableValues(solIdx, y, osinstance->getConstraintNumber());
		}
		//
		//
		// now put the reduced costs into the osrl
		// Symphony does not get reduced costs
		if( sSolverName.find( "symphony") == std::string::npos && osinstance->getNumberOfIntegerVariables() == 0 && osinstance->getNumberOfBinaryVariables() == 0){
			int numberOfOtherVariableResults = 1;
			int otherIdx = 0;
			// first set the number of Other Variable Results
			osresult->setNumberOfOtherVariableResults(solIdx, numberOfOtherVariableResults);
			ostringstream outStr;
			int numberOfVar =  osinstance->getVariableNumber();
			for(i=0; i < numberOfVar; i++){
				rcost[ i] = os_dtoa_format( solver->getReducedCost()[ i]);
			}
			osresult->setAnOtherVariableResult(solIdx, otherIdx, "reduced costs", "the variable reduced costs", rcost, osinstance->getVariableNumber());			
			// end of settiing reduced costs
		}					
	}
	else{ 
		if(solver->isProvenPrimalInfeasible() == true) 
			osresult->setSolutionStatus(solIdx, "infeasible", description);
		else
			if(solver->isProvenDualInfeasible() == true) 
				osresult->setSolutionStatus(solIdx, "dualinfeasible", description);
			else
				osresult->setSolutionStatus(solIdx, "other", description);
	}
	osrl = osrlwriter->writeOSrL( osresult);
	if(osinstance->getVariableNumber() > 0) delete[] x;
	x = NULL;
	if(osinstance->getConstraintNumber()) delete[] y;
	y = NULL;
	delete[] z;	
	z = NULL;
	if(osinstance->getVariableNumber() > 0){
		delete[] rcost;
		rcost = NULL;
	}
}

