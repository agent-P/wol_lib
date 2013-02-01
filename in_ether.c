/**
 * @file in_ether.c
 *
 * @author Perry Spagnola 
 * @date 2/8/11 - created
 * @version 1.0
 * @brief File for function to convert the MAC address
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

#include "in_ether.h"

#include <ctype.h>


/**
 * Function to convert a MAC address string (xx:xx:xx:xx:xx:xx) to a
 * hardware address string.
 *
 * @param macStr - the MAC address string to convert
 * @param hwAddr - the hardware address string to populate
 *
 * @return success or error converting the MAC address string
 * @retval 0 - success
 * @retval -1 - failure
 */
int in_ether (char *macStr, unsigned char *hwAddr)
{
    char c, *macStrOrig;
    int i;
    unsigned val;
	
    /** 
     * Initialize a pointer to the hardware address string argument
     * to be populated. 
     */
    unsigned char *ptr = hwAddr;

    /**
     * Initialize the loop counter to 0, and record the origin of the
     * MAC address string.
     */
    i = 0;
    macStrOrig = macStr;
    
    /**
     * Iterate through the values of the MAC address string until the 
     * end of the string is reached or the 6th iteration is completed.
     */
    while ((*macStr != '\0') && (i < 6)) {
        /**
         * Process the first character of the MAC address octet
         */
        val = 0;
        c = *macStr++;
        if (isdigit(c)) {
            val = c - '0';
        }
        else if (c >= 'a' && c <= 'f') {
            val = c - 'a' + 10;
        }
        else if (c >= 'A' && c <= 'F') {
            val = c - 'A' + 10;
        }
        else {
            /** 
             * If it is not a hex character (0 - f), it is an error. Return a -1. 
             */
            return (-1);
        }
        
        /**
         * Process the second character of the MAC address octet
         */
        val <<= 4;
        c = *macStr;
        if (isdigit(c)) {
            val |= c - '0';
        }
        else if (c >= 'a' && c <= 'f') {
            val |= c - 'a' + 10;
        }
        else if (c >= 'A' && c <= 'F') {
            val |= c - 'A' + 10;
        }
        else if (c == ':' || c == 0) {
            val >>= 4;
        }
        else {
            /** 
             * If it is not a hex character (0 - f), it is an error. Return a -1. 
             */
            return (-1);
        }
        
        if (c != 0) {
            macStr++;
        }
        
        /**
         * Set the hardware address character, increment the pointer, and 
         * increment the loop counter.
         */
        *ptr++ = (unsigned char) (val & 0377);
        i++;
		
        /**
         * Skip over the colon ":" octet delimiters. If on the last octet, do 
         * nothing. The loop will terminate.
         */
        if (*macStr == ':') {
            if (i != 6) {
				macStr++;
            }            
        }
    }
    
    /**
     * Last error check, if the algorithm hasn't processed 17 characters, the 
     * length of a valid MAC address string, return -1, an error. Else, return 0,
     * success.
     */
	if (macStr - macStrOrig != 17) {
        return (-1);
    } 
    else {
        return (0);
    }
}
