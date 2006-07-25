/**
 *
 *
 * @author  Robert Fourer, Jun Ma, Kipp Martin, 
 * @version 1.0, 10/05/2005
 * @since   OS1.0
 */ 
#include <OsiSolverInterface.hpp>   
#include <CoinMessageHandler.hpp> 
#include <CoinPackedMatrix.hpp>
 
  

#include "OSResult.h"
#include "OSiLReader.h"       
#include "OSiLWriter.h" 
#include "OSrLReader.h"        
#include "OSrLWriter.h"   
#include "OSInstance.h"  
#include "FileUtil.h"  

#include "OSConfig.h" 

#include "CoinSolver.h"
#include "DefaultSolver.h"  
#include "WSUtil.h" 
#include "OSSolverAgent.h"   
#include "OShL.h"     
#include "ErrorClass.h"
#include "OSmps2osil.h"   
#include "Base64.h"
#ifdef COIN_HAS_ASL
#include "OSnl2osil.h"
#endif
#ifdef COIN_HAS_LINDO    
#include "LindoSolver.h"
#endif  
 
 
#include <time.h>
#include<sstream>
  
#include "OsiClpSolverInterface.hpp" 

int main(int argC, char* argV[])
{
	cout << "START UNIT TEST" << endl; 
	//char *xmlFile; 
	//xmlFile = argV[ 1];
	// define the classes
	FileUtil *fileUtil = NULL;  
	#ifdef COIN_HAS_ASL
	OSnl2osil *nl2osil = NULL;
	#endif 
	OSmps2osil *mps2osil = NULL;
	DefaultSolver *m_Solver  = NULL;
	// end classes    
	string osilFileName;
	string nlFileName; 
	string mpsFileName;     
	string parserTestOSiLFileName;
	string osil;
	string osol = "<OSoL><other name=\"Solver\" value=\"coin_solver_clp\"></other></OSoL>";
		osilFileName =  "/Users/kmartin/Documents/files/code/cpp/Coin-OS/OS/data/lindoapiaddins.osil";
		osilFileName =  "../data/lindoapiaddins.osil";
		nlFileName = "/Users/kmartin/Documents/files/code/cpp/Coin-OS/OS/data/testfile2.nl";
		mpsFileName = "/Users/kmartin/Documents/files/code/cpp/Coin-OS/OS/data/parinc.mps";
		parserTestOSiLFileName = "/Users/kmartin/Documents/files/code/cpp/Coin-OS/OS/data/parincLinear.osil";
	fileUtil = new FileUtil();
	osil = fileUtil->getFileAsString( &osilFileName[0]);
	// solve using using the osil file
	try{
		cout << "Create a new COIN Solver" << endl;
		m_Solver = new CoinSolver();
		m_Solver->m_sSolverName = "clp";
		m_Solver->osil = osil;
		m_Solver->osol = osol; 
		m_Solver->osinstance = NULL; 
		cout << "call the COIN Solver" << endl;
		m_Solver->solve();
		cout << "Here is the COIN solver solution" << endl;
		cout << m_Solver->osrl << endl;
		m_Solver->osinstance = NULL;
		delete m_Solver;
		m_Solver = NULL;
		cout << "solution using an OSiL string a success" << endl;
	}
	catch(const ErrorClass& eclass){
		cout << "OSrL =  " <<  m_Solver->osrl <<  endl;
	}
	try{
	#ifdef COIN_HAS_LINDO
		cout << "create a new LINDO Solver for OSiL string solution" << endl;
		m_Solver = new LindoSolver();	
		m_Solver->osil = osil;
		m_Solver->osol = osol;
		m_Solver->osinstance = NULL;
		cout << "call the LINDO Solver" << endl;
		m_Solver->solve();
		cout << "Here is the LINDO solver solution" << endl;
		cout << m_Solver->osrl << endl;
		m_Solver->osinstance = NULL;
		delete m_Solver;
		m_Solver = NULL;
		//nl2osil = new OSnl2osil( nlFileName);
		
#endif
	}
	catch(const ErrorClass& eclass){
		cout << "OSrL =  " <<  m_Solver->osrl <<  endl;
	}
	// end solving using the osil file
	// now solve with an OSInstance created from an MPS file
	try{
		cout << endl;
		cout << "START MPS TESTING" << endl << endl;
		cout << "create a COIN Solver for MPS - OSInstance solution" << endl;
		m_Solver = new CoinSolver();
		m_Solver->m_sSolverName = "clp";
		mps2osil = new OSmps2osil( mpsFileName);
		mps2osil->createOSInstance() ;
		m_Solver->osinstance = mps2osil->osinstance;
		m_Solver->osol = osol;
		cout << "call COIN Solve" << endl;
		m_Solver->solve();
		cout << "Here is the COIN solver solution" << endl;
		cout << m_Solver->osrl << endl;
		m_Solver->osinstance = NULL;
		delete m_Solver;
		m_Solver = NULL;
		delete mps2osil; 
		mps2osil = NULL;
#ifdef COIN_HAS_LINDO
/*
		cout << "create a LINDO Solver for MPS - OSInstance solution" << endl;
		m_Solver = new LindoSolver();
		mps2osil = new OSmps2osil( mpsFileName);
		mps2osil->createOSInstance() ;
		m_Solver->osinstance = mps2osil->osinstance;
		m_Solver->osol = osol;
		cout << "call LINDO Solve" << endl;
		m_Solver->solve();
		cout << "Here is the LINDO solver solution" << endl;
		cout << m_Solver->osrl << endl;
		m_Solver->osinstance = NULL;
		delete m_Solver;	
		m_Solver = NULL;
		delete mps2osil;
		mps2osil = NULL;
*/
#endif

		cout << endl;
		cout << "DONE WITH MPS TESTING" << endl;
	}
	catch(const ErrorClass& eclass){
		cout << "OSrL =  " <<  m_Solver->osrl <<  endl;
	}
	// now solve with an OSInstance created from an AMPL nl file
	try{
		cout << endl;
		cout << "START AMPL TESTING" << endl << endl;
#ifdef COIN_HAS_ASL
#ifdef COIN_HAS_LINDO
		cout << "create a LINDO Solver for AMPL nl - OSInstance solution" << endl;
		m_Solver = new LindoSolver();
		nl2osil = new OSnl2osil( nlFileName);
		nl2osil->createOSInstance() ;
		m_Solver->osinstance = nl2osil->osinstance;
	
		m_Solver->osol = osol;
		cout << "call LINDO Solve" << endl;
	  
		m_Solver->solve();
		cout << "Here is the LINDO solver solution" << endl;
		cout << m_Solver->osrl << endl;
		m_Solver->osinstance = NULL;
		delete m_Solver;
		m_Solver = NULL;
		cout << "call delete nl2osil" << endl;
		delete nl2osil;
		nl2osil = NULL;	 
		//return 0;
#endif
		cout << "create a COIN Solver for AMPL nl - OSInstance solution" << endl;
		/*m_Solver = new CoinSolver();
		m_Solver->m_sSolverName = "cbc";
		nl2osil = new OSnl2osil( nlFileName);
		nl2osil->createOSInstance() ;
		m_Solver->osinstance = nl2osil->osinstance;
		m_Solver->osol = osol;
		cout << "call COIN Solve" << endl;
		m_Solver->solve();
		cout << "Here is the COIN solver solution" << endl;
		cout << m_Solver->osrl << endl;
		m_Solver->osinstance = NULL;
		delete m_Solver;
		m_Solver = NULL;
		delete nl2osil;
		nl2osil = NULL;	   
		cout << endl;
		*/
#endif
		cout << "END AMPL TESTING" << endl << endl;
	}	
		catch(const ErrorClass& eclass){
		cout << "OSrL =  " <<  m_Solver->osrl <<  endl;
		return 0;
	}
	//
	// Now test the b64 feature
	//
	try{
		cout << endl;
		cout << endl;
		cout << "TESTING BASE 64 WITH A COIN SOLVER"<< endl;
		cout << endl;
		OSiLWriter osilwriter;
		osilwriter.m_bWriteBase64 = true;
		m_Solver = new CoinSolver();
		m_Solver->m_sSolverName = "cbc";
		mps2osil = new OSmps2osil( mpsFileName);
		m_Solver->osinstance = NULL;
		m_Solver->osol = osol;
		mps2osil->createOSInstance() ;
		m_Solver->osil = osilwriter.writeOSiL( mps2osil->osinstance) ;
		// extra code
		string outputfile = mpsFileName+"_osil";
		char* testfile = &outputfile[0];
		fileUtil->writeFileFromString(testfile,  m_Solver->osil);		
		// end extra code
		m_Solver->solve();
		cout << endl << endl;
		cout << "COIN solution of a OSiL string in b64 format" << endl;
		cout << m_Solver->osrl;
		m_Solver->osinstance = NULL;
		delete m_Solver;
		m_Solver = NULL;
		delete mps2osil;
		mps2osil = NULL;
	}	
	catch(const ErrorClass& eclass){
		cout << endl << endl;
		cout << eclass.errormsg <<  endl << endl;
		//cout << "OSrL =  " <<  m_Solver->osrl <<  endl;

	}
	// try parsing OSrL	  
	//
	// Now test OSiL parser on large problem
	try{ 
		cout << endl;
		cout << endl;
		cout << "TEST PARSING A LARGE LP" << endl;
		cout << endl;
		osil = fileUtil->getFileAsString( &parserTestOSiLFileName[0]);
		OSiLReader *osilreader = NULL;
		osilreader = new OSiLReader(); 
		clock_t start, finish;
		double duration;
		start = clock();
				cout << "TEST PARSING A LARGE LP" << endl;
		//osilreader->readOSiL( osil);
				cout << "TEST PARSING A LARGE LP 2" << endl;
		delete osilreader;
		osilreader = 0;
		finish = clock();
		duration = (double) (finish - start) / CLOCKS_PER_SEC;
		cout << "Parsing took (seconds): "<< duration << endl;

	}	
		catch(const ErrorClass& eclass){
		cout << "OSrL =  " <<  m_Solver->osrl <<  endl;
	}		
	// 
	// now solve on a remote server
	/*
	try{
		//return 0;
		OSSolverAgent* osagent = NULL;
		//osagent = new OSSolverAgent("128.135.130.17/axis/OSSolverService");
		//osagent = new OSSolverAgent("gsbkip.chicagogsb.edu:80/os/ossolver/COINSolverService.jws");
		//osagent = new OSSolverAgent("localhost/os/ossolver/COINSolverService.jws");
		//osagent = new OSSolverAgent("localhost/axis/OSSolverService");
		osagent = new OSSolverAgent("http://127.0.0.1:8080/os/ossolver/CoinSolverService.jws");
		//osagent = new OSSolverAgent("128.135.130.17:8080/os/ossolver/CoinSolverService.jws");
		//osagent = new OSSolverAgent("127.0.0.1:8080/web-services-examples/ted/VrptsSolverService.jws");
		cout << "Place remote synchronous call" << endl;
		nl2osil = new OSnl2osil( nlFileName);
		nl2osil->createOSInstance() ;
		OSiLWriter *osilwriter = NULL;
		osilwriter = new OSiLWriter();
		osil = osilwriter->writeOSiL( nl2osil->osinstance) ;
		string osrl = osagent->solve(osil, osol);
		cout << "READ THE OSrL and WRITE IT AGAIN" << endl;
		cout << osrl  << endl;
		OSrLReader *osrlreader = NULL;
		OSResult *osresult = NULL;
		osrlreader = new OSrLReader();
		osresult = osrlreader->readOSrL( osrl);
		OSrLWriter *osrlwriter;
		if(osresult != NULL){
			osrlwriter = new OSrLWriter();
			osrl = osrlwriter->writeOSrL( osresult);
		}
		cout << osrl << endl;
		//
		delete nl2osil;
		nl2osil = NULL;
		delete osilwriter;
		osilwriter = NULL;
	}	
		catch(const ErrorClass& eclass){
		cout <<  eclass.errormsg <<  endl;
	}	
	*/
	try{
		cout << "HERE I AM" << endl;
		OSiLReader *osilreader = NULL;
		osilreader = new OSiLReader();
		OSInstance *osinstance;
		osinstance = new OSInstance();
		osinstance = osilreader->readOSiL( osil);
		double *aColSparse = osinstance->instanceData->linearConstraintCoefficients->value->el;
		int *rowIdx = osinstance->instanceData->linearConstraintCoefficients->rowIdx->el; 
		int *start = osinstance->instanceData->linearConstraintCoefficients->start->el; 
		int numConstraints = osinstance->instanceData->constraints->numberOfConstraints;
		double *aColDense = new double[ numConstraints ];
		for(int i = 0; i < numConstraints; i++){
				//aColDense[i] = 0.0;
		}
		for(int i = start[0]; i < start[ 1]; i++){
			aColDense[ rowIdx[ i] ] = aColSparse[ i];
			cout << aColDense[  rowIdx[ i]] << endl;
		}
		return 0;
	}
		catch(const ErrorClass& eclass){
		cout <<  eclass.errormsg <<  endl;
	}
}
