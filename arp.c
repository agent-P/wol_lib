/**
 * @file arp.c
 *
 * @author Perry Spagnola 
 * @date 2/19/11 - created
 * @version 1.0
 * @brief Sends "arp" and "ping" commands, and retrieves and processes the responses.
 * @details The "arp" and "ping" commands are invoked for very specific purposes. Please
 * see the function descriptions for details.
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

/**
 * Sends a single ping packet to the specified IP address. 
 * Calls the ping command: <code>ping -c 1 ip-address</code>.
 *
 * @param ipAddr - the IP address to ping.
 *
 * @return the success or error of the ping command. The specific error cannot be retrieved.
 * @retval 0 - success
 * @retval 1 - error
 */
int pingIP(char *ipAddr)
{
	FILE *in;
	extern FILE *popen();
	char buff[512];
	char command[512] = "ping -c 1 ";
	char errorCondition[512] = "1 packets transmitted, 0 packets received";
	int returnValue = 0;
	
	/**
     * Build the IP specific PING command by concatenating the IP address
     * to the command buffer initialized with the "ping -c 1" command string.
     */
	strcat(command, ipAddr);
	
	/** 
	 * <code>popen()</code> creates a pipe so we can read the output
	 * of the ping command we are invoking. If it fails, the return value is
     * set to one (1), an error.
	 */
	if (!(in = popen(command, "r"))) {
		returnValue = 1;
	}
	else {
		/** 
		 * Else, read the output of the ping command, one line at a time.
		 */
		while (fgets(buff, sizeof(buff), in) != NULL ) {
			//printf("Output: %s", buff);
			/**
			 * Look for the error condition: "1 packets transmitted, 0 packets received". 
             * If found, set the return value to one (1), an error, and break out of the loop.
			 */
			if (strncmp(errorCondition, buff, strlen(errorCondition)) == 0) {
				returnValue = 1;
				break;
			}
		}
	}
					
	/**
     * The function has completed. Close the pipe, and return the success or error value.
     */
	pclose(in);	
	
	return returnValue;
}


/**
 * Retrieves the MAC address for the specified IP address.
 * Sends the arp command: <code>arp ip-address</code>. Searches the return
 * strings for a colon, and returns the contiguous string containing the colon.
 * Note: The arp command does not return a fully formatted MAC address string.
 * Leading zeroes are missing from the octets, as an example. The MAC address
 * string is formatted by an internal call to <code>formatMAC()</code>.
 *
 * @param ipAddr - the IP address to retrieve the MAC address for.
 * @param macAddr - a pointer to the buffer to write the formatted MAC address into.
 *
 * @return the success or error of the arp command. The specific error cannot be retrieved.
 * @retval 0 - success
 * @retval 1 - error
 */
int macForIP(char *ipAddr, char *macAddr)
{
	FILE *in;
	extern FILE *popen();
	char buff[512];
	char command[1024] = "arp ";
	int returnValue = 0;
	
	/**
     * Build the IP specific ARP command by concatenating the IP address
     * to the command buffer initialized with the "arp" command string.
     */
	strcat(command, ipAddr);
	
	/**
     * Execute the "arp ipAddr" command. If it fails, set the return value
     * to one (1), an error.
     */
	if (!(in = popen(command, "r"))) {
		returnValue = 1;
	}
	else {
		/**
         * Else, read the output, one line at a time.
         */
		while (fgets(buff, sizeof(buff), in) != NULL ) {
			//printf("Output: %s", buff);
			
			/**
			 * Break the command output string into tokens separated
			 * by white space.
			 */
			char * pch;
			pch = strtok (buff," ");
			
			/**
			 * Search for the MAC address. Tokenize the line, and iterate
             * through the tokens. Look for a colon. It will be the only 
             * sub-string token with a colon. WHen the ":" is found terminate
             * iterating through the tokens.
			 */
			char macAddrTemp[32] = "";
			int found = 0;
			while (pch != NULL) {
				//printf ("%s\n",pch);
				if (strchr(pch, ':') != 0) {
					/* found the MAC address */
					strcpy(macAddrTemp, pch);
					found = 1;
					break;
				}
				pch = strtok (NULL, " ");
			}
			
			if (!found) {
                /**
                 * If the MAC address is not found, set the MAC address string
                 * to: "no MAC found".
                 */
				strcpy(macAddr, "no MAC found");
			}
			else {
				/**
				 * Else, found the MAC address. Format it to make sure each
				 * octet has two hex digits. If not, add the leading 0. Set the
                 * return value to the success or error condition of the formatMAC()
                 * function, and break out of the line iterator loop.
				 */
				returnValue = formatMAC(macAddrTemp, macAddr);
				break;
			}
		}			
	}
	
	/**
     * The function is complete. Close the pipe, and return the success or error value.
     */
	pclose(in);
	
	return returnValue;	
}


/**
 * Formats the MAC address string ito the classic six two hex digit octets
 * separated by colons. Mainly looks for single digit octets and adds a
 * leading zero.
 *
 * @param unformattedMAC - the MAC address string to format
 * @param formattedMAC - a pointer to the buffer to write the formatted MAC address into.
 *
 * @return zero (0) if successful, there is no failure mode for this function
 * @retval 0 - success
 */
int formatMAC(char *unformattedMAC, char *formattedMAC) {
	
	char tempbuf[3] = ""; /* buffer for a MAC address octet */
	char delims[] = ":";  /* the delimiter for octet separation */
	char *token = NULL;  /* the current token */
	
    /**
     * Tokenize the unformatted MAC address string using ":" as a delimiter.
     */
	token = strtok( unformattedMAC, delims );
	
    /**
     * Iterate through the tokens.
     */
	while( token != NULL ) {
		// DEBUG: printf( "result is \"%s\"\n", result );
		
		if (strlen(token) == 1) {
            /**
             * If the token is only one character, add a leading zero, and
             * concatenate it to the formatted MAC address string.
             */
			// DEBUG:printf("correcting: %s\n", result);
			strcpy(tempbuf, "0");
			strcat(tempbuf, token);
			// DEBUG:printf("corrected: %s\n", tempbuf);
			
			strcat(formattedMAC, tempbuf);
		}
		else {
            /**
             * Else, simply concatenate the token as-is.
             */
			strcat(formattedMAC, token);
		}
		
		/**
         * Get the next token. If it is not NULL, concatenate the delimiter ":" 
         * to the formatted MAC address string. The token will be processed by 
         * the next iteration of the loop. If it is NULL, nothing is concatenated,
         * and the loop will terminate.
         */
		token = strtok( NULL, delims );		
		if (token) {
			strcat(formattedMAC, delims);
		}
	}
	
    /**
     * The function has completed. Return a success value. There is no failure 
     * mode for this function.
     */
	return 0;	
}
