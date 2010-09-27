//===========================================================================//
// This file is part of the Decomp Solver Framework.                         //
//                                                                           //
// Decomp is distributed under the Common Public License as part of the      //
// COIN-OR repository (http://www.coin-or.org).                              //
//                                                                           //
// Author: Matthew Galati, Lehigh University                                 //
//                                                                           //
// Copyright (C) 2002-2008, Lehigh University, Matthew Galati, and Ted Ralphs//
// All Rights Reserved.                                                      //
//===========================================================================//

#ifndef BEARCAT_DECOMPAPP_INCLUDED
#define BEARCAT_DECOMPAPP_INCLUDED

// --------------------------------------------------------------------- //
#include "DecompApp.h"
#include "DecompParam.h"

// --------------------------------------------------------------------- //
#include "OS_DipInterface.h"
#include "Bearcat_DecompParam.h"

// --------------------------------------------------------------------- //
/*!
 * \class Bearcat_DecompApp
 * A DecompApp for solving the 
 *     Bearcat Problem.
 * 
 * \see
 * DecompApp
 *
 */
// --------------------------------------------------------------------- //
class Bearcat_DecompApp: public DecompApp {
public:
	/** Class id tag (for log / debugging). */
	const string m_classTag;

	/** Bearcat problem instance data */
	OS_DipInterface m_osInterface;

	/** Application specific parameters. */
	Bearcat_Param m_appParam;

	/** The model objective coefficients (original space). */
	double *m_objective;

	/** The model constraint systems used for different algos. */
	DecompConstraintSet *m_modelC;
	map<int, DecompConstraintSet*> m_modelR;

	/** Definition of blocks (by rows). */
	std::map<int, std::vector<int> > m_blocks;

	/** Initialize application. */
	void initializeApp(UtilParameters & utilParam);

	/** Create model parts. */
	void createModels();

	//this creates the coupling/core constraint set
	void createModelPart(DecompConstraintSet * model, const int nRowsPart,
			const int * rowsPart);

	//this is used to create each block
	void createModelPartSparse(DecompConstraintSet * model,
			const int nRowsPart, const int * rowsPart);


	/* @name Inherited (from virtual) methods. */
	int generateInitVars(DecompVarList & initVars);

	void createModelMasterOnlys2(vector<int> & masterOnlyCols);



	/** Default constructor. Takes an instance of UtilParameters */
	Bearcat_DecompApp(UtilParameters & utilParam) :
		DecompApp(utilParam), m_classTag("Bearcat-APP"), m_objective(NULL) {
		initializeApp(utilParam);
	}

	virtual ~Bearcat_DecompApp() {
		std::cout << "INSIDE BEARCAT DECOMP DESTRUCTOR" << std::endl;
		UTIL_DELARR( m_objective);
		UTIL_DELPTR( m_modelC);
		UtilDeleteMapPtr( m_modelR);
	}

};

#endif