/**
 * @(#)GeneralStatus 1.0 03/14/2004
 *
 * Copyright (c) 2004
 */
package org.optimizationservices.oscommon.datastructure.osresult;

/**
*
* <p>A data structure class that corresponds to an xml element in the OSaL schema.  
*  
* </p>
* @author Robert Fourer, Jun Ma, Kipp Martin
* @version 1.0, 03/14/2004
* @since OS 1.0
*/
public class GeneralStatus {
	
	public String type = null;
	
	public String description = null;
	
	public GeneralSubstatus[] substatus = null;
		
}//GeneralStatus
