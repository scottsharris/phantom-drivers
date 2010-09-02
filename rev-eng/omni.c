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
#include <netinet/in.h> // ntohl

#include "libraw1394/raw1394.h"
#include "libraw1394/csr.h"

#include "probe-node.h"
#include "easy-com.h"

// List of addresses which can be used to configure the PHANTOM device
#define ADDR_XMIT_CHANNEL          0x1000  /* sets the xmit isochronous channel */
#define ADDR_RECV_CHANNEL          0x1001  /* sets the recv isochronous channel */
#define ADDR_CONTROL               0x1087  /* control register? */
#define ADDR_CONTROL_enable_iso    (1<<3)  /*   bit 3 enables the isochronous data transfer of the device */
                                           /*   other bits are unknown and always seems to be zero? */

#define CHANNELS_AVAILABLE_ADDR    CSR_REGISTER_BASE + CSR_CHANNELS_AVAILABLE_HI

// Returns true of false depending whether the 'channel bit' is set in channels
#define CHANNEL_IS_FREE(channels, channel) (channels & (1L<<(63 - channel)))

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
  int recv_channel;
  int xmit_channel;

  struct data_read phantom_data;
  struct data_write force_data;
};

#define MAX_DEVICES 3
struct device_info devices[MAX_DEVICES];
unsigned int found_devices = 0;

int initialise_device(unsigned int device, int full_init);

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

int busreset_handler(raw1394handle_t handle, unsigned int generation)
{
  unsigned int device;

  printf("Bus reset!\n");
  raw1394_update_generation(handle, generation);
  for(device = 0; device < found_devices; device++)
  {
    if(devices[device].recv_handle == handle)
    {
      printf("Device %d need to re-initialise\n", device);
      printf("fd_xmit: %d, fd_recv: %d\n", devices[device].fd_xmit, devices[device].fd_recv);
      if(initialise_device(device, 0))
        fprintf(stderr, "initialise_device(%d, 0) failed\n", device);
      printf("fd_xmit: %d, fd_recv: %d\n", devices[device].fd_xmit, devices[device].fd_recv);
      break;
    }
  }
}

void show_data(unsigned int device)
{
  struct data_read *data = &devices[device].phantom_data;
  unsigned short *d = (unsigned short *) data;
  unsigned int i, j;
  unsigned long int tmp[3];

  // Print raw data
  printf("\n\n_________________________________________________________________\nDevice %u\n", device);
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
  printf("\n\n");

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
  printf("\n\n");

  // TODO In proprietary library it is possible to grab motor temperatures... We might want to add this

  if(devices[device].phantom_data.status.button1 == 0)
    printf("Release button 1 to disable forces\n");
  else
    printf("Hold button 1 to apply force on X axis\n");

  if(device == 0)
  {
    printf("Press button2 to perform a 'bus reset'\n");
    if(devices[device].phantom_data.status.docked == 0)
       printf("Undock and dock phantom to exit application\n");
    else
       printf("Dock phantom to exit application\n");
   }
}

void fill_default_data(struct data_write *data)
{
  data->force_x     = 0x500; // to enable force when button1 is pressed
  data->force_y     = 0x7ff;
  data->force_z     = 0x7ff;
  data->status.bits = 0x53c0;
  data->unused1     = 0;
  data->unused2     = 0;
}

/**
 * If block is true, the file descriptor is set to blocking, otherwise it is set to non-blocking
 *
 * @return true when an error occurred
 */
int set_fd_blocking(int fd, int block)
{
  int flags;
  if((flags = fcntl(fd, F_GETFL, 0)) < 0)
  {
    printf("Could not read fd flags for fd %d\n", fd);
    return 1;
  }
  if(block)
    flags &= ~O_NONBLOCK;
  else
    flags |= O_NONBLOCK;
  if(fcntl(fd, F_SETFL, flags) < 0)
  {
    printf("Could not set fd flag O_NONBLOCK for fd %d\n", fd);
    return 1;
  }
  return 0;
}

int initialise_device(unsigned int device, int full_init)
{
  unsigned char bufc;
  int i;
  int node = devices[device].node;
  int port = devices[device].port;

  if(full_init)
  {
    // Create isochronous handles
    devices[device].recv_handle = raw1394_new_handle_on_port(port);
    devices[device].xmit_handle = raw1394_new_handle_on_port(port);
    // Create configuration handle
    devices[device].config_handle = raw1394_new_handle_on_port(port);

    // Only add 1 handle to bus reset event, sicne the other (xmit) handle is updated at teh same time!
    raw1394_set_bus_reset_handler(devices[device].recv_handle, &busreset_handler);

    // File handles can be used to determine whether data is ready?
    devices[device].fd_recv = raw1394_get_fd(devices[device].recv_handle);
    devices[device].fd_xmit = raw1394_get_fd(devices[device].xmit_handle);

    // Does not seem to influence anything (enabling/disabling O_NONBLOCK)
    if(set_fd_blocking(devices[device].fd_recv, 0) || set_fd_blocking(devices[device].fd_xmit, 0))
      return 1;
  }

  raw1394handle_t config_handle = devices[device].config_handle;
  // Configure PHANTOM omni

  // Find free channels for the current port
  nodeid_t irm_node = raw1394_get_irm_id(config_handle);
  printf("IRM node = 0x%x\n", irm_node);
  octlet_t channels;
  quadlet_t *channelsq = (quadlet_t *) &channels;
  if(read_data(config_handle, irm_node, CHANNELS_AVAILABLE_ADDR, sizeof(octlet_t), (unsigned char *) &channels))
  {
    fprintf(stderr, "Failed to read free channels: (%d) %s\n", errno, strerror(errno));
    return 1;
  }
  // Convert to a more convenient order
  for(i = 0; i < 2; i++)
    channelsq[i] = ntohl(channelsq[i]);
  channels = ((octlet_t )channelsq[0])<<32 | channelsq[1]; // swap quadlets
  printf("Free channels: 0x%16.16lx\n", channels);

  devices[device].recv_channel = -1;
  devices[device].xmit_channel = -1;
  for(i = 0; i < 64; i++)
    if(CHANNEL_IS_FREE(channels, i))
    {
      devices[device].recv_channel = i;
      break;
    }
  for(i++; i < 64; i++)
    if(CHANNEL_IS_FREE(channels, i))
    {
      devices[device].xmit_channel = i;
      break;
    }
  if(devices[device].recv_channel == -1 || devices[device].xmit_channel == -1)
  {
    fprintf(stderr, "No free isochronous channels are present, cannot communicate with PHANTOM device...\n");
    return 1;
  }
  printf("recv_channel = %d, xmit_channel = %d\n", devices[device].recv_channel, devices[device].xmit_channel);

  // TODO What about other addresses (like 0x1000 and higher), is there more to configure or read status from?
  // TODO Need to find out what is happening/configuring?
  // Allocate/claim channels for out application
  raw1394_channel_modify(config_handle, devices[device].recv_channel, RAW1394_MODIFY_ALLOC);
  raw1394_channel_modify(config_handle, devices[device].xmit_channel, RAW1394_MODIFY_ALLOC);

  // Set isochronous xmit and recv channels
  if(write_data_char(config_handle, node, ADDR_XMIT_CHANNEL, devices[device].xmit_channel))
  {
    fprintf(stderr, "Failed to write xmit channel info: (%d) %s\n", errno, strerror(errno));
    return 1;
  }
  if(write_data_char(config_handle, node, ADDR_RECV_CHANNEL, devices[device].recv_channel))
  {
    fprintf(stderr, "Failed to write xmit channel info: (%d) %s\n", errno, strerror(errno));
    return 1;
  }

  // TODO Whithout this raw1394_write(), two PHANTOM devices seem to work...?! -> what is it doing and what should be changed to be able to enable this line??
  // It seems that it has something to do with timing: when this is enabled (with two devices) the data is *not* received in a steady flow (until it takes too long and the select() quits the application)
  //quadlet_t bufq = 0xf80f0000; raw1394_write(config_handle, node, 0x20010, 4, &bufq);

  if(get_expected_char(config_handle, node, ADDR_RECV_CHANNEL, devices[device].recv_channel, &bufc))
  {
    printf("line %d: Expected %d but got %d instead!\n", __LINE__, devices[device].recv_channel, bufc);
    return 1;
  }

  // Toggle bit 6 and see whether we can read it -> test to check whether device is working?
  // Especially since the proprietary library prints the 'Found PHANTOM Omni' message afterwards

  // Change value and see whether we can read it back
  if(write_data_char(config_handle, node, ADDR_RECV_CHANNEL, 0x40))
  {
    fprintf(stderr, "Failed to write test (?) data..\n");
    return 1;
  }
  if(get_expected_char(config_handle, node, ADDR_RECV_CHANNEL, 0x40, &bufc))
  {
    fprintf(stderr, "line %d: Expected 0x40 but got 0x%2.2x instead!\n", __LINE__, bufc);
    return 1;
  }

  // Write back our selected channel
  if(write_data_char(config_handle, node, ADDR_RECV_CHANNEL, devices[device].recv_channel))
  {
    fprintf(stderr, "Failed to write back our selected channel: (%d) %s\n", errno, strerror(errno));
    return 1;
  }

  // TODO What is this doing? (also called after leaving the main loop)
  if(get_expected_char(config_handle, node, 0x1083, 0xc0, &bufc)) // -> bufc: 0xc0 = bit 6 & 7
  {
    fprintf(stderr, "line %d: Expected 0xc0 but got 0x%2.2x instead!\n", __LINE__, bufc);
    return 1;
  }

  if(get_expected_char(config_handle, node, 0x1082, 0x00, &bufc))
  {
    fprintf(stderr, "line %d: Expected 0x00 but got 0x%2.2x instead!\n", __LINE__, bufc);
    return 1;
  }

  // Enable isochronous transfer
  // TODO Does other bits have any effect?
  if(read_data_char(config_handle, node, ADDR_CONTROL, &bufc))
  {
    fprintf(stderr, "Failed read ADDR_CONTROL: (%d) %s\n", errno, strerror(errno));
    return 1;
  }
  bufc |=  ADDR_CONTROL_enable_iso;
  if(write_data_char(config_handle, node, ADDR_CONTROL, bufc))
  {
    fprintf(stderr, "Failed to write ADDR_CONTROL: (%d) %s\n", errno, strerror(errno));
    return 1;
  }

  // Start isochronous receiving
  raw1394_iso_recv_init(devices[device].recv_handle, recv_handler, 1000, 64, devices[device].recv_channel, -1, 1);
  raw1394_iso_recv_start(devices[device].recv_handle, -1, -1, 0);

  // Start isochronous transmitting
  raw1394_iso_xmit_init(devices[device].xmit_handle, xmit_handler, 1000, 64, devices[device].xmit_channel, RAW1394_ISO_SPEED_100, 1);
  raw1394_iso_xmit_start(devices[device].xmit_handle, -1, -1);

  printf("initialise_device() ok\n\n");

  return 0;
}

int main()
{
  int error_quit = 0; // When 1 exit application, since something went wrong...
  int phantom_docked = 0; // Used to exit application
  int button2_pressed = 0; // Used to create a 'click' event (instead of a hold)
  int i;
  unsigned int device; // Used for iterations
  quadlet_t bufq;
  unsigned char bufc;

  // Get amount of ports (firewire cards)
  raw1394handle_t h0 = raw1394_new_handle();
  int ports = raw1394_get_port_info(h0, 0, 0);
  raw1394_destroy_handle(h0);
  if(ports == 0)
  {
    printf("No firewire ports found... Did you forgot to start the raw1394 module? (again...)\n");
    return 0;
  }

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
        if(!get_expected_quadlet(scan_handle, node, 0x1006000c, 0x00990b00, &bufq)/* && !get_expected_quadlet(scan_handle, node, 0x10060010, 0x0ed8a683)*/)
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
    printf("No PHANTOM devices found...\n");
    return 1;
  }
  printf("Found %d PHANTOM device(s):\n", found_devices);
  for(device = 0; device < found_devices; device++)
    printf("  on port %d, node 0x%x\n", devices[device].port, (unsigned int) devices[device].node);

  // Setup each device
  for(device = 0; device < found_devices; device++)
    if(initialise_device(device, 1))
      return 1;

  do
  {
      if(devices[0].phantom_data.status.docked)
        phantom_docked = 1;

      // It is also possible to use this contraction when writing
      // (this application writes when there was something to read)
      fd_set fdread;
      FD_ZERO(&fdread);
      int max_fd = 0;
      for(device = 0; device < found_devices; device++)
      {
        FD_SET(devices[device].fd_recv, &fdread);
        if(max_fd < devices[device].fd_recv)
          max_fd = devices[device].fd_recv;
      }

      struct timeval timeout;
      timeout.tv_sec = 1; // wait 1 second before giving up on new data
      timeout.tv_usec = 0;
      int ret = select(max_fd + 1, &fdread, 0, 0, &timeout);
      printf("select() returned: %d\n", ret);
      if(ret == 0)
      {
        printf("select() timed out, which means there was no communciation for 1 second...\n");
        printf("Connection got lost?\n");
        printf("Exiting... (no need to cleanup stuff since we do not have the connection anymore...)\n\n");
        return 0;
      }
      printf("\n\033[2J"); // Clear screen

      // Now update the devices which have data available according to select()
      for(device = 0; device < found_devices; device++)
      {

        if(FD_ISSET(devices[device].fd_recv,&fdread))
        {
          // Do an isochronous read
          if(raw1394_loop_iterate(devices[device].recv_handle))
          {
            if(errno != 0 && errno != EAGAIN)
            {
              printf("Something went wrong in the recv iterate loop... errno (%d) -> ", errno); fflush(stdout);
              perror(0);
              return 1;
             }
          }

          // Add some interaction with the buttons
          devices[device].force_data.status.motors_on = !devices[device].phantom_data.status.button1;
          devices[device].force_data.status.dl_flash = !devices[device].phantom_data.status.button1;
          devices[device].force_data.status.dl_fflash = !devices[device].phantom_data.status.button2;

          if(device == 0)
          {
            if(devices[0].phantom_data.status.button2 == 0)
            {
              button2_pressed = 1;
            }
            else
            {
              if(button2_pressed)
              {
                // Button2 (of device 0) got released
                button2_pressed = 0;
                // Reset bus on which device 0 is connected (to test whether the bus reset handler is functioning)
                raw1394_reset_bus(devices[0].config_handle);
              }
            }
          }

          // Do an isochronous write
          if(raw1394_loop_iterate(devices[device].xmit_handle))
          {
            if(errno != 0 && errno != EAGAIN)
            {
              printf("Something went wrong in the xmit iterate loop... errno (%d) -> ", errno); fflush(stdout);
              perror(0);
              return 1;
            }
          }
        }
        show_data(device);

      }
  } while((devices[0].phantom_data.status.docked != 0 || !phantom_docked) && error_quit == 0);

  for(device = 0; device < found_devices; device++)
  {
    int node = devices[device].node;
    raw1394handle_t config_handle = devices[device].config_handle;

    // TODO What is this doing (also called before entering the main loop)
    if(get_expected_char(config_handle, node, 0x1083, 0xc0, &bufc)) // -> bufc: 0xc0 = bit 6 & 7
    {
      printf("line %d: Expected 0xc0 but got 0x%2.2x instead!\n", __LINE__, bufc);
      return 1;
    }

    if(get_expected_char(config_handle, node, 0x1082, 0x00, &bufc))
    {
      printf("line %d: Expected 0x00 but got 0x%2.2x instead!\n", __LINE__, bufc);
      return 1;
    }

    // Turn off isochronous transfer of device
    raw1394_read(config_handle, node, ADDR_CONTROL, 1, (quadlet_t*) &bufc);
    bufc &=  ~ADDR_CONTROL_enable_iso;
    raw1394_write(config_handle, node, ADDR_CONTROL, 1, (quadlet_t*) &bufc);

    // Shutdown isochronous transfers
    raw1394_iso_shutdown(devices[device].xmit_handle);
    raw1394_iso_shutdown(devices[device].recv_handle);

    if(devices[device].recv_channel != -1 && devices[device].xmit_channel != -1)
    {
      raw1394_channel_modify(devices[device].config_handle, devices[device].recv_channel, RAW1394_MODIFY_FREE);
      raw1394_channel_modify(devices[device].config_handle, devices[device].xmit_channel, RAW1394_MODIFY_FREE);
    }

    raw1394_destroy_handle(devices[device].recv_handle);
    raw1394_destroy_handle(devices[device].xmit_handle);
    raw1394_destroy_handle(devices[device].config_handle);
  }
}
