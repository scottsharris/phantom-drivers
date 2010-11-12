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
 * probe-node reads the device information
 */

#ifndef __probe_node_h
#define __probe_node_h

#include "libraw1394/raw1394.h"

/*
 * A structure to store selected information from a node's config ROM 
 */
struct config_rom_t
{
  quadlet_t       magic_num;
  unsigned char   motu_fw_audio:1;
  unsigned char   irm_cap:1;
  unsigned char   cycle_master_cap:1;
  unsigned char   iso_cap:1;
  unsigned char   bus_manager_cap:1;
  unsigned char   cycle_clk_accuracy;
  unsigned int    max_async_bwrite_payload;
  unsigned int    link_speed;
  union
  {
    struct {
      quadlet_t       guid_lo;
      quadlet_t       guid_hi;
    };
    octlet_t        guid;
  };
  quadlet_t       node_capabilities;
  quadlet_t       vendor_id;
  quadlet_t       unit_spec_id;
  quadlet_t       unit_sw_version;
  quadlet_t       model_id;

  char           *vendor;
};

/*
 * Reads the node information for the given node and store it in the crom struct
 * handle needs to be a valid handle which is connected to the port containing the node to be scanned
 */
extern int probe_node(raw1394handle_t handle, int node, struct config_rom_t *crom);

/*
 * Read a 4 byte (quadlet) value from the config_rom memory range of the given node
 * cr_offset is the offset where to read the quadlet.
 */
int read_config_rom_quadlet(raw1394handle_t handle, unsigned int node, unsigned int cr_offset, quadlet_t *quadlet);

/*
 * Read content of the config_rom memory range of the given node
 * cr_offset is the offset where to read the quadlet and length the amount of quadlets to read
 */
extern int read_config_rom(raw1394handle_t handle, unsigned node, unsigned int cr_offset, quadlet_t *buffer, unsigned int length);

#endif
