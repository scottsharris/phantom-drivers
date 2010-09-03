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

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>

//TODO Take a look at the fields and remove the ones which do not influence finding PHANTOM devices (ie vendor name)
namespace LibPhantom
{
  struct config_rom {
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

    // Not part of config rom, but used to cache the config rom structs (and find them back)
    unsigned int port;
    unsigned int node;
  };
}

#include "lp-com.h"

// Depending on which FW_METHOD is selected, add headerfile for static implementations
#ifdef USE_libraw1394
#include "lp-com-libraw1394.h"
#endif

#define CSR_REGISTER_BASE  0xfffff0000000ULL
#define CONFIG_ROM_ADDR    CSR_REGISTER_BASE + 0x400

using namespace LibPhantom;

int Communication::ports = -1; // Indicates that the number of ports is not yet read
unsigned int Communication::number_of_config_roms = 0;
struct config_rom** Communication::config_roms = 0; // Nothing is cached yet

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
  if(this->port != -1 || port < 0 || port >= getPorts())
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

char *Communication::getVendorName(unsigned int node)
{
  if(node > getNumberOfNodes())
    return 0;

  struct config_rom *crom = getConfigRom(node);
  return crom == 0 ? 0 : crom->vendor;
}

struct config_rom* Communication::getConfigRom(unsigned int node)
{
  unsigned int i;

  // Try cached values
  for(i = 0; i < number_of_config_roms; i++)
    if(config_roms[i]->port == (unsigned int ) port && config_roms[i]->node == node)
      return config_roms[i];

  u_int32_t quadlet;
  unsigned long addr = CONFIG_ROM_ADDR;
  unsigned int node_id = (1023<<6) | node;
  struct config_rom crom;
  memset(&crom, 0, sizeof(struct config_rom));
  crom.port = port;
  crom.node = node;

  /* Read the length of the bus info block */
  if(read(node_id, addr, (char *) &quadlet, 4))
    // TODO Generate an error (and everywhere else in this function)
    return 0;
  quadlet = ntohl(quadlet);

  /* If the length isn't 4 it means the node doesn't have a general ROM
   * format and instead contains only a 24 bit vendor ID.
   */
  if(quadlet>>24 != 4)
  {
    crom.vendor_id = quadlet & 0x00ffffff;
  }
  else
  {
    unsigned int len, value;
    unsigned int unit_dir_addr = 0, textual_leaf_addr = 0;

    /* First data quadlet of bus info block in config ROM */
    addr += 4;
    if(read(node_id, addr, (char *) &quadlet, 4))
      return 0;

    // Should be a magic number (not interesting enough to store)
    if(quadlet != 0x34393331) {
//      fprintf(stderr,"node 0x%x: wrong config ROM magic number 0x%08x\n", node, quadlet);
      return 0;
    }

    /* Second data quadlet of bus info block in config ROM */
    addr += 4;
    if(read(node_id, addr, (char *) &quadlet, 4))
      return 0;
    quadlet = ntohl(quadlet);

    crom.irm_cap = quadlet >> 31;
    crom.cycle_master_cap = (quadlet>>30) & 0x01;
    crom.iso_cap = (quadlet>>29) & 0x01;
    crom.bus_manager_cap = (quadlet>>28) & 0x01;
    crom.cycle_clk_accuracy = (quadlet>>16) & 0xff;
    crom.max_async_bwrite_payload = 2<<((quadlet>>12) & 0x0f);
    crom.link_speed = quadlet & 7;

    /* Third data quadlet of bus info block in config ROM */
    addr += 4;
    if(read(node_id, addr, (char *) &quadlet, 4))
     return 0;
    quadlet = ntohl(quadlet);

    crom.vendor_id = quadlet >> 8;
    crom.guid_hi = quadlet & 0xff;

    /* Forth data quadlet of bus info block in config ROM */
    addr += 4;
    if(read(node_id, addr, (char *) &quadlet, 4))
      return 0;
    quadlet = ntohl(quadlet);

    crom.guid_lo = quadlet;

    /* The next quadlet contains the length of the root directory in 
     * quadlets (32 bits).
     */
    addr += 4;
    if(read(node_id, addr, (char *) &quadlet, 4))
     return 0;
    quadlet = ntohl(quadlet);

    len = quadlet >> 16;
    if(len > 16)
    {
//      fprintf(stderr,"node 0x%x: unexpected large root dir length %d set to 16\n", node, len);
      len = 16;
    }

    /* Scan the root directory for entries of interest */
    for(i = 0; i < len; i++)
    {
      addr += 4;
      if(read(node_id, addr, (char *) &quadlet, 4))
        return 0;
      quadlet = ntohl(quadlet);
      /* These quadlets are key-value pairs; the key is in the upper 8 bits
       * bits of the quadlet.
       */
      value = quadlet & 0x00ffffff;
      switch (quadlet >> 24)
      {
           case 0x0c: crom.node_capabilities = value; break;
           case 0x03:
//             if(value != crom.vendor_id)
//               fprintf(stderr,"node 0x%x: vendor ID mismatch: 0x%06x (bus info block) vs 0x%06x (root dir)\n", node, crom.vendor_id, value);
              break;
           case 0xd1: unit_dir_addr = addr + value * 4; break;
           case 0x8d:
             /* addr + value * 4 is address of Node_unique_id leaf in
              * config ROM.  Silently ignore this since we don't need
              * it at present.
              */
             break;
           case 0x81: textual_leaf_addr = addr + value * 4; break;
//           default: fprintf(stderr,"node 0x%x: unknown root dir key 0x%02x seen\n",node, quadlet >> 24);
      }
    }

    /* Next come the unit directories. If there was an entry in the root
     * directory indicating a unit directory was present, get its length and
     * parse it.
     */
    if(unit_dir_addr > 0)
    {
      addr = unit_dir_addr;
      if(read(node_id, addr, (char *) &quadlet, 4))
      {
//        fprintf(stderr, "Failed to read length of unit dir\n");
        return 0;
      }
      quadlet = ntohl(quadlet);
      len = quadlet >> 16;
//      fprintf(stderr,"node 0x%x: unit directory length is %d quadlets\n",node,len);

      for(i = 0; i < len; i++)
      {
        addr += 4;
        if(read(node_id, addr, (char *) &quadlet, 4))
          return 0;
        quadlet = ntohl(quadlet);
        /* Also, the unit dir has key-value pairs */
        value = quadlet & 0x00ffffff;
        switch (quadlet >> 24)
        {
          case 0x12: crom.unit_spec_id = value; break;
          case 0x13: crom.unit_sw_version = value; break;
          case 0x17: crom.model_id = value; break;
        }
      }
    }

    if(textual_leaf_addr > 0)
    {
      addr = textual_leaf_addr;
      if(!read(node_id, addr, (char *) &quadlet, 4))
      {
        quadlet = ntohl(quadlet);
        len = quadlet >> 16;
//        fprintf(stderr,"node 0x%x: textual leaf length is %d quadlets\n",node,len);

// Unused?(always 0)
// Encoding for vendor text?
//        addr += 4;
//        if(read_config_rom_quadlet(handle,node,addr,&quadlet) < 0) 
//          return 0;
//        printf("language_specifier_id %x\n", quadlet >> 8);

// Unused?(always 0)
// Encoding for vendor text?
//        addr += 4;
//        if(read_config_rom_quadlet(handle,node,addr,&quadlet) < 0) 
//          return 0;
//        printf("language_id %x\n", quadlet);

        len = (len - 2) * 4;
        crom.vendor = new char[len];

        addr += 12; // 4 + 8 (Skip language_specifier_id and langude_id)
        if(read(node_id, addr, crom.vendor, len))
        {
          delete crom.vendor;
          crom.vendor = 0;
        }
      }
    }
  }

  config_roms = (struct config_rom **) realloc((void *) config_roms, sizeof(struct config_rom *) * (number_of_config_roms + 1));
  // TODO Check if allocation succeeded
  config_roms[number_of_config_roms] = new struct config_rom;
  memcpy(config_roms[number_of_config_roms], &crom, sizeof(struct config_rom));
  number_of_config_roms++;

  return config_roms[number_of_config_roms - 1];
}
