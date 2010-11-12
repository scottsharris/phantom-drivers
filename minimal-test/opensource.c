/*
 * This file is part of phantom-drivers.
 *
 * phantom-drivers is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * phantom-drivers is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Phantom-drivers.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This application mimics the trace results of Calibration.h
 * Depending on node id of device (and maybe other things) this will work or not...
 * 
 * In this case 0xffc0 was the host firewire controller and 0xffc1 the PHANTOM Omni device
 */

#include <stdio.h>
#include <string.h>

#include "libraw1394/raw1394.h"

// Enable if PHANTOM ihas 0xffc1 as node id
//#define USE_SECOND_NODE



#ifndef USE_SECOND_NODE
#define NODE 0xffc0
#else
#define NODE 0xffc1
#endif

quadlet_t writebuf[4] = { 0x07ff07ff, 0x53c007ff, 0x00000000, 0x00000000  };
int readdata = 0;

enum raw1394_iso_disposition xmit_handler(raw1394handle_t handle, unsigned char *data, unsigned int *len, unsigned char *tag, unsigned char *sy, int cycle, unsigned int dropped)
{
  memcpy(data, writebuf, 16);
  *len = 16;
  *tag = 0;
  *sy = 0;
  return RAW1394_ISO_OK;
}


enum raw1394_iso_disposition recv_handler(raw1394handle_t handle, unsigned char *data, unsigned int len, unsigned char channel, unsigned char tag, unsigned char sy, unsigned int cycle, unsigned int dropped)
{
  readdata = 1;
  printf("Device position: xxx yyy zzz\n");
  return RAW1394_ISO_OK;
}

int main()
{
  // Initialise Phantom Omni device
  // Created from traces in libraw1394
  raw1394handle_t h0 = raw1394_new_handle();
  int ports = raw1394_get_port_info(h0, 0, 0);
  raw1394_destroy_handle(h0);
  raw1394handle_t h1 = raw1394_new_handle_on_port(0);
  int nodes = raw1394_get_nodecount(h1);
  { quadlet_t buf = 0; raw1394_read(h1, 0xffc0, 0x1006000c, 4, &buf); }
#ifdef  USE_SECOND_NODE
  { quadlet_t buf = 0; raw1394_read(h1, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h1, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h1, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h1, 0xffc1, 0x1006000c, 4, &buf); }
#endif
  { quadlet_t buf = 0; raw1394_read(h1, NODE, 0x10060010, 4, &buf); }
  raw1394_destroy_handle(h1);
  raw1394handle_t h2 = raw1394_new_handle();
  raw1394handle_t h3 = raw1394_new_handle();
  raw1394handle_t h4 = raw1394_new_handle();
  raw1394handle_t h5 = raw1394_new_handle();
  int ports1 = raw1394_get_port_info(h5, 0, 0);
  raw1394_destroy_handle(h5);
  raw1394handle_t h6 = raw1394_new_handle_on_port(0);
  int nodes1 = raw1394_get_nodecount(h6);
  { quadlet_t buf = 0; raw1394_read(h6, 0xffc0, 0x1006000c, 4, &buf); }
#ifdef  USE_SECOND_NODE
  { quadlet_t buf = 0; raw1394_read(h6, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h6, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h6, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h6, 0xffc1, 0x1006000c, 4, &buf); }
#endif
  { quadlet_t buf = 0; raw1394_read(h6, NODE, 0x10060010, 4, &buf); }
  raw1394_destroy_handle(h6);
  raw1394handle_t h7 = raw1394_new_handle();
  raw1394_get_port_info(h7, 0, 0);
  raw1394_destroy_handle(h7);
  raw1394handle_t h8 = raw1394_new_handle_on_port(0);
  int nodes2 = raw1394_get_nodecount(h8);
  { quadlet_t buf = 0; raw1394_read(h8, 0xffc0, 0x1006000c, 4, &buf); }
#ifdef  USE_SECOND_NODE
  { quadlet_t buf = 0; raw1394_read(h8, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h8, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h8, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h8, 0xffc1, 0x1006000c, 4, &buf); }
#endif
  { quadlet_t buf = 0; raw1394_read(h8, NODE, 0x10060010, 4, &buf); }
  raw1394_destroy_handle(h7);
  raw1394_set_port(h2, 0);
  raw1394_set_port(h3, 0);
  raw1394_set_port(h4, 0);
  raw1394_get_fd(h2);
  raw1394_get_fd(h3);
  int nodes3 = raw1394_get_nodecount(h2);
  { quadlet_t buf = 0; raw1394_read(h4, 0xffc0, 0x1006000c, 4, &buf); }
#ifdef  USE_SECOND_NODE
  { quadlet_t buf = 0; raw1394_read(h4, 0xffc1, 0x1006000c, 4, &buf); }
#endif
  raw1394handle_t h9 = raw1394_new_handle();
  raw1394_get_port_info(h9, 0, 0);
  raw1394_destroy_handle(h9);
  raw1394handle_t h10 = raw1394_new_handle_on_port(0);
  raw1394_get_nodecount(h10);
  { quadlet_t buf = 0; raw1394_read(h10, 0xffc0, 0x1006000c, 4, &buf); }
#ifdef  USE_SECOND_NODE
  { quadlet_t buf = 0; raw1394_read(h10, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h10, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h10, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h10, 0xffc1, 0x1006000c, 4, &buf); }
#endif
  { quadlet_t buf = 0; raw1394_read(h10, NODE, 0x10060010, 4, &buf); }
  raw1394_destroy_handle(h10);
  raw1394_destroy_handle(h2);
  raw1394_destroy_handle(h3);
  raw1394_destroy_handle(h4);

  raw1394handle_t h11 = raw1394_new_handle();
  raw1394handle_t h12 = raw1394_new_handle();
  raw1394handle_t h13 = raw1394_new_handle();
  raw1394handle_t h14 = raw1394_new_handle();
  raw1394_get_port_info(h14, 0, 0);
  raw1394_destroy_handle(h14);
  raw1394handle_t h15 = raw1394_new_handle_on_port(0);
  raw1394_get_nodecount(h15);
  { quadlet_t buf = 0; raw1394_read(h15, 0xffc0, 0x1006000c, 4, &buf); }
#ifdef  USE_SECOND_NODE
  { quadlet_t buf = 0; raw1394_read(h15, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h15, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h15, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h15, 0xffc1, 0x1006000c, 4, &buf); }
#endif
  { quadlet_t buf = 0; raw1394_read(h15, NODE, 0x10060010, 4, &buf); }
  raw1394_destroy_handle(h15);
  raw1394handle_t h16 = raw1394_new_handle();
  raw1394_get_port_info(h16, 0, 0);
  raw1394_destroy_handle(h16);
  raw1394handle_t h17 = raw1394_new_handle_on_port(0);
  raw1394_get_nodecount(h17);
  { quadlet_t buf = 0; raw1394_read(h17, 0xffc0, 0x1006000c, 4, &buf); }
#ifdef  USE_SECOND_NODE
  { quadlet_t buf = 0; raw1394_read(h17, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h17, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h17, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h17, 0xffc1, 0x1006000c, 4, &buf); }
#endif
  { quadlet_t buf = 0; raw1394_read(h17, NODE, 0x10060010, 4, &buf); }
  raw1394_destroy_handle(h17);
  raw1394_set_port(h11, 0);
  raw1394_set_port(h12, 0);
  raw1394_set_port(h13, 0);
  int fd11 = raw1394_get_fd(h11);
  int fd12 = raw1394_get_fd(h12);
  raw1394_get_nodecount(h11);
  { quadlet_t buf = 0; raw1394_read(h13, 0xffc0, 0x1006000c, 4, &buf); }
#ifdef  USE_SECOND_NODE
  { quadlet_t buf = 0; raw1394_read(h13, 0xffc1, 0x1006000c, 4, &buf); }
#endif
  raw1394handle_t h18 = raw1394_new_handle();
  raw1394_get_port_info(h18, 0, 0);
  raw1394_destroy_handle(h18);
  raw1394handle_t h19 = raw1394_new_handle_on_port(0);
  raw1394_get_nodecount(h19);
  { quadlet_t buf = 0; raw1394_read(h19, 0xffc0, 0x1006000c, 4, &buf); }
#ifdef  USE_SECOND_NODE
  { quadlet_t buf = 0; raw1394_read(h19, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h19, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h19, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h19, 0xffc1, 0x1006000c, 4, &buf); }
#endif
  { quadlet_t buf = 0; raw1394_read(h19, NODE, 0x10060010, 4, &buf); }
  raw1394_destroy_handle(h19);
  { unsigned char data = 0x01; raw1394_write(h13, NODE, 0x1000, 1, (quadlet_t*) &data); }
//      <- data: 0x01 

  { unsigned char data = 0; raw1394_write(h13, NODE, 0x1001, 1, (quadlet_t*) &data); }
//      <- data: 0x00 

  { quadlet_t data = 0xf80f0000; raw1394_write(h13, NODE, 0x20010, 4, &data); }
//      <- data: 0xf80f0000 

  { char buffer; raw1394_read(h13, NODE, 0x1001, 1, (quadlet_t*) &buffer); }
//      -> buffer: ??

  { unsigned char data = 0x40; raw1394_write(h13, NODE, 0x1001, 1, (quadlet_t*) &data); }
//      <- data: 0x40

  { char buffer = 0x40; raw1394_read(h13, NODE, 0x1001, 1, (quadlet_t*) &buffer); }
//      -> buffer: 0x40

  { unsigned char data = 0; raw1394_write(h13, NODE, 0x1001, 1, (quadlet_t*) &data); }
//      <- data: ??

  printf("Found PHANTOM Omni.\n\n");

  { char buffer; raw1394_read(h13, NODE, 0x1083, 1, (quadlet_t*) &buffer); }
//      -> buffer: ??

  { char buffer; raw1394_read(h13, NODE, 0x1082, 1, (quadlet_t*) &buffer); }
//      -> buffer: ??

  { unsigned char data = 0x08; raw1394_write(h13, NODE, 0x1087, 1, (quadlet_t*) &data); }
//      <- data: 0x08

  raw1394_iso_recv_init(h11, recv_handler, 1000, 64, 0, -1, 1);
  raw1394_iso_recv_start(h11, -1, -1, 0);
  raw1394_set_bus_reset_handler(h11, 0);

  raw1394_iso_xmit_init(h12, 0 /*xmit_handler*/, 1000, 64, 1, 0, 1);
  raw1394_iso_xmit_start(h12, -1, -1);
  raw1394_set_bus_reset_handler(h12, 0);

  while(!readdata)
  {
      raw1394_loop_iterate(h11);
      raw1394_iso_xmit_write(h12, (unsigned char *) writebuf, 16, 0, 0);
      writebuf[1] = 0x53c307ff;
  }


  { char buffer; raw1394_read(h13, NODE, 0x1083, 1, (quadlet_t*) &buffer); }
//      -> buffer: ??

  { char buffer; raw1394_read(h13, NODE, 0x1082, 1, (quadlet_t*) &buffer); }
//      -> buffer: ??

  { unsigned char data; raw1394_write(h13, NODE, 0x1087, 1, (quadlet_t*) &data); }
//      <- data: ??

  raw1394_set_bus_reset_handler(h11, 0);
  raw1394_set_bus_reset_handler(h12, 0);

  raw1394_iso_shutdown(h12);
  raw1394_iso_shutdown(h11);

  raw1394_destroy_handle(h11);
  raw1394_destroy_handle(h12);
  raw1394_destroy_handle(h13);
}
