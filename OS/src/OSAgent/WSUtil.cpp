/** @file WSUtil.cpp
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
#include "WSUtil.h"
#include "OSConfig.h"
#ifdef WIN_
#define NOMINMAX
#include <winsock.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>   
#include <unistd.h>  
#include <netdb.h>  
#endif
#define DEBUG

#include <stdlib.h>    
#include <string.h>    


#include <iostream>
#include <sstream> 
#include "ErrorClass.h"
#include "OSResult.h"
#include "OSrLWriter.h"

using std::string;
using std::cout;
using std::endl;
using std::ostringstream;


WSUtil::WSUtil(){
}

WSUtil::~WSUtil(){
}



string WSUtil::sendSOAPMessage(string theSOAP, string serviceIP, unsigned int servicePortNumber)
{
	try{
		/* code taken from "TCP/IP Sockets in C" by Donahoo and Calvert */
		unsigned long ResolveName(char *name);
		ostringstream ret_message;
		int sock;                        /* Socket descriptor */
		struct sockaddr_in httpServAddr; /* Echo server address */
		unsigned short httpServPort = servicePortNumber;     /* Echo server port */
		char *servIP = &serviceIP[0];      /* Server IP address (dotted quad) */
		char httpBuffer[RCVBUFSIZE];     /* Buffer for http string */
		unsigned int httpStringLen;      /* Length of string to http */
		int bytesRcvd, totalBytesRcvd;   /* Bytes read in single recv() and total bytes read */
		char* message = &theSOAP[0];         /* Second arg: string to http */
		#ifdef WIN_
		WSADATA wsaData;
		if( WSAStartup(MAKEWORD(2, 0), &wsaData) != 0 ) throw ErrorClass( "WSAStartup failed");
		#endif
		/* Create a reliable, stream socket using TCP */
		if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) throw ErrorClass( "failure creating socket");
		/* Construct the server address structure */
		memset(&httpServAddr, 0, sizeof(httpServAddr));     /* Zero out structure */
		httpServAddr.sin_family      = AF_INET;             /* Internet address family */
		httpServAddr.sin_addr.s_addr = ResolveName( servIP);   /* Server IP address */
		httpServAddr.sin_port        = htons(httpServPort); /* Server port */
		/* Establish the connection to the http server */
		if (connect(sock, (struct sockaddr *) &httpServAddr, sizeof(httpServAddr)) < 0){
			string sipadd = &serviceIP[0];
			string errormsg = "failure connecting with remote socket at address: " + sipadd  ;
			throw ErrorClass( errormsg );
		}
		#ifdef DEBUG
		cout << "Connection Established"  << endl;
		#endif
		httpStringLen = strlen( message);   
		#ifdef DEBUG
		cout << "HERE IS WHAT WE SEND" << endl;
		cout << message << endl;
		#endif
		/* Send the string to the server */	
		if (send(sock, message, httpStringLen, 0) != httpStringLen)
			throw ErrorClass("send() sent a different number of bytes than expected");
		#ifdef DEBUG
		cout << "OSiL sent to server" << endl;
		#endif
		int recvMsgSize = 1;
		while (recvMsgSize > 0) {
			#ifdef DEBUG
			cout << "start to receive" << endl;
			#endif
			if ((recvMsgSize = recv(sock, httpBuffer, RCVBUFSIZE-1, 0)) < 0)
				throw ErrorClass( "socket error receiving data");
			#ifdef DEBUG
			cout << "Message size =  " << recvMsgSize << endl;
			cout << "Message = " << httpBuffer << endl;
			#endif
			httpBuffer[ recvMsgSize ] = '\0';
			ret_message << httpBuffer;
			if(recvMsgSize == 0 ) {
			//if(recvMsgSize == 0 || recvMsgSize < RCVBUFSIZE - 1) {
			#ifdef WIN_
			closesocket( sock);
			WSACleanup();
			#else
			close( sock);
			#endif
			//ret_message << endl;
			break;
			}
		}

		#ifdef WIN_
		closesocket( sock);
		WSACleanup();
		#else
		close( sock);
		#endif
		
		return ret_message.str();
	}
	catch(const ErrorClass& eclass){
		OSResult osresult;
		OSrLWriter osrlwriter;
		string osrl;
		osresult.setGeneralMessage( eclass.errormsg);
		osresult.setGeneralStatusType( "error");
		osrl = osrlwriter.writeOSrL( &osresult);
		throw ErrorClass( osrl);
	}
}

std::string WSUtil::createSOAPMessage(int numInputs,  string solverAddress, string postURI, string smethod, 
	string* msInputs, string* msInputNames, string sSoapAction){
	ostringstream request, body, msg;
	int i;
	string mynamespace = "xmlns:ns1=\"http://www.optimizationservices.org\"";
	#ifdef DEBUG
	cout << "Solver address = " <<  solverAddress << endl;
	cout << "SOAP action = " <<  sSoapAction << endl;
	cout << "postURI = " <<  postURI << endl;
	#endif
	request << "POST "  <<  postURI << " HTTP/1.0" <<  "\r";
	request << "Host: " ;
	request << solverAddress << endl;
	request << "Content-Type: text/xml; charset=UTF-8" << endl;
	request << "Connection: close" << endl;
	request << "Accept: application/soap+xml, application/dime, multipart/related, text/*" << endl;
	request << "Cache-Control: no-cache" << endl;
	request << "Pragma: no-cache" << endl;
	request << "SOAPAction: ";
	request << "\"" << sSoapAction << "\"" << endl;
	body << "<?xml version='1.0' encoding='utf-8' ?>" << endl;
	body << "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">" << endl;
	body << "<SOAP-ENV:Body>" << endl;
	body << "<ns1:" << smethod << " " << mynamespace << ">" << endl;
	for(i = 0; i < numInputs; i++){
		body << "<" << msInputNames[ i] << " xsi:type=\"xsd:string\"" << ">" ;
		body << msInputs[ i] ;
		body << "</" << msInputNames[ i] << ">" << endl;
	}
	body << "</ns1:" << smethod << ">" << endl;
	body << "</SOAP-ENV:Body>" << endl;
	body << "</SOAP-ENV:Envelope>" << endl;
	body << "\n";
	request << "Content-Length: " << body.str().length();
	request << endl << endl;
	request << body.str();
	return request.str();
}// end createSOAPMessage

std::string WSUtil::createFormDataUpload(std::string solverAddress, std::string postURI, 
		std::string fileName,  std::string theFile, std::string boundaryName){
	ostringstream request, body;
	std::cout << "Solver address = " <<  solverAddress << std::endl;
	std::cout << "postURI = " <<  postURI << std::endl;
	request << "POST "  <<  postURI << " HTTP/1.0" <<  "\r";
	request << "Host: " ;
	request << solverAddress << std::endl;
	request << "Content-Type: multipart/form-data; boundary=" ;
	request << boundaryName << std::endl;
	request << "Connection: keep-alive" << std::endl;
	//request << "Referer: /servlets-examples/fileupload.html" << endl;


	body << "--" ;
	body << boundaryName << std::endl ;
	body << "Content-Disposition: form-data; name=\"";
	body << "myfile";
	body << "\"";
	body << ";";
	body << "filename=\"";
	body << fileName;
	body << "\"" << endl;
	
	body << "Content-Type: text/plain" << std::endl << std::endl;
	body << theFile ;
	body << "--" ;
	body << boundaryName;
	body << "--" ;;
	body << "\n";
	
	
	request << "Content-Length: " << body.str().length();
	request << endl << endl;
	request << body.str();
	//return request.str();
	return theFile;
}// end createFromDataUpload

/*
   Content-Type: multipart/form-data; boundary=AaB03x


   --AaB03x
   Content-Disposition: form-data; name="files"; filename="file1.txt"
   Content-Type: text/plain

   ... contents of file1.txt ...
   --AaB03x--


Referer: http://localhost:8181/servlets-examples/fileupload.html
Content-Type: multipart/form-data; boundary=---------------------------10102754414578508781458777923
Content-Length: 1787

-----------------------------10102754414578508781458777923
Content-Disposition: form-data; name="myfile"; filename="parincLinear.osil"
Content-Type: application/octet-stream

<?xml version="1.0" encoding="UTF-8"?>
   <osil xmlns="os.optimizationservices.org" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"	xsi:schemaLocation="os.optimizationservices.org ../schemas/OSiL.xsd">	
			
            </value>	
         </linearConstraintCoefficients>	
      </instanceData>
   </osil>-----------------------------10102754414578508781458777923Content-Disposition: form-data; name="Submit"Submit your files-----------------------------10102754414578508781458777923--
   
   
 */

string WSUtil::SOAPify(string inputstring){
	/* replace all occurances of "<" with "&lt;"  all 
	occurances of ">" with "&gt;" and all occurances of " or ' with &quote;
	*/
	#ifdef DEBUG
	cout << "prepare the XML for a SOAP envelope" << endl;
	#endif
	ostringstream body;
	int i = 0;
	int loopsize = inputstring.length();
	while (i < loopsize){
		switch( inputstring[i]){
			case '<':
				body << "&lt;";
				break;
			case '>':
				body << "&gt;";
				break;
			case '\"':
				body << "&quot;";
				break;
			case '\'':
				body << "&quot;";
				break;
			default:
			body << inputstring[i];
		}
		i++;
	}
	return body.str();
}

string WSUtil::deSOAPify(string inputstring){
	/* replace all occurances of "&lt;" with "<"  all 
	occurances of "&gt;" with ">" and all occurances of "&quote;" with "
	*/
	ostringstream body;
	int i = 0;
	int loopsize = inputstring.length();
	while (i < loopsize){
		//i = inputstring.find('&', i);
		//if (i == string::npos)  return ostringstream.str();
		if(inputstring[i] == '&'){
			switch (inputstring[i+1]) {
			case 'l':
				if (inputstring[i + 2] == 't' && inputstring[i + 3] == ';') {
					body << "<";
				}
				i = i + 4;
				break;
			case 'g':
				if (inputstring[i + 2] == 't' && inputstring[i + 3] == ';') {
					body << ">";
				}
				i = i + 4;
				break;
			case 'q':
				if (inputstring[i + 2] == 'u' && inputstring[i + 3] == 'o' && inputstring[i + 4] == 't' && inputstring[i + 5] == ';') {
					body << "\"";
				}
				i = i + 6;
				break;
			default:
				body << inputstring[i];
				i++;
				break;
			} //end switch
		} // end if'&"
		else{
			body << inputstring[i];
			i++;
		}
	}// end while
	return body.str();
}

unsigned long ResolveName(char *name){
	struct hostent *host;
	try{
		if ((host = gethostbyname(name)) == NULL){
			string s1 = &name[0];
			string errormsg = "cannot resolve the domain name:  " + s1;
			throw ErrorClass( errormsg);
		}
		return *((unsigned long *) host->h_addr_list[0]);
	}
	catch(const ErrorClass& eclass){
		throw eclass;
	}
}

//void DieWithError(char *errorMessage)
//{
//	#ifdef DEBUG
//    cout << errorMessage << endl;
//	#endif
//    exit(1);
//}

string WSUtil::getOSxL(string soapstring, string serviceMethod){
	/*  get the string that starts with <osxl
	 * inside the soap envelope
	 */
	string start = "";
	string end = "";
	string result = "";
	start = "<" + serviceMethod + "Return";
	end = "</"+serviceMethod + "Return";
	// strip off the return header information 
	// find start of XML information
	int startxml = soapstring.find(start , 1);
	if (startxml == string::npos){
		return result;	
	}    
	else{
		startxml = soapstring.find(">", startxml + 1);
		if(startxml == string::npos) return result;
		// find the end of the string
		int endxml = soapstring.find(end , startxml);
		if(endxml == string::npos){
			return result;
		}
		else{
			// get the > that ends </osxl
			//endxml = soapstring.find(">", endxml - 1);
			// now get the substring
			startxml++;
			result = soapstring.substr(startxml, endxml - startxml);
			return result;
		}
	}
	return result;
}
