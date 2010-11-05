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

#include <netinet/in.h>



#include "lp-com.h"

// Depending on which FW_METHOD is selected, add headerfile for static implementations
#ifdef USE_libraw1394
#include "lp-com-libraw1394.h"
#endif
#ifdef USE_macosx
#include "lp-com-macosx.h"
#endif

#define CSR_REGISTER_BASE  0xfffff0000000ULL
#define CONFIG_ROM_ADDR    CSR_REGISTER_BASE + 0x400

using namespace LibPhantom;


Communication::Communication()
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
#ifdef USE_macosx
  return new CommunicationMacOSX;
#endif
  return NULL;
}

FirewireDevice::FirewireDevice() :configRomRead(false),configRomValid(false)
{
}

FirewireDevice::~FirewireDevice() {

}


unsigned int FirewireDevice::getVendorId()
{

  struct config_rom *crom = getConfigRom();
  return crom == 0 ? 0 : crom->vendor_id;
}

char *FirewireDevice::getVendorName()
{
  struct config_rom *crom = getConfigRom();
  return crom == 0 ? 0 : crom->vendor;
}

bool FirewireDevice::isSensableDevice()
{
  // Check if node has a SensAble device
  if(getVendorId() != 0x000b99)
    return false;

  // Read vendor id from device memory as well
  uint32_t vendor;
  read(0x1006000c, (char *) &vendor, 4);
  //TODO: ntoh
  if (vendor != 0x00990b00) return false;

  return true;
}

struct config_rom* FirewireDevice::getConfigRom()
{
	if (!configRomRead) {
		readConfigRom();
		configRomRead=true;
	}

	if (!configRomValid) return 0;

	return &configRom;
}

void FirewireDevice::readConfigRom() {
  unsigned int i;

  u_int32_t quadlet;
  unsigned long addr = CONFIG_ROM_ADDR;

  memset(&configRom, 0, sizeof(struct config_rom));


  /* Read the length of the bus info block */
  read(addr, (char *) &quadlet, 4);

  quadlet = ntohl(quadlet);

  /* If the length isn't 4 it means the node doesn't have a general ROM
   * format and instead contains only a 24 bit vendor ID.
   */
  if(quadlet>>24 != 4)
  {
    configRom.vendor_id = quadlet & 0x00ffffff;
  }
  else
  {
    unsigned int len, value;
    unsigned int unit_dir_addr = 0, textual_leaf_addr = 0;

    /* First data quadlet of bus info block in config ROM */
    addr += 4;
    read(addr, (char *) &quadlet, 4);

    // Should be a magic number (not interesting enough to store)
    if(quadlet != 0x34393331) {
//      fprintf(stderr,"node 0x%x: wrong config ROM magic number 0x%08x\n", node, quadlet);
      return;
    }

    /* Second data quadlet of bus info block in config ROM */
    addr += 4;
    read(addr, (char *) &quadlet, 4);
    quadlet = ntohl(quadlet);

    configRom.irm_cap = quadlet >> 31;
    configRom.cycle_master_cap = (quadlet>>30) & 0x01;
    configRom.iso_cap = (quadlet>>29) & 0x01;
    configRom.bus_manager_cap = (quadlet>>28) & 0x01;
    configRom.cycle_clk_accuracy = (quadlet>>16) & 0xff;
    configRom.max_async_bwrite_payload = 2<<((quadlet>>12) & 0x0f);
    configRom.link_speed = quadlet & 7;

    /* Third data quadlet of bus info block in config ROM */
    addr += 4;
    read(addr, (char *) &quadlet, 4);

    quadlet = ntohl(quadlet);

    configRom.vendor_id = quadlet >> 8;
    configRom.guid_hi = quadlet & 0xff;

    /* Forth data quadlet of bus info block in config ROM */
    addr += 4;
    read(addr, (char *) &quadlet, 4);

    quadlet = ntohl(quadlet);

    configRom.guid_lo = quadlet;

    /* The next quadlet contains the length of the root directory in 
     * quadlets (32 bits).
     */
    addr += 4;
    read(addr, (char *) &quadlet, 4);

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
      read(addr, (char *) &quadlet, 4);

      quadlet = ntohl(quadlet);
      /* These quadlets are key-value pairs; the key is in the upper 8 bits
       * bits of the quadlet.
       */
      value = quadlet & 0x00ffffff;
      switch (quadlet >> 24)
      {
           case 0x0c: configRom.node_capabilities = value; break;
           case 0x03:
//             if(value != configRom.vendor_id)
//               fprintf(stderr,"node 0x%x: vendor ID mismatch: 0x%06x (bus info block) vs 0x%06x (root dir)\n", node, configRom.vendor_id, value);
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
      read(addr, (char *) &quadlet, 4);

      quadlet = ntohl(quadlet);
      len = quadlet >> 16;
//      fprintf(stderr,"node 0x%x: unit directory length is %d quadlets\n",node,len);

      for(i = 0; i < len; i++)
      {
        addr += 4;
        read(addr, (char *) &quadlet, 4);
        quadlet = ntohl(quadlet);
        /* Also, the unit dir has key-value pairs */
        value = quadlet & 0x00ffffff;
        switch (quadlet >> 24)
        {
          case 0x12: configRom.unit_spec_id = value; break;
          case 0x13: configRom.unit_sw_version = value; break;
          case 0x17: configRom.model_id = value; break;
        }
      }
    }

    if(textual_leaf_addr > 0)
    {
      addr = textual_leaf_addr;
      read(addr, (char *) &quadlet, 4);

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

        //TODO: fix this memory leak (if a device is deleted or if the next read fails)
        configRom.vendor = new char[len];

        addr += 12; // 4 + 8 (Skip language_specifier_id and langude_id)
        read(addr, configRom.vendor, len);


    }
  }
  configRomValid=true;
}
