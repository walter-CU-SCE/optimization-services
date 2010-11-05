/* $Id: OSRouteSolver.cpp 3038 2009-11-07 11:43:44Z kmartin $ */
/** @file OSRouteSolver.cpp
 * 
 *
 * @author  Gus Gassmann, Jun Ma, Kipp Martin, 
 * @version 1.0, 23/October/2020
 * @since   OS2.2
 *
 * \remarks
 * Copyright (C) 2005-2010, Gus Gassmann, Jun Ma, Kipp Martin,
 * Dalhousie University,  Northwestern University, and the University of Chicago.
 * All Rights Reserved.
 * This software is licensed under the Common Public License. 
 * Please see the accompanying LICENSE file in root directory for terms.
 * 
 */


#include "OSRouteSolver.h"

#include "OSErrorClass.h" 
#include "OSDataStructures.h"

#include "OSInstance.h"
#include "OSCoinSolver.h"
#include "OSConfig.h"
#include "OSResult.h" 

#include "OSiLReader.h"        
#include "OSiLWriter.h" 
#include "OSoLReader.h"        
#include "OSoLWriter.h" 
#include "OSrLReader.h"          
#include "OSrLWriter.h"      
#include "OSInstance.h"  
#include "OSFileUtil.h"  


#include "ClpSimplex.hpp"
#include "ClpFactorization.hpp"
#include "ClpNetworkMatrix.hpp"
#include "OsiClpSolverInterface.hpp"


std::string makeStringFromInt(std::string theString, int theInt);


OSRouteSolver::OSRouteSolver() {
	std::cout << "INSIDE OSRouteSolver CONSTRUCTOR with OSOption argument" << std::endl;
}//end default OSRouteSolver constructor

OSRouteSolver::OSRouteSolver(OSOption *osoption) {
	std::cout << "INSIDE OSRouteSolver CONSTRUCTOR with OSOption argument" << std::endl;
	
	m_u = NULL;
	m_v = NULL;
	m_g = NULL;
	m_px = NULL;
	m_tx =NULL;
	m_varIdx = NULL;
	
	m_optL = NULL;
	m_optD = NULL;
	m_vv = NULL;
	m_vvpnt = NULL;
	
	m_demand = NULL;
	m_cost = NULL;
	
	m_rc = NULL;

	try{
		
		getOptions( osoption);
		
		if(m_routeCapacity > m_totalDemand){
			m_upperBoundL = m_totalDemand;
		}else{
			m_upperBoundL = m_routeCapacity;
		}
		
		
		
		//m_varIdx = new int[ m_numNodes];
		m_varIdx = new int[ m_upperBoundL + 1];
		
		
		m_u = new double*[ m_numNodes];
		m_v = new double*[ m_numNodes];
		m_g = new double*[ m_numNodes];
		
		m_px = new int*[ m_numNodes];
		m_tx = new int*[ m_numNodes];
		
		int k;
		int i;
	
		int l;
		
		
		for (i = 0; i < m_numNodes; i++) {
			
			
			m_u[ i] = new double[ m_upperBoundL + 1];
			m_v[ i] = new double[ m_upperBoundL + 1];
			
			for(l = 0; l <= m_upperBoundL; l++){
				m_u[ i][l] = OSDBL_MAX;
				m_v[ i][l] = OSDBL_MAX;
			}
			
			m_g[ i] = new double[ m_numNodes];
			m_px[ i] = new int[ m_upperBoundL + 1];
			m_tx[ i] = new int[m_upperBoundL + 1];
			
			
		}
		
		
		//outer dynamic programming arrays
		m_optL = new int[ m_numHubs];
		m_optD = new int[ m_numHubs];
		
		m_vv = new double*[ m_numHubs];
		m_vvpnt = new int*[ m_numHubs];
		m_cost = new double*[ m_numHubs];
		
		for (k = 0; k < m_numHubs; k++) {
			
			
			m_vv[ k] = new double[ m_totalDemand + 1];
			m_vvpnt[ k] = new int[ m_totalDemand + 1];
			m_cost[ k] = new double[ m_numNodes*m_numNodes - m_numNodes];
			
			
		}
		
		//allocate memory for the reduced cost vector. 
		//assume order is k, l, i, j
		m_rc = new double[ m_numHubs*m_upperBoundL*(m_numNodes*m_numNodes - m_numNodes)];
		
		
		//allocate memory for convexity row
		m_psi = new double[ m_numHubs];
		
		//allocate memory for node assignment
		m_phi = new double[ m_numNodes];
		
		m_optValHub = new double[ m_numHubs];
		
		m_variableNames = new string[ m_numNodes*(m_numNodes - 1)];
		
		createVariableNames();
		
		//these are constraints that say we must be incident to each
		//non-hub node -- there are  m_numNodes - m_numHubs of these
		m_pntAmatrix = new int[ m_numNodes - m_numHubs + 1];
		//the variables -- the variable space we are in is x_{ij} NOT
		// x_{ijk} -- a bit tricky
		//m_Amatrix[ j] is a variable index -- this logic works
		//since the Amatrix coefficient is 1 -- we don't need a value
		//it indexes variable that points into the node
		m_Amatrix = new  int[ (m_numNodes - m_numHubs)*(m_numNodes - 1) ];
		createAmatrix();
		
		//this has size of the number of x variables
		int numVar = m_numNodes*m_numNodes - m_numHubs ;
		m_tmpScatterArray = new int[ numVar ];
		for(i = 0; i < numVar; i++){
			
			m_tmpScatterArray[ i] = 0;
			
		}
		
		m_nonzVec = new int[ m_numHubs] ;
		m_costVec = new double[ m_numHubs];
		
		
		m_newColumnRowIdx = new int*[ m_numHubs];
		m_newColumnRowValue = new double*[ m_numHubs];
		
//kipp change -- put the 1000 in as an option
//hardcoding
/// the 1000 is the number of rows -- coupling plus tour breaking		
		for (k = 0; k < m_numHubs; k++) {
			
			m_newColumnRowValue[ k] = new double[ 1000];
			m_newColumnRowIdx[ k] = new int[ 1000];
			
		}
		//for now, the number of columns will be 10000
		//for now number of nonzeros will be 500000
		m_thetaPnt = new int[ 10000];
		m_thetaCost = new double[ 10000];
		m_thetaIndex = new int[ 500000];
		m_numThetaVar = 0;
		m_numThetaNonz = 0;
		m_thetaPnt[ m_numThetaVar++ ] = 0;

		//kipp -- move this later
		getSeparationInstance();
	} catch (const ErrorClass& eclass) {

		throw ErrorClass(eclass.errormsg);

	}	
	
	
}//end OSRouteSolver Constructor


OSRouteSolver::~OSRouteSolver(){
	
	std::cout << "INSIDE ~OSRouteSolver DESTRUCTOR" << std::endl;


	
	//delete data structures for arrays used in calculating minimum reduced cost
	int i;
	
	for(i = 0; i < m_numNodes; i++){
		 
		
		
	    delete[] m_v[i];
	    delete[] m_g[i];
	    delete[] m_px[i];
	    delete[] m_tx[i];
		delete[] m_u[i];

	
	}
	
	delete[] m_u;
	m_u= NULL;

	delete[] m_v;
	m_v= NULL;
	
	delete[] m_g;
	m_g= NULL;
	
	delete[] m_px;
	m_px= NULL;
	
	delete[] m_tx;
	m_tx= NULL;
	
	
	
	if(m_demand != NULL){
		//std::cout << "I AM DELETING m_demand" << std::endl;
		delete[] m_demand;
	}

	
	if(m_varIdx != NULL) delete[] m_varIdx;
	
	for(i = 0; i < m_numHubs; i++){
		
		delete[] m_vv[i];
		delete[] m_vvpnt[i];
		delete[] m_cost[ i];
		
		
	}
	delete[] m_optL;
	m_optL = NULL;
	delete[] m_optD;
	m_optD = NULL;
	delete[] m_vv;
	m_vv = NULL;
	delete[] m_vvpnt;
	m_vvpnt = NULL;
	
	delete[] m_cost;
	m_cost = NULL;
	
	delete[] m_rc;
	m_rc = NULL;
	
	delete[] m_phi;
	m_phi = NULL;
	
	delete[] m_psi;
	m_psi = NULL;
	
	delete[] m_optValHub;
	m_optValHub = NULL;
	
	delete[] m_variableNames;
	m_variableNames = NULL;
	
	delete[] m_pntAmatrix;
	m_pntAmatrix = NULL;
	
	delete[] m_Amatrix;
	m_Amatrix = NULL;
	
	delete[] m_tmpScatterArray;
	m_tmpScatterArray = NULL;
	
	delete[] m_nonzVec  ;
	m_nonzVec = NULL;
	delete[] m_costVec ;
	m_costVec = NULL;
	
	for(i = 0; i < m_numHubs; i++){
		
		delete[] m_newColumnRowIdx[i];
		delete[] m_newColumnRowValue[i];
	}
	
	delete[] m_newColumnRowIdx;
	m_newColumnRowIdx = NULL;
	
	delete[] m_newColumnRowValue;
	m_newColumnRowValue = NULL;
	
	delete[] m_thetaPnt;
	m_thetaPnt = NULL;
	
	delete[] m_thetaIndex;
	m_thetaIndex = 0;

	
	delete[] m_thetaCost;
	m_thetaCost = 0;


}//end ~OSRouteSolver







void OSRouteSolver::getOptL(const  double* c) {
	
	//initialize the first HUB
	
	int k;
	int d;
	int d1;
	int kountVar;
	double testVal;
	int l;
	//int startPntInc;
	double trueMin;
	
	kountVar = 0;
	//startPntInc = m_upperBoundL*(m_numNodes*m_numNodes - m_numNodes);
	
	m_vv[ 0][ 0] = 0;
	for(d = 1; d <=  m_totalDemand; d++){
		
		m_vv[ 0][ d] = OSDBL_MAX;
		
	}
	
	//now loop over the other HUBS
	

	
	for(k = 1; k < m_numHubs; k++){
		
		for(d = 1; d <= m_totalDemand; d++){
			
			m_vv[ k][ d] = OSDBL_MAX;
			
			//d1 is the state variable at stage k -1
			for(d1 = 0; d1 <= m_totalDemand; d1++){
			
				l = d - d1;
				
				if( (m_vv[ k - 1][ d1] < OSDBL_MAX) &&  (l <= m_upperBoundL) && (l >= 1) ){
				
					
					// l was the decision at state d1 in stage k-1
					// l + d1 brings us to state d at stage k
					// d is the total carried on routes 0 -- k-1
				
					testVal = qrouteCost(k - 1,  l,  c,  &kountVar);
					
					//std::cout << "L = " << l << std::endl;
					//std::cout << "testVal " << testVal << std::endl;
					
					if( m_vv[ k-1][ d1]  +  testVal < m_vv[  k][ d] ){
						
						m_vv[ k][ d] =  m_vv[ k-1][ d1]  +  testVal;
						//now point to the best way to get to d
						m_vvpnt[ k][ d]  = d1;
						
					}
					
					
				}
				
			}
			
		}
		
		//c+=  startPntInc ;	
		
	}//  end for on k
	
	trueMin = OSDBL_MAX;
	//we now enter the last stage through the other hubs
	// have satisfied total demand d

	int  dlower = 0;
	if (m_numHubs > 1) dlower = 1;
	
	for(d = dlower; d < m_totalDemand; d++){
		
		//std::cout << "m_vv[ m_numHubs - 1 ][ d]  " << m_vv[ m_numHubs - 1 ][ d]  << std::endl;
		l = m_totalDemand - d;
		
		if(m_vv[ m_numHubs - 1 ][ d]  < OSDBL_MAX  && l <= m_upperBoundL && l >= 1){
		
			
			
			//std::cout << "LL = " <<  l  << std::endl;
			
			
			testVal = qrouteCost(m_numHubs -1 ,  l,  c,  &kountVar);
			
			//std::cout << "l = " << l << std::endl;
			//std::cout << "testVal = " << testVal << std::endl;
			
			if(m_vv[ m_numHubs - 1][ d] + testVal < trueMin){
				
				trueMin = m_vv[ m_numHubs -1][ d] + testVal;
				m_optD[  m_numHubs -1 ] = d;
				m_optL[  m_numHubs -1 ] = l;
				
			}
			
			
		}
	}
	
	std::cout << "TRUE MIN = " <<  trueMin << std::endl;

	k = m_numHubs -1;
	
	while( k - 1 >= 0) {
		
		m_optD[  k - 1 ] = m_vvpnt[ k][ m_optD[  k  ] ];
		
		m_optL[ k - 1 ] =  m_optD[  k  ] - m_optD[  k - 1 ] ;
		
		//std::cout << "k = " <<  k << std::endl;
		//std::cout << "m_optD[  k  ]  = " <<  m_optD[  k  ] << std::endl;
		//std::cout << "m_optD[  k -1 ] " << m_optD[  k - 1 ]  << std::endl;
		
		k--;
		
		
	}
	
}//end getOptL






double OSRouteSolver::qrouteCost(const int& k, const int& l, const double* c, int* kountVar){
	
	//critical -- nodes 0, ..., m_numNodes - 1 are the hub nodes
	// we are doing the calculation for hub k, k <= m_numNodes - 1
	//l  should be the total demand on the network -- we are NOT using 0 based counting
	double rcost;
	rcost = OSDBL_MAX;
	

	
	if(l < 0){
		
		std::cout  << "LVALUE  NEGATIVE " << l  << std::endl;
		exit( 1);
	}


	
	if(l > m_upperBoundL){
		
		std::cout  << "LVALUE  BIGGER THAN UPPER BOUND " << l  << std::endl;
		exit( 1);
	}	
	
	//start of the cost vector for hub k plus move to start of l demands
	// now move the pointer up
	int startPnt = k*m_upperBoundL*(m_numNodes*m_numNodes - m_numNodes) + (l - 1)*(m_numNodes*m_numNodes - m_numNodes);
	
	//int startPnt = (l - 1)*(m_numNodes*m_numNodes - m_numNodes);
	c+=  startPnt ;
	


	*kountVar = 0;
	int bestLastNode;
	int i;
	int j;
	int l1;
	int l2;
	//for(i = 0; i < 20; i++){
	//	std::cout << "i =  " << i  <<  " c[i] = " << *(c + i) << std::endl ;
	//}



	// l  is the total demand on this network
	//address of node (j, i) is j*(m_numNodes-1) + i when i < j
	//address of node (j, i) is j*(m_numNodes-1) + i - 1 when i > j
	
	//
	// initialize
	
	for(i = m_numHubs; i < m_numNodes; i++){
		
		
		for(l1 = m_minDemand; l1 <= l; l1++){  //l-1  is total demand on network
			
			m_u[i][l1] = OSDBL_MAX;
			m_v[i][l1] = OSDBL_MAX;
			m_px[i][l1] = -1; //a node we don't have
			if(l1 == *(m_demand + i) ){
				
				m_px[i][l1] = k;
				// want the cost for arc (k, i)
				// note: k < i so use that formula
				m_u[i][l1] = *(c + k*(m_numNodes - 1) + i - 1);  // put in correct cost

				
			}
		}	
	}
	//end initialize
	
	
	//
	
	//if l = minDemand we visit only one node, let's get the reduced cost in this case
	if(l == m_minDemand){
		
		for(i = m_numHubs; i < m_numNodes; i++){
			
			
			if(  m_u[i][l] + *(c + i*(m_numNodes-1) + k )  < rcost){
							
				rcost = m_u[i][l] + *(c + i*(m_numNodes-1) + k );
				
				//std::cout << " m_u[i][l2] = "  << m_u[i][l2] << std::endl;
				
				//std::cout << " *(c + i*(m_numNodes-1) + k ) = "  << *(c + i*(m_numNodes-1) + k ) << std::endl;
				//push node back
				bestLastNode = i;
			}
			
		}
		
		//go from node bestLastNode to node k		
		//node bestLastNode is a higher number than k
		*(m_varIdx + (*kountVar)++) = startPnt + bestLastNode*(m_numNodes - 1)  +  k ;
		*(m_varIdx + (*kountVar)++) = startPnt + k*(m_numNodes - 1)  + bestLastNode - 1;
		

		return rcost;	
	}//end if on l == minDemand
	

	
	
// now calculate values for demand 2 or greater 	
	//address of node (j, i) is j*(m_numNodes-1) + i when i < j
	//address of node (j, i) is j*(m_numNodes-1) + i - 1 when i > j
	// we start l2 at 2 since demand must be at least 1
	// change to min demand + 1
	int lowerVal = m_minDemand + 1;
	for(l2 = lowerVal; l2 <= l; l2++){// loop over possible demand values assuming we have already gone to at least one node
			
		for(i = m_numHubs; i < m_numNodes; i++) { //we are finding least cost to node i
			
			
			if( *(m_demand + i) < l2 ){ // kipp < OR <= ????
					
				for(j = m_numHubs; j < i; j++){ //we are coming from node j
					

						
					//calculate g  -- l2 - d[ i]  is the demand trough and including node j
					l1 = l2 - *(m_demand + i);
					
					if( m_px[j][ l1 ] != i ){//check to make sure we did not come into j from i
						
						
						m_g[j][i] = m_u[ j][ l1 ] + *(c + j*(m_numNodes-1) + i - 1) ;
						
						
						
						
					}else{
												
						m_g[j][i] = m_v[ j][ l1] + *(c + j*(m_numNodes-1) + i - 1) ;
						
						
						
					}
					
					// update u and the pointer for p
					
					if(m_g[j][i] < m_u[i][l2] ){
						
						m_u[i][l2] = m_g[j][i];
						m_px[i][l2] =  j;
						
					}


					
				}//end of first for loop on j, j < i
				
				
				for(j = i + 1; j < m_numNodes; j++){ //we are coming from node j
					
		
					//calculate g  -- l2 - d[ i]  is the demand trough and including node j
					l1 = l2 - *(m_demand + i);
					
					if( m_px[j][ l1 ] != i ){//check to make sure we did not come into j from i
						
						
						m_g[j][i] = m_u[ j][ l1 ] + *(c + j*(m_numNodes-1) + i ) ;
						
						
					}else{
												
						m_g[j][i] = m_v[ j][ l1] + *(c + j*(m_numNodes-1) + i ) ;
						
					}
					
					// update u and the pointer for p
					
					if(m_g[j][i] < m_u[i][l2] ){
						
						m_u[i][l2] = m_g[j][i];
						m_px[i][l2] =  j;
						
					}

					
				}//end of second for loop on j, j > i				
				
				
				//now calculate the second best solution and point
				//right now m_px[i][l2] points to the best j node
				
				for(j =m_numHubs; j < m_numNodes; j++){ //we are coming from node j
					
					if(j != i){
						
						if( (m_g[j][i] < m_v[i][l2] ) && (m_px[i][l2] != j)  ){ // kipp the && gives the second best
							
							//if(g(j, i) < v(i, l2),
							
							m_v[i][l2] = m_g[j][i];
							m_tx[i][l2] = j;	
							
							
						}
						
					}
					
					
				}//end second best calculation, anothe for loop on j
				
				//now if l2 = l  we are done
				if(l2 == l ){
					
					if(  m_u[i][l2] + *(c + i*(m_numNodes-1) + k )  < rcost){
						
						rcost = m_u[i][l2] + *(c + i*(m_numNodes-1) + k );

						bestLastNode = i;
					}
					
				}
				
				
			}//end if on demand less than l2
			
			
		}//i loop
		
		
	}//l2 loop

	
	//std::cout << "best Last Node = "  << bestLastNode << std::endl;
	
	// now get the path that gives the reduced cost

	
	int currentNode;
	int successorNode;
	int lvalue;
	
	//initialize
	// we work backwords from bestLastNode
	//in our recursion we recurse on the currentNode and assume
	//it is in the optimal path
	
	//node bestLastNode is a higher number than k
	*(m_varIdx + (*kountVar)++) = startPnt + bestLastNode*(m_numNodes - 1)  +  k ;
	
	
	

	//by successor, I mean node after current node in the path
	successorNode = k;
	currentNode = bestLastNode;
	//the lvalue is the demand through the currentNode
	lvalue = l ;


	while(currentNode != k){

		if( m_px[ currentNode][ lvalue ] != successorNode){
			

			
			//update nodes
			successorNode = currentNode;
			currentNode = m_px[ currentNode][ lvalue ];
			
			
			if(currentNode - successorNode > 0){
				 //below diagonal

				*(m_varIdx  + (*kountVar)++) = startPnt + currentNode*(m_numNodes - 1)  +  successorNode;

				
			}else{
				 //above diagonal

				*(m_varIdx + (*kountVar)++) = startPnt + currentNode*(m_numNodes - 1)  +  successorNode  - 1 ;

			}

			
		}else{ //take second best
			
			
			//update nodes
			successorNode = currentNode;
			currentNode = m_tx[ currentNode][ lvalue ];
			
			if(currentNode - successorNode > 0){
				 //below diagonal
				*(m_varIdx  + (*kountVar)++) = startPnt + currentNode*(m_numNodes - 1)  +  successorNode;
				
			}else{
				 //above diagonal
				*(m_varIdx + (*kountVar)++) = startPnt + currentNode*(m_numNodes - 1)  +  successorNode  - 1 ;
				
			}
			
		}
		
		//update lvalue
		lvalue = lvalue - *(m_demand + successorNode);
	
		

	}//end while

	
	//go from node k to bestLastNode -- already done in loop above
	//*(m_varIdx + (*kountVar)++) = startPnt + k*(m_numNodes - 1)  +  currentNode - 1;
	

	return rcost;
	
}//end qroute




void OSRouteSolver::getColumns(const  double* y, const int numRows,
		int &numColumns, int* numNonzVec, double* costVec, double* rcostVec,
		int** rowIdxVec, double** valuesVec, double &lowerBound) 
{
	
//first strip of the phi dual values and then the convexity row costs
	
	int i;
	int j;
	int numCoulpingConstraints;
	numCoulpingConstraints = m_numNodes - m_numHubs;
	
	int numVar;
	numVar = m_numNodes*m_numNodes - m_numHubs;
	int numNonz;
	
	try{
		

		
		if(numRows != m_numNodes) throw ErrorClass("inconsistent row count in getColumns");
		
		for(i = 0; i < m_numHubs; i++){
			
			m_phi[i] = 0.0;
		}
		
		for(i = 0; i < numCoulpingConstraints; i++){
			
			m_phi[ i + m_numHubs] = y[ i];
			
		}
		
		for(i = 0; i < m_numHubs; i++){
			
			m_psi[ i ] = y[ i + numCoulpingConstraints];
			
		}
		
		//get the reduced costs 
		calcReducedCost( m_cost,  m_phi,  m_rc);
		
		
		
		int kountVar;
		double testVal;
		testVal = 0;
		int k;
		int startPntInc;
		int rowCount;
		
		
		
		getOptL( m_rc);
		m_lowerBnd = 0.0;
		for(k = 0; k < m_numHubs; k++){
			
			startPntInc  =  k*m_upperBoundL*(m_numNodes*m_numNodes - m_numNodes) + (m_optL[ k] - 1)*(m_numNodes*m_numNodes - m_numNodes);
			
			std::cout << " whichBlock =  " << k << "  L = " << m_optL[ k] << std::endl;
			
			testVal += m_optL[ k];
			
			kountVar = 0;
			
			m_optValHub[ k] = qrouteCost(k,  m_optL[ k], m_rc,  &kountVar);
			
			m_optValHub[ k] -= m_psi[ k ];
			

			
			std::cout << "Best Reduced Cost Hub " << k << " =  "  << m_optValHub[ k] << std::endl;
			m_lowerBnd += m_optValHub[ k];
			
			//loop over the rows, scatter each row and figure
			//out the column coefficients in this row
			//first scatter the sparse array m_varIdx[ j]
			
			m_costVec[ k] = 0.0;
			
			for(j = 0; j < kountVar; j++){
				
				
				//we are counting the NUMBER of times the variable used
				//the same variable can appear more than once in m_varIdx
				m_tmpScatterArray[ m_varIdx[ j] - startPntInc  ] += 1;
				
				// is variable m_varIdx[ j] - startPntInc in this row	
				
				m_costVec[ k] += m_cost[k][  m_varIdx[ j] - startPntInc  ];
				
			}
			
			
			
			numNonz = 0;
			//multiply the sparse array by each constraint
			for(i = 0; i < numCoulpingConstraints; i++){
				
				rowCount = 0;
				
				for(j = m_pntAmatrix[ i]; j < m_pntAmatrix[ i + 1]; j++){
					
					//m_Amatrix[ j] is a variable index -- this logic works
					//since the Amatrix coefficient is 1 -- we don't need a value
					//it indexes variable that points into the node
					rowCount += m_tmpScatterArray[  m_Amatrix[ j] ];
					

				}
				
				if(rowCount > 0){
					
					//std::cout << "GAIL NODE " << i + m_numHubs <<  " COUNT = " << rowCount << std::endl;
					m_newColumnRowIdx[ k][ numNonz] = i;
					m_newColumnRowValue[ k][ numNonz] = rowCount;
					numNonz++;
				}
					
					
			}//end loop on coupling constraints
			
			//add a 1 in the convexity row
			m_newColumnRowIdx[ k][ numNonz] = m_numNodes - m_numHubs + k;
			m_newColumnRowValue[ k][ numNonz] = 1.0;
			numNonz++;
			
			m_nonzVec[ k] = numNonz;
			
			//zero out the scatter vector and store the generated column
			for(j = 0; j < kountVar; j++){
				
				
				m_thetaIndex[ m_numThetaNonz++ ] =  m_varIdx[ j] - startPntInc ;
				
				m_tmpScatterArray[ m_varIdx[ j] - startPntInc  ]  = 0;
				
				// is variable m_varIdx[ j] - startPntInc in this row	
				
			}
			
			m_costVec[ k] =  m_optL[ k]*m_costVec[ k];
			m_thetaCost[ m_numThetaVar ] = m_costVec[ k];
			m_thetaPnt[ m_numThetaVar++ ]  = m_numThetaNonz;
			
			//std::cout << "GAIL COST VECTOR " <<  m_costVec[ k] << std::endl;
			
			//stuff for debugging
			//*****************************//
			/**
			int ivalue;
			int jvalue;
			for(j = 0; j < kountVar; j++){
				
				startPntInc  =  k*m_upperBoundL*(m_numNodes*m_numNodes - m_numNodes) + (m_optL[ k] - 1)*(m_numNodes*m_numNodes - m_numNodes);
				
				
				std::cout << "Variable Index = " <<  m_varIdx[ j] - startPntInc ;
				std::cout << "  Variable = " << m_variableNames[  m_varIdx[ j] - startPntInc ]	<< std::endl ;	
				
				//tmp -- get the node
				
				//tmp = fmod(m_varIdx[ j] - startPntInc, m_numNodes) ;  
				
				ivalue = floor( (m_varIdx[ j] - startPntInc)/(m_numNodes - 1) );
				jvalue = (m_varIdx[ j] - startPntInc) - ivalue*(m_numNodes - 1);
				std::cout << " i NODE NUMBER = " << ivalue  ;
				
				if(  jvalue  > ivalue ){
					
					std::cout << " j NODE NUMBER = " <<  jvalue + 1   << std::endl;
				}else{
					
					std::cout << " j NODE NUMBER = " <<  jvalue   << std::endl;
				}
				
				
			}
			
			std::cout << "Route True Cost = " << m_costVec[ k] << std::endl;
			*/
			//**************************//
			//end debugging stuff
	
			
		}//end of loop on hubs
		
		std::cout << "Lower Bound = " << m_lowerBnd << std::endl;
		
		
		if(testVal != m_totalDemand) {
			
			std::cout  << "TOTAL DEMAND = " << m_totalDemand << std::endl;
			std::cout  << "Test Value = " << testVal << std::endl;
			throw  ErrorClass( "inconsistent demand calculation" );
		}
		
		
		

		
		
	} catch (const ErrorClass& eclass) {

		throw ErrorClass(eclass.errormsg);

	}
	
	
	//set method parameters
	numColumns = m_numHubs;
	lowerBound =  m_lowerBnd;
	
	std::cout << "LEAVING GET COLUMNS" << std::endl;
	return;
	
}//end getColumns




OSInstance* OSRouteSolver::getInitialRestrictedMaster( ){

	
	std::cout << "Executing OSRouteSolver::getInitialRestrictedMaster( )" << std::endl;
	
	// define the classes
	FileUtil *fileUtil = NULL;
	OSiLReader *osilreader = NULL;
	DefaultSolver *solver  = NULL;
	OSInstance *osinstance = NULL;


	// end classes    

	std::string testFileName;
	std::string osil;
	
	//std::vector< int> indexes;
	fileUtil = new FileUtil();
	
	
	std::map<int, std::map<int, std::vector<int> > >::iterator  mit;
	std::map<int, std::vector<int> >::iterator  mit2;
	std::vector<int>::iterator  vit;
	
	m_osinstanceMaster = NULL;
	
	//add linear constraint coefficients
	//number of values will nodes.size() the coefficients in the node constraints
	//plus coefficients in convexity constraints which is the number of varaibles
	int kountNonz;
	int kount;
	int numThetaVar = m_numberOfSolutions*m_numHubs;
	double *values = new double[ m_numberOfSolutions*(m_numNodes-m_numHubs) + numThetaVar];
	int *indexes = new int[ m_numberOfSolutions*(m_numNodes-m_numHubs) + numThetaVar];
	int *starts = new int[ numThetaVar + 1]; 
	kount = 0;
	
	starts[ 0] = 0;	
	
	int startsIdx;
	startsIdx = 0;

	std::vector<IndexValuePair*> primalValPair;

	try {
		
		if(m_initOSiLFile.size() == 0) throw ErrorClass("OSiL file to generate restricted master missing");
		osil = fileUtil->getFileAsString( m_initOSiLFile.c_str());

		osilreader = new OSiLReader();
		osinstance = osilreader->readOSiL(osil);
		

		
		int i;
		int j;
		int k;
		//fill in the cost vector first
		//the x vector starts at 2*m_numHubs
		
		int idx1;
		int idx2;
		
		
		idx2 = 0;  //zRouteDemand have 0 coefficients in obj
		//fill in m_cost from the cost coefficient in osil
		for(k = 0; k < m_numHubs; k++){
			
			idx1 = 0;
			
			for(i = 0; i < m_numNodes; i++){
				
				for(j = 0; j < i; j++){
				
					m_cost[k][idx1++ ] = osinstance->instanceData->objectives->obj[0]->coef[ idx2++ ]->value;
				}
				
				for(j = i + 1; j < m_numNodes; j++){
					
					m_cost[k][idx1++ ] = osinstance->instanceData->objectives->obj[0]->coef[ idx2++ ]->value;
					
				}
			}
		}
	

	
		//get variable names for checking purposes
		std::string* varNames;
		varNames =  osinstance->getVariableNames();

		
		//start building the restricted master here
		
		
		m_osinstanceMaster = new OSInstance();
		m_osinstanceMaster->setInstanceDescription("The Initial Restricted Master");
		
		// first the variables
		m_osinstanceMaster->setVariableNumber( m_numberOfSolutions*m_numHubs);   
		
		// now add the objective function
		m_osinstanceMaster->setObjectiveNumber( 1);
		SparseVector *objcoeff = new SparseVector( m_numberOfSolutions*m_numHubs);   

		
		// now the constraints
		m_osinstanceMaster->setConstraintNumber( m_numNodes); 		
		
		
		//addVariable(int index, string name, double lowerBound, double upperBound, char type, double init, string initString);
		// we could use setVariables() and add all the variable with one method call -- below is easier

		//

		int varNumber;
		varNumber = 0;
		std::string masterVarName;
		kountNonz = 0;
		// now get the primal solution
		//solve the model for solution in the osoption object
		for ( mit = m_initSolMap.begin() ; mit != m_initSolMap.end(); mit++ ){
			
			//kipp change upper and lower bounds here on z variables
			//loop over nodes and routes and set bound
			//set kount to the start of the z variables
			//go past the x variables
			kount  =  2*m_numHubs + m_numHubs*(m_numNodes*m_numNodes - m_numNodes);
			osinstance->bVariablesModified = true;
			for ( mit2 = mit->second.begin() ; mit2 != mit->second.end(); mit2++ ){ //we are looping over routes in solution mit
				
				
				
				startsIdx++;
				starts[ startsIdx ] = kountNonz + mit2->second.size() + 1; //the +1 comes from the convexity row
				
				
				//make sure all lower bounds on z variables on this route back to 0.0
				for(i = 0; i < m_numNodes; i++){
					osinstance->instanceData->variables->var[ kount + mit2->first*m_numNodes + i]->lb = 0.0;
				}
				
	
				for ( vit = mit2->second.begin() ; vit != mit2->second.end(); vit++ ){	
					
							
					osinstance->instanceData->variables->var[ kount + mit2->first*m_numNodes + *vit]->lb = 1.0;
					//std::cout << "FIXING LOWER BOUND ON VARIABLE " << osinstance->getVariableNames()[ kount + mit2->first*m_numNodes + *vit ] << std::endl;
					
					values[ kountNonz] = 1.0;
					indexes[ kountNonz ] = *vit - m_numHubs ;  //0 based counting
					kountNonz++;
					
				}
				
				
				 //now for the convexity row coefficient	
				values[ kountNonz] = 1;
				indexes[ kountNonz ] = m_numNodes - m_numHubs +  mit2->first  ;
				kountNonz++;
				
				
			}
			
			solver = new CoinSolver();
			solver->sSolverName ="cbc"; 
			solver->osinstance = osinstance;	
			
			solver->buildSolverInstance();
			solver->solve();
			
			//get the solver solution status
			
			std::cout << "Solution Status =  " << solver->osresult->getSolutionStatusType( 0 ) << std::endl;
			
			//get the optimal objective function value
			
			
			primalValPair = solver->osresult->getOptimalPrimalVariableValues( 0);

			//loop over routes again to create master objective coefficients

			for(k = 0; k < m_numHubs; k++){
				
				
				//lets get the x variables
				//the variables for this route should be from 2*numHubs + k*(numNodes - 1*)*(numNodes - 1)
				idx1 = 2*m_numHubs + k*m_numNodes*(m_numNodes - 1);
				idx2 = idx1 + m_numNodes*(m_numNodes - 1);
				//end of x variables
			
				//std::cout << "HUB " <<  k  << " VARIABLES" << std::endl;
				
			

				for(i = idx1; i < idx2; i++){
					if(  primalValPair[ i]->value > .1 ){
						//std::cout <<  osinstance->getVariableNames()[  primalValPair[ i]->idx  ] << std::endl;
						//std::cout <<  m_variableNames[  primalValPair[ i]->idx  -  k*(m_numNodes - 1)*m_numNodes - 2*m_numHubs  ] << std::endl;
						m_thetaIndex[ m_numThetaNonz++ ] = primalValPair[ i]->idx  -  k*(m_numNodes - 1)*m_numNodes - 2*m_numHubs;
					}
					
				}
				
				m_thetaCost[ m_numThetaVar ] = primalValPair[ k]->value*primalValPair[ k + m_numHubs]->value;
				m_thetaPnt[ m_numThetaVar++ ] = m_numThetaNonz;
				
				masterVarName = makeStringFromInt("theta(", mit->first);
				masterVarName += makeStringFromInt(",", k);
				masterVarName += ")";
				std::cout << masterVarName << std::endl;
				m_osinstanceMaster->addVariable(varNumber++, masterVarName, 0, 1, 'C');
				
				std::cout << "Optimal Objective Value = " << primalValPair[ k]->value*primalValPair[ k + m_numHubs]->value << std::endl;
				
				objcoeff->indexes[ k + (mit->first)*m_numHubs] = k +  (mit->first)*m_numHubs;
				objcoeff->values[ k + (mit->first)*m_numHubs] = primalValPair[ k]->value*primalValPair[ k + m_numHubs]->value;

				
				std::cout <<  osinstance->getVariableNames()[ k ] << std::endl;
				std::cout <<  osinstance->getVariableNames()[ k + m_numHubs ] << std::endl;
				

			}//end for on k -- hubs
			
			
			primalValPair.clear();
			delete solver;
			solver = NULL;
		}//end for on number of solutions
		
		//add the constraints
		//add the row saying we must visit each node
		for( i =  0; i < m_numNodes - m_numHubs ; i++){
			
			m_osinstanceMaster->addConstraint(i,  makeStringFromInt("visitNode_", i + m_numHubs) , 1.0, 1.0, 0); 
		}
		
		kount = 0;
		
		//add the convexity row
		for( i =  m_numNodes - m_numHubs; i < m_numNodes ; i++){
			
			m_osinstanceMaster->addConstraint(i,  makeStringFromInt("convexityRowRoute_", kount++ ) , 1.0, 1.0, 0); 
		}
		
		m_osinstanceMaster->addObjective(-1, "objfunction", "min", 0.0, 1.0, objcoeff);
		
		std::cout << "kountNonz = " << kountNonz << std::endl;
		
		//add the linear constraints coefficients
		m_osinstanceMaster->setLinearConstraintCoefficients(kountNonz , true, 
				values, 0, kountNonz - 1,  indexes, 0, kountNonz - 1, starts, 0, startsIdx);
	
		

		delete objcoeff;
		
		//delete[] values;
		//delete[] starts;
		//delete[] indexes;
		delete osilreader;
		osilreader = NULL;

		

	} catch (const ErrorClass& eclass) {
		std::cout << std::endl << std::endl << std::endl;
		if (osilreader != NULL)
			delete osilreader;
		if (solver != NULL)
			delete solver;


		//  Problem with the parser
		throw ErrorClass(eclass.errormsg);
	}

	delete fileUtil;
	fileUtil = NULL;

	return m_osinstanceMaster;
}//end generateInitialRestrictedMaster



void OSRouteSolver::getOptions(OSOption *osoption) {
	
	
	std::cout << "Executing getOptions(OSOption *osoption)" << std::endl;
	//get any options relevant to OSColGenApp
	try{
		std::vector<SolverOption*> solverOptions;
		std::vector<SolverOption*>::iterator vit;
		std::vector<int >demand;
	
		m_numberOfSolutions = 0;
		solverOptions = osoption->getSolverOptions("decomp");
		//iterate over the vector
		
		int tmpVal;
		
		std::string routeString; //variable for parsing a category option
		std::string solutionString; //variable for parsing a category option
		string::size_type pos1; //variable for parsing a category option
		string::size_type pos2; //variable for parsing a category option
		string::size_type pos3; //variable for parsing a category option
		
		
		std::map<int, std::map<int, std::vector<int> > >::iterator  mit;
		std::map<int, std::vector<int> >::iterator  mit2;
		int solutionNumber;
		int routeNumber;
		
	
		for (vit = solverOptions.begin(); vit != solverOptions.end(); vit++) {
			
			
			//std::cout << (*vit)->name << std::endl;
			
			//(*vit)->name.compare("initialCol") == 0
			//if(rowNames[ i3].find("routeCapacity(1)") == string::npos )
			
			if( (*vit)->name.find("numHubs") !=  std::string::npos){
				
				
				std::istringstream hubBuffer( (*vit)->value);
				hubBuffer >> m_numHubs;
				std::cout << "numHubs = " << m_numHubs <<  std::endl;
				
			}else{
				
				if((*vit)->name.find("numNodes") !=  std::string::npos){
					
					
					std::istringstream numNodesBuffer( (*vit)->value);
					numNodesBuffer >> m_numNodes;
					std::cout << "numNodes = " <<  m_numNodes <<  std::endl;
					
				}else{
					if((*vit)->name.find("totalDemand") !=  std::string::npos){
						
						
						std::istringstream totalDemandBuffer( (*vit)->value);
						totalDemandBuffer >> m_totalDemand;
						std::cout << "m_totalDemand = " << m_totalDemand <<  std::endl;
						
					}else{
						if((*vit)->name.find("minDemand") !=  std::string::npos){
							
							
							std::istringstream minDemandBuffer( (*vit)->value);
							minDemandBuffer >> m_minDemand;
							std::cout << "m_minDemand = " << m_minDemand <<  std::endl;
						
						}else{
							if( (*vit)->name.find("demand") !=  std::string::npos ){
								
								
								std::istringstream demandBuffer( (*vit)->value);
								demandBuffer >> tmpVal;
								demand.push_back( tmpVal);
								//std::cout << "demand = " << tmpVal <<  std::endl;
								
							}else{
								if((*vit)->name.find("routeCapacity") !=  std::string::npos ){
									std::istringstream routeCapacityBuffer( (*vit)->value);
									routeCapacityBuffer >> m_routeCapacity;
									std::cout << "m_routeCapacity = " << m_routeCapacity <<  std::endl;
									
								}else{
									
									if((*vit)->name.find("osilFile") !=  std::string::npos ){
										m_initOSiLFile = (*vit)->value;
										std::cout << "m_initOSiLFile = " << m_initOSiLFile <<  std::endl;
										
									}else{
										
										if( (*vit)->name.find("restrictedMasterSolution") !=  std::string::npos  ){
											//std::istringstream buffer( (*vit)->value);
											//buffer >> m_numberOfSolutions;
	
											//get the block number and solution number
											//first get routeString and soluionString
											//parse the category string base on :
											pos1 = (*vit)->category.find( ":");
											if(pos1 == std::string::npos ) throw ErrorClass("OSoL category attribute not properly defined");
											
											//solutionString = (*vit)->category.substr( pos1 + 1, pos2 - pos1 - 1);
											solutionString = (*vit)->category.substr( 0,  pos1);
											routeString = (*vit)->category.substr( pos1 + 1);
	
											pos2 = solutionString.find( "n");
											if(pos2  == std::string::npos ) throw ErrorClass("OSoL category attribute not properly defined");
											
											std::istringstream solutionBuffer( solutionString.substr( pos2 + 1)  );
											solutionBuffer >> solutionNumber;
											//std::cout << "solution number = " << solutionNumber  << std::endl;
											
											
											pos3 = routeString.find( "e");
											if(pos3  == std::string::npos ) throw ErrorClass("OSoL category attribute not properly defined");
											std::istringstream routeBuffer( routeString.substr( pos3 + 1)  );
											routeBuffer >> routeNumber;
											//std::cout << "route number = " <<  routeNumber << std::endl;
											std::istringstream nodeBuffer( (*vit)->value);
											nodeBuffer >> tmpVal;
											
											mit = m_initSolMap.find(  solutionNumber  );
											
											if( mit  != m_initSolMap.end() ){
												// we have solution from before
												// do we have a new route?
												
												mit2 = mit->second.find( routeNumber);
												
												if(mit2 != mit->second.end() ){
												// we have a route from before and solution from before
	
													
													mit2->second.push_back( tmpVal);
													
													
												}else{
													
													//we have a new route, but old solution
													std::vector<int> tmpVec;
													tmpVec.push_back( tmpVal) ;
													mit->second.insert( std::pair<int,std::vector<int> >(routeNumber, tmpVec) );	
													
												
												}
												
											}else{
												// we have a new solution
												std::vector<int> tmpVec;
												tmpVec.push_back( tmpVal) ;
												
												std::map<int, std::vector<int> > tmpMap;
												tmpMap.insert( std::pair<int,std::vector<int> >(routeNumber, tmpVec) );
												m_initSolMap.insert( std::pair<int, std::map<int, std::vector<int> > >(solutionNumber, tmpMap) )  ;
												
											}
										}
									}
								}
							}
						}
					}
				}
			}//end if on solver options
			
		}//end for loop on options
	
		//now fill in demand
		m_demand = new int[ m_numNodes];
		std::vector<int>::iterator vit2;
		if(m_numNodes != demand.size( ) ) throw ErrorClass("inconsistent number of demand nodes");
		int i;
		i = 0;
		for (vit2 = demand.begin(); vit2 != demand.end(); vit2++) {
			
			*(m_demand + i++) = *vit2;
			
		}
		demand.clear();
		
		//kipp -- fill in numberOfRestricedMasterSolutions from map size
		 m_numberOfSolutions = m_initSolMap.size();
		
	
	} catch (const ErrorClass& eclass) {

		throw ErrorClass(eclass.errormsg);

	}		
	
}//end getOptions


void OSRouteSolver::getCuts(const  double* x){
	
	
}//end getCuts

void OSRouteSolver::calcReducedCost( double** c, double* phi, double* d){
	
	int k;
	int i;
	int j;
	int l;
	int kount;
	kount = 0;
	int tmpVal;
	tmpVal = m_numNodes - 1;
	
	for(k = 0; k < m_numHubs; k++){
		
		for(l = 0; l < m_upperBoundL; l++){
			
			
			for(i = 0; i< m_numNodes; i++){
				
				//if we have (i, j) where j is hub then do not subtract off phi[ j]
				for(j = 0; j < i; j++){
					
					m_rc[ kount++] = (l + 1)*c[k][ i*tmpVal + j ] - phi[ j];
					
				}
				
				for(j = i + 1; j < m_numNodes; j++){
					
					m_rc[ kount++] = (l + 1)*c[k][ i*tmpVal + j - 1 ] - phi[ j];
					
				}
				
				
			}
			
			
		}
		
		
	}
	
}//end calcReducedCost


void OSRouteSolver::createVariableNames( ){
	
	int i;
	int j;
	int kount;
	
	kount = 0;
	
	for(i = 0; i< m_numNodes; i++){
		
		//if we have (i, j) where j is hub then do not subtract off phi[ j]
		for(j = 0; j < i; j++){
			
			m_variableNames[ kount] = makeStringFromInt("x(" , i);
			m_variableNames[ kount] += makeStringFromInt( "," , j);
			m_variableNames[ kount] +=  ")";
			//std::cout << "GAIL VARIABLE NAME " << m_variableNames[ kount] << std::endl;
			
			kount++;
			
		}
		
		for(j = i + 1; j < m_numNodes; j++){
			
			m_variableNames[ kount] = makeStringFromInt("x(" , i);
			m_variableNames[ kount] += makeStringFromInt( "," , j);
			m_variableNames[ kount] +=  ")";
			
			//std::cout << "GAIL VARIABLE NAME " << m_variableNames[ kount] << std::endl;
			kount++;
			
		}
		
		
	}	
}//end createVariableNames

void OSRouteSolver::createAmatrix(){
	
	//arrays for the coupling constraint matrix
	//int* m_pntAmatrix;
	//int* m_Amatrix;
	
	
	int i;
	int j;

	int numNonz;
	
	//loop over nodes 
	m_pntAmatrix[ 0] = 0; 
	numNonz = 0;
	
	for(j = m_numHubs; j < m_numNodes; j++){
		
		
		for(i = 0; i < j; i++){
			
			m_Amatrix[ numNonz++] = i*(m_numNodes - 1) + j - 1 ;

		}
		
		for(i = j + 1; i < m_numNodes; i++){
			
			m_Amatrix[ numNonz++] = i*(m_numNodes - 1) + j ;		
			
		}
		
		m_pntAmatrix[ j - m_numHubs + 1]  = numNonz;
	
	}
	
	/*
	for(i = 0; i < m_numNodes - m_numHubs; i++){
		
		for(j = m_pntAmatrix[ i]; j <  m_pntAmatrix[ i + 1]; j++){
			
			std::cout << m_variableNames[  m_Amatrix[ j ] ] << std::endl;
			
		}

	}
	*/
	
}//end createAmatrix

void OSRouteSolver::pauHana(const double* theta){
	
	std::cout <<  std::endl;
	std::cout << "     PAU HANA TIME! " << std::endl;
	
	int i;
	int j;
	int k;
	double cost = 0;
	for(i = 0; i < m_numThetaVar - 1  ; i++){
	
		cost += theta[ i]*m_thetaCost[ i + 1];
		//std::cout << "COLUMN VALUE = " << theta[ i] << std::endl;
	}
	
	
	std::cout << "FINAL SOLUTION VALUE = " << cost << std::endl;
	std::cout << "NUMBER OF GENERATED COLUMNS = " << m_numThetaVar - 1 << std::endl;
	std::cout << std::endl <<  std::endl;
	
	int numSets;
	int kount;
	
	numSets = floor( (m_numThetaVar - 1 ) / m_numHubs);
	
	//kipp throw exception if number of columns not an even multiple of m_numHubs
	kount = 0;
	
	
	for(i = 0; i < numSets  ; i++){
		
		for(k = 0; k < m_numHubs; k++){
			
			if( theta[ kount ] > .001){
				
				std::cout << "HUB = "  <<  k << "  THETA = " << kount << " = "  << theta[ kount] << std::endl;
				
				for(j = m_thetaPnt[ kount ];  j <  m_thetaPnt[ kount + 1];  j++){
					
					std::cout << "VARIABLE "  <<  m_variableNames[ m_thetaIndex[ j] ]   << std::endl;
					
				}
				
			}//loop on if positive
			
			kount++;
			
		}//loop on hubs
	
	}//loop on sets
		
}//end pauHana -- no pun intended


OSInstance* OSRouteSolver::getSeparationInstance(){
	

	
	
	m_osinstanceSeparation = NULL;
	
	//add linear constraint coefficients
	//number of values will nodes.size() the coefficients in the node constraints
	//plus coefficients in convexity constraints which is the number of varaibles
	int kountNonz;
	int kount;
	int startsIdx;
	//we build these on nodes that do not include the hubs
	int numYvar = (m_numNodes - m_numHubs)*(m_numNodes - m_numHubs - 1);
	int numVvar = m_numNodes - m_numHubs;
	//the plus 1 is for the kludge row
	int numCon = (m_numNodes - m_numHubs) + (m_numNodes - m_numHubs)*(m_numNodes - m_numHubs - 1)/2 + 1;
	double *values = new double[ 2*numYvar + 2*numVvar];
	int *indexes = new int[ 2*numYvar + 2*numVvar];
	int *starts = new int[ numYvar + numVvar + 1]; 
	starts[ 0] = 0;	
	startsIdx = 0;
	startsIdx++;
	kountNonz = 0;
	int i;
	int j;
	
			
	std::string separationVarName;
	std::string separationConName;

	try {
		
		m_osinstanceSeparation = new OSInstance();
		
		//start building the separation instance

		m_osinstanceSeparation->setInstanceDescription("The Tour Breaking Separation Problem");

		
		// now the constraints
		m_osinstanceSeparation->setConstraintNumber( numCon); 
		
		
		//add the node rows
		for( i =  0; i < m_numNodes - m_numHubs ; i++){
			
			m_osinstanceSeparation->addConstraint(i,  makeStringFromInt("nodeRow_", i+  m_numHubs ) , 0.0, 1.0, 0); 
			
		}
		
		//add the variable rows rows
	
		int rowKounter;
		rowKounter = m_numNodes - m_numHubs;
		
		for(i = m_numHubs; i < m_numNodes; i++){
			
			
			
			for(j = i+1; j < m_numNodes; j++){
				separationConName = makeStringFromInt("Row_(", i);
				separationConName += makeStringFromInt(",", j);
				separationConName += ")";
				
				m_osinstanceSeparation->addConstraint(rowKounter++,  separationConName , 0, 0, 0); 
			}
			
		}	
		
		// the klude row so we have +/-1 in every column
		m_osinstanceSeparation->addConstraint(rowKounter++,  "kludgeRow" , 0, m_numNodes, 0);
		
		//  the variables
		m_osinstanceSeparation->setVariableNumber(  numYvar + numVvar);   
		
		
		
		std::cout << "NUMBER OF VARIABLES SET = " << numYvar + numVvar << std::endl;
		//add the v variables
		for(i = 0; i < numVvar; i++){
			
			separationVarName = makeStringFromInt("v", i + m_numHubs);
			
			m_osinstanceSeparation->addVariable(i, separationVarName, 0, 1, 'C');
			
			values[ kountNonz ] = -1.0;
			indexes[ kountNonz ] = i;
			kountNonz++;
			
			values[ kountNonz ] = 1.0;
			indexes[ kountNonz ] = rowKounter - 1;
			kountNonz++;
			
			
			
			starts[ startsIdx++ ] = kountNonz;
			
			
		}
		//add the y variables
		kount = numVvar;
		
		int i1;
		int j1;
		int kountCon;
	
		//adjust for fact we don't use hub nodes
		i1 = i - m_numHubs;
		j1 = j - m_numHubs;
		
		kountCon = m_numNodes - m_numHubs;
		
		for(i1 = 0; i1 < m_numNodes - m_numHubs; i1++){
			

			
			for(j1 = i1 + 1; j1 < m_numNodes - m_numHubs; j1++){
				
	
				
				separationVarName = makeStringFromInt("y(", i1 + m_numHubs);
				separationVarName += makeStringFromInt(",", j1 + m_numHubs);
				separationVarName += ")";
				m_osinstanceSeparation->addVariable(kount++, separationVarName, 0, 1, 'C');
				
				values[ kountNonz ] = 1.0;
				indexes[ kountNonz ] = i1;
				kountNonz++;
						
				values[ kountNonz ] = -1.0;
				indexes[ kountNonz ] = kountCon ;
				kountNonz++;
			
				starts[ startsIdx++ ] = kountNonz;
				
				
				
				
				separationVarName = makeStringFromInt("y(", j1 + m_numHubs);
				separationVarName += makeStringFromInt(",", i1 + m_numHubs);
				separationVarName += ")";
				m_osinstanceSeparation->addVariable(kount++, separationVarName, 0, 1, 'C');
				
				values[ kountNonz ] = 1.0;
				indexes[ kountNonz ] = j1;
				kountNonz++;
						
				values[ kountNonz ] = -1.0;
				indexes[ kountNonz ] = kountCon ;
				kountNonz++;
			
				starts[ startsIdx++ ] = kountNonz;
				
				
				kountCon++;
				
				
			}
			
		}
		
		std::cout << "NUMBER OF VARIABLES ADDED = " << kount << std::endl;
		
		// now add the objective function
		m_osinstanceSeparation->setObjectiveNumber( 1);
		SparseVector *objcoeff = new SparseVector( numVvar);  

		
		for(i = 0; i < numVvar; i++){
			
			objcoeff->indexes[ i] = i;
			objcoeff->values[ i] = 1.0;
		
		}
		

	
	
		
		m_osinstanceSeparation->addObjective(-1, "objfunction", "min", 0.0, 1.0, objcoeff);
		//now for the nonzeros
		//add the linear constraints coefficients
		m_osinstanceSeparation->setLinearConstraintCoefficients(kountNonz , true, 
				values, 0, kountNonz - 1,  indexes, 0, kountNonz - 1, starts, 0, startsIdx);
	
		
		
		std::cout << m_osinstanceSeparation->printModel(  ) << std::endl;
		//below is temporty see if we can setup as a Clp network problem
	    CoinPackedMatrix* matrix;
	    bool columnMajor = true;
	    double maxGap = 0;
		matrix = new CoinPackedMatrix(
		columnMajor, //Column or Row Major
		columnMajor? m_osinstanceSeparation->getConstraintNumber() : m_osinstanceSeparation->getVariableNumber(), //Minor Dimension
		columnMajor? m_osinstanceSeparation->getVariableNumber() : m_osinstanceSeparation->getConstraintNumber(), //Major Dimension
		m_osinstanceSeparation->getLinearConstraintCoefficientNumber(), //Number of nonzeroes
		columnMajor? m_osinstanceSeparation->getLinearConstraintCoefficientsInColumnMajor()->values : m_osinstanceSeparation->getLinearConstraintCoefficientsInRowMajor()->values, //Pointer to matrix nonzeroes
		columnMajor? m_osinstanceSeparation->getLinearConstraintCoefficientsInColumnMajor()->indexes : m_osinstanceSeparation->getLinearConstraintCoefficientsInRowMajor()->indexes, //Pointer to start of minor dimension indexes -- change to allow for row storage
		columnMajor? m_osinstanceSeparation->getLinearConstraintCoefficientsInColumnMajor()->starts : m_osinstanceSeparation->getLinearConstraintCoefficientsInRowMajor()->starts, //Pointers to start of columns.
		0,   0, maxGap ); 
		
		ClpNetworkMatrix network( *matrix);
		

		ClpSimplex  model;
		
		//if( m_osinstanceSeparation->getObjectiveMaxOrMins()[0] == "min") osiSolver->setObjSense(1.0);
		
		
	    model.setOptimizationDirection( 1);
		model.loadProblem( network, m_osinstanceSeparation->getVariableLowerBounds(), 
				m_osinstanceSeparation->getVariableUpperBounds(),  
				m_osinstanceSeparation->getDenseObjectiveCoefficients()[0], 
				m_osinstanceSeparation->getConstraintLowerBounds(), m_osinstanceSeparation->getConstraintUpperBounds()
		);
		

		
		//model.loadProblem(network, lowerColumn, upperColumn, objective,
	    //lower, upper);
		
	     model.factorization()->maximumPivots(200 + model.numberRows() / 100);
	     model.factorization()->maximumPivots(1000);
	     //model.factorization()->maximumPivots(1);
	     if (model.numberRows() < 50)
	          model.messageHandler()->setLogLevel( 10);
	     //model.dual();
		model.primal();
		
		
	     if (model.numberRows() < 50)
	          model.messageHandler()->setLogLevel(10);
	     //model.dual();
		model.primal();
		for(i = 0; i < numYvar + numVvar ; i++){
			std::cout <<   m_osinstanceSeparation->getVariableNames()[ i]   << " = " << model.getColSolution()[ i] << std::endl;
		}
		
		for(i = 0; i < rowKounter ; i++){
			std::cout <<   m_osinstanceSeparation->getConstraintNames()[ i]   << " = " << model.getRowPrice()[ i] << std::endl;
		}
		
				
		
		
		
		//exit( 1);
		
		//
		delete objcoeff;
		
	}catch (const ErrorClass& eclass) {

		throw ErrorClass(eclass.errormsg);

	}	
	
	return NULL;
}//end getSeparationInstance


std::string makeStringFromInt(std::string theString, int theInt){
	ostringstream outStr;
	outStr << theString;
	outStr << theInt;
	return outStr.str();
}//end makeStringFromInt


