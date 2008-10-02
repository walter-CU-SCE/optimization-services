/** @file parseosol.y
 * 
 * @author  Gus Gassmann, Jun Ma, Kipp Martin, 
 * @version 1.1, 10/07/2008
 * @since   OS1.1
 *
 * \remarks
 * Copyright (C) 2005-2008, Robert Fourer, Gus Gassmann, Jun Ma, Kipp Martin,
 * Northwestern University, Dalhousie University, and the University of Chicago.
 * All Rights Reserved.
 * This software is licensed under the Common Public License. 
 * Please see the accompanying LICENSE file in root directory for terms.
 * 
 */

%{


 
#include "OSParameters.h"
#include "OSConfig.h"
#include "OSErrorClass.h"
#include "OSOption.h"
#include "OSoLParserData.h"
#include <iostream>
#include <sstream> 
#include <stdio.h>


typedef struct yy_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE osol_scan_string (const char *yy_str , void* yyscanner  );
int osollex_init(void** ptr_yy_globals);
int osollex_destroy (void* yyscanner );
int osolget_lineno( void* yyscanner);
char *osolget_text (void* yyscanner );
void osolset_lineno (int line_number , void* yyscanner );
void osolset_extra (OSoLParserData* parserData , void* yyscanner );
void  yygetOSOption(const char *ch, OSOption* m_osoption, OSoLParserData *m_parserData ) throw(ErrorClass);


%}


%pure-parser
%locations
%defines
%parse-param{OSOption *osoption}
%parse-param{OSoLParserData *parserData}
%lex-param {void* scanner}




%union {
	double dval;
	int ival;
	char* sval;
}

/* %name-prefix="osol" 
this fails in Mac OS X
*/

%{

void osolerror(YYLTYPE* type, OSOption *osoption,  OSoLParserData *parserData, const char* errormsg ) ;
int osollex(YYSTYPE* lvalp,  YYLTYPE* llocp, void* scanner);
 
#define scanner parserData->scanner
%}

%token <sval> ATTRIBUTETEXT
%token <sval> ELEMENTTEXT
%token <ival> INTEGER
%token <dval> DOUBLE
%token <sval> QUOTE


%token GREATERTHAN ENDOFELEMENT 
%token OSOLSTART OSOLATTRIBUTETEXT OSOLEND


%token LOCATIONTYPEATT TRANSPORTTYPEATT NUMBEROFOTHEROPTIONSATT;
%token NUMBEROFJOBIDSATT NUMBEROFPATHSATT NUMBEROFPATHPAIRSATT;
%token FROMATT TOATT MAKECOPYATT SOLVERATT CATEGORYATT TYPEATT;
%token NUMBEROFPROCESSESATT NUMBEROFSOLVEROPTIONSATT;
%token NUMBEROFVARIABLESATT NUMBEROFOBJECTIVESATT NUMBEROFCONSTRAINTSATT;
%token NUMBEROFOTHERVARIABLEOPTIONSATT NUMBEROFOTHEROBJECTIVEOPTIONSATT;
%token NUMBEROFOTHERCONSTRAINTOPTIONSATT;
%token NUMBEROFVARATT NUMBEROFOBJATT NUMBEROFCONATT;
%token NAMEATT IDXATT VALUEATT UNITATT DESCRIPTIONATT LBVALUEATT UBVALUEATT;

%token GENERALSTART GENERALEND SYSTEMSTART SYSTEMEND SERVICESTART SERVICEEND;
%token JOBSTART JOBEND OPTIMIZATIONSTART OPTIMIZATIONEND;
%token SERVICEURISTART SERVICEURIEND SERVICENAMESTART SERVICENAMEEND; 
%token INSTANCENAMESTART INSTANCENAMEEND INSTANCELOCATIONSTART INSTANCELOCATIONEND;
%token JOBIDSTART JOBIDEND SOLVERTOINVOKESTART SOLVERTOINVOKEEND;
%token LICENSESTART LICENSEEND USERNAMESTART USERNAMEEND PASSWORDSTART PASSWORDEND;
%token CONTACTSTART CONTACTEND OTHEROPTIONSSTART OTHEROPTIONSEND OTHERSTART OTHEREND;
%token MINDISKSPACESTART MINDISKSPACEEND MINMEMORYSIZESTART MINMEMORYSIZEEND MINCPUSPEEDSTART MINCPUSPEEDEND;
%token MINCPUNUMBERSTART MINCPUNUMBEREND SERVICETYPESTART SERVICETYPEEND;
%token MAXTIMESTART MAXTIMEEND SCHEDULEDSTARTTIMESTART SCHEDULEDSTARTTIMEEND; 
%token DEPENDENCIESSTART DEPENDENCIESEND;
%token REQUIREDDIRECTORIESSTART REQUIREDDIRECTORIESEND REQUIREDFILESSTART REQUIREDFILESEND; 
%token PATHSTART PATHEND PATHPAIRSTART PATHPAIREND;
%token DIRECTORIESTOMAKESTART DIRECTORIESTOMAKEEND FILESTOCREATESTART FILESTOCREATEEND;
%token DIRECTORIESTODELETESTART DIRECTORIESTODELETEEND FILESTODELETESTART FILESTODELETEEND;
%token  INPUTDIRECTORIESTOMOVESTART  INPUTDIRECTORIESTOMOVEEND  INPUTFILESTOMOVESTART  INPUTFILESTOMOVEEND;
%token OUTPUTDIRECTORIESTOMOVESTART OUTPUTDIRECTORIESTOMOVEEND OUTPUTFILESTOMOVESTART OUTPUTFILESTOMOVEEND;
%token PROCESSESTOKILLSTART PROCESSESTOKILLEND PROCESSSTART PROCESSEND;
%token VARIABLESSTART VARIABLESEND;
%token INITIALVARIABLEVALUESSTART INITIALVARIABLEVALUESEND VARSTART VAREND;
%token INITIALVARIABLEVALUESSTRINGSTART INITIALVARIABLEVALUESSTRINGEND;
%token OBJECTIVESSTART OBJECTIVESEND;
%token INITIALOBJECTIVEVALUESSTART INITIALOBJECTIVEVALUESEND OBJSTART OBJEND;
%token INITIALOBJECTIVEBOUNDSSTART INITIALOBJECTIVEBOUNDSEND;
%token CONSTRAINTSSTART CONSTRAINTSEND;
%token INITIALCONSTRAINTVALUESSTART INITIALCONSTRAINTVALUESEND CONSTART CONEND;
%token INITIALDUALVALUESSTART INITIALDUALVALUESEND;
%token SOLVEROPTIONSSTART SOLVEROPTIONSEND SOLVEROPTIONSTART SOLVEROPTIONEND;

%token DUMMY

%%


osoldoc: osolstart GREATERTHAN osolcontent OSOLEND;
	| osolstart ENDOFELEMENT;

osolstart:	OSOLSTART   
	| OSOLSTART OSOLATTRIBUTETEXT ;

osolcontent: osolgeneral osolsystem osolservice osoljob osoloptimization ;

osolgeneral: | generalhead generalbody;

generalhead: GENERALSTART 
{	if (parserData->osolgeneralPresent)
	{	osolerror( NULL, osoption, parserData, "only one <general> element allowed");
	}
	else
	{	parserData->osolgeneralPresent = true;	
		osoption->general = new GeneralOption();
	}
}; 

generalbody:
	  GREATERTHAN generalcontent GENERALEND
	| ENDOFELEMENT;

generalcontent: | generalcontent generaloption;

generaloption: serviceURI | servicename | instancename | instancelocation | jobid
 | solvertoinvoke | license | username | password | contact | othergeneraloptions;


serviceURI: serviceURIhead serviceURIbody;

serviceURIhead: SERVICEURISTART 
{	if (parserData->serviceURIPresent)
	{	osolerror( NULL, osoption, parserData, "only one <serviceURI> element allowed");
	}
	else
	{	parserData->serviceURIPresent = true;	
	}
}; 

serviceURIbody: ENDOFELEMENT
	| GREATERTHAN SERVICEURIEND
	| GREATERTHAN ELEMENTTEXT {osoption->general->serviceURI = $2;} SERVICEURIEND;


servicename: servicenamehead servicenamebody;

servicenamehead: SERVICENAMESTART 
{	if (parserData->serviceNamePresent)
	{	osolerror( NULL, osoption, parserData, "only one <serviceName> element allowed");
	}
	else
	{	parserData->serviceNamePresent = true;	
	}
}; 

servicenamebody: ENDOFELEMENT
	| GREATERTHAN SERVICENAMEEND
	| GREATERTHAN ELEMENTTEXT {osoption->general->serviceName = $2;} SERVICENAMEEND;


instancename: instancenamehead instancenamebody;

instancenamehead: INSTANCENAMESTART 
{	if (parserData->instanceNamePresent)
	{	osolerror( NULL, osoption, parserData, "only one <instanceName> element allowed");
	}
	else
	{	parserData->instanceNamePresent = true;	
	}
}; 

instancenamebody: ENDOFELEMENT
	| GREATERTHAN INSTANCENAMEEND
	| GREATERTHAN ELEMENTTEXT {osoption->general->instanceName = $2;} INSTANCENAMEEND;


instancelocation: instancelocationhead locationtypeatt instancelocationbody;

instancelocationhead: INSTANCELOCATIONSTART 
{	if (parserData->instanceLocationPresent)
	{	osolerror( NULL, osoption, parserData, "only one <instanceLocation> element allowed");
	}
	else
	{	parserData->instanceLocationPresent = true;
		osoption->general->instanceLocation = new InstanceLocationOption();
	}
}; 

locationtypeatt: | LOCATIONTYPEATT ATTRIBUTETEXT {osoption->general->instanceLocation->locationType = $2;} QUOTE;

instancelocationbody: ENDOFELEMENT
	|   GREATERTHAN instancelocationtext INSTANCELOCATIONEND;

instancelocationtext: | ELEMENTTEXT {osoption->general->instanceLocation->value = $1;};


jobid: jobidhead jobidbody;

jobidhead: JOBIDSTART 
{	if (parserData->jobIDPresent)
	{	osolerror( NULL, osoption, parserData, "only one <jobID> element allowed");
	}
	else
	{	parserData->jobIDPresent = true;	
	}
}; 

jobidbody: ENDOFELEMENT
	| GREATERTHAN JOBIDEND
	| GREATERTHAN ELEMENTTEXT {osoption->general->jobID = $2;} JOBIDEND;


solvertoinvoke: solvertoinvokehead solvertoinvokebody;

solvertoinvokehead: SOLVERTOINVOKESTART 
{	if (parserData->solverToInvokePresent)
	{	osolerror( NULL, osoption, parserData, "only one <solverToInvoke> element allowed");
	}
	else
	{	parserData->solverToInvokePresent = true;	
	}
}; 

solvertoinvokebody: ENDOFELEMENT
	| GREATERTHAN SOLVERTOINVOKEEND
	| GREATERTHAN ELEMENTTEXT {osoption->general->solverToInvoke = $2;} SOLVERTOINVOKEEND;


license: licensehead licensebody;

licensehead: LICENSESTART 
{	if (parserData->licensePresent)
	{	osolerror( NULL, osoption, parserData, "only one <license> element allowed");
	}
	else
	{	parserData->licensePresent = true;	
	}
}; 

licensebody: ENDOFELEMENT
	| GREATERTHAN LICENSEEND
	| GREATERTHAN ELEMENTTEXT {osoption->general->license = $2;} LICENSEEND;


username: usernamehead usernamebody;

usernamehead: USERNAMESTART 
{	if (parserData->usernamePresent)
	{	osolerror( NULL, osoption, parserData, "only one <userName> element allowed");
	}
	else
	{	parserData->usernamePresent = true;	
	}
}; 

usernamebody: ENDOFELEMENT
	| GREATERTHAN USERNAMEEND
	| GREATERTHAN ELEMENTTEXT {osoption->general->userName = $2;} USERNAMEEND;


password: passwordhead passwordbody;

passwordhead: PASSWORDSTART 
{	if (parserData->passwordPresent)
	{	osolerror( NULL, osoption, parserData, "only one <password> element allowed");
	}
	else
	{	parserData->passwordPresent = true;	
	}
}; 

passwordbody: ENDOFELEMENT
	| GREATERTHAN PASSWORDEND
	| GREATERTHAN ELEMENTTEXT {osoption->general->password = $2;} PASSWORDEND;


contact: contacthead transporttypeatt contactbody;

contacthead: CONTACTSTART 
{	if (parserData->contactPresent)
	{	osolerror( NULL, osoption, parserData, "only one <contact> element allowed");
	}
	else
	{	parserData->contactPresent = true;
		osoption->general->contact = new ContactOption();
	}
}; 

transporttypeatt: | TRANSPORTTYPEATT ATTRIBUTETEXT {osoption->general->contact->transportType = $2;} QUOTE;

contactbody: ENDOFELEMENT
	|   GREATERTHAN contacttext CONTACTEND;

contacttext: | ELEMENTTEXT {osoption->general->contact->value = $1;};


othergeneraloptions: othergeneraloptionshead numberofothergeneraloptions GREATERTHAN othergeneraloptionsbody;

othergeneraloptionshead: OTHEROPTIONSSTART 
{	if (parserData->otherGeneralOptionsPresent)
	{	osolerror( NULL, osoption, parserData, "only one <otherOptions> element allowed");
	}
	else
	{	parserData->otherGeneralOptionsPresent = true;
		osoption->general->otherOptions = new OtherOptions();	
	}
}; 

numberofothergeneraloptions: NUMBEROFOTHEROPTIONSATT QUOTE INTEGER QUOTE
{	osoption->general->otherOptions->numberOfOtherOptions = $3;
	osoption->general->otherOptions->other = new OtherOption*[$3];
	for (int i=0; i < $3; i++) osoption->general->otherOptions->other[i] = new OtherOption();
};

othergeneraloptionsbody: othergeneraloptionslist OTHEROPTIONSEND
{	if (parserData->numberOfOtherGeneralOptions != osoption->general->otherOptions->numberOfOtherOptions)
		osolerror (NULL, osoption, parserData, "wrong number of other options in <general> element"); 
};

othergeneraloptionslist: | othergeneraloptionslist othergeneraloption;

othergeneraloption: OTHERSTART 
	{	if (parserData->numberOfOtherGeneralOptions >= osoption->general->otherOptions->numberOfOtherOptions)
		{	osolerror (NULL, osoption, parserData, "too many other options in <general> element");
		};
	} 
    othergeneralattributes othergeneraloptionend
{	if (!parserData->otherOptionNamePresent)
		osolerror (NULL, osoption, parserData, "name attribute must be present");
	/* reset defaults for the next option */
	parserData->otherOptionNamePresent = false;
	parserData->otherOptionValuePresent = false;
	parserData->otherOptionDescriptionPresent = false;
	parserData->numberOfOtherGeneralOptions++;
};


othergeneralattributes: | othergeneralattributes othergeneralattribute;

othergeneralattribute: generaloptionnameatt | generaloptionvalueatt | generaloptiondescriptionatt;

generaloptionnameatt: NAMEATT ATTRIBUTETEXT 
{	if (parserData->otherOptionNamePresent)
	{	osolerror( NULL, osoption, parserData, "only one name attribute allowed");
	}
	else
	{	parserData->otherOptionNamePresent = true;
		osoption->general->otherOptions->other[parserData->numberOfOtherGeneralOptions]->name = $2;	
	}
}
QUOTE; 

generaloptionvalueatt: VALUEATT ATTRIBUTETEXT 
{	if (parserData->otherOptionValuePresent)
	{	osolerror( NULL, osoption, parserData, "only one value attribute allowed");
	}
	else
	{	parserData->otherOptionValuePresent = true;
		osoption->general->otherOptions->other[parserData->numberOfOtherGeneralOptions]->value = $2;	
	}
}
QUOTE;

generaloptiondescriptionatt: DESCRIPTIONATT ATTRIBUTETEXT 
{	if (parserData->otherOptionDescriptionPresent)
	{	osolerror( NULL, osoption, parserData, "only one description attribute allowed");
	}
	else
	{	parserData->otherOptionDescriptionPresent = true;
		osoption->general->otherOptions->other[parserData->numberOfOtherGeneralOptions]->description = $2;	
	}
}
QUOTE;

othergeneraloptionend: ENDOFELEMENT | GREATERTHAN OTHEREND;


osolsystem: | systemhead systembody;

systemhead: SYSTEMSTART 
{	if (parserData->osolsystemPresent)
	{	osolerror( NULL, osoption, parserData, "only one <system> element allowed");
	}
	else
	{	parserData->osolsystemPresent = true;	
		osoption->system = new SystemOption();
	}
}; 

systembody:
	  GREATERTHAN systemcontent SYSTEMEND
	| ENDOFELEMENT;

systemcontent: | systemcontent systemoption;

systemoption: mindiskspace | minmemorysize | mincpuspeed | mincpunumber | othersystemoptions;

mindiskspace: mindiskspacehead mindiskspaceunit mindiskspacebody;

mindiskspacehead: MINDISKSPACESTART
{	if (parserData->minDiskSpacePresent)
	{	osolerror( NULL, osoption, parserData, "only one <minDiskSpace> element allowed");
	}
	else
	{	parserData-> minDiskSpacePresent = true;	
		osoption->system->minDiskSpace = new MinDiskSpace();
	}
}; 

mindiskspaceunit: | UNITATT ATTRIBUTETEXT {osoption->system->minDiskSpace->unit = $2;} QUOTE;

mindiskspacebody: ENDOFELEMENT
	| GREATERTHAN MINDISKSPACEEND 
	| GREATERTHAN DOUBLE  {osoption->system->minDiskSpace->value = $2;} MINDISKSPACEEND
	| GREATERTHAN INTEGER {osoption->system->minDiskSpace->value = $2;} MINDISKSPACEEND;


minmemorysize: minmemorysizehead minmemoryunit minmemorysizebody;

minmemorysizehead: MINMEMORYSIZESTART 
{	if (parserData->minMemorySizePresent)
	{	osolerror( NULL, osoption, parserData, "only one <minMemorySize> element allowed");
	}
	else
	{	parserData-> minMemorySizePresent = true;	
		osoption->system->minMemorySize = new MinMemorySize();
	}
}; 

minmemoryunit: | UNITATT ATTRIBUTETEXT {osoption->system->minMemorySize->unit = $2;} QUOTE;

minmemorysizebody: ENDOFELEMENT
	| GREATERTHAN MINMEMORYSIZEEND 
	| GREATERTHAN DOUBLE  {osoption->system->minMemorySize->value = $2;} MINMEMORYSIZEEND
	| GREATERTHAN INTEGER {osoption->system->minMemorySize->value = $2;} MINMEMORYSIZEEND;


mincpuspeed: mincpuspeedhead mincpuspeedunit mincpuspeedbody;

mincpuspeedhead: MINCPUSPEEDSTART 
{	if (parserData->minCPUSpeedPresent)
	{	osolerror( NULL, osoption, parserData, "only one <minCPUSpeed> element allowed");
	}
	else
	{	parserData-> minCPUSpeedPresent = true;	
		osoption->system->minCPUSpeed = new MinCPUSpeed();
	}
}; 
 
mincpuspeedunit: | UNITATT ATTRIBUTETEXT {osoption->system->minCPUSpeed->unit = $2;} QUOTE;

mincpuspeedbody: ENDOFELEMENT
	| GREATERTHAN MINCPUSPEEDEND
	| GREATERTHAN DOUBLE  {osoption->system->minCPUSpeed->value = $2;} MINCPUSPEEDEND
	| GREATERTHAN INTEGER {osoption->system->minCPUSpeed->value = $2;} MINCPUSPEEDEND;


mincpunumber: mincpunumberhead mincpunumberbody;

mincpunumberhead: MINCPUNUMBERSTART 
{	if (parserData->minCPUNumberPresent)
	{	osolerror( NULL, osoption, parserData, "only one <minCPUNumber> element allowed");
	}
	else
	{	parserData->minCPUNumberPresent = true;	
	}
}; 

mincpunumberbody: ENDOFELEMENT
	| GREATERTHAN MINCPUNUMBEREND
	| GREATERTHAN INTEGER {osoption->system->minCPUNumber = $2;} MINCPUNUMBEREND;


othersystemoptions: othersystemoptionshead numberofothersystemoptions GREATERTHAN othersystemoptionsbody;

othersystemoptionshead: OTHEROPTIONSSTART
{	if (parserData->otherSystemOptionsPresent)
	{	osolerror( NULL, osoption, parserData, "only one <otherOptions> element allowed");
	}
	else
	{	parserData->otherSystemOptionsPresent = true;
		osoption->system->otherOptions = new OtherOptions();	
	}
}; 
 
numberofothersystemoptions: NUMBEROFOTHEROPTIONSATT QUOTE INTEGER QUOTE
{	osoption->system->otherOptions->numberOfOtherOptions = $3;
	osoption->system->otherOptions->other = new OtherOption*[$3];
	for (int i=0; i < $3; i++) osoption->system->otherOptions->other[i] = new OtherOption();
};

othersystemoptionsbody: othersystemoptionslist OTHEROPTIONSEND
{	if (parserData->numberOfOtherSystemOptions != osoption->system->otherOptions->numberOfOtherOptions)
		osolerror (NULL, osoption, parserData, "wrong number of other options in <system> element"); 
};

othersystemoptionslist: | othersystemoptionslist othersystemoption;

othersystemoption: OTHERSTART 
	{	if (parserData->numberOfOtherSystemOptions >= osoption->system->otherOptions->numberOfOtherOptions)
		{	osolerror (NULL, osoption, parserData, "too many other options in <system> element");
		};
	} 
    othersystemattributes othersystemoptionend
{	if (!parserData->otherOptionNamePresent)
		osolerror (NULL, osoption, parserData, "name attribute must be present");
	/* reset defaults for the next option */
	parserData->otherOptionNamePresent = false;
	parserData->otherOptionValuePresent = false;
	parserData->otherOptionDescriptionPresent = false;
	parserData->numberOfOtherSystemOptions++;
};

othersystemattributes: | othersystemattributes othersystemattribute;

othersystemattribute: systemoptionnameatt | systemoptionvalueatt | systemoptiondescriptionatt;

systemoptionnameatt: NAMEATT ATTRIBUTETEXT 
{	if (parserData->otherOptionNamePresent)
	{	osolerror( NULL, osoption, parserData, "only one name attribute allowed");
	}
	else
	{	parserData->otherOptionNamePresent = true;
		osoption->system->otherOptions->other[parserData->numberOfOtherSystemOptions]->name = $2;	
	}
}
QUOTE; 

systemoptionvalueatt: VALUEATT ATTRIBUTETEXT 
{	if (parserData->otherOptionValuePresent)
	{	osolerror( NULL, osoption, parserData, "only one value attribute allowed");
	}
	else
	{	parserData->otherOptionValuePresent = true;
		osoption->system->otherOptions->other[parserData->numberOfOtherSystemOptions]->value = $2;	
	}
}
QUOTE;


systemoptiondescriptionatt: DESCRIPTIONATT ATTRIBUTETEXT 
{	if (parserData->otherOptionDescriptionPresent)
	{	osolerror( NULL, osoption, parserData, "only one description attribute allowed");
	}
	else
	{	parserData->otherOptionDescriptionPresent = true;
		osoption->system->otherOptions->other[parserData->numberOfOtherSystemOptions]->description = $2;	
	}
}
QUOTE;

othersystemoptionend: {printf("%s","firing />");} ENDOFELEMENT | {printf("%s","firing </other>");} GREATERTHAN OTHEREND;


osolservice: | servicehead servicebody;

servicehead: SERVICESTART 
{	if (parserData->osolservicePresent)
	{	osolerror( NULL, osoption, parserData, "only one <service> element allowed");
	}
	else
	{	parserData->osolservicePresent = true;	
		osoption->service = new ServiceOption();
	}
}; 

servicebody:
	  GREATERTHAN servicecontent SERVICEEND
	| ENDOFELEMENT;

servicecontent: | servicecontent serviceoption;

serviceoption: servicetype | otherserviceoptions;

servicetype: servicetypehead | servicetypebody;

servicetypehead: SERVICETYPESTART 
{	if (parserData->serviceTypePresent)
	{	osolerror( NULL, osoption, parserData, "only one <type> element allowed");
	}
	else
	{	parserData->serviceTypePresent = true;	
	}
}; 

servicetypebody: ENDOFELEMENT
	| GREATERTHAN SERVICETYPEEND
	| GREATERTHAN INTEGER {osoption->service->type = $2;} SERVICETYPEEND;

otherserviceoptions: otherserviceoptionshead numberofotherserviceoptions GREATERTHAN otherserviceoptionsbody;

otherserviceoptionshead: OTHEROPTIONSSTART
{	if (parserData->otherServiceOptionsPresent)
	{	osolerror( NULL, osoption, parserData, "only one <otherOptions> element allowed");
	}
	else
	{	parserData->otherServiceOptionsPresent = true;
		osoption->service->otherOptions = new OtherOptions();	
	}
}; 

numberofotherserviceoptions: NUMBEROFOTHEROPTIONSATT QUOTE INTEGER QUOTE
{	osoption->service->otherOptions->numberOfOtherOptions = $3;
	osoption->service->otherOptions->other = new OtherOption*[$3];
	for (int i=0; i < $3; i++) osoption->service->otherOptions->other[i] = new OtherOption();
};

otherserviceoptionsbody: otherserviceoptionslist OTHEROPTIONSEND
{	if (parserData->numberOfOtherServiceOptions != osoption->service->otherOptions->numberOfOtherOptions)
		osolerror (NULL, osoption, parserData, "wrong number of other options in <service> element"); 
};

otherserviceoptionslist: | otherserviceoptionslist otherserviceoption;

otherserviceoption: OTHERSTART 
	{	if (parserData->numberOfOtherServiceOptions >= osoption->service->otherOptions->numberOfOtherOptions)
		{	osolerror (NULL, osoption, parserData, "too many other options in <service> element");
		};
	} 
otherserviceattributes otherserviceoptionsend
{	if (!parserData->otherOptionNamePresent)
		osolerror (NULL, osoption, parserData, "name attribute must be present");
	/* reset defaults for the next option */
	parserData->otherOptionNamePresent = false;
	parserData->otherOptionValuePresent = false;
	parserData->otherOptionDescriptionPresent = false;
	parserData->numberOfOtherServiceOptions++;
};

otherserviceattributes: | otherserviceattributes otherserviceattribute;

otherserviceattribute: serviceoptionnameatt | serviceoptionvalueatt | serviceoptiondescriptionatt;

serviceoptionnameatt: NAMEATT ATTRIBUTETEXT 
{	if (parserData->otherOptionNamePresent)
	{	osolerror( NULL, osoption, parserData, "only one name attribute allowed");
	}
	else
	{	parserData->otherOptionNamePresent = true;
		osoption->service->otherOptions->other[parserData->numberOfOtherServiceOptions]->name = $2;	
	}
}
QUOTE; 

serviceoptionvalueatt: VALUEATT ATTRIBUTETEXT 
{	if (parserData->otherOptionValuePresent)
	{	osolerror( NULL, osoption, parserData, "only one value attribute allowed");
	}
	else
	{	parserData->otherOptionValuePresent = true;
		osoption->service->otherOptions->other[parserData->numberOfOtherServiceOptions]->value = $2;	
	}
}
QUOTE;

serviceoptiondescriptionatt: DESCRIPTIONATT ATTRIBUTETEXT 
{	if (parserData->otherOptionDescriptionPresent)
	{	osolerror( NULL, osoption, parserData, "only one description attribute allowed");
	}
	else
	{	parserData->otherOptionDescriptionPresent = true;
		osoption->service->otherOptions->other[parserData->numberOfOtherServiceOptions]->description = $2;	
	}
}
QUOTE;

otherserviceoptionsend: ENDOFELEMENT | GREATERTHAN OTHEREND;


osoljob: jobhead jobbody;

jobhead: JOBSTART 
{	if (parserData->osoljobPresent)
	{	osolerror( NULL, osoption, parserData, "only one <job> element allowed");
	}
	else
	{	parserData->osoljobPresent = true;	
		osoption->job = new JobOption();
	}
}; 

jobbody: GREATERTHAN jobcontent JOBEND
	|  ENDOFELEMENT;

jobcontent: | jobcontent joboption; 

joboption: maxtime | scheduledstarttime | dependencies | requireddirectories | requiredfiles
| directoriestomake | filestocreate | inputdirectoriestomove | inputfilestomove | outputdirectoriestomove 
| outputfilestomove | filestodelete | directoriestodelete | processestokill | otherjoboptions;


maxtime: maxtimehead maxtimeunit maxtimebody;

maxtimehead: MAXTIMESTART 
{	if (parserData->maxTimePresent)
	{	osolerror( NULL, osoption, parserData, "only one <maxTime> element allowed");
	}
	else
	{	parserData-> maxTimePresent = true;	
		osoption->job->maxTime = new MaxTime();
	}
}; 

maxtimeunit: | UNITATT ATTRIBUTETEXT {osoption->job->maxTime->unit = $2;} QUOTE;

maxtimebody: ENDOFELEMENT
	| GREATERTHAN MAXTIMEEND
	| GREATERTHAN DOUBLE  {osoption->job->maxTime->value = $2;} MAXTIMEEND
	| GREATERTHAN INTEGER {osoption->job->maxTime->value = $2;} MAXTIMEEND;

scheduledstarttime: starttimehead starttimebody;

starttimehead: SCHEDULEDSTARTTIMESTART 
{	if (parserData->scheduledStartTimePresent)
	{	osolerror( NULL, osoption, parserData, "only one <scheduledStartTime> element allowed");
	}
	else
	{	parserData->scheduledStartTimePresent = true;	
	}
}; 

starttimebody: ENDOFELEMENT
	| GREATERTHAN SCHEDULEDSTARTTIMEEND
	| GREATERTHAN ELEMENTTEXT {osoption->general->userName = $2;} SCHEDULEDSTARTTIMEEND;

dependencies: dependencieshead numberofjobidsatt GREATERTHAN dependencieslist DEPENDENCIESEND;

dependencieshead: DEPENDENCIESSTART 
{	if (parserData->dependenciesPresent)
	{	osolerror( NULL, osoption, parserData, "only one <dependencies> element allowed");
	}
	else
	{	parserData->dependenciesPresent = true;
		osoption->job->dependencies = new JobDependencies();	
	}
}; 

numberofjobidsatt: NUMBEROFJOBIDSATT QUOTE INTEGER QUOTE
{	osoption->job->dependencies->numberOfJobIDs = $3;
	osoption->job->dependencies->jobID = new std::string[$3];
}
;

dependencieslist: | dependencieslist dependencyjobid;

dependencyjobid: JOBIDSTART GREATERTHAN ELEMENTTEXT 
{	
	if (parserData->numberOfDependencies >= osoption->job->dependencies->numberOfJobIDs)
	{	osolerror (NULL, osoption, parserData, "too many job IDs in <dependencies> element");
	}
	else
	{	osoption->job->dependencies->jobID[parserData->numberOfDependencies] = $3;
		parserData->numberOfDependencies++; 
	};
} 
JOBIDEND;


requireddirectories: requireddirectorieshead numberofreqdirpathsatt GREATERTHAN reqdirpathlist 
   REQUIREDDIRECTORIESEND;

requireddirectorieshead: REQUIREDDIRECTORIESSTART 
{	if (parserData->requiredDirectoriesPresent)
	{	osolerror( NULL, osoption, parserData, "only one <requiredDirectories> element allowed");
	}
	else
	{	parserData->requiredDirectoriesPresent = true;
		osoption->job->requiredDirectories = new DirectoriesAndFiles();	
	}
}; 

numberofreqdirpathsatt: NUMBEROFPATHSATT QUOTE INTEGER QUOTE
{	osoption->job->requiredDirectories->numberOfPaths = $3;
	osoption->job->requiredDirectories->path = new std::string[$3];
};

reqdirpathlist: | reqdirpathlist reqdirpath;

reqdirpath: PATHSTART GREATERTHAN ELEMENTTEXT {	
	if (parserData->numberOfRequiredDirectories >= osoption->job->requiredDirectories->numberOfPaths)
	{	osolerror (NULL, osoption, parserData, "too many job IDs in <requiredDirectories> element");
	}
	else
	{	osoption->job->requiredDirectories->path[parserData->numberOfRequiredDirectories] = $3;
		parserData->numberOfRequiredDirectories++; 
	};
} 
PATHEND;


requiredfiles: requiredfileshead numberofreqfilespathsatt GREATERTHAN reqfilespathlist 
   REQUIREDFILESEND;

requiredfileshead: REQUIREDFILESSTART 
{	if (parserData->requiredFilesPresent)
	{	osolerror( NULL, osoption, parserData, "only one <requiredFiles> element allowed");
	}
	else
	{	parserData->requiredFilesPresent = true;
		osoption->job->requiredFiles = new DirectoriesAndFiles();	
	}
}; 

numberofreqfilespathsatt: NUMBEROFPATHSATT QUOTE INTEGER QUOTE
{	osoption->job->requiredFiles->numberOfPaths = $3;
	osoption->job->requiredFiles->path = new std::string[$3];
};

reqfilespathlist: | reqfilespathlist reqfilepath;

reqfilepath: PATHSTART GREATERTHAN ELEMENTTEXT {	
	if (parserData->numberOfRequiredFiles >= osoption->job->requiredFiles->numberOfPaths)
	{	osolerror (NULL, osoption, parserData, "too many job IDs in <requiredFiles> element");
	}
	else
	{	osoption->job->requiredFiles->path[parserData->numberOfRequiredFiles] = $3;
		parserData->numberOfRequiredFiles++; 
	};
} 
PATHEND;


directoriestomake: directoriestomakehead numberofdirtomakepathsatt GREATERTHAN dirtomakepathlist
   DIRECTORIESTOMAKEEND;

directoriestomakehead: DIRECTORIESTOMAKESTART 
{	if (parserData->directoriesToMakePresent)
	{	osolerror( NULL, osoption, parserData, "only one <directoriesToMake> element allowed");
	}
	else
	{	parserData->directoriesToMakePresent = true;
		osoption->job->directoriesToMake = new DirectoriesAndFiles();	
	}
}; 
numberofdirtomakepathsatt: NUMBEROFPATHSATT QUOTE INTEGER QUOTE
{	osoption->job->directoriesToMake->numberOfPaths = $3;
	osoption->job->directoriesToMake->path = new std::string[$3];
};

dirtomakepathlist: | dirtomakepathlist dirtomakepath;

dirtomakepath: PATHSTART GREATERTHAN ELEMENTTEXT 
{	if (parserData->numberOfDirectoriesToMake >= osoption->job->directoriesToMake->numberOfPaths)
	{	osolerror (NULL, osoption, parserData, "too many job IDs in <directoriesToMake> element");
	}
	else
	{	osoption->job->directoriesToMake->path[parserData->numberOfDirectoriesToMake] = $3;
		parserData->numberOfDirectoriesToMake++; 
	};
} 
PATHEND;


filestocreate: filestocreatehead numberoffilestomakepathsatt GREATERTHAN filestomakepathlist
   FILESTOCREATEEND;

filestocreatehead: FILESTOCREATESTART 
{	if (parserData->filesToCreatePresent)
	{	osolerror( NULL, osoption, parserData, "only one <filesToCreate> element allowed");
	}
	else
	{	parserData->filesToCreatePresent = true;
		osoption->job->filesToCreate = new DirectoriesAndFiles();	
	}
}; 

numberoffilestomakepathsatt: NUMBEROFPATHSATT QUOTE INTEGER QUOTE
{	osoption->job->filesToCreate->numberOfPaths = $3;
	osoption->job->filesToCreate->path = new std::string[$3];
};

filestomakepathlist: | filestomakepathlist filestomakepath;

filestomakepath: PATHSTART GREATERTHAN ELEMENTTEXT 
{	if (parserData->numberOfFilesToCreate >= osoption->job->filesToCreate->numberOfPaths)
	{	osolerror (NULL, osoption, parserData, "too many job IDs in <filesToCreate> element");
	}
	else
	{	osoption->job->filesToCreate->path[parserData->numberOfFilesToCreate] = $3;
		parserData->numberOfFilesToCreate++; 
	};
} 
PATHEND;


inputdirectoriestomove: inputdirectoriestomovehead numberofindirtomovepathpairsatt 
   GREATERTHAN indirtomovepathpairlist INPUTDIRECTORIESTOMOVEEND;

inputdirectoriestomovehead: INPUTDIRECTORIESTOMOVESTART 
{	if (parserData->inputDirectoriesToMovePresent)
	{	osolerror( NULL, osoption, parserData, "only one <inputDirectoriesToMove> element allowed");
	}
	else
	{	parserData->inputDirectoriesToMovePresent = true;
		osoption->job->inputDirectoriesToMove = new PathPairs();
	}
}; 

numberofindirtomovepathpairsatt: NUMBEROFPATHPAIRSATT QUOTE INTEGER QUOTE
{	if ($3 <= 0) osolerror (NULL, osoption, parserData, "Require positive number of directories to move");
	osoption->job->inputDirectoriesToMove->numberOfPathPairs = $3;
	osoption->job->inputDirectoriesToMove->pathPair = new PathPair*[$3];
	for (int i = 0; i < $3; i++)
		osoption->job->inputDirectoriesToMove->pathPair[i] = new PathPair();
};

indirtomovepathpairlist: | indirtomovepathpairlist indirtomovepathpair;

indirtomovepathpair: indirtomovepathpairhead indirtomovepathpairattlist indirtomovepathpairend
{	if (!parserData->pathPairFromPresent)
		osolerror (NULL, osoption, parserData, "\"from\" attribute must be present");
	if (!parserData->pathPairToPresent)
		osolerror (NULL, osoption, parserData, "\"to\" attribute must be present");
	/* reset defaults for the next option */	
	parserData->pathPairFromPresent= false;
	parserData->pathPairToPresent= false;
	parserData->pathPairMakeCopyPresent= false;
	parserData->numberOfInputDirectoriesToMove++; 
};

indirtomovepathpairhead: PATHPAIRSTART 
{	if (parserData->numberOfInputDirectoriesToMove >= osoption->job->inputDirectoriesToMove->numberOfPathPairs)
	{	osolerror (NULL, osoption, parserData, "too many path pairs in <inputDirectoriesToMove> element");
	};
}; 

indirtomovepathpairattlist: | indirtomovepathpairattlist indirtomovepathpairatt;

indirtomovepathpairatt: indirtomovefromatt | indirtomovetoatt | indirtomovemakecopyatt;

indirtomovefromatt: FROMATT ATTRIBUTETEXT 
{	if (parserData->pathPairFromPresent)
	{	osolerror( NULL, osoption, parserData, "only one \"from\" attribute allowed");
	}
	else
	{	parserData->pathPairFromPresent = true;
		osoption->job->inputDirectoriesToMove->pathPair[parserData->numberOfInputDirectoriesToMove]->from = $2;	
	}
}
QUOTE; 

indirtomovetoatt: TOATT ATTRIBUTETEXT 
{	if (parserData->pathPairToPresent)
	{	osolerror( NULL, osoption, parserData, "only one \"to\" attribute allowed");
	}
	else
	{	parserData->pathPairToPresent = true;
		osoption->job->inputDirectoriesToMove->pathPair[parserData->numberOfInputDirectoriesToMove]->to = $2;	
	}
}
QUOTE; 

indirtomovemakecopyatt: MAKECOPYATT ATTRIBUTETEXT 
{	if (parserData->pathPairMakeCopyPresent)
	{	osolerror( NULL, osoption, parserData, "only one \"makeCopy\" attribute allowed");
	}
	else
	{	parserData->pathPairMakeCopyPresent = true;
		if ($2 == "true")
			osoption->job->inputDirectoriesToMove->pathPair[parserData->numberOfInputDirectoriesToMove]->makeCopy = $2;	
	}
}
QUOTE; 

indirtomovepathpairend: GREATERTHAN PATHPAIREND | ENDOFELEMENT;


inputfilestomove: inputfilestomovehead numberofinfilestomovepathpairsatt 
   GREATERTHAN infilestomovepathpairlist INPUTFILESTOMOVEEND;

inputfilestomovehead: INPUTFILESTOMOVESTART 
{	if (parserData->inputFilesToMovePresent)
	{	osolerror( NULL, osoption, parserData, "only one <inputFilesToMove> element allowed");
	}
	else
	{	parserData->inputFilesToMovePresent = true;
		osoption->job->inputFilesToMove = new PathPairs();	
	}
}; 

numberofinfilestomovepathpairsatt: NUMBEROFPATHPAIRSATT QUOTE INTEGER QUOTE
{	if ($3 <= 0) osolerror (NULL, osoption, parserData, "Require positive number of files to move");
	osoption->job->inputFilesToMove->numberOfPathPairs = $3;
	osoption->job->inputFilesToMove->pathPair = new PathPair*[$3];
	for (int i = 0; i < $3; i++) osoption->job->inputFilesToMove->pathPair[i] = new PathPair();
};

infilestomovepathpairlist: | infilestomovepathpairlist infilestomovepathpair;

infilestomovepathpair: infilestomovepathpairhead infilestomovepathpairattlist infilestomovepathpairend
{	if (!parserData->pathPairFromPresent)
		osolerror (NULL, osoption, parserData, "\"from\" attribute must be present");
	if (!parserData->pathPairToPresent)
		osolerror (NULL, osoption, parserData, "\"to\" attribute must be present");
	/* reset defaults for the next option */	
	parserData->pathPairFromPresent= false;
	parserData->pathPairToPresent= false;
	parserData->pathPairMakeCopyPresent= false;
	parserData->numberOfInputFilesToMove++; 
};

infilestomovepathpairhead: PATHPAIRSTART 
{	if (parserData->numberOfInputFilesToMove >= osoption->job->inputFilesToMove->numberOfPathPairs)
	{	osolerror (NULL, osoption, parserData, "too many path pairs in <inputFilesToMove> element");
	};
} 

infilestomovepathpairattlist: | infilestomovepathpairattlist infilestomovepathpairatt;

infilestomovepathpairatt: infilestomovefromatt | infilestomovetoatt | infilestomovemakecopyatt;

infilestomovefromatt: FROMATT ATTRIBUTETEXT 
{	if (parserData->pathPairFromPresent)
	{	osolerror( NULL, osoption, parserData, "only one \"from\" attribute allowed");
	}
	else
	{	parserData->pathPairFromPresent = true;
		osoption->job->inputFilesToMove->pathPair[parserData->numberOfInputFilesToMove]->from = $2;	
	}
}
QUOTE; 

infilestomovetoatt: TOATT ATTRIBUTETEXT 
{	if (parserData->pathPairToPresent)
	{	osolerror( NULL, osoption, parserData, "only one \"to\" attribute allowed");
	}
	else
	{	parserData->pathPairToPresent = true;
		osoption->job->inputFilesToMove->pathPair[parserData->numberOfInputFilesToMove]->to = $2;	
	}
}
QUOTE; 

infilestomovemakecopyatt: MAKECOPYATT ATTRIBUTETEXT 
{	if (parserData->pathPairMakeCopyPresent)
	{	osolerror( NULL, osoption, parserData, "only one \"makeCopy\" attribute allowed");
	}
	else
	{	parserData->pathPairMakeCopyPresent = true;
		if ($2 == "true")
			osoption->job->inputFilesToMove->pathPair[parserData->numberOfInputFilesToMove]->makeCopy = $2;	
	}
}
QUOTE; 

infilestomovepathpairend: GREATERTHAN PATHPAIREND | ENDOFELEMENT;


outputdirectoriestomove: outputdirectoriestomovehead numberofoutdirtomovepathpairsatt 
   GREATERTHAN outdirtomovepathpairlist OUTPUTDIRECTORIESTOMOVEEND;

outputdirectoriestomovehead: OUTPUTDIRECTORIESTOMOVESTART 
{	if (parserData->outputDirectoriesToMovePresent)
	{	osolerror( NULL, osoption, parserData, "only one <outputDirectoriesToMove> element allowed");
	}
	else
	{	parserData->outputDirectoriesToMovePresent = true;
		osoption->job->outputDirectoriesToMove = new PathPairs();	
	}
}; 

numberofoutdirtomovepathpairsatt: NUMBEROFPATHPAIRSATT QUOTE INTEGER QUOTE
{	if ($3 <= 0) osolerror (NULL, osoption, parserData, "Require positive number of directories to move");
	osoption->job->outputDirectoriesToMove->numberOfPathPairs = $3;
	osoption->job->outputDirectoriesToMove->pathPair = new PathPair*[$3];
	for (int i = 0; i < $3; i++)
		osoption->job->outputDirectoriesToMove->pathPair[i] = new PathPair();
};

outdirtomovepathpairlist: | outdirtomovepathpairlist outdirtomovepathpair;

outdirtomovepathpair: outdirtomovepathpairhead outdirtomovepathpairattlist outdirtomovepathpairend
{	if (!parserData->pathPairFromPresent)
		osolerror (NULL, osoption, parserData, "\"from\" attribute must be present");
	if (!parserData->pathPairToPresent)
		osolerror (NULL, osoption, parserData, "\"to\" attribute must be present");
	/* reset defaults for the next option */	
	parserData->pathPairFromPresent= false;
	parserData->pathPairToPresent= false;
	parserData->pathPairMakeCopyPresent= false;
	parserData->numberOfOutputDirectoriesToMove++; 
};

outdirtomovepathpairhead: PATHPAIRSTART 
{	if (parserData->numberOfOutputDirectoriesToMove >= osoption->job->outputDirectoriesToMove->numberOfPathPairs)
	{	osolerror (NULL, osoption, parserData, "too many path pairs in <outputDirectoriesToMove> element");
	};
} 

outdirtomovepathpairattlist: | outdirtomovepathpairattlist outdirtomovepathpairatt;

outdirtomovepathpairatt: outdirtomovefromatt | outdirtomovetoatt | outdirtomovemakecopyatt;

outdirtomovefromatt: FROMATT ATTRIBUTETEXT 
{	if (parserData->pathPairFromPresent)
	{	osolerror( NULL, osoption, parserData, "only one \"from\" attribute allowed");
	}
	else
	{	parserData->pathPairFromPresent = true;
		osoption->job->outputDirectoriesToMove->pathPair[parserData->numberOfOutputDirectoriesToMove]->from = $2;	
	}
}
QUOTE; 

outdirtomovetoatt: TOATT ATTRIBUTETEXT 
{	if (parserData->pathPairToPresent)
	{	osolerror( NULL, osoption, parserData, "only one \"to\" attribute allowed");
	}
	else
	{	parserData->pathPairToPresent = true;
		osoption->job->outputDirectoriesToMove->pathPair[parserData->numberOfOutputDirectoriesToMove]->to = $2;	
	}
}
QUOTE; 

outdirtomovemakecopyatt: MAKECOPYATT ATTRIBUTETEXT 
{	if (parserData->pathPairMakeCopyPresent)
	{	osolerror( NULL, osoption, parserData, "only one \"makeCopy\" attribute allowed");
	}
	else
	{	parserData->pathPairMakeCopyPresent = true;
		if ($2 == "true")
			osoption->job->outputDirectoriesToMove->pathPair[parserData->numberOfOutputDirectoriesToMove]->makeCopy = $2;	
	}
}
QUOTE; 

outdirtomovepathpairend: GREATERTHAN PATHPAIREND | ENDOFELEMENT;


outputfilestomove: outputfilestomovehead numberofoutfilestomovepathpairsatt 
   GREATERTHAN outfilestomovepathpairlist OUTPUTFILESTOMOVEEND;

outputfilestomovehead: OUTPUTFILESTOMOVESTART 
{	if (parserData->outputFilesToMovePresent)
	{	osolerror( NULL, osoption, parserData, "only one <outputFilesToMove> element allowed");
	}
	else
	{	parserData->outputFilesToMovePresent = true;
		osoption->job->outputFilesToMove = new PathPairs();	
	}
}; 

numberofoutfilestomovepathpairsatt: NUMBEROFPATHPAIRSATT QUOTE INTEGER QUOTE
{	if ($3 <= 0) osolerror (NULL, osoption, parserData, "Require positive number of files to move");
	osoption->job->outputFilesToMove->numberOfPathPairs = $3;
	osoption->job->outputFilesToMove->pathPair = new PathPair*[$3];
	for (int i = 0; i < $3; i++)
		osoption->job->outputFilesToMove->pathPair[i] = new PathPair();};

outfilestomovepathpairlist: | outfilestomovepathpairlist outfilestomovepathpair;

outfilestomovepathpair: outfilestomovepathpairhead outfilestomovepathpairattlist outfilestomovepathpairend
{	if (!parserData->pathPairFromPresent)
		osolerror (NULL, osoption, parserData, "\"from\" attribute must be present");
	if (!parserData->pathPairToPresent)
		osolerror (NULL, osoption, parserData, "\"to\" attribute must be present");
	/* reset defaults for the next option */	
	parserData->pathPairFromPresent= false;
	parserData->pathPairToPresent= false;
	parserData->pathPairMakeCopyPresent= false;
	parserData->numberOfOutputFilesToMove++; 
};

outfilestomovepathpairhead: PATHPAIRSTART 
{	if (parserData->numberOfOutputFilesToMove >= osoption->job->outputFilesToMove->numberOfPathPairs)
	{	osolerror (NULL, osoption, parserData, "too many path pairs in <outputFilesToMove> element");
	};
} 

outfilestomovepathpairattlist: | outfilestomovepathpairattlist outfilestomovepathpairatt;

outfilestomovepathpairatt: outfilestomovefromatt | outfilestomovetoatt | outfilestomovemakecopyatt;

outfilestomovefromatt: FROMATT ATTRIBUTETEXT 
{	if (parserData->pathPairFromPresent)
	{	osolerror( NULL, osoption, parserData, "only one \"from\" attribute allowed");
	}
	else
	{	parserData->pathPairFromPresent = true;
		osoption->job->outputFilesToMove->pathPair[parserData->numberOfOutputFilesToMove]->from = $2;	
	}
}
QUOTE; 

outfilestomovetoatt: TOATT ATTRIBUTETEXT 
{	if (parserData->pathPairToPresent)
	{	osolerror( NULL, osoption, parserData, "only one \"to\" attribute allowed");
	}
	else
	{	parserData->pathPairToPresent = true;
		osoption->job->outputFilesToMove->pathPair[parserData->numberOfOutputFilesToMove]->to = $2;	
	}
}
QUOTE; 

outfilestomovemakecopyatt: MAKECOPYATT ATTRIBUTETEXT 
{	if (parserData->pathPairMakeCopyPresent)
	{	osolerror( NULL, osoption, parserData, "only one \"makeCopy\" attribute allowed");
	}
	else
	{	parserData->pathPairMakeCopyPresent = true;
		if ($2 == "true")
			osoption->job->outputFilesToMove->pathPair[parserData->numberOfOutputFilesToMove]->makeCopy = $2;	
	}
}
QUOTE; 

outfilestomovepathpairend: GREATERTHAN PATHPAIREND | ENDOFELEMENT;


filestodelete: filestodeletehead numberoffilestodeletepathsatt GREATERTHAN filestodeletepathlist
   FILESTODELETEEND;

filestodeletehead: FILESTODELETESTART 
{	if (parserData->filesToDeletePresent)
	{	osolerror( NULL, osoption, parserData, "only one <filesToDelete> element allowed");
	}
	else
	{	parserData->filesToDeletePresent = true;
		osoption->job->filesToDelete = new DirectoriesAndFiles();	
	}
}; 

numberoffilestodeletepathsatt: NUMBEROFPATHSATT QUOTE INTEGER QUOTE
{	osoption->job->filesToDelete->numberOfPaths = $3;
	osoption->job->filesToDelete->path = new std::string[$3];
};

filestodeletepathlist: | filestodeletepathlist filestodeletepath;

filestodeletepath: PATHSTART GREATERTHAN ELEMENTTEXT 
{	if (parserData->numberOfFilesToDelete >= osoption->job->filesToDelete->numberOfPaths)
	{	osolerror (NULL, osoption, parserData, "too many job IDs in <filesToDelete> element");
	}
	else
	{	osoption->job->filesToDelete->path[parserData->numberOfFilesToDelete] = $3;
		parserData->numberOfFilesToDelete++; 
	};
} 
PATHEND;


directoriestodelete: directoriestodeletehead numberofdirtodeletepathsatt GREATERTHAN dirtodeletepathlist
   DIRECTORIESTODELETEEND;

directoriestodeletehead: DIRECTORIESTODELETESTART 
{	if (parserData->directoriesToDeletePresent)
	{	osolerror( NULL, osoption, parserData, "only one <directoriesToDelete> element allowed");
	}
	else
	{	parserData->directoriesToDeletePresent = true;
		osoption->job->directoriesToDelete = new DirectoriesAndFiles();	
	}
}; 

numberofdirtodeletepathsatt: NUMBEROFPATHSATT QUOTE INTEGER QUOTE
{	osoption->job->directoriesToDelete->numberOfPaths = $3;
	osoption->job->directoriesToDelete->path = new std::string[$3];
};

dirtodeletepathlist: | dirtodeletepathlist dirtodeletepath;

dirtodeletepath: PATHSTART GREATERTHAN ELEMENTTEXT 
{	if (parserData->numberOfDirectoriesToDelete >= osoption->job->directoriesToDelete->numberOfPaths)
	{	osolerror (NULL, osoption, parserData, "too many job IDs in <directoriesToDelete> element");
	}
	else
	{	osoption->job->directoriesToDelete->path[parserData->numberOfDirectoriesToDelete] = $3;
		parserData->numberOfDirectoriesToDelete++; 
	};
} 
PATHEND;


processestokill: processestokillhead numberofprocesstokillatt GREATERTHAN processestokilllist
   PROCESSESTOKILLEND;


processestokillhead: PROCESSESTOKILLSTART 
{	if (parserData->processesToKillPresent)
	{	osolerror( NULL, osoption, parserData, "only one <processesToKill> element allowed");
	}
	else
	{	parserData->processesToKillPresent = true;
		osoption->job->processesToKill = new Processes();	
	}
}; 

numberofprocesstokillatt: NUMBEROFPROCESSESATT QUOTE INTEGER QUOTE
{	osoption->job->processesToKill->numberOfProcesses = $3;
	osoption->job->processesToKill->process = new std::string[$3];
};

processestokilllist: | processestokilllist processtokill;

processtokill: PROCESSSTART GREATERTHAN ELEMENTTEXT 
{	if (parserData->numberOfProcessesToKill >= osoption->job->processesToKill->numberOfProcesses)
	{	osolerror (NULL, osoption, parserData, "too many job IDs in <processesToKill> element");
	}
	else
	{	osoption->job->processesToKill->process[parserData->numberOfProcessesToKill] = $3;
		parserData->numberOfProcessesToKill++; 
	};
} 
PROCESSEND;


otherjoboptions: otherjoboptionshead numberofotherjoboptions GREATERTHAN otherjoboptionsbody;

otherjoboptionshead: OTHEROPTIONSSTART
{	if (parserData->otherJobOptionsPresent)
	{	osolerror( NULL, osoption, parserData, "only one <otherOptions> element allowed");
	}
	else
	{	parserData->otherJobOptionsPresent = true;
		osoption->job->otherOptions = new OtherOptions();	
	}
}; 

numberofotherjoboptions: NUMBEROFOTHEROPTIONSATT QUOTE INTEGER QUOTE
{	osoption->job->otherOptions->numberOfOtherOptions = $3;
	osoption->job->otherOptions->other = new OtherOption*[$3];
	for (int i=0; i < $3; i++) osoption->job->otherOptions->other[i] = new OtherOption();
};

otherjoboptionsbody: otherjoboptionslist OTHEROPTIONSEND
{	if (parserData->numberOfOtherJobOptions != osoption->job->otherOptions->numberOfOtherOptions)
		osolerror (NULL, osoption, parserData, "wrong number of other options in <job> element"); 
};

otherjoboptionslist: | otherjoboptionslist otherjoboption;

otherjoboption: OTHERSTART 
	{	if (parserData->numberOfOtherJobOptions >= osoption->job->otherOptions->numberOfOtherOptions)
		{	osolerror (NULL, osoption, parserData, "too many other options in <job> element");
		};
	} 
otherjobattributes otherjoboptionsend
{	if (!parserData->otherOptionNamePresent)
		osolerror (NULL, osoption, parserData, "name attribute must be present");
	/* reset defaults for the next option */
	parserData->otherOptionNamePresent = false;
	parserData->otherOptionValuePresent = false;
	parserData->otherOptionDescriptionPresent = false;
	parserData->numberOfOtherJobOptions++;
};

otherjobattributes: | otherjobattributes otherjobattribute;

otherjobattribute: joboptionnameatt | joboptionvalueatt | joboptiondescriptionatt;

joboptionnameatt: NAMEATT ATTRIBUTETEXT 
{	if (parserData->otherOptionNamePresent)
	{	osolerror( NULL, osoption, parserData, "only one name attribute allowed");
	}
	else
	{	parserData->otherOptionNamePresent = true;
		osoption->job->otherOptions->other[parserData->numberOfOtherJobOptions]->name = $2;	
	}
}
QUOTE; 

joboptionvalueatt: VALUEATT ATTRIBUTETEXT 
{	if (parserData->otherOptionValuePresent)
	{	osolerror( NULL, osoption, parserData, "only one value attribute allowed");
	}
	else
	{	parserData->otherOptionValuePresent = true;
		osoption->job->otherOptions->other[parserData->numberOfOtherJobOptions]->value = $2;	
	}
}
QUOTE;

joboptiondescriptionatt: DESCRIPTIONATT ATTRIBUTETEXT 
{	if (parserData->otherOptionDescriptionPresent)
	{	osolerror( NULL, osoption, parserData, "only one description attribute allowed");
	}
	else
	{	parserData->otherOptionDescriptionPresent = true;
		osoption->job->otherOptions->other[parserData->numberOfOtherJobOptions]->description = $2;	
	}
}
QUOTE;

otherjoboptionsend: ENDOFELEMENT | GREATERTHAN OTHEREND;


osoloptimization: | optimizationhead optimizationattlist optimizationbody;

optimizationhead: OPTIMIZATIONSTART 
{	if (parserData->osoloptimizationPresent)
	{	osolerror( NULL, osoption, parserData, "only one <optimization> element allowed");
	}
	else
	{	parserData->osoloptimizationPresent = true;	
		osoption->optimization = new OptimizationOption();
	}
}; 

optimizationattlist: | optimizationattlist optimizationatt;

optimizationatt: optimizationnvar | optimizationncon | optimizationnobj;

optimizationnvar: NUMBEROFVARIABLESATT QUOTE INTEGER QUOTE
{	osoption->optimization->numberOfVariables = $3;
};

optimizationncon: NUMBEROFCONSTRAINTSATT QUOTE INTEGER QUOTE
{	osoption->optimization->numberOfConstraints = $3;
};

optimizationnobj: NUMBEROFOBJECTIVESATT QUOTE INTEGER QUOTE
{	osoption->optimization->numberOfObjectives = $3;
};

optimizationbody: GREATERTHAN optimizationcontent OPTIMIZATIONEND | ENDOFELEMENT;

optimizationcontent: variables objectives constraints solveroptions
{printf("\n%s\n","Finished <optimization element>");};

variables: | variablesstart numberofothervariablesatt restofvariables;

variablesstart: VARIABLESSTART 
{	osoption->optimization->variables = new VariableOption();
};

numberofothervariablesatt: | NUMBEROFOTHERVARIABLEOPTIONSATT QUOTE INTEGER QUOTE
{	if ($3 < 0) osolerror (NULL, osoption, parserData, "number of <other> variable options cannot be negative");
	osoption->optimization->variables->numberOfOtherVariableOptions = $3;
	osoption->optimization->variables->other = new OtherVariableOption*[$3];
	for (int i= 0; i < $3; i++)
		osoption->optimization->variables->other[i] = new OtherVariableOption();
};

restofvariables: GREATERTHAN initialvariablevalues initialvariablevaluesstring othervariableoptions VARIABLESEND
   | ENDOFELEMENT;

initialvariablevalues: | INITIALVARIABLEVALUESSTART numberofvar GREATERTHAN varlist INITIALVARIABLEVALUESEND;

numberofvar: NUMBEROFVARATT QUOTE INTEGER QUOTE 
{	if ($3 <= 0) osolerror (NULL, osoption, parserData, "number of <var> elements must be positive");
	osoption->optimization->variables->initialVariableValues = new InitVariableValues();
	osoption->optimization->variables->initialVariableValues->numberOfVar = $3;
	osoption->optimization->variables->initialVariableValues->var = new InitVarValue*[$3];
	for (int i = 0; i < $3; i++)
		osoption->optimization->variables->initialVariableValues->var[i] = new InitVarValue();
};

varlist: | varlist initvarvalue;

initvarvalue: varstart initvarvalueattlist initvarvalueend
{	if (!parserData->idxAttributePresent)
		osolerror (NULL, osoption, parserData, "variable index required");
	parserData->idxAttributePresent = false;
	parserData->valAttributePresent = false;
	parserData->numberOfVar++;
};

varstart: VARSTART 
{	if (parserData->numberOfVar >= osoption->optimization->variables->initialVariableValues->numberOfVar)
		osolerror(NULL, osoption, parserData, "too many initial variable values");
};

initvarvalueattlist: | initvarvalueattlist initvarvalueatt;

initvarvalueatt: initvarvalueidxatt | initvarvaluevalueatt;

initvarvalueidxatt: IDXATT QUOTE INTEGER QUOTE
{	if (parserData->idxAttributePresent)
		osolerror (NULL, osoption, parserData, "only one variable index allowed");
	parserData->idxAttributePresent = true;
	if ($3 < 0) osolerror (NULL, osoption, parserData, "variable index must be nonnegative");
	if (parserData->numberOfVariablesPresent)
	{	if ($3 >= parserData->numberOfVariables)
			osolerror (NULL, osoption, parserData, "variable index exceeds upper limit");
	};
	osoption->optimization->variables->initialVariableValues->var[parserData->numberOfVar]->idx = $3;  
};

initvarvaluevalueatt: VALUEATT ATTRIBUTETEXT 
{	if (parserData->valAttributePresent)
		osolerror (NULL, osoption, parserData, "only one variable value allowed");
	parserData->valAttributePresent = true;
	osoption->optimization->variables->initialVariableValues->var[parserData->numberOfVar]->value = strtod($2, NULL);  
}
QUOTE;

initvarvalueend: GREATERTHAN VAREND | ENDOFELEMENT;


initialvariablevaluesstring: | INITIALVARIABLEVALUESSTRINGSTART numberofvarstr GREATERTHAN 
    varstrlist INITIALVARIABLEVALUESSTRINGEND;

numberofvarstr: NUMBEROFVARATT QUOTE INTEGER QUOTE 
{	if ($3 <= 0) osolerror (NULL, osoption, parserData, "number of <var> elements must be positive");
	osoption->optimization->variables->initialVariableValuesString = new InitVariableValuesString ();
	osoption->optimization->variables->initialVariableValuesString->numberOfVar = $3;
	osoption->optimization->variables->initialVariableValuesString->var = new InitVarValueString*[$3];
	for (int i = 0; i < $3; i++)
		osoption->optimization->variables->initialVariableValuesString->var[i] = new InitVarValueString ();
};

varstrlist: | varstrlist initvarstrvalue;

initvarstrvalue: varstrstart initvarstrvalueattlist initvarstrvalueend
{	if (!parserData->idxAttributePresent)
		osolerror (NULL, osoption, parserData, "variable index required");
	parserData->idxAttributePresent = false;
	parserData->valAttributePresent = false;
	parserData->numberOfVarStr++;
};

varstrstart: VARSTART
{	if (parserData->numberOfVarStr >= osoption->optimization->variables->initialVariableValuesString->numberOfVar)
		osolerror(NULL, osoption, parserData, "too many initial variable strings");
};

initvarstrvalueattlist: | initvarstrvalueattlist initvarstrvalueatt;

initvarstrvalueatt: initvarstrvalueidxatt | initvarstrvaluevalueatt;

initvarstrvalueidxatt: IDXATT QUOTE INTEGER QUOTE
{	if (parserData->idxAttributePresent)
		osolerror (NULL, osoption, parserData, "only one variable index allowed");
	parserData->idxAttributePresent = true;
	if ($3 < 0) osolerror (NULL, osoption, parserData, "variable index must be nonnegative");
	if (parserData->numberOfVariablesPresent)
	{	if ($3 >= parserData->numberOfVariables)
			osolerror (NULL, osoption, parserData, "variable index exceeds upper limit");
	};
	osoption->optimization->variables->initialVariableValuesString->var[parserData->numberOfVarStr]->idx = $3;  
};

initvarstrvaluevalueatt: VALUEATT ATTRIBUTETEXT 
{	if (parserData->valAttributePresent)
		osolerror (NULL, osoption, parserData, "only one variable index allowed");
	parserData->valAttributePresent = true;
	osoption->optimization->variables->initialVariableValuesString->var[parserData->numberOfVarStr]->value = strtod($2, NULL);  
}
QUOTE;

initvarstrvalueend: GREATERTHAN VAREND | ENDOFELEMENT;


othervariableoptions: | OTHERSTART othervariableoptionsattlist restofothervariableoptions;

othervariableoptionsattlist: | othervariableoptionsattlist othervariableoptionsatt;

othervariableoptionsatt: 
     othervariableoptionnumberofvar 
   | othervariableoptionname
   | othervariableoptionvalue
   | othervariableoptionsolver
   | othervariableoptioncategory
   | othervariableoptiontype
   | othervariableoptiondescription;

othervariableoptionnumberofvar: NUMBEROFVARATT QUOTE INTEGER 
{
}QUOTE;

othervariableoptionname: NAMEATT ATTRIBUTETEXT 
{
}QUOTE;

othervariableoptionvalue: VALUEATT ATTRIBUTETEXT 
{
}QUOTE;

othervariableoptionsolver: SOLVERATT ATTRIBUTETEXT 
{
}QUOTE;

othervariableoptioncategory: CATEGORYATT ATTRIBUTETEXT 
{
}QUOTE;

othervariableoptiontype: TYPEATT ATTRIBUTETEXT 
{
}QUOTE;

othervariableoptiondescription: DESCRIPTIONATT ATTRIBUTETEXT 
{
}QUOTE;


restofothervariableoptions: GREATERTHAN othervariableoptionsvarlist OTHEREND;

othervariableoptionsvarlist: VARSTART othervaroptionattlist othervaroptionend;

othervaroptionattlist: | othervaroptionattlist othervaroptionatt;

othervaroptionatt: 
     othervaroptionidx
   | othervaroptionvalue
   | othervaroptionlbvalue
   | othervaroptionubvalue;

othervaroptionidx: IDXATT INTEGER QUOTE
{	
};

othervaroptionvalue: VALUEATT ATTRIBUTETEXT QUOTE
{
};

othervaroptionlbvalue: LBVALUEATT ATTRIBUTETEXT QUOTE
{
};

othervaroptionubvalue: UBVALUEATT ATTRIBUTETEXT QUOTE
{
};

othervaroptionend: GREATERTHAN VAREND | ENDOFELEMENT;


objectives: | objectivesstart numberofotherobjectivesatt restofobjectives;

objectivesstart: OBJECTIVESSTART
{	osoption->optimization->objectives = new ObjectiveOption();
};

numberofotherobjectivesatt: | NUMBEROFOTHEROBJECTIVEOPTIONSATT QUOTE INTEGER QUOTE
{	if ($3 < 0) osolerror (NULL, osoption, parserData, "number of <other> objective options cannot be negative");
        osoption->optimization->objectives->numberOfOtherObjectiveOptions = $3;
	osoption->optimization->objectives->other = new OtherObjectiveOption*[$3];
	for (int i= 0; i < $3; i++)
		osoption->optimization->objectives->other[i] = new OtherObjectiveOption();
};

restofobjectives: GREATERTHAN initialobjectivevalues initialobjectivebounds otherobjectiveoptions OBJECTIVESEND
   | ENDOFELEMENT;

initialobjectivevalues: | INITIALOBJECTIVEVALUESSTART numberofobjval GREATERTHAN
    objvaluelist INITIALOBJECTIVEVALUESEND;

numberofobjval: NUMBEROFOBJATT QUOTE INTEGER QUOTE
{	if ($3 <= 0) osolerror (NULL, osoption, parserData, "number of <obj> elements must be positive");
	osoption->optimization->objectives->initialObjectiveValues = new InitObjectiveValues();
	osoption->optimization->objectives->initialObjectiveValues->numberOfObj = $3;
	osoption->optimization->objectives->initialObjectiveValues->obj = new InitObjValue*[$3];
	for (int i = 0; i < $3; i++)
		osoption->optimization->objectives->initialObjectiveValues->obj[i] = new InitObjValue();
};

objvaluelist: | objvaluelist initobjvalue;

initobjvalue: objvaluestart initobjvalueattlist initobjvalueend
{	if (!parserData->idxAttributePresent)
		osolerror (NULL, osoption, parserData, "variable index required");
	parserData->idxAttributePresent = false;
	parserData->valAttributePresent = false;
	parserData->numberOfObjValues++;
};

objvaluestart: OBJSTART
{	if (parserData->numberOfObjValues >= osoption->optimization->objectives->initialObjectiveValues->numberOfObj)
		osolerror(NULL, osoption, parserData, "too many initial objective values");
};

initobjvalueattlist: | initobjvalueattlist initobjvalueatt;

initobjvalueatt: initobjvalueidxatt | initobjvaluevalueatt;

initobjvalueidxatt: IDXATT QUOTE INTEGER QUOTE
{	if (parserData->idxAttributePresent)
		osolerror (NULL, osoption, parserData, "only one objective index allowed");
	parserData->idxAttributePresent = true;
	if ($3 >= 0) osolerror (NULL, osoption, parserData, "objective index must be negative");
	if (parserData->numberOfObjectivesPresent)
	{	if (-$3 > parserData->numberOfObjectives)
			osolerror (NULL, osoption, parserData, "objective index out of range");
	};
	osoption->optimization->objectives->initialObjectiveValues->obj[parserData->numberOfObjValues]->idx = $3;  
};


initobjvaluevalueatt: VALUEATT ATTRIBUTETEXT 
{	if (parserData->valAttributePresent)
		osolerror (NULL, osoption, parserData, "only one objective value allowed");
	parserData->valAttributePresent = true;
	osoption->optimization->objectives->initialObjectiveValues->obj[parserData->numberOfObjValues]->value = strtod($2, NULL);  
}
QUOTE;

initobjvalueend: GREATERTHAN OBJEND | ENDOFELEMENT;


initialobjectivebounds: | INITIALOBJECTIVEBOUNDSSTART numberofobjbounds GREATERTHAN
    objboundlist INITIALOBJECTIVEBOUNDSEND;

numberofobjbounds: NUMBEROFOBJATT QUOTE INTEGER QUOTE 
{	if ($3 <= 0) osolerror (NULL, osoption, parserData, "number of <obj> elements must be positive");
	osoption->optimization->objectives->initialObjectiveBounds = new InitObjectiveBounds();
	osoption->optimization->objectives->initialObjectiveBounds->numberOfObj = $3;
	osoption->optimization->objectives->initialObjectiveBounds->obj = new InitObjBound*[$3];
	for (int i = 0; i < $3; i++)
		osoption->optimization->objectives->initialObjectiveBounds->obj[i] = new InitObjBound ();
};

objboundlist: | objboundlist initobjbound;

initobjbound: objboundstart initobjboundattlist initobjboundend
{	if (!parserData->idxAttributePresent)
		osolerror (NULL, osoption, parserData, "variable index required");
	parserData->idxAttributePresent = false;
	parserData->lbvalAttributePresent = false;
	parserData->ubvalAttributePresent = false;
	parserData->numberOfObjBounds++;
};

objboundstart: OBJSTART 
{	if (parserData->numberOfObjBounds >= osoption->optimization->objectives->initialObjectiveBounds->numberOfObj)
		osolerror(NULL, osoption, parserData, "too many initial objective bounds");
};

initobjboundattlist: | initobjboundattlist initobjboundatt;

initobjboundatt: initobjboundidxatt | initobjvaluelowerboundatt | initobjvalueupperboundatt;

initobjboundidxatt: IDXATT QUOTE INTEGER QUOTE
{	if (parserData->idxAttributePresent)
		osolerror (NULL, osoption, parserData, "only one objective index allowed");
	parserData->idxAttributePresent = true;
	if ($3 >= 0) osolerror (NULL, osoption, parserData, "objective index must be negative");
	if (parserData->numberOfObjectivesPresent)
	{	if (-$3 > parserData->numberOfObjectives)
			osolerror (NULL, osoption, parserData, "objective index out of range");
	};
	osoption->optimization->objectives->initialObjectiveBounds->obj[parserData->numberOfObjBounds]->idx = $3;  
};

initobjvaluelowerboundatt: LBVALUEATT ATTRIBUTETEXT 
{	if (parserData->lbvalAttributePresent)
		osolerror (NULL, osoption, parserData, "only one objective lower bound allowed");
	parserData->lbvalAttributePresent = true;
	osoption->optimization->objectives->initialObjectiveBounds->obj[parserData->numberOfObjBounds]->lbValue = strtod($2, NULL);  
}
QUOTE;

initobjvalueupperboundatt: UBVALUEATT ATTRIBUTETEXT 
{	if (parserData->ubvalAttributePresent)
		osolerror (NULL, osoption, parserData, "only one objective upper bound allowed");
	parserData->ubvalAttributePresent = true;
	osoption->optimization->objectives->initialObjectiveBounds->obj[parserData->numberOfObjBounds]->ubValue = strtod($2, NULL);  
}
QUOTE;

initobjboundend: GREATERTHAN OBJEND | ENDOFELEMENT;


otherobjectiveoptions: | OTHERSTART otherobjectiveoptionsattlist restofotherobjectiveoptions;

otherobjectiveoptionsattlist: | otherobjectiveoptionsattlist otherobjectiveoptionsatt;

otherobjectiveoptionsatt: 
     otherobjectiveoptionnumberofvar 
   | otherobjectiveoptionname
   | otherobjectiveoptionvalue
   | otherobjectiveoptionsolver
   | otherobjectiveoptioncategory
   | otherobjectiveoptiontype
   | otherobjectiveoptiondescription;

otherobjectiveoptionnumberofvar: NUMBEROFOBJATT QUOTE INTEGER QUOTE
{
};

otherobjectiveoptionname: NAMEATT ATTRIBUTETEXT QUOTE
{
};

otherobjectiveoptionvalue: VALUEATT ATTRIBUTETEXT QUOTE
{
};

otherobjectiveoptionsolver: SOLVERATT ATTRIBUTETEXT QUOTE
{
};

otherobjectiveoptioncategory: CATEGORYATT ATTRIBUTETEXT QUOTE
{
};

otherobjectiveoptiontype: TYPEATT ATTRIBUTETEXT QUOTE
{
};

otherobjectiveoptiondescription: DESCRIPTIONATT ATTRIBUTETEXT QUOTE
{
};


restofotherobjectiveoptions: GREATERTHAN otherobjectiveoptionsvarlist OTHEREND;

otherobjectiveoptionsvarlist: OBJSTART otherobjoptionattlist otherobjoptionend;

otherobjoptionattlist: | otherobjoptionattlist otherobjoptionatt;

otherobjoptionatt: 
     otherobjoptionidx
   | otherobjoptionvalue
   | otherobjoptionlbvalue
   | otherobjoptionubvalue;

otherobjoptionidx: IDXATT ATTRIBUTETEXT QUOTE
{	
};

otherobjoptionvalue: VALUEATT ATTRIBUTETEXT QUOTE
{
};

otherobjoptionlbvalue: LBVALUEATT ATTRIBUTETEXT QUOTE
{
};

otherobjoptionubvalue: UBVALUEATT ATTRIBUTETEXT QUOTE
{
};

otherobjoptionend: GREATERTHAN VAREND | ENDOFELEMENT;


constraints: | constraintsstart numberofotherconstraintsatt restofconstraints;

constraintsstart: CONSTRAINTSSTART 
{	osoption->optimization->constraints = new ConstraintOption();
};

numberofotherconstraintsatt: | NUMBEROFOTHERCONSTRAINTOPTIONSATT QUOTE INTEGER QUOTE
{	if ($3 < 0) osolerror (NULL, osoption, parserData, "number of <other> constraint options cannot be negative");
        osoption->optimization->constraints->numberOfOtherConstraintOptions = $3;
	osoption->optimization->constraints->other = new OtherConstraintOption*[$3];
	for (int i= 0; i < $3; i++)
		osoption->optimization->constraints->other[i] = new OtherConstraintOption();
};

restofconstraints: GREATERTHAN initialconstraintvalues initialdualvalues
   otherconstraintoptions CONSTRAINTSEND
   | ENDOFELEMENT;

initialconstraintvalues: | INITIALCONSTRAINTVALUESSTART numberofconval GREATERTHAN 
    conlist INITIALCONSTRAINTVALUESEND;

numberofconval: NUMBEROFCONATT QUOTE INTEGER QUOTE 
{	if ($3 <= 0) osolerror (NULL, osoption, parserData, "number of <con> elements must be positive");
	osoption->optimization->constraints->initialConstraintValues = new InitConstraintValues();
	osoption->optimization->constraints->initialConstraintValues->numberOfCon = $3;
	osoption->optimization->constraints->initialConstraintValues->con = new InitConValue*[$3];
	for (int i = 0; i < $3; i++)
		osoption->optimization->constraints->initialConstraintValues->con[i] = new InitConValue();
};

conlist: | conlist initconvalue;

initconvalue: constart initconvalueattlist initconvalueend
{	if (!parserData->idxAttributePresent)
		osolerror (NULL, osoption, parserData, "variable index required");
	parserData->idxAttributePresent = false;
	parserData->valAttributePresent = false;
	parserData->numberOfCon++;
};

constart: CONSTART
{	if (parserData->numberOfCon >= osoption->optimization->constraints->initialConstraintValues->numberOfCon)
		osolerror(NULL, osoption, parserData, "too many initial constraint values");
};

initconvalueattlist: | initconvalueattlist initconvalueatt;

initconvalueatt: initconvalueidxatt | initconvaluevalueatt;

initconvalueidxatt: IDXATT QUOTE INTEGER QUOTE
{	if (parserData->idxAttributePresent)
		osolerror (NULL, osoption, parserData, "only one constraint index allowed");
	parserData->idxAttributePresent = true;
	if ($3 < 0) osolerror (NULL, osoption, parserData, "constraint index must be nonnegative");
	if (parserData->numberOfConstraintsPresent)
	{	if ($3 > parserData->numberOfConstraints)
			osolerror (NULL, osoption, parserData, "constraint index out of range");
	};
	osoption->optimization->constraints->initialConstraintValues->con[parserData->numberOfCon]->idx = $3;  
};


initconvaluevalueatt: VALUEATT ATTRIBUTETEXT 
{	if (parserData->valAttributePresent)
		osolerror (NULL, osoption, parserData, "only one constraint value allowed");
	parserData->valAttributePresent = true;
	osoption->optimization->constraints->initialConstraintValues->con[parserData->numberOfCon]->value = strtod($2, NULL);  
}
QUOTE;

initconvalueend: GREATERTHAN CONEND | ENDOFELEMENT;


initialdualvalues: | INITIALDUALVALUESSTART numberofduals GREATERTHAN 
    duallist INITIALDUALVALUESEND;

numberofduals: NUMBEROFCONATT QUOTE INTEGER QUOTE 
{	if ($3 <= 0) osolerror (NULL, osoption, parserData, "number of <con> elements must be positive");
	osoption->optimization->constraints->initialDualValues = new InitDualVariableValues();
	osoption->optimization->constraints->initialDualValues->numberOfCon = $3;
	osoption->optimization->constraints->initialDualValues->con = new InitDualVarValue*[$3];
	for (int i = 0; i < $3; i++)
		osoption->optimization->constraints->initialDualValues->con[i] = new InitDualVarValue ();
};

duallist: | duallist initdualvalue;

initdualvalue: dualstart initdualvalueattlist initdualvalueend
{	if (!parserData->idxAttributePresent)
		osolerror (NULL, osoption, parserData, "constraint index required");
	parserData->idxAttributePresent = false;
	parserData->lbvalAttributePresent = false;
	parserData->ubvalAttributePresent = false;
	parserData->numberOfDuals++;
};

dualstart: CONSTART 
{	if (parserData->numberOfDuals >= osoption->optimization->constraints->initialDualValues->numberOfCon)
		osolerror(NULL, osoption, parserData, "too many initial dual variable bounds");
};

initdualvalueattlist: | initdualvalueattlist initdualvalueatt;

initdualvalueatt: initdualvalueidxatt | initdualvaluelowerboundatt | initdualvalueupperboundatt;

initdualvalueidxatt: IDXATT QUOTE INTEGER QUOTE
{	if (parserData->idxAttributePresent)
		osolerror (NULL, osoption, parserData, "only one constraint index allowed");
	parserData->idxAttributePresent = true;
	if ($3 < 0) osolerror (NULL, osoption, parserData, "constraint index must be nonnegative");
	if (parserData->numberOfConstraintsPresent)
	{	if ($3 > parserData->numberOfConstraints)
			osolerror (NULL, osoption, parserData, "constraint index out of range");
	};
	osoption->optimization->constraints->initialDualValues->con[parserData->numberOfDuals]->idx = $3;  
};

initdualvaluelowerboundatt: LBVALUEATT ATTRIBUTETEXT 
{	if (parserData->lbvalAttributePresent)
		osolerror (NULL, osoption, parserData, "only one dual variable lower bound allowed");
	parserData->lbvalAttributePresent = true;
	osoption->optimization->constraints->initialDualValues->con[parserData->numberOfDuals]->lbValue = strtod($2, NULL);  
}
QUOTE;

initdualvalueupperboundatt: UBVALUEATT ATTRIBUTETEXT 
{	if (parserData->ubvalAttributePresent)
		osolerror (NULL, osoption, parserData, "only one dual variable upper bound allowed");
	parserData->ubvalAttributePresent = true;
	osoption->optimization->constraints->initialDualValues->con[parserData->numberOfDuals]->ubValue = strtod($2, NULL);  
}
QUOTE;


initdualvalueend: GREATERTHAN CONEND | ENDOFELEMENT;


otherconstraintoptions: | OTHERSTART otherconstraintoptionsattlist restofotherconstraintoptions;

otherconstraintoptionsattlist: | otherconstraintoptionsattlist otherconstraintoptionsatt;

otherconstraintoptionsatt: 
     otherconstraintoptionnumberofvar 
   | otherconstraintoptionname
   | otherconstraintoptionvalue
   | otherconstraintoptionsolver
   | otherconstraintoptioncategory
   | otherconstraintoptiontype
   | otherconstraintoptiondescription;

otherconstraintoptionnumberofvar: NUMBEROFCONATT QUOTE INTEGER QUOTE
{
};

otherconstraintoptionname: NAMEATT ATTRIBUTETEXT QUOTE
{
};

otherconstraintoptionvalue: VALUEATT ATTRIBUTETEXT QUOTE
{
};

otherconstraintoptionsolver: SOLVERATT ATTRIBUTETEXT QUOTE
{
};

otherconstraintoptioncategory: CATEGORYATT ATTRIBUTETEXT QUOTE
{
};

otherconstraintoptiontype: TYPEATT ATTRIBUTETEXT QUOTE
{
};

otherconstraintoptiondescription: DESCRIPTIONATT ATTRIBUTETEXT QUOTE
{
};


restofotherconstraintoptions: GREATERTHAN otherconstraintoptionsvarlist OTHEREND;

otherconstraintoptionsvarlist: CONSTART otherconoptionattlist otherconoptionend;

otherconoptionattlist: | otherconoptionattlist otherconoptionatt;

otherconoptionatt: 
     otherconoptionidx
   | otherconoptionvalue
   | otherconoptionlbvalue
   | otherconoptionubvalue;

otherconoptionidx: IDXATT ATTRIBUTETEXT QUOTE
{	
};

otherconoptionvalue: VALUEATT ATTRIBUTETEXT QUOTE
{
};

otherconoptionlbvalue: LBVALUEATT ATTRIBUTETEXT QUOTE
{
};

otherconoptionubvalue: UBVALUEATT ATTRIBUTETEXT QUOTE
{
};

otherconoptionend: GREATERTHAN CONEND | ENDOFELEMENT;


solveroptions: | SOLVEROPTIONSSTART numberofsolveroptionsatt GREATERTHAN solveroptionlist SOLVEROPTIONSEND;

numberofsolveroptionsatt: NUMBEROFSOLVEROPTIONSATT QUOTE INTEGER QUOTE
{
};

solveroptionlist: | solveroptionlist solveroption;

solveroption: SOLVEROPTIONSTART solveroptionattlist solveroptionend;

solveroptionattlist: | solveroptionattlist solveroptionatt;

solveroptionatt: 
     solveroptionname
   | solveroptionvalue
   | solveroptionsolver
   | solveroptioncategory
   | solveroptiontype
   | solveroptiondescription;

solveroptionname: NAMEATT ATTRIBUTETEXT QUOTE
{
};

solveroptionvalue: VALUEATT ATTRIBUTETEXT QUOTE
{
};

solveroptionsolver: SOLVERATT ATTRIBUTETEXT QUOTE
{
};

solveroptioncategory: CATEGORYATT ATTRIBUTETEXT QUOTE
{
};

solveroptiontype: TYPEATT ATTRIBUTETEXT QUOTE
{
};

solveroptiondescription: DESCRIPTIONATT ATTRIBUTETEXT QUOTE
{
};


solveroptionend: GREATERTHAN SOLVEROPTIONEND | ENDOFELEMENT;




//quote: xmlWhiteSpace QUOTE;
//
//xmlWhiteSpaceChar: ' ' 
//				| '\t'
//				| '\r'
//				| '\n' ;
//				
//xmlWhiteSpace: 
//			| xmlWhiteSpace xmlWhiteSpaceChar ;
			

%%

void osolerror(YYLTYPE* mytype, OSOption *osoption, OSoLParserData* parserData, const char* errormsg )
{
	std::ostringstream outStr;
	std::string error = errormsg;
	error = "Input is either not valid or well formed: "  + error;
	outStr << error << std::endl;
	outStr << "See line number: " << osolget_lineno( scanner) << std::endl; 
	outStr << "The offending text is: " << osolget_text ( scanner ) << std::endl; 
	error = outStr.str();
	//printf("THIS DID NOT GET DESTROYED:   %s\n", parserData->errorText);
	//if( (parserData->errorText != NULL) &&  (strlen(parserData->errorText) > 0) ) free(  parserData->errorText);
	//osollex_destroy( scanner);
	throw ErrorClass( error);
} //end osolerror

void  yygetOSOption(const char *parsestring, OSOption *osoption, OSoLParserData *parserData) throw(ErrorClass){
	try{
		osol_scan_string( parsestring, scanner);
		osolset_lineno (1 , scanner );
		//
		// call the Bison parser
		//
		if(  osolparse( osoption,  parserData) != 0) {
			//osollex_destroy(scanner);
		  	throw ErrorClass(  "Error parsing the OSoL file");
		 }
	}
	catch(const ErrorClass& eclass){
		throw ErrorClass(  eclass.errormsg); 
	}
} //end yygetOSOption

