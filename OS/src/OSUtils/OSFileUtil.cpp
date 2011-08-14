/* $Id$ */
/** @file FileUtil.cpp
 * 
 * @author  Robert Fourer, Horand Gassmann, Jun Ma, Kipp Martin, 
 *
 * \remarks
 * Copyright (C) 2005-2011, Robert Fourer, Horand Gassmann, Jun Ma, Kipp Martin,
 * Dalhousie University, Northwestern University, and the University of Chicago.
 * All Rights Reserved.
 * This software is licensed under the Eclipse Public License. 
 * Please see the accompanying LICENSE file in root directory for terms.
 * 
 * <p>The <code>FileUtil</code> class contains methods for reading
 * and writing files from strings used by many classes in the
 * Optimization Services (OS) framework. </p>
 */

#include <cstring>
#include "OSFileUtil.h"
#include "OSErrorClass.h"
using std::cout;
using std::endl;


FileUtil::FileUtil(){
} // end constructor


FileUtil::~FileUtil(){  
} // end destructor

std::string FileUtil::getFileAsString( const char* fname){
	try{
		std::ostringstream outStr;
		std::ostringstream fileName;
		fileName << fname << std::endl;
		std::string soutString;
		char ch;
		//std::cout << "Inside FileUtil:getFileAsString and calling inFile " << fname << std::endl;
		std::ifstream inFile( fname);
		if( !inFile){
			throw ErrorClass(" Could not read the given file: " + fileName.str());
		}
		//std::cout << "Inside FileUtil:getFileAsString, file read put into ostringstream" << std::endl;
		//while((ch = inFile.get() ) != EOF){
		//	outStr << ch;
		//	std::cout << ch ;
		//}
		
		while( inFile.get( ch ) ){
			outStr << ch;
			//std::cout << ch ;
		}
		
		if( !inFile.eof() ){
			throw ErrorClass(" There was a problem reading the file: " + fileName.str() );
		}
		//std::cout << std::endl;
		//std::cout << "Inside FileUtil:getFileAsString, convert to a string" << std::endl;
		soutString = outStr.str();
		inFile.close();
		return soutString;
	}
	catch(const ErrorClass& eclass){
		throw ErrorClass( eclass.errormsg) ;
	}
} // end getFileAsString



char* FileUtil::getFileAsChar(const  char* fname){
	try{
	
		std::ostringstream fileName;
		fileName << fname << std::endl;
		std::filebuf *pbuf;
		long bufsize = 0;
		char *xml;
		char ch;
		std::ifstream inFile;
		std::cout << fname << std::endl;
		inFile.open( fname);
		if(!inFile){
			throw ErrorClass(" Could not read the given file: " + fileName.str() );
		}
		// get the input file stream into the buffer
		pbuf = inFile.rdbuf();
		// now get the size
		bufsize = pbuf->pubseekoff(0,std::ios_base::end);
		// set back to zero
		pbuf ->pubseekpos(0, std::ios::in);
		// allocate the character array
		xml = new char[bufsize + 1];
		xml[ bufsize] =  '\0';
		bufsize = 0;
		while( inFile.get( ch ) ){
			xml[ bufsize] = ch;
			bufsize++;
		}
		
		if( !inFile.eof() ){
			throw ErrorClass(" There was a problem reading the file: " + fileName.str());
		}
		//while((ch = inFile.get()) != EOF ){
		//	xml[ bufsize] = ch;
		//	bufsize++;
		//}
		return xml;
	}
	catch(const ErrorClass& eclass){
		throw ErrorClass( eclass.errormsg) ;
	}
} // end getFileAsChar





bool FileUtil::writeFileFromString(char* fname, std::string sname){
	//std::ofstream outFile;
	//std::fstream outFile;
	//outFile.open( fname);
	//if(!outFile.is_open()){
	//	return false;
	//}
	//outFile << sname;
	//outFile.close();
	//return true;
	
	std::ostringstream fileName;
	fileName << fname << std::endl;
	FILE *ft ;
	try{
		ft = fopen ( fname, "w") ;
	    if ( ft == NULL ) {
	    	throw ErrorClass(" There was a problem opening the file: " + fileName.str());
	    }
		char *cstr;
		cstr = new char [sname.size() + 1];
		strcpy (cstr, sname.c_str());
	    size_t i;
	    for(i = 0; i < sname.size() + 1; i++){
	    	if(cstr[ i] != '\0')     fputc ( cstr[ i], ft )  ;   	
	    }
//	    fputc( '\n', ft);
	    fclose ( ft);
		 delete[] cstr;
	    cstr = NULL;
		return true;
	}
	catch(const ErrorClass& eclass){
		throw ErrorClass( eclass.errormsg) ;
	}
} // end writeFileFromString


bool FileUtil::writeFileFromString(std::string  fname, std::string sname){

	std::ostringstream fileName;
	fileName << fname << std::endl;	
	FILE *ft ;
	try{
		ft = fopen ( fname.c_str(), "w") ;
	    if ( ft == NULL ) {
	    	throw ErrorClass(" There was a problem opening the file: " + fileName.str());
	    }
		char *cstr;
		cstr = new char [sname.size() + 1];
		strcpy (cstr, sname.c_str());
	    size_t i;
	    for(i = 0; i < sname.size() + 1; i++){
	    	if(cstr[ i] != '\0')     fputc ( cstr[ i], ft )  ;   	
	    }
//	    fputc( '\n', ft);
	    fclose ( ft);
		 delete[] cstr;
	    cstr = NULL;
		return true;
	}
	catch(const ErrorClass& eclass){
		throw ErrorClass( eclass.errormsg) ;
	}
} // end writeFileFromString

bool FileUtil::writeFileFromChar(char* fname, char* ch){
	//std::ofstream outFile;
	std::fstream outFile;
	outFile.open( fname);
	if(!outFile.is_open()){
		return false;
	}
	outFile << *ch;
	outFile.close();
	return true;
} // end writeFileFromChar
 
