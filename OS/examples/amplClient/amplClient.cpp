/** @file OSSolverService.cpp
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





/*
Using AMLP with nl2fml.  Here is the sequence of AMPL commands
with the parinc problem:

If you want to name the file and not use the AMPL randomly 
generate file name, do the following:
   model hs71.mod;
   option solver amplClient;
   option amplClient_options "solver lindo";
   option lindo_options "http://128.135.130.17:8080/lindo/LindoSolverService.jws";
   write gtestfile;
   solve;
	
of course, replacing gtestfile with btestfile generates a binary nl file. 
// glpk is the default -- replace lindo with clp for that solver
// on the Mac it wants the nl file to be text and not binary 
solve;
display x;
//
you should get x1 = 540, x2 = 252
*/



 





#include "CoinSolver.h"
#include "OSConfig.h"
#include "OSnl2osil.h"
#include "OSiLReader.h"
#include "OSrLReader.h"
#include "OSiLWriter.h"
#include "OSrLWriter.h"
#include "OSInstance.h"
#include "OSResult.h"
#ifdef COIN_HAS_LINDO    
#include "LindoSolver.h"
#endif 
#ifdef COIN_HAS_IPOPT    
#include "IpoptSolver.h"
#endif  

#include "DefaultSolver.h"
#include "OSSolverAgent.h"
#include "OShL.h"
#include "ErrorClass.h"
#include <sstream>
#include <string.h> 
#include <string>

//AMPL includes must be last.
#include <asl.h>
using std::cerr;
using std::string;
using std::cout;
using std::endl;


int main(int argc, char **argv)
{
	char *stub;
	// set AMPL structures
	ASL *asl;
	asl = ASL_alloc(ASL_read_fg);
    stub = argv[1];
	jac0dim((char*)stub, (fint)strlen(stub));
	OSnl2osil *nl2osil = NULL;
	//initialize object with stub -- the nl file
	nl2osil = new OSnl2osil( stub);
	// create an osinstance object
	OSInstance *osinstance;
	std::cout << " call nl2osil" << std::endl;
	try{
		nl2osil->createOSInstance() ;
	}
	catch(const ErrorClass& eclass){
		std::cout << eclass.errormsg << std::endl;
		return 0;
	}
	std::cout << " return from  nl2osil" << std::endl;
	osinstance = nl2osil->osinstance;
	std::cout << " osinstance created" << std::endl;
	// turn the osinstance into osil 
	// not needed for a local solve
	// send an osinstance object in memory
	/**  amplclient_option: 
	 *   1. solver:
	 *		possible values - clp, glpk, cplex, cplex, lindo
	 *   2. location:
	 *      possible values - NULL (empty) or url of the solver service
	 *      
	 */
	char *amplclient_options = NULL;
	char *agent_address = NULL;
	// set solver type default to clp
	DefaultSolver *solverType  = NULL;	
	OSrLReader *osrlreader = NULL;
	osrlreader = new OSrLReader();
	OSrLWriter *osrlwriter;
	osrlwriter = new OSrLWriter();
	OSResult *osresult = NULL;
	std::string osrl = "";
	// note that default solver is coin and default subSolver is clp
	std::string osol = "<osol></osol>";
	// get the solver set by AMPLl
	amplclient_options = getenv("amplClient_options");
	if(amplclient_options != NULL) cout << "HERE ARE THE AMPL CLIENT OPTIONS " <<   amplclient_options << endl;
	try{
		if(amplclient_options == NULL ) throw ErrorClass( "a local solver was not specified in AMPL option");
		else{
			if( strstr(amplclient_options, "lindo") != NULL) {
				// we are requesting the Lindo solver
				bool bLindoIsPresent = false;
				#ifdef COIN_HAS_LINDO
				bLindoIsPresent = true;
				solverType = new LindoSolver();
				agent_address = getenv("lindo_options");
				if( agent_address != NULL) cout << "HERE ARE THE LINDO OPTIONS " <<   agent_address << endl;
				#endif
				if(bLindoIsPresent == false) throw ErrorClass( "the Lindo solver requested is not present");
			}
			else{ 
				if( strstr(amplclient_options, "clp") != NULL){
					solverType = new CoinSolver();
					solverType->m_sSolverName = "clp";
				}
				else{
					if( strstr(amplclient_options, "cbc") != NULL){
						solverType = new CoinSolver();
						solverType->m_sSolverName = "cbc";
						agent_address = getenv("cbc_options");
						if( agent_address != NULL) cout << "HERE ARE THE CBC OPTIONS " <<   agent_address << endl;
					}
					else{
						if( strstr(amplclient_options, "cplex") != NULL){
							solverType = new CoinSolver();
							solverType->m_sSolverName = "cplex";
							agent_address = getenv("cplex_options");
							if( agent_address != NULL) cout << "HERE ARE THE CPLEX OPTIONS " <<   agent_address << endl;
						}
						else{
							if( strstr(amplclient_options, "glpk") != NULL){
								solverType = new CoinSolver();
								solverType->m_sSolverName = "glpk";
								agent_address = getenv("glpk_options");
								if( agent_address != NULL) cout << "HERE ARE THE GLPK OPTIONS " <<   agent_address << endl;
							}
							else{
								if( strstr(amplclient_options, "ipopt") != NULL){
									// have to act differently since Ipopt uses smart pointers
									// we are requesting the Ipopt solver
									agent_address = getenv("lindo_options");
									if( agent_address != NULL) cout << "HERE ARE THE IPOPT OPTIONS " <<   agent_address << endl;
									bool bIpoptIsPresent = false;
									if(agent_address == NULL ){
										#ifdef COIN_HAS_IPOPT
										bIpoptIsPresent = true;
										//std::cout << "Create an Ipopt solver and optimize"<< std::endl;
										SmartPtr<IpoptSolver> ipoptSolver  = new IpoptSolver();	
										ipoptSolver->osol = osol;
										ipoptSolver->osinstance = osinstance;
										ipoptSolver->solve();
										//std::cout << "Done optimizing with Ipopt"<< std::endl;
										osrl = ipoptSolver->osrl ;
										//std::cout << "Have Ipopt writ out osrl"<< std::endl;
										#endif
										if(bIpoptIsPresent == false) throw ErrorClass( "the Ipopt solver requested is not present");
									}
								}
								else{
									throw ErrorClass( "a supported solver is not present");
								}
							}
						}
					}
				}
			}
		}
		// do a local solve
		if( (strstr(amplclient_options, "ipopt") == NULL) && (agent_address == NULL)){
			solverType->osol = osol;
			solverType->osinstance = osinstance;
			solverType->solve();
			osrl = solverType->osrl ;
		}
	}
	catch(const ErrorClass& eclass){
		osrl = solverType->osrl;
	}
	// do the following for a remote solve
	if(agent_address != NULL){
		OSSolverAgent* osagent = NULL;
		OSiLWriter *osilwriter = NULL;
		osilwriter = new OSiLWriter();
		std::string  osil = osilwriter->writeOSiL( osinstance);
		osagent = new OSSolverAgent( agent_address);
		cout << "Place remote synchronous call" << endl;
		osrl = osagent->solve(osil, osol);
	}
	// okay start to test osrl parser 
	try{
		cout << osrl << endl << endl <<endl;	
		osresult = osrlreader->readOSrL( osrl);
		if( osresult->getSolutionNumber( ) > 1)
		write_sol((char*)osresult->getSolutionMessage( 0).c_str(), 
			osresult->getOptimalPrimalVariableValues( -1), 
			osresult->getOptimalDualVariableValues( -1), NULL);
	}
	catch(const ErrorClass& eclass){
		cout << "There was an error parsing the OSrL" << endl << eclass.errormsg << endl << endl;
	}
	delete osrlreader;
	osrlreader = NULL;
	if( strstr(amplclient_options, "ipopt") == NULL){
		delete solverType;
		solverType = NULL;
	}
	delete osrlwriter;
	osrlwriter = NULL;
	delete nl2osil;
	nl2osil = NULL;
	return 0;
} // end main
