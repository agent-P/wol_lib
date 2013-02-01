/**
 * @file send_wol.c
 *
 * @author Perry Spagnola 
 * @date 2/8/11 - created
 * @version 1.0
 * @brief File for the function to send the Wake on LAN packet
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
#include "in_ether.h"

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>


/**
 * Function to send a "magic packet" to the argument specified MAC address.
 * The MAC address is a string in the format: <code>xx:xx:xx:xx:xx:xx</code>.
 * The magic packet is 6 x 0xff then 16 x the converted MAC address.
 *
 * @param macAddr - the MAC address string to send the magic packet to
 *
 * @return success or failure of the send attempt
 * @retval 0 - success
 * @retval -1 - failure
 */
int send_wol (char *macAddr)
{
	int i, j;
	int packet;
	struct sockaddr_in sap;
	unsigned char ethaddr[8];
	unsigned char *ptr;
	unsigned char packetBuf [128];
	int optval = 1;
	
	/** 
     * Convert the MAC address to the hardware address. If the conversion
     * fails exit the function, and return an error.
     */
	if (in_ether (macAddr, ethaddr) < 0) {
		//fprintf (stderr, "\r%s: invalid hardware address\n", Program);
		return (-1);
	}
	
	/**
     * Setup the packet socket. If the socket creation fails exit the function,
     * and return an error.
     */
	if ((packet = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		//fprintf (stderr, "\r%s: socket failed\n", Program);
		return (-1);
	}
	
	/**
     * Set socket options. If the operation fails, close the packet socket,
     * exit the function, and return an error.
     */
	if (setsockopt (packet, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof (optval)) < 0) {
		//fprintf (stderr, "\r%s: setsocket failed %s\n", Program, strerror (errno));
		close (packet);
		return (-1);
	}
	
	/** 
     * Set up broadcast address <code>0xffffffff</code>. 
     */
	sap.sin_family = AF_INET;
	sap.sin_addr.s_addr = htonl(0xffffffff);
	sap.sin_port = htons(60000);
	
	/** 
     * Build the message to send. Populate the packet buffer with:  
     * 6 x <code>0xff</code> then 16 x converted MAC address.
     */
	ptr = packetBuf;
	for (i = 0; i < 6; i++) {
		*ptr++ = 0xff;
    }
	for (j = 0; j < 16; j++) {
		for (i = 0; i < 6; i++) {
			*ptr++ = ethaddr [i];
        }
    }
	
	/**
     * Send the magic packet. If the sendto() fails, close the packet socket,
     * exit the function, and return an error.
     */
	if (sendto (packet, (char *)packetBuf, 102, 0, (struct sockaddr *)&sap, sizeof (sap)) < 0) {
		//fprintf (stderr, "\r%s: sendto failed, %s\n", Program, strerror(errno));
		close (packet);
		return (-1);
	}
    
    /**
     * Else, everthing worked. close the packet socket,
     * exit the function, and return success.
     */
	close (packet);
	//fprintf (stderr, "\r%s: magic packet sent to %s %s\n", Program, mac, host);
	
	return (0);
}