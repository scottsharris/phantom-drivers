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
  * Phantom Library Communications: Generic implementation of communication functionalities
  */

#include <sys/types.h>

namespace LibPhantom
{
  struct config_rom {
    unsigned int node; // Not part of config rom, but used to cache the config rom structs (and find them back)

    u_int32_t magic_num;
    unsigned char motu_fw_audio:1;
    unsigned char irm_cap:1;
    unsigned char cycle_master_cap:1;
    unsigned char iso_cap:1;
    unsigned char bus_manager_cap:1;
    unsigned char cycle_clk_accuracy;
    unsigned int max_async_bwrite_payload;
    unsigned int link_speed;
    union
    {
      struct {
        u_int32_t guid_lo;
        u_int32_t guid_hi;
      };
      u_int64_t guid;
    };
    u_int32_t node_capabilities;
    u_int32_t vendor_id;
    u_int32_t unit_spec_id;
    u_int32_t unit_sw_version;
    u_int32_t model_id;

    char *vendor;
  };
}

#include "lp-com.h"

// Depending on which FW_METHOD is selected, add headerfile for static implementations
#ifdef USE_libraw1394
#include "lp-com-libraw1394.h"
#endif

using namespace LibPhantom;

int Communication::ports = -1; // Indicates that the number of ports is not yet read

Communication::Communication() : port(-1), nodes(-1)
{
}

Communication::~Communication()
{
}

Communication* Communication::createInstance()
{
#ifdef USE_libraw1394
  return new CommunicationLibraw1394;
#endif
}

int Communication::getPorts()
{
  if(ports == -1)
#ifdef USE_libraw1394
    ports =  CommunicationLibraw1394::getPorts();
#endif
  return ports;
}

int Communication::setPort(int port)
{
  // TODO Generate error
  if(this->port != -1 || port < 0 || port > getPorts())
    return -1;

  this->port = port;
  return 0;
}

unsigned int Communication::getNumberOfNodes()
{
  if(port == -1)
    return 0; // Not conencted to a port yet
  // TODO Upon bus reset: nodes needs to be set to -1 again!
  if(nodes == -1)
    nodes = getRealNumberOfNodes();

  return (unsigned int) nodes;
}

unsigned int Communication::getVendorId(unsigned int node)
{
  if(node > getNumberOfNodes())
    return 0;

  struct config_rom *crom = getConfigRom(node);
  return crom == 0 ? 0 : crom->vendor_id;
}

struct config_rom* Communication::getConfigRom(unsigned int node)
{
  // TODO Check if it is already in cache

  // TODO fill struct (see reveng/probe-node.c)

  return 0;
}
