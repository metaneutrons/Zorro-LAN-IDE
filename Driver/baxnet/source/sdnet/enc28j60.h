/*
 * enc28j60.h - Microchip ENC28J60 interface
 *
 * Written by
 *  Henryk Richter <henryk.richter@gmx.net>
 * 
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#ifndef _INC_ENC28J60_H
#define _INC_ENC28J60_H

/* result values */
#define PIO_OK            0
#define PIO_NOT_FOUND     1
#define PIO_TOO_LARGE     2
#define PIO_IO_ERR        3

/* init flags */
#define PIO_INIT_FULL_DUPLEX    1
#define PIO_INIT_LOOP_BACK      2
#define PIO_INIT_BROAD_CAST     4
#define PIO_INIT_FLOW_CONTROL   8
#define PIO_INIT_MULTI_CAST     16
#define PIO_INIT_PROMISC        32

/* status flags */
#define PIO_STATUS_VERSION      0
#define PIO_STATUS_LINK_UP      1 
#define PIO_STATUS_FULLDPX      2

/* control ids */
#define PIO_CONTROL_FLOW        0

/* sigh, old gcc does not have stdint.h */
typedef unsigned char u08;
typedef unsigned short u16;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

/* API functions */
u08 enc28j60_init(const u08 macaddr[6], u08 flags);
void enc28j60_exit(void);
u08 enc28j60_setOnline( void );
u08 enc28j60_setOffline( void );
void enc28j60_SetSPISpeed( unsigned long clock_divider );

u08 enc28j60_send(const u08 *data, u16 size);
u08 enc28j60_recv(u08 *data, u16 max_size, u16 *got_size);
u08 enc28j60_status( u08 status_id,  u08 *value);
u08 enc28j60_has_recv(void);
void enc28j60_setMAC( const u08 macaddr[6] );
void enc28j60_getMAC( u08 macaddr[6] ); /* mainly for debug */
void enc28j60_dump_regs(void);
void enc28j60_broadcast_multicast_filter( u08 flags  );

#endif /* _INC_ENC28J60_H */
