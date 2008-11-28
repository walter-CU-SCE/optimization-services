/** @file OSCouenneSolver.h
 * 
 * @author  Jun Ma, Gus Gassmann, Kipp Martin, 
 * @version 1.0, 07/05/2008
 * @since   OS1.0
 *
 * \remarks
 * Copyright (C) 2008, Jun Ma, Kipp Martin,
 * Northwestern University, and the University of Chicago.
 * All Rights Reserved.
 * This software is licensed under the Common Public License. 
 * Please see the accompanying LICENSE file in root directory for terms.
 * 
 */ 
 
#ifndef COUENNESOLVER_H 
#define COUENNESOLVER_H

#include "OSConfig.h" 
#include "OSDefaultSolver.h"
#include "OSBonminSolver.h"
#include "OSrLWriter.h"
#include "OSInstance.h"
#include "OSParameters.h"
#include "OSiLReader.h"
#include "OSExpressionTree.h"
#include "OSnLNode.h"
#include "OSDataStructures.h"
#include "OSFileUtil.h"  
#include "OSErrorClass.h"
#include "OSResult.h"
#include "OSOption.h"

//Couenne stuff
#include "CouenneProblem.hpp"
//#include "BonCouenneSetup.hpp"

#ifdef HAVE_CTIME
# include <ctime>
#else
# ifdef HAVE_TIME_H
#  include <time.h>
# else
#  error "don't have header file for time"
# endif
#endif 

 
#ifdef HAVE_CMATH
# include <cmath>
#else
# ifdef HAVE_MATH_H
#  include <math.h>
# else
#  error "don't have header file for math"
# endif
#endif
#include<vector>
#include <map>  






/*! \class CouenneSolver
 *  \brief The CouenneSolver class solves problems using Ipopt.
 * 
 * @author Jun Ma, Guss Gassmann, Kipp Martin
 * @version 1.0, 07/05/2008
 * @since OS 1.0
 * 
 * \remarks
 * this class takes an OSiL instance and optimizes it using
 * the COIN-OR Ipopt solver
 * 
 */

using namespace Bonmin;
class CouenneSolver : public DefaultSolver{ 	
public:
	


	/** the CouenneSolver class constructor */
	CouenneSolver();
	
	/** the IpoptSolver class destructor */
	~CouenneSolver();
	
	


	
	
	/** solve results in an instance being read into the Couenne
	 * data structrues and optimized */
	virtual void  solve() throw (ErrorClass) ;
	
	/*! \fn void CoinSolver::buildSolverInstance() 
	 *  \brief The implementation of the virtual functions. 
	 *  \return void.
	 */	
	virtual void  buildSolverInstance() throw(ErrorClass);
	
	/*! \fn void setSolverOptions() 
	 *  \brief The implementation of the virtual functions. 
	 *  \return void.
	 */	
	virtual void  setSolverOptions() throw(ErrorClass);
	
   	/**
   	 * use this for debugging, print out the instance that
   	 * the solver thinks it has and compare this with the OSiL
   	 * file
   	 */	
	void dataEchoCheck();
	
	/** 
	 * m_osilreader is an OSiLReader object used to create an osinstance from an
	 * osil string if needed	 
	 */		
	OSiLReader *m_osilreader;
	
	
	CouenneProblem *couenne;
	
	
	SmartPtr<BonminProblem> tminlp;


private:
	OSrLWriter  *osrlwriter;

	//CouenneSetup couenne;
	
	BonminSetup bonmin_couenne;

	std::string couenneErrorMsg;

	expression* createCouenneExpression(OSnLNode* node);
};


#endif /*COUENNESOLVER_H*/
