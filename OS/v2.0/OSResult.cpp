/** @file OSResult.cpp
 * 
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

#include "OSResult.h"
#include "OSParameters.h"
#include "OSErrorClass.h"


#include "OSDataStructures.h"
#include "OSParameters.h" 
#include "OSCommonUtil.h"
#include "OSMathUtil.h"

#include<iostream>
#include<sstream>
//#define DEBUG

using namespace std;

GeneralStatus::GeneralStatus():
	type( ""),
	description( "")
{    
	#ifdef DEBUG
	cout << "Inside GeneralStatus Constructor" << endl;
	#endif
}// end GeneralStatus constructor  

GeneralStatus::~GeneralStatus(){
	#ifdef DEBUG
	cout << "GeneralStatus Destructor Called" << endl;
	#endif
}//end GeneralStatus destructor

OSResult::OSResult():
	m_iVariableNumber( -1),
	m_iObjectiveNumber( -1),
	m_iConstraintNumber( -1),
	m_iNumberOfOtherVariableResults( -1),
	m_mdPrimalValues( NULL),
	m_mdDualValues( NULL)

{    
	#ifdef DEBUG
	cout << "Inside OSResult Constructor" << endl;
	#endif
	this->general = new GeneralResult();
	this->system = new SystemResult();
	this->service = new SeviceResult();
	this->job = new JobResult();
	this->optimization = new OptimizationResult();
}// end OSResult constructor  

OSResult::~OSResult(){
	#ifdef DEBUG
	cout << "OSResult Destructor Called" << endl;
	#endif
	// delete the children of OSResult
	// delete general object
	if(general != NULL) delete general;
	general = NULL;
	//delete system object
	if(system != NULL)  delete system;
	system = NULL;
	//delete service object
	if(service != NULL)  delete service;
	service = NULL;
	//delete job object
	if(job != NULL)  delete job;
	job = NULL;
	//delete optimization object
	if(optimization != NULL)  delete optimization;
	optimization = NULL;

	if(m_mdPrimalValues != NULL) delete[] m_mdPrimalValues;
	m_mdPrimalValues = 0;
	if(m_mdDualValues != NULL) delete[] m_mdDualValues;
	m_mdDualValues = 0;
}//end OSResult destructor


GeneralResult::GeneralResult():
	serviceURI(""),
	serviceName(""),
	instanceName(""),
	jobID(""),
	time("0.0"),
	message("")
{ 
	#ifdef DEBUG
	cout << "Inside the GeneralResult Constructor" << endl;
	#endif
	generalStatus = new GeneralStatus();
}//end GeneralResult constructor


GeneralResult::~GeneralResult(){
	#ifdef DEBUG  
	cout << "Inside the GeneralResult Destructor" << endl;
	#endif
	delete generalStatus;
	generalStatus = NULL;
}// end GeneralResult destructor


SystemResult::SystemResult():
{ 
	#ifdef DEBUG
	cout << "Inside the SystemResult Constructor" << endl;
	#endif
}//end SystemResult constructor


SystemResult::~SystemResult(){
	#ifdef DEBUG  
	cout << "Inside the SystemResult Destructor" << endl;
	#endif
}// end SystemResult destructor



ServiceResult::ServiceResult():
{ 
	#ifdef DEBUG
	cout << "Inside the ServiceResult Constructor" << endl;
	#endif
}//end ServiceResult constructor


ServiceResult::~ServiceResult(){
	#ifdef DEBUG  
	cout << "Inside the ServiceResult Destructor" << endl;
	#endif
}// end ServiceResult destructor


JobResult::JobResult():
{ 
	#ifdef DEBUG
	cout << "Inside the JobResult Constructor" << endl;
	#endif
}//end JobResult constructor


JobResult::~JobResult(){
	#ifdef DEBUG  
	cout << "Inside the JobResult Destructor" << endl;
	#endif
}// end JobResult destructor

VarValue::VarValue():
	idx( -1),
	value( 0)
{ 
	#ifdef DEBUG
	cout << "Inside the VarValue Constructor" << endl;
	#endif
}//end VarValue constructor


VarValue::~VarValue(){
	#ifdef DEBUG  
	cout << "Inside the VarValue Destructor" << endl;
	#endif
}// end VarValue destructor 


OtherVarResult::OtherVarResult():
	idx( -1),
	value( "")
{ 
	#ifdef DEBUG
	cout << "Inside the OtherVarResult Constructor" << endl;
	#endif
}//end OtherVarResult constructor


OtherVarResult::~OtherVarResult(){
	#ifdef DEBUG  
	cout << "Inside the OtherVarResult Destructor" << endl;
	#endif
}// end OtherVarResult destructor 


OtherObjResult::OtherObjResult():
	idx( -1),
	value( "")
{ 
	#ifdef DEBUG
	cout << "Inside the OtherObjResult Constructor" << endl;
	#endif
}//end OtherObjResult constructor


OtherObjResult::~OtherObjResult(){
	#ifdef DEBUG  
	cout << "Inside the OtherObjResult Destructor" << endl;
	#endif
}//end OtherObjResult destructor


OtherConResult::OtherConResult():
	idx( -1),
	value( "")
{ 
	#ifdef DEBUG
	cout << "Inside the OtherConResult Constructor" << endl;
	#endif
}//end OtherConResult constructor


OtherConResult::~OtherConResult(){
	#ifdef DEBUG  
	cout << "Inside the OtherConResult Destructor" << endl;
	#endif
}//end OtherConResult destructor


ObjValue::ObjValue():
	idx( -1),
	value( OSNAN)
{ 
	#ifdef DEBUG
	cout << "Inside the ObjValue Constructor" << endl;
	#endif
}//end ObjValue constructor

ObjValue::~ObjValue(){
	#ifdef DEBUG  
	cout << "Inside the ObjValue Destructor" << endl;
	#endif
}// end ObjValue destructor 

DualVarValue::DualVarValue():
	idx( -1),
	lbValue( 0),
	ubValue( 0),
	value( 0)
{ 
	#ifdef DEBUG
	cout << "Inside the DualVarValue Constructor" << endl;
	#endif
}//end DualVarValue constructor


DualVarValue::~DualVarValue(){
	#ifdef DEBUG  
	cout << "Inside the DualVarValue Destructor" << endl;
	#endif
}// end DualVarValue destructor



VariableValues::VariableValues() { 
	#ifdef DEBUG
	cout << "Inside the VariableValues Constructor" << endl;
	#endif

}//end VariableValues constructor


VariableValues::~VariableValues(){
	#ifdef DEBUG  
	cout << "Inside the VariableValues Destructor" << endl;
	#endif
	int n = var.size();
	if(n > 0 ){
		for(int i = 0; i < n; i++){
			delete var[i];
			var[i] = NULL;
		}
	}
	var.clear(); 

}// end VariableValues destructor 


OtherVariableResult::OtherVariableResult():
	name(""),
	description("")
{ 
	#ifdef DEBUG
	cout << "Inside the OtherVariableResult Constructor" << endl;
	#endif

}//end OtherVariableResult constructor


OtherVariableResult::~OtherVariableResult(){
	#ifdef DEBUG  
	cout << "Inside the OtherVariableResult Destructor" << endl;
	#endif
	int n = var.size();
	if(n  > 0) {
		for(int i = 0; i < n; i++){
			delete var[i];
			var[i] = NULL;
		}
	}
	var.clear(); 
}// end OtherVariableResult destructor 


OtherObjectiveResult::OtherObjectiveResult():
	name(""),
	description("")
{ 
	#ifdef DEBUG
	cout << "Inside the OtherObjectiveResult Constructor" << endl;
	#endif
}//end OtherObjectiveResult constructor


OtherObjectiveResult::~OtherObjectiveResult(){
	#ifdef DEBUG  
	cout << "Inside the OtherObjectiveResult Destructor" << endl;
	#endif
	int n = obj.size();
	if(n > 0) {
		for(int i = 0; i < n; i++){
			delete obj[i];
			obj[i] = NULL;
		}
	}
	obj.clear(); 
}// end OtherObjectiveResult destructor


OtherConstraintResult::OtherConstraintResult():
	name(""),
	description("")
{ 
	#ifdef DEBUG
	cout << "Inside the OtherConstraintResult Constructor" << endl;
	#endif
}//end OtherConstraintResult constructor


OtherConstraintResult::~OtherConstraintResult(){
	#ifdef DEBUG  
	cout << "Inside the OtherConstraintResult Destructor" << endl;
	#endif
	int n = con.size();
	if(n > 0){
		for(int i = 0; i < n; i++){
			delete con[i];
			con[i] = NULL;
		}
	}
	con.clear(); 
}// end OtherObjectiveResult destructor


ObjectiveValues::ObjectiveValues() { 
	#ifdef DEBUG
	cout << "Inside the ObjectiveValues Constructor" << endl;
	#endif

}//end ObjectiveValues constructor


ObjectiveValues::~ObjectiveValues(){
	#ifdef DEBUG  
	cout << "Inside the ObjectiveValues Destructor" << endl;
	#endif
	int n = obj.size();
	if(n > 0 ){
		for(int i = 0; i < n; i++){
			delete obj[i];
			obj[i] = NULL;
		}
	}
	obj.clear(); 

}// end ObjectiveValues destructor



DualVariableValues::DualVariableValues() { 
	#ifdef DEBUG
	cout << "Inside the DualVariableValues Constructor" << endl;
	#endif

}//end DualVariableValues constructor


DualVariableValues::~DualVariableValues(){
	#ifdef DEBUG  
	cout << "Inside the DualVariableValues Destructor" << endl;
	#endif
	int n = con.size();
	if(n > 0){
		for(int i = 0; i < n; i++){
			delete con[i];
			con[i] = NULL;
		}
	}	
	con.clear(); 
}// end DualVariableValues destructor 



VariableSolution::VariableSolution():
	numberOfOtherVariableResults( 0),
	values( NULL),
	other( NULL)

{ 
	#ifdef DEBUG
	cout << "Inside the VariableSolution Constructor" << endl;
	#endif
}//end VariableSolution constructor


VariableSolution::~VariableSolution(){
	#ifdef DEBUG  
	cout << "Inside the VariableSolution Destructor" << endl;
	#endif
	delete  values;
	values = NULL;
	if(numberOfOtherVariableResults > 0 && other != NULL){
		for(int i = 0; i < numberOfOtherVariableResults; i++){
			delete other[i];
			other[i] = NULL;
		}
	}
	delete[] other;
	other = NULL; 
}// end VariableSolution destructor 

ObjectiveSolution::ObjectiveSolution():
	numberOfOtherObjectiveResults( 0),
	values( NULL),
	other( NULL)

{ 
	#ifdef DEBUG
	cout << "Inside the ObjectiveSolution Constructor" << endl;
	#endif
}//end ObjectiveSolution constructor


ObjectiveSolution::~ObjectiveSolution(){
	#ifdef DEBUG  
	cout << "Inside the ObjectieSolution Destructor" << endl;
	#endif
	delete  values;
	values = NULL;
	if(numberOfOtherObjectiveResults > 0 && other != NULL){
		for(int i = 0; i < numberOfOtherObjectiveResults; i++){
			delete other[i];
			other[i] = NULL;
		}
	}
	delete[] other;
	other = NULL; 
}// end ObjectiveSolution destructor 



ConstraintSolution::ConstraintSolution():
	numberOfOtherConstraintResults( 0),
	dualValues( NULL),
	other( NULL)
{ 
	#ifdef DEBUG
	cout << "Inside the ConstraintSolution Constructor" << endl;
	#endif
}//end ConstraintSolution constructor


ConstraintSolution::~ConstraintSolution(){
	#ifdef DEBUG  
	cout << "Inside the ConstraintSolution Destructor" << endl;
	#endif
	delete  dualValues;
	dualValues = NULL;
	if(numberOfOtherConstraintResults > 0 && other != NULL){
		for(int i = 0; i < numberOfOtherConstraintResults; i++){
			delete other[i];
			other[i] = NULL;
		}
	}
	delete[] other;
	other = NULL; 
}// end ConstraintSolution destructor



OptimizationSolutionStatus::OptimizationSolutionStatus():
	type(""),
	description("")
	//substatus( NULL),
{ 
	#ifdef DEBUG
	cout << "Inside the OptimizationSolutionStatus Constructor" << endl;
	#endif
}//end OptimizationSolutionStatus constructor


OptimizationSolutionStatus::~OptimizationSolutionStatus(){
	#ifdef DEBUG  
	cout << "Inside the OptimzationSolutionStatus Destructor" << endl;
	#endif
}// end OptimizationSolutionStatus destructor 


OptimizationSolution::OptimizationSolution():
	objectiveIdx( -1),
	message( ""),
	variables( NULL),
	objectives( NULL),
	constraints( NULL)
	//other(NULL)
{ 
	#ifdef DEBUG
	cout << "Inside the OptimizationSolution Constructor" << endl;
	#endif
	status = new OptimizationSolutionStatus();
}//end OptimizationSolution constructor


OptimizationSolution::~OptimizationSolution(){
	#ifdef DEBUG  
	cout << "Inside the OptimzationSolution Destructor" << endl;
	#endif
	delete variables;
	variables = NULL;
	delete constraints;
	constraints = NULL;
	delete objectives;
	objectives = NULL;
	delete status;
	status = NULL;
}// end OptimizationSolution destructor 
 
 
 
OptimizationResult::OptimizationResult():
	numberOfVariables( -1),
	numberOfObjectives( -1),
	numberOfConstraints(-1),
	numberOfSolutions( -1),
	solution(NULL)
{ 
	#ifdef DEBUG
	cout << "Inside the OptimizationResult Constructor" << endl;
	#endif
}//end OptimizationResult constructor


OptimizationResult::~OptimizationResult(){
	#ifdef DEBUG  
	cout << "Inside the OptimzationResult Destructor" << endl;
	cout << "Number of solutions = " << this->numberOfSolutions << endl;
	#endif
	if( solution != NULL){
		for(int i = 0; i < this->numberOfSolutions; i++){
			delete solution[i];
			solution[i] = NULL;
			#ifdef DEBUG  
			cout << "Delelting Solution: " << i << endl;
			#endif
			i++;
		}
	}
	delete[] solution;
	solution = NULL; 
}// end OptimizationResult destructor 
 


//
// get methods

GeneralStatus* OSResult::getGeneralStatus(){
	return general->generalStatus;
}//getGeneralStatus


string OSResult::getGeneralStatusType(){
	if(general->generalStatus == NULL) return "";
	return general->generalStatus->type;
}//getGeneralStatusType

string OSResult::getGeneralStatusDescription(){
	if(general->generalStatus == NULL) return NULL;
	return general->generalStatus->description;
}//getGeneralStatusDescription

string OSResult::getServiceName(){
	return general->serviceName;
}//getServiceName

string OSResult::getServiceURI(){
	return general->serviceURI;
}//getServiceURI

string OSResult::getInstanceName(){
	return general->instanceName;
}//getInstanceName

string OSResult::getJobID(){
	return general->jobID;
}//getJobID


string OSResult::getGeneralMessage(){
	return general->message;
}//getGeneralMessage

int OSResult::getVariableNumber(){
	if(m_iVariableNumber == -1){
		if(optimization == NULL) return -1;
		m_iVariableNumber = optimization->numberOfVariables;
	}
	return m_iVariableNumber;
}//getVariableNumber



int OSResult::getObjectiveNumber(){
	if(m_iObjectiveNumber == -1){
		if(optimization == NULL) return -1;
		m_iObjectiveNumber = optimization->numberOfObjectives;
	}
	return m_iObjectiveNumber;
}//getObjectiveNumber


int OSResult::getConstraintNumber(){
	if(m_iConstraintNumber == -1){
		if(optimization == NULL) return -1;
		m_iConstraintNumber = optimization->numberOfConstraints;
	}
	return m_iConstraintNumber;
}//getConstraintNumber

int OSResult::getSolutionNumber(){
	if(optimization == NULL) return 0;
	if(optimization->solution == NULL) return 0;
	return optimization->numberOfSolutions;
}//getSolutionNumber


int OSResult::getNumberOfOtherVariableResults(int solIdx){
	if(m_iNumberOfOtherVariableResults == -1){
		if(optimization->solution[solIdx]->variables->other == NULL) return -1;
		m_iNumberOfOtherVariableResults = optimization->solution[solIdx]->variables->numberOfOtherVariableResults;
	}
	return m_iNumberOfOtherVariableResults;
}//getNumberOfOtherVariableResults


OptimizationSolutionStatus* OSResult::getSolutionStatus( int solIdx){
	if(optimization == NULL || optimization->solution == NULL) return NULL;
	if( optimization->numberOfSolutions <= 0 || 
	   solIdx < 0 || solIdx >=  optimization->numberOfSolutions) return NULL;
	if(optimization->solution[solIdx] == NULL) return NULL;
	return optimization->solution[solIdx]->status;
}//getSolutionStatus

string OSResult::getSolutionStatusType(int solIdx){
	if(optimization == NULL || optimization->solution == NULL) return NULL;
	if( optimization->numberOfSolutions <= 0 || 
	   solIdx < 0 || solIdx >=  optimization->numberOfSolutions) return NULL;
	if(optimization->solution[solIdx] == NULL) return NULL;
	if(optimization->solution[solIdx]->status == NULL) return NULL;
	return optimization->solution[solIdx]->status->type;		
}//getSolutionStatusType

string OSResult::getSolutionStatusDescription(int solIdx){
	if(optimization == NULL || optimization->solution == NULL) return NULL;
	if( optimization->numberOfSolutions <= 0 || 
	   solIdx < 0 || solIdx >=  optimization->numberOfSolutions) return NULL;
	if(optimization->solution[solIdx] == NULL) return NULL;
	if(optimization->solution[solIdx]->status == NULL) return NULL;
	return optimization->solution[solIdx]->status->description;		
}//getSolutionStatusDescription

string OSResult::getSolutionMessage(int solIdx){
	if(optimization == NULL) return "there is no solution";
	if(optimization->solution == NULL || 
	   solIdx < 0 || solIdx >=  optimization->numberOfSolutions) return "there is no solution";
	if(optimization->solution[solIdx] == NULL) return "there is no solution";
	return optimization->solution[solIdx]->message;
}//getSolutionMessage

double* OSResult::getOptimalPrimalVariableValues(int objIdx){
	if(optimization == NULL || optimization->solution == NULL) return NULL;
	int iNumberOfVariables = this->getVariableNumber();
	if(iNumberOfVariables <= 0) return NULL;
	int iSolutions = this->getSolutionNumber();
	for(int i = 0; i < iSolutions; i++){
		if(optimization->solution[i] == NULL) continue;
		if(optimization->solution[i]->objectiveIdx != objIdx) continue;
		if(optimization->solution[i]->variables == NULL) continue;
		if(optimization->solution[i]->variables->values == NULL) continue;
		if((optimization->solution[i]->status->type.find("ptimal") != string::npos && m_mdPrimalValues == NULL) ||
			optimization->solution[i]->status->type.compare("globallyOptimal") == 0){				
			m_mdPrimalValues = new double[iNumberOfVariables];
			for(int j = 0; j < iNumberOfVariables; j++){
				m_mdPrimalValues[ j] = optimization->solution[i]->variables->values->var[j]->value;
			}
		}	
		if(optimization->solution[i]->status->type.compare("globallyOptimal") == 0){
			return m_mdPrimalValues;
		}
	}
	return m_mdPrimalValues;		
}//getOptimalPrimalVariableValues


double* OSResult::getOptimalDualVariableValues(int objIdx){
	if(optimization == NULL || optimization->solution == NULL) return NULL;
	int iNumberOfConstraints = this->getConstraintNumber();
	if(iNumberOfConstraints <= 0) return NULL;
	int iSolutions = this->getSolutionNumber();
	for(int i = 0; i < iSolutions; i++){
		if(optimization->solution[i] == NULL) continue;
		if(optimization->solution[i]->objectiveIdx != objIdx) continue;
		if(optimization->solution[i]->constraints == NULL) continue;
		if(optimization->solution[i]->constraints->dualValues == NULL) continue;
		if((optimization->solution[i]->status->type.find("ptimal") != string::npos && m_mdDualValues == NULL) ||
			optimization->solution[i]->status->type.compare("globallyOptimal") == 0){						
			m_mdDualValues = new double[iNumberOfConstraints];
			for(int j = 0; j < iNumberOfConstraints; j++){
				m_mdDualValues[ j] = optimization->solution[i]->constraints->dualValues->con[j]->value;
			}
		}	
		if(optimization->solution[i]->status->type.compare("globallyOptimal")){
			return m_mdDualValues;
		}
	}
	return m_mdDualValues;		
}//getOptimalDualVariableValues

// set methods

bool OSResult::setGeneralStatus(GeneralStatus *status){
	general->generalStatus = status;
	return true;
}//setGeneralStatus

bool OSResult::setGeneralStatusType(string type){
	if(general->generalStatus == NULL) general->generalStatus = new GeneralStatus();
	general->generalStatus->type = type;
	return true;
}//setGeneralStatusType

bool OSResult::setGeneralStatusDescription(string description){
	if(general->generalStatus == NULL) general->generalStatus = new GeneralStatus();
	general->generalStatus->description = description;
	return true;
}//setGeneralStatusDescription



bool OSResult::setServiceName(string serviceName){
	general->serviceName = serviceName;
	return true;
}//setServiceName

bool OSResult::setServiceURI(string serviceURI){
	general->serviceURI = serviceURI;
	return true;
}//setServiceURI

bool OSResult::setInstanceName(string instanceName){
	general->instanceName = instanceName;
	return true;
}//setInstanceName

bool OSResult::setJobID(string jobID){
	general->jobID = jobID;
	return true;
}//setJobID

bool OSResult::setTime(double time){
	general->time = os_dtoa_format( time);
	return true;
}//setTime

bool OSResult::setGeneralMessage(string message){
	general->message = message;
	return true;
}//setGeneralMessage

bool OSResult::setVariableNumber(int variableNumber){
	if(variableNumber <= 0){
		return false;
	}
	if(optimization == NULL) optimization = new OptimizationResult();
	optimization->numberOfVariables = variableNumber;
	return true;
}//setVariableNumber

bool OSResult::setObjectiveNumber(int objectiveNumber){
	if(objectiveNumber < 0){
		return false;
	}
	if(optimization == NULL) optimization = new OptimizationResult();
	optimization->numberOfObjectives = objectiveNumber;
	return true;
}//setObjectiveNumber

bool OSResult::setConstraintNumber(int constraintNumber){
	if(constraintNumber < 0){
		return false;
	}
	if(optimization == NULL) optimization = new OptimizationResult();
	optimization->numberOfConstraints = constraintNumber;
	return true;
}//setConstraintNumber

bool OSResult::setSolutionNumber(int number){
	//if(getVariableNumber() <= 0) return false;
	//if(getObjectiveNumber() < 0) return false;
	//if(getConstraintNumber() < 0) return false;
	if(number < 0) return false; 
	if(number == 0) return true;
	if(optimization == NULL) optimization = new OptimizationResult();
	optimization->numberOfSolutions = number;
	optimization->solution = new OptimizationSolution*[number];
	for(int i = 0; i < number; i++){
		std::cout << "CREATING A NEW OPTIMIZATION SOLUTION" << std::endl;
		optimization->solution[i] = new OptimizationSolution();
		std::cout << "DONE CREATING A NEW OPTIMIZATION SOLUTION" << std::endl;
	}
	return true;
}//setSolutionNumber

bool OSResult::setSolutionStatus(int solIdx, string type, string description){
 	int nSols = this->getSolutionNumber();
	if(optimization == NULL) return false;
	if(nSols <= 0) return false;
	if(optimization->solution == NULL || 
	   solIdx < 0 || solIdx >=  nSols) return false;
	if(optimization->solution[solIdx] == NULL){
		optimization->solution[solIdx] = new OptimizationSolution();
	}
	if(optimization->solution[solIdx]->status == NULL){
		optimization->solution[solIdx]->status = new OptimizationSolutionStatus();
	}
	// Kipp later when we finalize on type come back and require that the type be correct
	optimization->solution[solIdx]->status->type = type;
	optimization->solution[solIdx]->status->description = description;
	return true;
}//setSolutionStatus

bool OSResult::setSolutionObjectiveIndex(int solIdx, int objectiveIdx){
	int nSols = this->getSolutionNumber();
	if(optimization == NULL) return false;
	if(nSols <= 0) return false;
	if(optimization == NULL) return false;
	if(optimization->solution == NULL || 
	   solIdx < 0 || solIdx >=  nSols) return false;
	if(optimization->solution[solIdx] == NULL){
		optimization->solution[solIdx] = new OptimizationSolution();
	}
	if(objectiveIdx >= 0) return false;
	optimization->solution[solIdx]->objectiveIdx = objectiveIdx;
	return true;		
}//setSolutionObjectiveIndex

bool OSResult::setPrimalVariableValues(int solIdx, double *x){
	int iNumberOfVariables = this->getVariableNumber();
	if(iNumberOfVariables <= 0) return false;
	if(x == NULL) return false;
	int nSols = this->getSolutionNumber();
	if(optimization == NULL) return false;
	if(nSols <= 0) return false;
	if(optimization->solution == NULL || 
	   solIdx < 0 || solIdx >=  nSols) return false;
	if(optimization->solution[solIdx] == NULL){
		optimization->solution[solIdx] = new OptimizationSolution();
	}
	if(optimization->solution[solIdx]->variables == NULL){
		optimization->solution[solIdx]->variables = new VariableSolution();
	}
	if(optimization->solution[solIdx]->variables->values == NULL){
		optimization->solution[solIdx]->variables->values = new VariableValues();
	}
	if(x == NULL){
		//optimization->solution[solIdx]->variables->values->var = NULL;
		return true;
	}
	for(int i = 0; i < iNumberOfVariables; i++){
		optimization->solution[solIdx]->variables->values->var.push_back(new VarValue());
		optimization->solution[solIdx]->variables->values->var[i]->idx = i;
		optimization->solution[solIdx]->variables->values->var[i]->value = x[i];
	}
	return true;
}//setPrimalVariableValues


bool OSResult::setNumberOfOtherVariableResults(int solIdx, int numberOfOtherVariableResults){
	int iNumberOfVariables = this->getVariableNumber();
	if(iNumberOfVariables <= 0) return false;
	int nSols = this->getSolutionNumber();
	if(optimization == NULL) return false;
	if(nSols <= 0) return false;
	if(optimization == NULL) return false;
	if(optimization->solution == NULL || 
	   solIdx < 0 || solIdx >=  nSols) return false;
	if(optimization->solution[solIdx] == NULL){
		optimization->solution[solIdx] = new OptimizationSolution();
	}
	if(optimization->solution[solIdx]->variables == NULL){
		optimization->solution[solIdx]->variables = new VariableSolution();
	}
	optimization->solution[solIdx]->variables->numberOfOtherVariableResults =  numberOfOtherVariableResults;
	optimization->solution[solIdx]->variables->other = new OtherVariableResult*[ numberOfOtherVariableResults];
	for(int i = 0; i < numberOfOtherVariableResults; i++){
		optimization->solution[solIdx]->variables->other[ i]  = new OtherVariableResult();
	}
	return true;		
}//setNumberOfOtherVariableResults



bool OSResult::setAnOtherVariableResult(int solIdx, int otherIdx, string name, string description, string *s){
	int iNumberOfVariables = this->getVariableNumber();
	if(iNumberOfVariables <= 0) return false;
	int nSols = this->getSolutionNumber();
	if(optimization == NULL) return false;
	if(nSols <= 0) return false;
	if(optimization == NULL) return false;
	if(optimization->solution == NULL || 
	   solIdx < 0 || solIdx >=  nSols) return false;
	if(optimization->solution[solIdx] == NULL) return false;
	if(optimization->solution[solIdx]->variables == NULL)return false;
	if(optimization->solution[solIdx]->variables->other == NULL) return false;
	if(optimization->solution[solIdx]->variables->other[ otherIdx] == NULL) return false;
	optimization->solution[solIdx]->variables->other[ otherIdx]->name = name;
	optimization->solution[solIdx]->variables->other[ otherIdx]->description = description;
	for(int i = 0; i < iNumberOfVariables; i++){
		optimization->solution[solIdx]->variables->other[ otherIdx]->var.push_back(new OtherVarResult());
		optimization->solution[solIdx]->variables->other[ otherIdx]->var[i]->idx = i;
		optimization->solution[solIdx]->variables->other[ otherIdx]->var[i]->value = s[i];
	}
	return true;
}//setAnOtherVariableResult



bool OSResult::setObjectiveValues(int solIdx, double *objectiveValues){
	int iNumberOfObjectives = this->getObjectiveNumber();
	if(iNumberOfObjectives < 0) return false;
	if(iNumberOfObjectives == 0) return true;
	if(objectiveValues == NULL) return false;
	int nSols = this->getSolutionNumber();
	if(optimization == NULL) return false;
	if(nSols <= 0) return false;
	if(optimization == NULL) return false;
	if(optimization->solution == NULL || 
	   solIdx < 0 || solIdx >=  nSols) return false;
	if(optimization->solution[solIdx] == NULL){
		optimization->solution[solIdx] = new OptimizationSolution();
	}
	if(optimization->solution[solIdx]->objectives == NULL){
		optimization->solution[solIdx]->objectives = new ObjectiveSolution();
	}
	if(optimization->solution[solIdx]->objectives->values == NULL){
		optimization->solution[solIdx]->objectives->values = new ObjectiveValues();
	}
	for(int i = 0; i < iNumberOfObjectives; i++){
		optimization->solution[solIdx]->objectives->values->obj.push_back( new ObjValue());
		optimization->solution[solIdx]->objectives->values->obj[i]->idx = -(i+1);
		optimization->solution[solIdx]->objectives->values->obj[i]->value = objectiveValues[i];
	}
	return true;
}//setObjectiveValues

bool OSResult::setDualVariableValues(int solIdx, double *lbValues, double *ubValues){
	int iNumberOfConstraints = this->getConstraintNumber();
	if(iNumberOfConstraints < 0) return false;
	if(iNumberOfConstraints == 0) return true;
	int nSols = this->getSolutionNumber();
	if(optimization == NULL) return false;
	if(nSols <= 0) return false;
	if(optimization == NULL) return false;
	if(optimization->solution == NULL || 
	   solIdx < 0 || solIdx >=  nSols) return false;
	if(optimization->solution[solIdx] == NULL){
		optimization->solution[solIdx] = new OptimizationSolution();
	}
	if(optimization->solution[solIdx]->constraints == NULL){
		optimization->solution[solIdx]->constraints = new ConstraintSolution();
	}
	if(optimization->solution[solIdx]->constraints->dualValues == NULL){
		optimization->solution[solIdx]->constraints->dualValues = new DualVariableValues();
	}
	if(lbValues == NULL && ubValues == NULL){
		//optimization->solution[solIdx]->constraints->dualValues->con = NULL;
		return true;
	}
	int iCons = 0;
	if(lbValues == NULL){
		for(int i = 0; i < iNumberOfConstraints; i++){
			if(ubValues[i] != 0) iCons++;
		}
	}
	else if(ubValues == NULL){
		for(int i = 0; i < iNumberOfConstraints; i++){
			if(lbValues[i] != 0) iCons++;
		}
	}
	else{
		for(int i = 0; i < iNumberOfConstraints; i++){
			if(lbValues[i] != 0 || ubValues[i] != 0) iCons++;
		}
	}
	
	//optimization->solution[solIdx]->constraints->dualValues->con = new DualVarValue*[iCons];
	for(int i = 0; i < iCons; i++) optimization->solution[solIdx]->constraints->dualValues->con.push_back( new DualVarValue());
	int j = 0;
	if(lbValues == NULL){
		for(int i = 0; i < iNumberOfConstraints; i++){
			if(ubValues[i] != 0){
				optimization->solution[solIdx]->constraints->dualValues->con[j]->idx = i;
				optimization->solution[solIdx]->constraints->dualValues->con[j]->ubValue = 0;
				optimization->solution[solIdx]->constraints->dualValues->con[j]->ubValue = ubValues[i];
				j++;
			}
		}			
	}
	else if(ubValues == NULL){
		for(int i = 0; i < iNumberOfConstraints; i++){
			if(lbValues[i] != 0){
				optimization->solution[solIdx]->constraints->dualValues->con[j]->idx = i;
				optimization->solution[solIdx]->constraints->dualValues->con[j]->lbValue = lbValues[i];
				optimization->solution[solIdx]->constraints->dualValues->con[j]->ubValue = 0;
				j++;
			}
		}
	}
	else{
		for(int i = 0; i < iNumberOfConstraints; i++){
			if(lbValues[i] != 0 || ubValues[i] != 0){
				optimization->solution[solIdx]->constraints->dualValues->con[j]->idx = i;
				optimization->solution[solIdx]->constraints->dualValues->con[j]->lbValue = lbValues[i];
				optimization->solution[solIdx]->constraints->dualValues->con[j]->ubValue = ubValues[i];
				j++;
			}
		}
	}
	return true;
}//setDualVariableValues

bool OSResult::setDualVariableValues(int solIdx, double *y){
	int iNumberOfConstraints = this->getConstraintNumber();
	if(iNumberOfConstraints < 0) return false;
	if(iNumberOfConstraints == 0) return true;
	int nSols = this->getSolutionNumber();
	if(optimization == NULL) return false;
	if(nSols <= 0) return false;
	if(optimization == NULL) return false;
	if(optimization->solution == NULL || 
	   solIdx < 0 || solIdx >=  nSols) return false;
	if(optimization->solution[solIdx] == NULL){
		optimization->solution[solIdx] = new OptimizationSolution();
	}
	if(optimization->solution[solIdx]->constraints == NULL){
		optimization->solution[solIdx]->constraints = new ConstraintSolution();
	}
	if(optimization->solution[solIdx]->constraints->dualValues == NULL){
		optimization->solution[solIdx]->constraints->dualValues = new DualVariableValues();
	}
	if(y == NULL){
		optimization->solution[solIdx]->constraints->dualValues = NULL;
		return true;
	} 
	for(int i = 0; i < iNumberOfConstraints; i++){
		optimization->solution[solIdx]->constraints->dualValues->con.push_back( new DualVarValue());
		optimization->solution[solIdx]->constraints->dualValues->con[i]->idx = i;
		optimization->solution[solIdx]->constraints->dualValues->con[i]->value = y[i];
	}
	return true;
}//setDualVariableValues

bool OSResult::setConstraintValues(int solIdx, double *dualVarValues){
	int iNumberOfConstraints = this->getConstraintNumber();
	if(iNumberOfConstraints <= 0) return false;
	int nSols = this->getSolutionNumber();
	if(optimization == NULL) return false;
	if(nSols <= 0) return false;
	if(optimization == NULL) return false;
	if(optimization->solution == NULL || 
	   solIdx < 0 || solIdx >=  nSols) return false;
	if(optimization->solution[solIdx] == NULL){
		optimization->solution[solIdx] = new OptimizationSolution();
	}
	if(optimization->solution[solIdx]->constraints == NULL){
		optimization->solution[solIdx]->constraints = new ConstraintSolution();
	}
	if(optimization->solution[solIdx]->constraints->dualValues == NULL){
		optimization->solution[solIdx]->constraints->dualValues = new DualVariableValues();
	}
	if(dualVarValues == NULL){
		optimization->solution[solIdx]->constraints->dualValues = NULL;
		return true;
	}
	//optimization->solution[solIdx]->constraints->dualValues->con = new ConValue*[ iNumberOfConstraints];
	
	for(int i = 0; i < iNumberOfConstraints; i++){
		optimization->solution[solIdx]->constraints->dualValues->con.push_back( new DualVarValue());
		//optimization->solution[solIdx]->constraints->dualValues->con[i] = new ConValue();
		optimization->solution[solIdx]->constraints->dualValues->con[i]->idx = i;
		optimization->solution[solIdx]->constraints->dualValues->con[i]->value = dualVarValues[i];
	}
	return true;
}//setConstraintValues

