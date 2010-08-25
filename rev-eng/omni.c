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

quadlet_t writebuf[4] = { 0x07ff07ff, 0x53c007ff, 0x00000000, 0x00000000  };

// Taking the average of the gimbal values, results in a more stable screen display
// (Might not be necessary for controllers...)
#define GIMBAL_AVG_STEPS 25
unsigned short gimbal_avg[3][GIMBAL_AVG_STEPS];
int gimbal_avg_count = 0;

/**
 * Contains the values of the gimbal. For each axis the 5 most LSB are unused (too erratic)
 */
union gimbal_u {
  unsigned short raw[3];
  struct {
    unsigned short x_unused:5;
    unsigned short x:11;
    unsigned short y_unused:5;
    unsigned short y:11;
    unsigned short z_unused:5;
    unsigned short z:11;
  };
};

/**
 * Data structure of the data presented when the read callback handler is called
 */ 
struct data_read {
    short unknown0; // always 0x0000 ??
    short unknown1; // always 0x001e ??
    unsigned short encoder_x;
    unsigned short encoder_y;
    unsigned short encoder_z;
    union gimbal_u gimbal;
    short unknown8;
    unsigned char  unknown9a; // Also status bits?
    union {
      unsigned char bits;
      struct {
        unsigned char button1 : 1;
        unsigned char button2 : 1;
        unsigned char docked  : 1;
        unsigned char bit3    : 1;
        unsigned char bit4    : 1;
        unsigned char bit5    : 1;
        unsigned char bit6    : 1;
        unsigned char bit7    : 1;
      };
    } status;
    short unknown10; // always 0x1007 ??
    union gimbal_u gimbal_inv; // seems to be the inverse of gimbal
    short unknown14; // not part of data?
    short unknown15; // not part of data?
};

struct data_read phantom_data;

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
  unsigned short *d = (unsigned short *) data;
  memcpy(&phantom_data, data, sizeof(struct data_read));

  // Print raw data
  printf("Data: %4.4x %4.4x %4.4x %4.4x\n", d[0], d[1], d[2], d[3]);
  printf("Data: %4.4x %4.4x %4.4x %4.4x\n", d[4], d[5], d[6], d[7]);
  printf("Data: %4.4x %4.4x %4.4x %4.4x\n", d[8], d[9], d[10], d[11]);
  printf("Data: %4.4x %4.4x %4.4x %4.4x\n", d[12], d[13], d[14], d[15]);

  return RAW1394_ISO_OK;
}

void show_data(struct data_read *data)
{
  struct data_read *dr = (struct data_read *) data;
  int i, j;
  unsigned long int tmp[3];
  
  // Show assumed status bits
  printf("Status bits:");
  for(i = 0; i < 8; i++)
    printf(" %d->%d", i, (dr->status.bits & (1<<i)) != 0);
  printf("\n");
  printf("\n");

  // Update average array with new gimbal values
  for(i = 0; i < 3; i++)
    gimbal_avg[i][gimbal_avg_count] = dr->gimbal.raw[i];
  gimbal_avg_count++;
  if (gimbal_avg_count >= GIMBAL_AVG_STEPS)
    gimbal_avg_count = 0;

  // Calcualte gimbal averages
  for(i = 0; i < 3; i++)
  {
    tmp[i] = 0;
    for(j = 0; j < GIMBAL_AVG_STEPS; j++)
      tmp[i] += gimbal_avg[i][j];
    tmp[i] /= (GIMBAL_AVG_STEPS * (1<<5));
  }

  // Show formatted output
  printf("Encoder      X %6hd Y %6hd Z %6hd\n", dr->encoder_x, dr->encoder_y, dr->encoder_z);
  printf("Gimbal       X %6hd Y %6hd Z %6hd\n", dr->gimbal.x, dr->gimbal.y, dr->gimbal.z);
  printf("Gimbal (avg) X %6hd Y %6hd Z %6hd\n", (int) tmp[0], (int) tmp[1], (int) tmp[2]);
  printf("Gimbal (inv) X %6hu Y %6hu Z %6hu\n", dr->gimbal_inv.x, dr->gimbal_inv.y, dr->gimbal_inv.z);
  if(dr->status.button1 == 0)
    printf("Button1 pressed\n");
  else
    printf("\n");
  if(dr->status.button2 == 0)
    printf("Button2 pressed\n");
  else
    printf("\n");
  if(dr->status.docked == 0)
    printf("Gimbal docked\n");
  else
    printf("\n");
}

int main()
{
  int phantom_docked = 0; // Used to exit application

  // Initialise Phantom Omni device
  // Created from traces in libraw1394
  raw1394handle_t h0 = raw1394_new_handle();
  int ports = raw1394_get_port_info(h0, 0, 0);
  raw1394_destroy_handle(h0);
  raw1394handle_t h1 = raw1394_new_handle_on_port(0);
  int nodes = raw1394_get_nodecount(h1);
  { quadlet_t buf = 0; raw1394_read(h1, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h1, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h1, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h1, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h1, 0xffc1, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h1, 0xffc1, 0x10060010, 4, &buf); }
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
  { quadlet_t buf = 0; raw1394_read(h6, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h6, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h6, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h6, 0xffc1, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h6, 0xffc1, 0x10060010, 4, &buf); }
  raw1394_destroy_handle(h6);
  raw1394handle_t h7 = raw1394_new_handle();
  raw1394_get_port_info(h7, 0, 0);
  raw1394_destroy_handle(h7);
  raw1394handle_t h8 = raw1394_new_handle_on_port(0);
  int nodes2 = raw1394_get_nodecount(h8);
  { quadlet_t buf = 0; raw1394_read(h8, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h8, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h8, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h8, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h8, 0xffc1, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h8, 0xffc1, 0x10060010, 4, &buf); }
  raw1394_destroy_handle(h7);
  raw1394_set_port(h2, 0);
  raw1394_set_port(h3, 0);
  raw1394_set_port(h4, 0);
  raw1394_get_fd(h2);
  raw1394_get_fd(h3);
  int nodes3 = raw1394_get_nodecount(h2);
  { quadlet_t buf = 0; raw1394_read(h4, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h4, 0xffc1, 0x1006000c, 4, &buf); }
  raw1394handle_t h9 = raw1394_new_handle();
  raw1394_get_port_info(h9, 0, 0);
  raw1394_destroy_handle(h9);
  raw1394handle_t h10 = raw1394_new_handle_on_port(0);
  raw1394_get_nodecount(h10);
  { quadlet_t buf = 0; raw1394_read(h10, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h10, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h10, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h10, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h10, 0xffc1, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h10, 0xffc1, 0x10060010, 4, &buf); }
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
  { quadlet_t buf = 0; raw1394_read(h15, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h15, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h15, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h15, 0xffc1, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h15, 0xffc1, 0x10060010, 4, &buf); }
  raw1394_destroy_handle(h15);
  raw1394handle_t h16 = raw1394_new_handle();
  raw1394_get_port_info(h16, 0, 0);
  raw1394_destroy_handle(h16);
  raw1394handle_t h17 = raw1394_new_handle_on_port(0);
  raw1394_get_nodecount(h17);
  { quadlet_t buf = 0; raw1394_read(h17, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h17, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h17, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h17, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h17, 0xffc1, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h17, 0xffc1, 0x10060010, 4, &buf); }
  raw1394_destroy_handle(h17);
  raw1394_set_port(h11, 0);
  raw1394_set_port(h12, 0);
  raw1394_set_port(h13, 0);
  int fd11 = raw1394_get_fd(h11);
  int fd12 = raw1394_get_fd(h12);
  raw1394_get_nodecount(h11);
  { quadlet_t buf = 0; raw1394_read(h13, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h13, 0xffc1, 0x1006000c, 4, &buf); }
  raw1394handle_t h18 = raw1394_new_handle();
  raw1394_get_port_info(h18, 0, 0);
  raw1394_destroy_handle(h18);
  raw1394handle_t h19 = raw1394_new_handle_on_port(0);
  raw1394_get_nodecount(h19);
  { quadlet_t buf = 0; raw1394_read(h19, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h19, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h19, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h19, 0xffc0, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h19, 0xffc1, 0x1006000c, 4, &buf); }
  { quadlet_t buf = 0; raw1394_read(h19, 0xffc1, 0x10060010, 4, &buf); }
  raw1394_destroy_handle(h19);
  { unsigned char data = 0x01; raw1394_write(h13, 0xffc1, 0x1000, 1, (quadlet_t*) &data); }
//      <- data: 0x01 

  { unsigned char data = 0; raw1394_write(h13, 0xffc1, 0x1001, 1, (quadlet_t*) &data); }
//      <- data: 0x00 

  { quadlet_t data = 0xf80f0000; raw1394_write(h13, 0xffc1, 0x20010, 4, &data); }
//      <- data: 0xf80f0000 

  { char buffer; raw1394_read(h13, 0xffc1, 0x1001, 1, (quadlet_t*) &buffer); }
//      -> buffer: ??

  { unsigned char data = 0x40; raw1394_write(h13, 0xffc1, 0x1001, 1, (quadlet_t*) &data); }
//      <- data: 0x40

  { char buffer = 0x40; raw1394_read(h13, 0xffc1, 0x1001, 1, (quadlet_t*) &buffer); }
//      -> buffer: 0x40

  { unsigned char data = 0; raw1394_write(h13, 0xffc1, 0x1001, 1, (quadlet_t*) &data); }
//      <- data: ??

  printf("Found PHANTOM Omni.\n\n");

  { char buffer; raw1394_read(h13, 0xffc1, 0x1083, 1, (quadlet_t*) &buffer); }
//      -> buffer: ??

  { char buffer; raw1394_read(h13, 0xffc1, 0x1082, 1, (quadlet_t*) &buffer); }
//      -> buffer: ??

  { unsigned char data = 0x08; raw1394_write(h13, 0xffc1, 0x1087, 1, (quadlet_t*) &data); }
//      <- data: 0x08

  raw1394_iso_recv_init(h11, recv_handler, 1000, 64, 0, -1, 1);
  raw1394_iso_recv_start(h11, -1, -1, 0);

  raw1394_iso_xmit_init(h12, 0 /*xmit_handler*/, 1000, 64, 1, 0, 1);
  raw1394_iso_xmit_start(h12, -1, -1);

  do
  {
      if(phantom_data.status.docked)
        phantom_docked = 1;

      printf("\033[2J"); // Clear screen

      raw1394_loop_iterate(h11);
      raw1394_iso_xmit_write(h12, (unsigned char *) writebuf, 16, 0, 0);
      writebuf[1] = 0x53c307ff; // Does not seem to be required

      show_data(&phantom_data);
      if(!phantom_docked)
        printf("Undock and dock phantom to exit application\n");
      else
        printf("Dock phantom to exit application\n");
  } while(phantom_data.status.docked != 0 || !phantom_docked);

  { char buffer; raw1394_read(h13, 0xffc1, 0x1083, 1, (quadlet_t*) &buffer); }
//      -> buffer: ??

  { char buffer; raw1394_read(h13, 0xffc1, 0x1082, 1, (quadlet_t*) &buffer); }
//      -> buffer: ??

  { unsigned char data; raw1394_write(h13, 0xffc1, 0x1087, 1, (quadlet_t*) &data); }
//      <- data: ??

  raw1394_iso_shutdown(h12);
  raw1394_iso_shutdown(h11);

  raw1394_destroy_handle(h11);
  raw1394_destroy_handle(h12);
  raw1394_destroy_handle(h13);
}
