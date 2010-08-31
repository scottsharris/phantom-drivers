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

/**
 * This application tests probe-node.c */

#include "probe-node.h"

#include "libraw1394/raw1394.h"

int main()
{

  // Get amount of ports (firewire cards)
  raw1394handle_t h0 = raw1394_new_handle();
  int ports = raw1394_get_port_info(h0, 0, 0);
  raw1394_destroy_handle(h0);

  printf("Found %d port(s)\n", ports);
  int port;
  if(ports > 1)
  {
      printf("Which port do you want to select?");
      port = 0; // Select port here
  }
  else
  {
    port = 0;
  }

  raw1394handle_t scan_handle = raw1394_new_handle_on_port(port);
  int nodes = raw1394_get_nodecount(scan_handle);
  if(nodes < 0)
  {
    printf("Failed to read number of nodes...\n");
    return 1;
  }
  printf("Found %d nodes:\n", nodes);
  
  int i;
  nodeaddr_t node;
  struct config_rom_t crom;
  for(i = 0; i < nodes; i++)
  {
      node = (1023<<6) | i; // 1023 is the local bus id
      if(probe_node(scan_handle, node, &crom) == 0)
      {
          printf("Node 0x%x:\n", node);
	  printf("Vendor id: 0x%8.8x %s\n", crom.vendor_id, crom.vendor);
	  printf("Model id: 0x%x\n", crom.model_id);
	  printf("\n\n");
      }
  }

  return 1;
}
