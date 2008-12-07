/** @file CouenneSolver.cpp
 * 
 * \brief This file defines the Couenne Solver class.
 * \detail Read an OSInstance object and convert in Ipopt data structures
 *
 * @author  Jun Ma, Horand Gassmann, Kipp Martin, 
 * @version 1.1, 11/04/2008
 * @since   OS1.0
 *
 * \remarks
 * Copyright (C) 2008, Jun Ma, Horand Gassmann, Kipp Martin,
 * Northwestern University, Dalhousie University and the University of Chicago.
 * All Rights Reserved.
 * This software is licensed under the Common Public License. 
 * Please see the accompanying LICENSE file in root directory for terms.
 * 
 */
#include <iostream>


//OS stuff
#include "OSDataStructures.h"
#include "OSParameters.h" 
#include "OSCommonUtil.h"
#include "OSMathUtil.h"
#include "OSCouenneSolver.h"
# include <cppad/cppad.hpp>
//end OS stuff


// Couenne stuff

#include "CouenneTypes.hpp"
#include "exprClone.hpp"
#include "exprGroup.hpp"
#include "exprAbs.hpp"
#include "exprConst.hpp"
#include "exprCos.hpp"
#include "exprDiv.hpp"
#include "exprExp.hpp"
#include "exprInv.hpp"
#include "exprLog.hpp"
#include "exprMax.hpp"
#include "exprMin.hpp"
#include "exprMul.hpp"
#include "exprOpp.hpp"
#include "exprPow.hpp"
#include "exprSin.hpp"
#include "exprSub.hpp"
#include "exprSum.hpp"
#include "exprVar.hpp"
#include "CouenneJournalist.hpp"
// end Couenne stuff


//Bonmin stuff

#include "BonOsiTMINLPInterface.hpp"





#include "CoinTime.hpp"
#include "BonminConfig.h"
#include "BonCouenneInterface.hpp"
#include "BonIpoptSolver.hpp"

#include "BonCouenneSetup.hpp"

#include "BonCbc.hpp"
#ifdef COIN_HAS_FILTERSQP
#include "BonFilterSolver.hpp"
#endif

#include "CbcCutGenerator.hpp"
#include "CouenneProblem.hpp"
#include "CouenneCutGenerator.hpp"


# include <cstddef>
# include <cstdlib>
# include <cctype>
# include <cassert>
# include <stack>
#include <string>
#include<iostream>



using namespace Bonmin;
using std::cout; 
using std::endl; 
using std::ostringstream;


CouenneSolver::CouenneSolver() {
using namespace  Ipopt;
	osrlwriter = new OSrLWriter();
	osresult = new OSResult();
	m_osilreader = NULL;
	couenneErrorMsg = "";
	couenne = NULL;


} 

CouenneSolver::~CouenneSolver() {
	#ifdef DEBUG
	cout << "inside CouenneSolver destructor" << endl;
	#endif
	cout << "inside CouenneSolver destructor" << endl;
	if(couenne != NULL){
		cout << "BEFORE DELETE COUENNE" << endl;
		//delete couenne;
		cout << "AFTER DELETE COUENNE" << endl;
	}
	if(m_osilreader != NULL) delete m_osilreader;
	m_osilreader = NULL;
	delete osresult;
	osresult = NULL;
	delete osrlwriter;
	osrlwriter = NULL;
	//delete osinstance;
	//osinstance = NULL;
	#ifdef DEBUG
	cout << "leaving CouenneSolver destructor" << endl;
	#endif
}


void CouenneSolver::buildSolverInstance() throw (ErrorClass) {
	// Much of the following is taken from Stefan Vigerske
	try{
		
		// do some initialization
		
		osinstance->initializeNonLinearStructures( );
		
		int i, j;
		
		if(osil.length() == 0 && osinstance == NULL) throw ErrorClass("there is no instance");
		if(osinstance == NULL){
			m_osilreader = new OSiLReader();
			osinstance = m_osilreader->readOSiL( osil);
		}	
		

		Ipopt::Journalist* jnlst = new Ipopt::Journalist();
		jnlst->AddFileJournal("console", "stdout", J_STRONGWARNING);
		couenne = new CouenneProblem(NULL, NULL, jnlst);
		int n_allvars = osinstance->getVariableNumber();
		if( n_allvars <= 0 )throw ErrorClass("Couenne solver Needs Variables");
		std::cout << "NUMBER OF VARIABLES = " <<  n_allvars <<  std::endl;
	
		// create room for problem's variables and bounds
		CouNumber *x_ = (CouNumber *) malloc ((n_allvars) * sizeof (CouNumber));
		CouNumber	*lb = NULL, *ub = NULL;
	
		// now get variable upper and lower bounds
		ub = osinstance->getVariableUpperBounds();
		lb = osinstance->getVariableLowerBounds();
		
		//declare the variable types
		char *varType;
		varType = osinstance->getVariableTypes();
		for (i = 0; i < n_allvars; ++i) {
			if( (varType[i] == 'B') || (varType[i]) == 'I' ) {
				couenne->addVariable(true, couenne->domain() );
			}
			else{
				
				couenne->addVariable(false, couenne->domain() );
				
			}

			x_[i] = 0.;
		}
		
		couenne->domain()->push(n_allvars, x_, lb, ub);
  		free(x_);
	
		// now for the objective function -- assume just one for now
		//just worry about linear coefficients
	
		expression *body = NULL;
		
		SparseVector* sv = osinstance->getObjectiveCoefficients()[ 0];
		
		int nterms = sv->number;
		
		exprGroup::lincoeff lin( nterms);
		
		
		for ( i = 0; i < nterms; ++i){
	
			lin[i].first = couenne->Var( sv->indexes[ i] );
			lin[i].second = sv->values[ i];

		}
		
		OSExpressionTree* exptree = osinstance->getNonlinearExpressionTree(-1);
		if (exptree) {
			expression** nl = new expression*[1];
			nl[0] = createCouenneExpression(exptree->m_treeRoot);
			body = new exprGroup(0., lin, nl, 1);
		} else {
			body = new exprGroup(0., lin, NULL, 0);			
		}
	
		if( osinstance->getObjectiveMaxOrMins()[0] == "min"){
			couenne->addObjective(body, "min");
		}
		else{
			throw ErrorClass( "Problem must be minimimization and not maximization");
		} 
		
		// get the constraints in row format
		
		SparseMatrix* sm =  osinstance->getLinearConstraintCoefficientsInRowMajor();
		
		if( sm){
			sm->isColumnMajor = false;
			 sm->bDeleteArrays = true;
		}		
		
		int nconss = osinstance->getConstraintNumber();		
		int row_nonz = 0;
		int kount = 0;
		
		double *rowlb = osinstance->getConstraintLowerBounds();
		double *rowub = osinstance->getConstraintUpperBounds();
		
		for (i = 0; i < nconss; ++i) {
			row_nonz = 0;
			if( sm)
				row_nonz = sm->starts[ i +1] - sm->starts[ i];
			exprGroup::lincoeff con_lin( row_nonz);
			

			if ( row_nonz  > 0){  // test for nonzeros in row i
				
				for (j = 0; j  <  row_nonz;  ++j){

					con_lin[j].first = couenne->Var( sm->indexes[ kount] );
					con_lin[j].second = sm->values[ kount];
					std::cout << "lin[j].first  " << sm->indexes[ kount]  << std::endl;
					std::cout << "lin[j].second  " << con_lin[j].second  << std::endl;
					kount++;

				}
			}
	
			expression *con_body = NULL;
			OSExpressionTree* exptree = osinstance->getNonlinearExpressionTree(i);
			if (exptree) {
				expression** nl = new expression*[1];
				nl[0] = createCouenneExpression(exptree->m_treeRoot);
				con_body = new exprGroup(0., con_lin, nl, 1);
			} else {
				std::cout << " THERE WERE NO NONLINEAR TERMS " << std::endl;
				con_body = new exprGroup(0., con_lin, NULL, 0);			
			}
		
			if (rowlb[ i] == rowub[ i])
			{
				couenne->addEQConstraint(con_body, new exprConst( rowub[ i] ));
			}
			else if (rowlb[ i]  ==  -OSDBL_MAX)
			{
				assert(rowub[ i]  !=  -OSDBL_MAX);
				couenne->addLEConstraint(con_body, new exprConst( rowub[ i] ));
			}
			else if (rowub[ i] ==  OSDBL_MAX)
			{
				assert(rowlb[ i]  != OSDBL_MAX);
				couenne->addGEConstraint(con_body, new exprConst( rowlb[ i] ));			
			}
			else
				couenne->addRNGConstraint(con_body, new exprConst( rowlb[ i]), new
					exprConst( rowub[ i] ));	
			
			//garbage collection
				
	}

	if (sm) delete sm;


	//couenne->AuxSet() = new std::set <exprAux *, compExpr>;

  	// reformulation
  	//couenne->standardize();

  	// give a value to all auxiliary variables
  	//couenne->initAuxs();

  	// clear all spurious variables pointers not referring to the

	//couenne->print();

	}
	catch(const ErrorClass& eclass){
		std::cout << "THERE IS AN ERROR" << std::endl;
		osresult->setGeneralMessage( eclass.errormsg);
		osresult->setGeneralStatusType( "error");
		osrl = osrlwriter->writeOSrL( osresult);
		throw ErrorClass( osrl) ;
	}				
}//end buildSolverInstance() 


expression* CouenneSolver::createCouenneExpression(OSnLNode* node) {
  switch (node->inodeInt) {
     case OS_PLUS :
    	 return new exprSum(createCouenneExpression(node->m_mChildren[0]), createCouenneExpression(node->m_mChildren[1]));
     case OS_SUM :
    	 switch (node->inumberOfChildren==0) {
    		 case 0:
    			 return new exprConst(0.);
    		 case 1:
    			 return createCouenneExpression(node->m_mChildren[0]);
    		 default:
    			 expression** args = new expression*[node->inumberOfChildren];
           for(int i=1;i<node->inumberOfChildren;i++)
          	 args[i] = createCouenneExpression(node->m_mChildren[i]);
           expression* base = new exprSum(args, node->inumberOfChildren);
           delete[] args;
           return base;
        }
     case OS_MINUS :
    	 return new exprSub(createCouenneExpression(node->m_mChildren[0]), createCouenneExpression(node->m_mChildren[1]));
     case OS_NEGATE :
    	 return new exprOpp(createCouenneExpression(node->m_mChildren[0]));
     case OS_TIMES :
    	 return new exprMul(createCouenneExpression(node->m_mChildren[0]), createCouenneExpression(node->m_mChildren[1]));
     case OS_DIVIDE :
    	 // couenne does not like expressions of the form exp1/exp2 with exp1 a constant, so we write them as exp1 * 1/exp2
    	 if (node->m_mChildren[1]->inodeInt == OS_NUMBER)
      	 return new exprMul(createCouenneExpression(node->m_mChildren[0]), new exprInv(createCouenneExpression(node->m_mChildren[1])));
    	 else
    		 return new exprDiv(createCouenneExpression(node->m_mChildren[0]), createCouenneExpression(node->m_mChildren[1]));
     case OS_POWER :
    	 // couenne does not like expressions of the form exp1 ^ exp2 with exp2 not a constant, so we write them as exp(log(exp1)*exp2)
    	 if (node->m_mChildren[1]->inodeInt != OS_NUMBER)
    		 return new exprExp(new exprMul(new exprLog(createCouenneExpression(node->m_mChildren[0])), createCouenneExpression(node->m_mChildren[1])));
    	 else
    		 return new exprPow(createCouenneExpression(node->m_mChildren[0]), createCouenneExpression(node->m_mChildren[1]));
     case OS_PRODUCT:
    	 switch (node->inumberOfChildren==0) {
    		 case 0:
    			 return new exprConst(1.);
    		 case 1:
    			 return createCouenneExpression(node->m_mChildren[0]);
    		 default:
    			 expression** args = new expression*[node->inumberOfChildren];
           for(int i=1;i<node->inumberOfChildren;i++)
          	 args[i] = createCouenneExpression(node->m_mChildren[i]);
           expression* base = new exprMul(args, node->inumberOfChildren);
           delete[] args;
           return base;
        }
     case OS_ABS :
    	 return new exprAbs(createCouenneExpression(node->m_mChildren[0]));
     case OS_SQUARE :
    	 return new exprPow(createCouenneExpression(node->m_mChildren[0]), new exprConst(2.));
     case OS_SQRT :
    	 return new exprPow(createCouenneExpression(node->m_mChildren[0]), new exprConst(0.5));
     case OS_LN :
    	 return new exprLog(createCouenneExpression(node->m_mChildren[0]));
     case OS_EXP :
    	 return new exprExp(createCouenneExpression(node->m_mChildren[0]));
     case OS_SIN :
    	 return new exprSin(createCouenneExpression(node->m_mChildren[0]));
     case OS_COS :
    	 return new exprCos(createCouenneExpression(node->m_mChildren[0]));
     case OS_MIN :
    	 switch (node->inumberOfChildren==0) {
    		 case 0:
    			 return new exprConst(0.);
    		 case 1:
    			 return createCouenneExpression(node->m_mChildren[0]);
    		 default:
    			 expression** args = new expression*[node->inumberOfChildren];
           for(int i=1;i<node->inumberOfChildren;i++)
          	 args[i] = createCouenneExpression(node->m_mChildren[i]);
           expression* base = new exprMin(args, node->inumberOfChildren);
           delete[] args;
           return base;
        }
     case OS_MAX :
    	 switch (node->inumberOfChildren==0) {
    		 case 0:
    			 return new exprConst(0.);
    		 case 1:
    			 return createCouenneExpression(node->m_mChildren[0]);
    		 default:
    			 expression** args = new expression*[node->inumberOfChildren];
           for(int i=1;i<node->inumberOfChildren;i++)
          	 args[i] = createCouenneExpression(node->m_mChildren[i]);
           expression* base = new exprMax(args, node->inumberOfChildren);
           delete[] args;
           return base;
        }
     case OS_NUMBER :
    	 return new exprConst(((OSnLNodeNumber*)node)->value);
     case OS_PI :
    	 assert(false);
    	 //TODO
//    	 return new exprConst(PI);
     case OS_VARIABLE : {
        OSnLNodeVariable* varnode = (OSnLNodeVariable*)node;
        if (varnode->coef == 0.)
       		return new exprConst(0.);
        if (varnode->coef == 1.)
       		return couenne->Variables()[varnode->idx]->clone(couenne->domain());
        if (varnode->coef == -1.)
       		return new exprOpp(couenne->Variables()[varnode->idx]->clone(couenne->domain()));
     		return new exprMul(new exprConst(varnode->coef), couenne->Variables()[varnode->idx]->clone(couenne->domain()));
     }
     default:
        cout << node->snodeName << " NOT IMPLEMENTED!!" << endl;
        break;
  }
	
	return NULL;
}


void CouenneSolver::setSolverOptions() throw (ErrorClass) {
	try{
		
	}
	
	catch(const ErrorClass& eclass){
		std::cout << "THERE IS AN ERROR" << std::endl;
		osresult->setGeneralMessage( eclass.errormsg);
		osresult->setGeneralStatusType( "error");
		osrl = osrlwriter->writeOSrL( osresult);
		throw ErrorClass( osrl) ;
	}				
}//end setSolverOptions() 


using namespace Ipopt;
	

void CouenneSolver::solve() throw (ErrorClass) {
#define PRINTED_PRECISION 1e-5
	const int infeasible = 1;
	double time_start = CoinCpuTime();
	try{
		
		
		couenne->print();
		
		char **argv = NULL;

		Bab bb;
    	bb.setUsingCouenne (true);

		//using namespace Ipopt;
		
		
		tminlp_ = new BonminProblem( osinstance, osoption, osresult);
		
		//app = new BonMinApplication();
		
		//nlp = new BonminProblem( osinstance, osoption, osresult);
		//app = new IpoptApplication();

		//this->bCallbuildSolverInstance = true;
		//Now initialize from tminlp
		
		

		
		CouenneInterface *ci = NULL;
		
		ci = new CouenneInterface();
		

 
		
/***
 * 
 * 
 * 	OsiTMINLPInterface::initialize(Ipopt::SmartPtr<Bonmin::RegisteredOptions> roptions,
369 	                               Ipopt::SmartPtr<Ipopt::OptionsList> options,
370 	                               Ipopt::SmartPtr<Ipopt::Journalist> journalist,
371 	                               const std::string & prefix,
372 	                               Ipopt::SmartPtr<TMINLP> tminlp)
 */
 
 		Ipopt::SmartPtr<Bonmin::RegisteredOptions> roptions;
 		Ipopt::SmartPtr<Ipopt::OptionsList> options;
 		Ipopt::SmartPtr<Ipopt::Journalist> jnlst;
 		
 		const std::string  prefix="bonmin.";
 		
		std::cout << "INITIALIZE COUENNE " << std::endl;
		bonmin_couenne.initializeOptionsAndJournalist();
		ci->initialize (bonmin_couenne.roptions(),//GetRawPtr(roptions),  
				bonmin_couenne.options(),//GetRawPtr( options), 
				bonmin_couenne.journalist(),//GetRawPtr(jnlst),  
				prefix, GetRawPtr( tminlp_));	
	      	
 		app_ = new Bonmin::IpoptSolver(bonmin_couenne.roptions(),//GetRawPtr(roptions),  
					       bonmin_couenne.options(),//GetRawPtr( options), 					       
					       bonmin_couenne.journalist(),//GetRawPtr(jnlst),  
					       prefix); 		
 	
	      	
		ci->setModel( GetRawPtr( tminlp_) );
		ci->setSolver( GetRawPtr( app_) );
			
		std::cout << "INITIALIZE COUENNE " << std::endl;
		bonmin_couenne.InitializeCouenne(argv, couenne, ci);	
		std::cout << " CALL BB " << std::endl;
		
   		bb ( bonmin_couenne); // do branch and bound
   		std::cout << " END BB " << std::endl;
    	 
	// Pietro's stuff!
	
    //////////////////////////////////

    std::cout.precision (10);

    //////////////////////////////
    CouenneCutGenerator *cg = NULL;

    if (bb.model (). cutGenerators ())
      cg = dynamic_cast <CouenneCutGenerator *> 
	(bb.model (). cutGenerators () [0] -> generator ());

    ////////////////////////////////
    int nr=-1, nt=-1;
    double st=-1;

    if (cg){
    	 cg -> getStats (nr, nt, st);
    	 std::cout << "Cut Generator Created for Solution Report "   << std::endl; 
    }
    else printf ("Warning, could not get pointer to CouenneCutGenerator\n");

    CouenneProblem *cp = cg ? cg -> Problem () : NULL;

    // retrieve test value to check
    double global_opt;
    bonmin_couenne.options () -> GetNumericValue ("couenne_check", global_opt, "couenne.");
    
    std::cout << "Couenne Global Optimum Value = " << global_opt  << std::endl; 
    std::cout << "GET BEST POSSIBLE OBJ VALUE =  " << bb.model (). getBestPossibleObjValue ()  << std::endl;
    std::cout << "GET OBJ VALUE   =  " << bb.model (). getObjValue () << std::endl;
    std::cout << "Display Stats Value   " <<  bonmin_couenne.displayStats () << std::endl;
 
    

    if (global_opt < COUENNE_INFINITY) { // some value found in couenne.opt

      double opt = bb.model (). getBestPossibleObjValue ();

      printf ("Global Optimum Test on %-40s %s\n", 
	      cp ? cp -> problemName ().c_str () : "unknown", 
	      (fabs (opt - global_opt) / 
	       (1. + CoinMax (fabs (opt), fabs (global_opt))) <  PRINTED_PRECISION) ? 
	      (const char *) "OK" : (const char *) "FAILED");
	      //opt, global_opt,
	      //fabs (opt - global_opt));

    } else // good old statistics

    if (bonmin_couenne.displayStats ()) { // print statistics

      // CAUTION: assuming first cut generator is our CouenneCutGenerator

      if (cg && !cp) printf ("Warning, could not get pointer to problem\n");
      else
	printf ("Stats: %-15s %4d [var] %4d [int] %4d [con] %4d [aux] "
		"%6d [root] %8d [tot] %6g [sep] %8g [time] %8g [bb] "
		"%20e [lower] %20e [upper] %7d [nodes]\n",// %s %s\n",
		cp ? cp -> problemName ().c_str () : "unknown",
		(cp) ? cp -> nOrigVars     () : -1, 
		(cp) ? cp -> nOrigIntVars  () : -1, 
		(cp) ? cp -> nOrigCons     () : -1,
		(cp) ? (cp -> nVars     () - 
			cp -> nOrigVars ()): -1,
		nr, nt, st, 
		CoinCpuTime () - time_start,
		cg ? (CoinCpuTime () - cg -> rootTime ()) : CoinCpuTime (),
		bb.model (). getBestPossibleObjValue (),
		bb.model (). getObjValue (),
		//bb.bestBound (),
		//bb.bestObj (),
		bb.numNodes ());
		//bb.iterationCount ());
		//status.c_str (), message.c_str ());	


	} 
	
	} //end try
	
	catch(const ErrorClass& eclass){
		osresult->setGeneralMessage( eclass.errormsg);
		osresult->setGeneralStatusType( "error");
		osrl = osrlwriter->writeOSrL( osresult);
		throw ErrorClass( osrl) ;
	}
	
	// Pietro's catch
	
	  catch(TNLPSolver::UnsolvedError *E) {
	     E->writeDiffFiles();
	     E->printError(std::cerr);
	    //There has been a failure to solve a problem with Ipopt.
	    //And we will output file with information on what has been changed in the problem to make it fail.
	    //Now depending on what algorithm has been called (B-BB or other) the failed problem may be at different place.
	    //    const OsiSolverInterface &si1 = (algo > 0) ? nlpSolver : *model.solver();
	  }
	  catch(OsiTMINLPInterface::SimpleError &E) {
	    std::cerr<<E.className()<<"::"<<E.methodName()
		     <<std::endl
		     <<E.message()<<std::endl;
	  }
	  catch(CoinError &E) {
	    std::cerr<<E.className()<<"::"<<E.methodName()
		     <<std::endl
		     <<E.message()<<std::endl;
	  }
	  catch (Ipopt::OPTION_INVALID &E)
	  {
	   std::cerr<<"Ipopt exception : "<<E.Message()<<std::endl;
	  }
	  catch (int generic_error) {
	    if (generic_error == infeasible)
	      printf ("problem infeasible\n");
	  }

}  //solve




/*
BonminProblem::BonminProblem(OSInstance *osinstance_,  OSOption *osoption_, OSResult *osresult_) {
	osinstance = osinstance_;
	osoption = osoption_;
	osresult = osresult_;
	printSol_ = false;
}

BonminProblem::~BonminProblem() {

}
*/

