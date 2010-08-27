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
 * This application is used to find out how to use/drive the PHANTOM omni
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "libraw1394/raw1394.h"

#include "probe-node.h"

/*
// Taking the average of the gimbal values, results in a more stable screen display
// (Might not be necessary for controllers...)
#define GIMBAL_AVG_STEPS 25
unsigned short gimbal_avg[3][GIMBAL_AVG_STEPS];
int gimbal_avg_count = 0;
*/

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
 * Data structure of the data presented when writing to the PHANTOM device
 */
struct data_write {
   short force_x;
   short force_y;
   short force_z;
   union {
     unsigned short bits;
     struct {
       unsigned short dl_flash : 1;  // Dock Light flash (when enabled with dl_fflash, the light is permanently on)
       unsigned short dl_fflash : 1; // Dock Light Fast flash
       unsigned short bit2  : 1;
       unsigned short motors_on : 1; // When set the motors are on (and the given forces are applied)
       unsigned short bit4  : 1;
       unsigned short bit5  : 1;
       unsigned short bit6  : 1;
       unsigned short bit7  : 1;
       unsigned short bit8  : 1;
       unsigned short bit9  : 1;
       unsigned short bit10 : 1;
       unsigned short bit11 : 1;
       unsigned short bit12 : 1;
       unsigned short bit13 : 1;
       unsigned short bit14 : 1;
       unsigned short bit15 : 1;
     };
   } status;
   quadlet_t unused1; // Must be zero
   quadlet_t unused2; // Must be zero
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
        unsigned char button1 : 1; // When 0 the button is pressed
        unsigned char button2 : 1; // When 0 the button is pressed
        unsigned char docked  : 1; // When 0 the gimbal is docked
        unsigned char bit3    : 1;
        unsigned char bit4    : 1;
        unsigned char bit5    : 1;
        unsigned char bit6    : 1;
        unsigned char bit7    : 1;
      };
    } status;
    short unknown10; // always 0x1007 -> not always: also have seen 0x0007 
    union gimbal_u gimbal_inv; // seems to be the inverse of gimbal
    short unknown14; // always 0x5746
    short unknown15;
    quadlet_t count0; // msg nr?? since this value is increawed by one every time a messege is received (even when app is not running)
    short unknown18;
    short unknown19;
    short count1; // Seems to be counting, slower than count0 (half the speed?)
    short unknown21;
    // After this: all zeros (not part of data anymore?)
};

struct device_info
{
  int port;
  nodeaddr_t node;
  raw1394handle_t config_handle;
  raw1394handle_t recv_handle;
  raw1394handle_t xmit_handle;
  int fd_recv;
  int fd_xmit;

  struct data_read phantom_data;
  struct data_write force_data;
};

#define MAX_DEVICES 3
struct device_info devices[MAX_DEVICES];
unsigned int found_devices = 0;

enum raw1394_iso_disposition xmit_handler(raw1394handle_t handle, unsigned char *data, unsigned int *len, unsigned char *tag, unsigned char *sy, int cycle, unsigned int dropped)
{
  unsigned int device;
  for(device = 0; device < found_devices; device++)
    if(devices[device].xmit_handle == handle)
      break;
  if(device == found_devices)
  {
      printf("recv_handler(): Could not find device handle...\n");
      return 1;
  }

  memcpy(data, &devices[device].force_data, sizeof(struct data_write));
  *len = sizeof(struct data_write);
  *tag = 0;
  *sy = 0;
  return RAW1394_ISO_OK;
}


enum raw1394_iso_disposition recv_handler(raw1394handle_t handle, unsigned char *data, unsigned int len, unsigned char channel, unsigned char tag, unsigned char sy, unsigned int cycle, unsigned int dropped)
{
  unsigned int device;
  for(device = 0; device < found_devices; device++)
    if(devices[device].recv_handle == handle)
      break;
  if(device == found_devices)
  {
      printf("recv_handler(): Could not find device handle...\n");
      return 1;
  }


  memcpy(&devices[device].phantom_data, data, sizeof(struct data_read));

  return RAW1394_ISO_OK;
}

void show_data(unsigned int device)
{
  struct data_read *data = &devices[device].phantom_data;
  unsigned short *d = (unsigned short *) data;
  unsigned int i, j;
  unsigned long int tmp[3];

  // Print raw data
  printf("Device %u:\n", device);
  for(i = 0; i < sizeof(struct data_read) / sizeof(short); i+= 4)
  {
    printf("Data %2d-%2d:", i, i + 3);
    for(j = i; j < i + 4 && j < sizeof(struct data_read) / sizeof(short); j++)
      printf(" %4.4x", d[j]);
    printf("\n");
  }

  // Show assumed status bits
  printf("Status bits:");
  for(i = 0; i < 8; i++)
    printf(" %d->%d", i, (data->status.bits & (1<<i)) != 0);
  printf("\n");
  printf("\n");

/*
  // Update average array with new gimbal values
  for(i = 0; i < 3; i++)
    gimbal_avg[i][gimbal_avg_count] = dr->gimbal.raw[i];
  gimbal_avg_count++;
  if (gimbal_avg_count >= GIMBAL_AVG_STEPS)
    gimbal_avg_count = 0;
*/

  // Show formatted output
  printf("Encoder      X %6hd Y %6hd Z %6hd\n", data->encoder_x, data->encoder_y, data->encoder_z);
  printf("Gimbal       X %6hd Y %6hd Z %6hd\n", data->gimbal.x, data->gimbal.y, data->gimbal.z);
  printf("Gimbal (inv) X %6hu Y %6hu Z %6hu\n", data->gimbal_inv.x, data->gimbal_inv.y, data->gimbal_inv.z);
  if(data->status.button1 == 0)
    printf("Button1 pressed\n");
  else
    printf("\n");
  if(data->status.button2 == 0)
    printf("Button2 pressed\n");
  else
    printf("\n");
  if(data->status.docked == 0)
    printf("Gimbal docked\n");
  else
    printf("\n");

  printf("\n");
  printf("Force X: 0x%4.4x\n", devices[device].force_data.force_x);
  printf("Force Y: 0x%4.4x\n", devices[device].force_data.force_y);
  printf("Force Z: 0x%4.4x\n", devices[device].force_data.force_z);
  printf("Status : 0x%4.4x\n", devices[device].force_data.status.bits);
  printf("Status bits:");
  for(i = 0; i < 16; i++)
    printf(" %d->%d", i, (devices[device].force_data.status.bits & (1<<i)) != 0);
  printf("\n");
  printf("\n");

  // TODO In proprietary library it is possible to grab motor temperatures... We might want to add this
}

void fill_default_data(struct data_write *data)
{
  data->force_x     = 0x7ff;
  data->force_y     = 0x7ff;
  data->force_z     = 0x7ff;
  data->status.bits = 0x53c0;
  data->unused1     = 0;
  data->unused2     = 0;
}

/**
 * @return false when expected_response did not match received response (or an error occurred)
 */
int got_expected_char(raw1394handle_t h, nodeaddr_t node, unsigned int address, const unsigned char expected_response, unsigned char *buf)
{
  if(raw1394_read(h, node, address, 1, (quadlet_t *) buf))
    return 0;
  return *buf == expected_response;
}

/**
 * @return false when expected_response did not match received response (or an error occurred)
 */
int got_expected_quadlet(raw1394handle_t h, nodeaddr_t node, unsigned int address, const quadlet_t expected_response, quadlet_t *buf)
{
  if(raw1394_read(h, node, address, sizeof(quadlet_t), buf))
    return 0;
  return *buf == expected_response;
}

int main()
{
  int error_quit = 0; // When 1 exit application, since something went wrong...
  int i;
  unsigned int device; // Used for iterations
  quadlet_t bufq;
  unsigned char bufc;

  // Get amount of ports (firewire cards)
  raw1394handle_t h0 = raw1394_new_handle();
  int ports = raw1394_get_port_info(h0, 0, 0);
  raw1394_destroy_handle(h0);

  // Try to find a PHANTOM device -> currently there is no detection for the PHANTOME device type
  int port;
  for(port = 0; port < ports; port++)
  {
    // Create a handle of the port which gets scanned
    raw1394handle_t scan_handle = raw1394_new_handle_on_port(port);
    int nodes = raw1394_get_nodecount(scan_handle);
    for(i = 0; i < nodes; i++)
    {
      nodeaddr_t node = (1023<<6) | i; // 1023 is the local bus id
      struct config_rom_t crom;
      if(probe_node(scan_handle, node, &crom) == 0 && crom.vendor_id == 0x000b99)
      {
        // Found a SensAble device
      
      // TODO Check if it is a PHANTOM omni

      // The PHANTOM omni does not seem to support the config rom and uses custom addresses to read the type id?
      // 0x00990b00 is the vendor id (0x000b99, see http://standards.ieee.org/regauth/oui/oui.txt), with 1 zero-byte padded to it
      // 0x0ed8a683 is serial of PHANTOM device -> need to find out how to get type id
        if(got_expected_quadlet(scan_handle, node, 0x1006000c, 0x00990b00, &bufq)/* && got_expected_quadlet(scan_handle, node, 0x10060010, 0x0ed8a683)*/)
        {
           // Found PHANTOM device!
           devices[found_devices].port = port;
           devices[found_devices].node = node;
           fill_default_data(&devices[found_devices].force_data);
         found_devices++;
       }
      }
    }
    raw1394_destroy_handle(scan_handle);
  }

  if(found_devices == 0)
  {
    printf("No PHANTOM device found...\n");
    return 1;
  }
  printf("Found %d PHANTOM device(s):\n", found_devices);
  for(device = 0; device < found_devices; device++)
    printf("  on port %d, node 0x%x\n", devices[device].port, (unsigned int) devices[device].node);

  // Setup each device
  for(device = 0; device < found_devices; device++)
  {
    int node = devices[device].node;
    int port = devices[device].port;
    // Create isochronous handles
    devices[device].recv_handle = raw1394_new_handle_on_port(port);
    devices[device].xmit_handle = raw1394_new_handle_on_port(port);
    // Create configuration handle
    devices[device].config_handle = raw1394_new_handle_on_port(port);

    // File handles can be used to determine whether data is ready?
    devices[device].fd_recv = raw1394_get_fd(devices[device].recv_handle);
    devices[device].fd_xmit = raw1394_get_fd(devices[device].xmit_handle);

    // Does not seem to influence anything (enabling/disabling O_NONBLOCK)
    int flags;
    if((flags = fcntl(devices[device].fd_recv, F_GETFL, 0)) < 0)
    {
      printf("Could not read fd flags for device %d\n", device);
      return;
    }
    if(fcntl(devices[device].fd_recv, F_SETFL, flags | O_NONBLOCK) < 0)
    {
      printf("Could not set fd flag O_NONBLOCK for device %d\n", device);
      return;
    }
    if((flags = fcntl(devices[device].fd_xmit, F_GETFL, 0)) < 0)
    {
      printf("Could not read fd flags for device %d\n", device);
      return;
    }
    if(fcntl(devices[device].fd_xmit, F_SETFL, flags | O_NONBLOCK) < 0)
    {
      printf("Could not set fd flag O_NONBLOCK for device %d\n", device);
      return;
    }


    raw1394handle_t config_handle = devices[device].config_handle;
    // Configure PHANTOM omni
    // TODO Need to find out what is happening/configuring?

    // Set isochronous xmit channel
    bufc = device * 2 + 0x01; raw1394_write(config_handle, node, 0x1000, 1, (quadlet_t*) &bufc);
    // Set isochronous  recv channel
    bufc = device * 2 + 0x00; raw1394_write(config_handle, node, 0x1001, 1, (quadlet_t*) &bufc);

    // TODO Whithout raw1394_write(), two PHANTOM devices seem to work...?!
    // Set isochronous channel? -> When writing by accident only 1 byte of the quadlet, two devices were working, but writing to the same channel...
    //bufq = 0xf80f0000; raw1394_write(config_handle, node, 0x20010, 4, &bufq);

    if(!got_expected_char(config_handle, node, 0x1001, device * 2 + 0x00, &bufc))
    {
      printf("line %d: Expected 0x00 but got 0x%2.2x instead!\n", __LINE__, bufc);
      return 1;
    }



    // Toggle bit 6 and see whether we can read it -> test to check whether device is working?
    // Especially since the proprietary library prints the 'Found PHANTOM Omni' message afterwards

    // Read value, since we'd like to keep our selected channel!
    unsigned char old_value;
    raw1394_read(config_handle, node, 0x1001, 1, (quadlet_t *) &old_value);

    // Change value and see whether we can read it back
    bufc = 0x40; raw1394_write(config_handle, node, 0x1001, 1, (quadlet_t*) &bufc);
    if(!got_expected_char(config_handle, node, 0x1001, 0x40, &bufc))
    {
      printf("line %d: Expected 0x40 but got 0x%2.2x insead!\n", __LINE__, bufc);
      return 1;
    }
    printf("Found PHANTOM Omni.\n\n"); // fabulous test worked...?

    // Write back our selected channel
    raw1394_write(config_handle, node, 0x1001, 1, (quadlet_t*) &old_value);




    // TODO What is this doing? (also called after leaving the main loop)
    if(!got_expected_char(config_handle, node, 0x1083, 0xc0, &bufc)) // -> bufc: 0xc0 = bit 6 & 7
    {
      printf("line %d: Expected 0xc0 but got 0x%2.2x insead!\n", __LINE__, bufc);
      return 1;
    }

    if(!got_expected_char(config_handle, node, 0x1082, 0x00, &bufc))
    {
      printf("line %d: Expected 0x00 but got 0x%2.2x insead!\n", __LINE__, bufc);
      return 1;
    }

    // Enabling device
    // bit 3: Enable isochronous mode of device (If later 0x00 is written, the device is disabled)
    // TODO Does other bits have any effect?
    bufc = 0x08; raw1394_write(config_handle, node, 0x1087, 1, (quadlet_t*) &bufc);

    // Start isochronous receiving
    raw1394_iso_recv_init(devices[device].recv_handle, recv_handler, 1000, 64, device * 2, -1, 1);
    raw1394_iso_recv_start(devices[device].recv_handle, -1, -1, 0);

    // Start isochronous transmitting
    raw1394_iso_xmit_init(devices[device].xmit_handle, xmit_handler, 1000, 64, device * 2 + 1, 0, 1);
    raw1394_iso_xmit_start(devices[device].xmit_handle, -1, -1);
  }

  int phantom_docked = 0; // Used to exit application
  do
  {
      if(devices[0].phantom_data.status.docked)
        phantom_docked = 1;

      printf("\n\033[2J"); // Clear screen

      fd_set fdread;
      fd_set fdwrite;
      FD_ZERO(&fdread);
      FD_ZERO(&fdwrite);

      int max_fd = 0;
      for(device = 0; device < found_devices; device++)
      {
        FD_SET(devices[device].fd_recv, &fdread);
        if(max_fd < devices[device].fd_recv)
          max_fd = devices[device].fd_recv;
        FD_SET(devices[device].fd_xmit, &fdwrite);
        if(max_fd < devices[device].fd_xmit)
          max_fd = devices[device].fd_xmit;
      }

      struct timeval timeout;
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;
      int ret = select(max_fd + 1, &fdread, 0, 0, &timeout);
      printf("select() returned: %d\n", ret);
      if(ret == 0)
      {
        printf("select() timed out, which means there was no communciation for 1 second...\n");
        printf("Connection got lost?\n");
        printf("Exiting...");
        error_quit = 1;
      }

      for(device = 0; device < found_devices; device++)
      {

        if(FD_ISSET(devices[device].fd_recv,&fdread))
        {
          // Do an isochronous read
          if(raw1394_loop_iterate(devices[device].recv_handle))
          {
            if(errno != EAGAIN)
            {
              printf("Something went wrong in the recv iterate loop... errno (%d) -> ", errno); fflush(stdout);
              perror(0);
              return 1;
 	    }
          }
/*        }


        if(FD_ISSET(devices[device].fd_xmit,&fdwrite))
        {*/
          // Do an isochronous write
          if(raw1394_loop_iterate(devices[device].xmit_handle))
          {
            if(errno != EAGAIN)
            {
              printf("Something went wrong in the xmit iterate loop... errno (%d) -> ", errno); fflush(stdout);
              perror(0);
              return 1;
            }
          }
        }
        show_data(device);

        // Add some interaction with the buttons
        if(FD_ISSET(devices[device].fd_recv,&fdread))
        {
          if(devices[device].phantom_data.status.button1 == 0)
          {
            printf("Release button 1 to disable forces\n");
            devices[device].force_data.status.motors_on = 1;
            devices[device].force_data.force_x = 0x500;
          }
          else
          {
            printf("Hold button 1 to apply force on X axis\n");
            devices[device].force_data.status.motors_on = 0;
          }
          devices[device].force_data.status.dl_flash = !devices[device].phantom_data.status.button1;
          devices[device].force_data.status.dl_fflash = !devices[device].phantom_data.status.button2;

          if(device == 0)
          {
            if(!phantom_docked)
              printf("Undock and dock phantom to exit application\n");
            else
              printf("Dock phantom to exit application\n");
          }
        }
      }
  } while((devices[0].phantom_data.status.docked != 0 || !phantom_docked) && error_quit == 0);

  for(device = 0; device < found_devices; device++)
  {
    int node = devices[device].node;
    raw1394handle_t config_handle = devices[device].config_handle;

    // TODO What is this doing (also called before entering the main loop)
    if(!got_expected_char(config_handle, node, 0x1083, 0xc0, &bufc)) // -> bufc: 0xc0 = bit 6 & 7
    {
      printf("line %d: Expected 0xc0 but got 0x%2.2x insead!\n", __LINE__, bufc);
      return 1;
    }

    if(!got_expected_char(config_handle, node, 0x1082, 0x00, &bufc))
    {
      printf("line %d: Expected 0x00 but got 0x%2.2x insead!\n", __LINE__, bufc);
      return 1;
    }

    // Turn off isochronous transfer (bit 3)
    bufc = 0x00; raw1394_write(config_handle, node, 0x1087, 1, (quadlet_t*) &bufc);

    // Shutdown isochronous transfers
    raw1394_iso_shutdown(devices[device].xmit_handle);
    raw1394_iso_shutdown(devices[device].recv_handle);

    raw1394_destroy_handle(devices[device].recv_handle);
    raw1394_destroy_handle(devices[device].xmit_handle);
    raw1394_destroy_handle(devices[device].config_handle);
  }
}
