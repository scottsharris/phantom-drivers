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
 * along with phantom-drivers.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Phantom Library: FireWire communication driver for libraw1394
 */

#include <stdio.h>      // sprintf
#include <string.h>     // stderror
#include <stdlib.h>     // NULL
#include <errno.h>
#include <netinet/in.h> // ntohl
#include "libraw1394/csr.h"

#include "FirewireDeviceLibraw1394.h"

#define CHANNELS_AVAILABLE_ADDR    CSR_REGISTER_BASE + CSR_CHANNELS_AVAILABLE_HI

// Returns true of false depending whether the 'channel bit' is set in channels
#define CHANNEL_IS_FREE(channels, channel) (channels & (1L<<(63 - channel)))

using namespace LibPhantom;

unsigned int FirewireDeviceLibraw1394::number_of_open_devices = 0;
unsigned int FirewireDeviceLibraw1394::max_open_devices = 2;
FirewireDeviceLibraw1394** FirewireDeviceLibraw1394::open_devices = (FirewireDeviceLibraw1394**) malloc(
    sizeof(FirewireDeviceLibraw1394**) * FirewireDeviceLibraw1394::max_open_devices);

FirewireDeviceLibraw1394::FirewireDeviceLibraw1394(u_int32_t port, nodeid_t node) :
  port(port), node(node)
{
  if (number_of_open_devices == max_open_devices)
  {
    max_open_devices += 2;
    open_devices = (FirewireDeviceLibraw1394**) realloc(open_devices, sizeof(FirewireDeviceLibraw1394**)
        * FirewireDeviceLibraw1394::max_open_devices);
    // TODO Recover & throw error if failed
  }
  open_devices[number_of_open_devices] = this;
  number_of_open_devices++;
  handle = raw1394_new_handle_on_port(port);
  // TODO throw error if failed

  // TODO on busreset the irm node needs to be updated!
  irm_node = raw1394_get_irm_id(handle) - 0xffc0;
}

FirewireDeviceLibraw1394::~FirewireDeviceLibraw1394()
{
  // 'Close' the current Firewire Device
  number_of_open_devices--;
  unsigned int i;
  for (i = 0; i < number_of_open_devices; i++)
    if (open_devices[i] == this)
      break;
  for (i++; i < number_of_open_devices; i++)
    open_devices[i - 1] = open_devices[i];

  raw1394_destroy_handle( handle);
}

bool FirewireDeviceLibraw1394::deviceIsOpen(u_int32_t port, nodeid_t node)
{
  for (unsigned int i = 0; i < number_of_open_devices; i++)
    if (open_devices[i]->port == port && open_devices[i]->node == node)
      return true;
  return false;
}

unsigned int FirewireDeviceLibraw1394::getFreeChannel()
{
  int i;
  octlet_t channels;
  quadlet_t *channelsq = (quadlet_t *) &channels;
  read(irm_node, CHANNELS_AVAILABLE_ADDR, (char *) &channels, sizeof(octlet_t));

  // Convert to a more convenient order
  for (i = 0; i < 2; i++)
  {
    channelsq[i] = ntohl(channelsq[i]);
  }
  channels = ((octlet_t) channelsq[0]) << 32 | channelsq[1]; // swap quadlets

  for (i = 0; i < 64; i++)
  {
    if (CHANNEL_IS_FREE(channels, i))
    {
      return i;
    }
  }

  // TODO Create some library exception and throw that one
  throw "No free isochronous channels available";
}

void FirewireDeviceLibraw1394::claimChannel(unsigned int channel)
{
  if (raw1394_channel_modify(handle, channel, RAW1394_MODIFY_ALLOC))
  {
    throw "Failed to claim channel";
  }
}

void FirewireDeviceLibraw1394::releaseChannel(unsigned int channel)
{
  if (raw1394_channel_modify(handle, channel, RAW1394_MODIFY_FREE))
  {
    throw "Failed to claim channel";
  }
}

u_int32_t FirewireDeviceLibraw1394::getPort()
{
  return port;
}

void FirewireDeviceLibraw1394::read(nodeid_t node, u_int64_t address, char *buffer, unsigned int length)
{
  if (raw1394_read(handle, node | 0xffc0, address, length, (quadlet_t *) buffer))
  {
    if (errno != EAGAIN)
    {
      // TODO Create some library exception and throw that one
      char *buffer = new char[256];
      sprintf(buffer, "Failed to read data at address 0x%lx from device %d: (%d) %s\n", address, node, errno, strerror(
          errno));
      throw buffer;
    }
  }
}

void FirewireDeviceLibraw1394::read(u_int64_t address, char *buffer, unsigned int length)
{
  read(node, address, buffer, length);
}

void FirewireDeviceLibraw1394::write(nodeid_t node, u_int64_t address, char *buffer, unsigned int length)
{
  if (raw1394_write(handle, node, address, length, (quadlet_t *) buffer))
  {
    if (errno != EAGAIN)
    {
      // TODO Create some library exception and throw that one
      char *buffer = new char[256];
      sprintf(buffer, "Failed to read data at address 0x%lx from device %d: (%d) %s\n", address, node, errno, strerror(
          errno));
      throw buffer;
    }
  }
}

void FirewireDeviceLibraw1394::write(u_int64_t address, char *buffer, unsigned int length)
{
  write(node, address, buffer, length);
}
