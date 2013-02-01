/**
 * @file dig.c
 *
 * @author Perry Spagnola 
 * @date 2/24/11 - created
 * @version 1.0
 * @brief Functions to support device information for a host.
 * @details dig (domain information groper) is a network administration command-line tool 
 * for querying Domain Name System (DNS) name servers. Needed to get device info for the 
 * host. These functions build the dig command string, send the command, read its results,
 * and extract the formatted model id of the specified host.
 * 
 * @copyright Copyright 2011 Perry M. Spagnola. All rights reserved.
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 */

#include "wol_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define mDNS_BCAST_ADDRESS  "224.0.0.251"
#define mDNS_PORT  "5353"
#define mDNS_QUERY  "TXT"
#define DEVICE_INFO_SERVICE  "_device-info._tcp"
#define LOCAL_DOMAIN  "local"
#define DOT_DELIMITER  "."
#define SPACE  " "


/**
 * Function to build the dig command. Used by the deviceInfoForHost() function
 * to build an mDNS query.
 *
 * @param server - the server address
 * @param hostName - the name of the host
 * @param port - the network port
 * @param serviceType - the type of the service
 * @param domain - the domain
 * @param query - the mDNS query
 * @param cmd - the command string that will be populated by the function
 *
 * @return zero (0) if successful, there is no failure mode for this function
 * @retval 0 - success
 */
int buildDigCmd(char *server, char *hostName, char *port, char *serviceType, char *domain, char *query, char *cmd)
{
    /**
     * Initialize the dig comamnd string with the "dig" command.
     */
	strcpy(cmd, "dig @");
	
	if (server == NULL || strcmp(server, "") == 0) {
        /** 
         * If no server is specified, concatenate the mDNS broadcast address to
         * the command string.
         */
		strcat(cmd, mDNS_BCAST_ADDRESS);
	}
	else {
        /**
         * Else, concatenate the server address.
         */
		strcat(cmd, server);
	}
	
    /**
     * Concatenate the rest of the command parameters to the command string.
     */
	strcat(cmd, SPACE);
	strcat(cmd, "-p");
	strcat(cmd, port);
	strcat(cmd, SPACE);
	strcat(cmd, hostName);
	strcat(cmd, DOT_DELIMITER);
	strcat(cmd, serviceType);
	strcat(cmd, DOT_DELIMITER);
	strcat(cmd, domain);
	strcat(cmd, SPACE);
	strcat(cmd, query);
    
    /**
     * Reached the end of the function. There is no error condition for this
     * function. Return zero (0) for success.
     */
	return 0;
}


/**
 * Function to format the argument specified host model identifier.
 * 
 * @param unformattedModelID - pointer to the unformated model identifier string
 * @param formattedModelID - pointer to the string to put the formatted model identifier in
 * @return the success or failure status of the function
 * @retval 0 - success
 * @retval 1 - failure
 */
int formatModelIdentifier(char *unformattedModelID, char *formattedModelID)
{
	int returnValue = 1; /** initialize to failure condition */
	char delims[] = "\"=";  /** initialize the delimiters for parsing the model identifier from its label */
	char *token = NULL;  /** initialize the current token */
	int modelLabel = 0;
	
    /** Tokenize the unformated model identifier string. */
	token = strtok( unformattedModelID, delims );
	
	while( token != NULL ) {
        /**
         * Iterate, tokenizing the unformatted model ID string, and extract the value 
         * for the "model" key.
         */
		// DEBUG: printf( "result is \"%s\"\n", token );
		if (strcmp(token, "model")) {
            /** 
             * If we found the "model" key, concatenate the model value to
             * the formatted model ID string. Set the return value to 0 for
             *  success, and break out of the loop.
             */
			if (modelLabel == 1) {
				strcat(formattedModelID, token);
				returnValue = 0;
				break;
			}
		}
		else {
			/* model label found */
			modelLabel = 1;
		}
		
		token = strtok( NULL, delims );
	}
	
    /** Retrun the success or error value. */
	return returnValue;
}


/**
 * Function to retrieve the device information for the argument specified
 * host. Retrieves device info for the specified host using a "dig" command.
 *
 * @param host - the host to retrieve the device info for
 * @param hostIP - the IP address of the host
 * @param devInfo - the string populated with the retrieved device info
 *
 * @return the success or failure status of the function
 * @retval 0 - success
 * @retval 1 - failure
 */
int deviceInfoForHost(char *host, char *hostIP, char *devInfo)
{
	FILE *in;
	extern FILE *popen();
	char buff[512];
	char address[512];
	char command[1024] = "";
	int returnValue = 0;
	
	if (hostIP == NULL || strcmp(hostIP, "") == 0) {
        /** Build host domain address. If IP missing or NULL set to NULL. */
		strcpy(address, "");
	}
	else {
        /** Else, use the host IP argument. */
		strcpy(address, hostIP);
	}

	
	/** Build the host and service specific dig command. */
	buildDigCmd(address, host, mDNS_PORT, DEVICE_INFO_SERVICE, LOCAL_DOMAIN, mDNS_QUERY, command);
	
	// DEBUG: printf("dig command: %s\n", command);
	
	/** Open a pipe and execute the "dig" command. */
	if (!(in = popen(command, "r"))) {
        /** If the "dig" command fails, set the return value to 1. */
		returnValue = 1;
	}
	else {
		/** Else, read the "dig" command output, one line at a time */
		while (fgets(buff, sizeof(buff), in) != NULL ) {
			// DEBUG: printf("Output: %s", buff);
			
			/**
			 * Break the command output string into tokens separated
			 * by white space.
			 */
			char *token;
			token = strtok (buff, " ");
			
			/**
			 * Search for the H/W model identifier. Look for the character 
			 * "=" It will be the only instance of the sub-string token.
			 */
			char modelID[64] = "";
			int found = 0;
			while (token != NULL)
			{
				// DEBUG: printf ("%s\n",token);
				if (strchr(token, '=') != 0) {
					// DEBUG: printf ("found model=: %s\n",token);
					/* found the H/W model identifier */
					strcpy(modelID, token);
					found = 1;
					break;
				}
				token = strtok (NULL, " ");
			}
			
			if (!found) {
                /**
                 * If the H/W model identifier is not found, assign an empty
                 * string to the device info string, and return 1, an error.
                 */
				strcpy(devInfo, "");
				returnValue = 1;
			}
			else {
				/**
				 * Else, found the model identifier. Call the formatModelIdentifier()
                 * function, and break out of the loop. It will set the device info
                 * string, and the return value.
				 */
				returnValue = formatModelIdentifier(modelID, devInfo);
				break;
			}
		}
	}
	
	/** Close the pipe. */
	pclose(in);
	
    /** Return the success or error results. */
	return returnValue;	
}
	


