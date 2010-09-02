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

#include "probe-node.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h> // ntohl

#include "libraw1394/csr.h"
#define CONFIG_ROM_ADDR    CSR_REGISTER_BASE + CSR_CONFIG_ROM

int read_config_rom(raw1394handle_t handle, unsigned node, unsigned int cr_offset, quadlet_t *buffer, unsigned int length)
{
	unsigned long long int offset = CONFIG_ROM_ADDR + cr_offset;
	unsigned int phy_id = (1023 << 6) | node;

	if (raw1394_read(handle, phy_id, offset, length * 4, buffer) < 0)
	{
		fprintf(stderr,"node 0x%x: error reading %d quadlets config ROM offset 0x%x\n  (%d) %s\n", node, length, cr_offset, errno, strerror(errno));
		return 1;
	}
	return 0;
}

int read_config_rom_quadlet(raw1394handle_t handle, unsigned int node, unsigned int cr_offset, quadlet_t *quadlet)
{
	*quadlet = 0;
	if(read_config_rom(handle, node, cr_offset, quadlet, 1) < 0)
		return 1;
	*quadlet = ntohl(*quadlet);
	return 0;
}

int probe_node(raw1394handle_t handle, int node, struct config_rom_t *crom)
{
	quadlet_t quadlet;
	unsigned int unit_dir_offset = 0, offset = 0, textual_leaf_offset = 0;
	unsigned int i, value, len;

	memset(crom, 0, sizeof(struct config_rom_t));

	/* Read the length of the bus info block */
	if (read_config_rom_quadlet(handle,node,offset,&quadlet) < 0)
		return 1;
//  	fprintf(stderr,"node 0x%x: Bus info block length: %d\n", node, quadlet>>24);

	/* If the length isn't 4 it means the node doesn't have a general ROM
	 * format and instead contains only a 24 bit vendor ID.
	 */
	if (quadlet>>24 != 4)
	{
		crom->vendor_id = quadlet & 0x00ffffff;
		return 0;
	}

	/* First data quadlet of bus info block in config ROM */
	offset += 4;
	if (read_config_rom_quadlet(handle,node,offset,&quadlet) < 0) {
		return 1;
	}
	crom->magic_num = quadlet;
	if (quadlet != 0x31333934) {
		fprintf(stderr,"node 0x%x: wrong config ROM magic number 0x%08x\n", node, quadlet);
    		return 1;
	}

	/* Second data quadlet of bus info block in config ROM */
	offset += 4;
	if (read_config_rom_quadlet(handle,node,offset,&quadlet) < 0)
	    return 1;

	crom->irm_cap = quadlet >> 31;
	crom->cycle_master_cap = (quadlet>>30) & 0x01;
	crom->iso_cap = (quadlet>>29) & 0x01;
	crom->bus_manager_cap = (quadlet>>28) & 0x01;
	crom->cycle_clk_accuracy = (quadlet>>16) & 0xff;
	crom->max_async_bwrite_payload = 2<<((quadlet>>12) & 0x0f);
	crom->link_speed = quadlet&7;

	/* Third data quadlet of bus info block in config ROM */
	offset += 4;
	if (read_config_rom_quadlet(handle,node,offset,&quadlet) < 0)
	    return 1;

	crom->vendor_id = quadlet >> 8;
	crom->guid_hi = quadlet & 0xff;

	/* Forth data quadlet of bus info block in config ROM */
	offset += 4;
	if (read_config_rom_quadlet(handle,node,offset,&quadlet) < 0)
		return 1;

	crom->guid_lo = quadlet;

	/* The next quadlet contains the length of the root directory in 
	 * quadlets.
	 */
	offset += 4;
	if (read_config_rom_quadlet(handle,node,offset,&quadlet) < 0)
	    return 1;
  
	len = quadlet >> 16;
	if (len > 16)
	{
		fprintf(stderr,"node 0x%x: unexpected large root dir length %d set to 16\n", node, len);
		len = 16;
	}

	/* Scan the root directory for entries of interest */
	for(i = 0; i < len; i++)
	{
		offset += 4;
		if (read_config_rom_quadlet(handle,node,offset,&quadlet) < 0)
			return 1;
		/* These quadlets are key-value pairs; the key is in the upper 8 bits
		 * bits of the quadlet.
		 */
		value = quadlet & 0x00ffffff;
		switch (quadlet >> 24)
		{
			case 0x0c: crom->node_capabilities = value; break;
			case 0x03:
				if (value != crom->vendor_id)
					fprintf(stderr,"node 0x%x: vendor ID mismatch: 0x%06x (bus info block) vs 0x%06x (root dir)\n", node, crom->vendor_id, value);
		                 break;
			case 0xd1: unit_dir_offset = offset+value*4; break;
			case 0x8d: 
				/* offset+value*4 is address of Node_unique_id leaf in
				 * config ROM.  Silently ignore this since we don't need
				 * it at present.
				 */
				break;
			case 0x81: textual_leaf_offset = offset+value*4; break;
			default: fprintf(stderr,"node 0x%x: unknown root dir key 0x%02x seen\n",node, quadlet >> 24);
		}
	}

	/* Next come the unit directories.  If there was an entry in the root
	 * directory indicating a unit directory was present, get its length and
	 * parse it.
	 */
	if (unit_dir_offset > 0)
	{
		offset = unit_dir_offset;
		if (read_config_rom_quadlet(handle,node,offset,&quadlet) < 0) 
			return 1;
		len = quadlet >> 16;
//		fprintf(stderr,"node 0x%x: unit directory length is %d quadlets\n",node,len);

		for(i = 0; i < len; i++)
		{
			offset += 4;
			if (read_config_rom_quadlet(handle,node,offset,&quadlet) < 0) 
				return 1;
			/* Again, the unit dir has key-value pairs */
			value = quadlet & 0x00ffffff;
			switch (quadlet >> 24)
			{
				case 0x12: crom->unit_spec_id = value; break;
				case 0x13: crom->unit_sw_version = value; break;
				case 0x17: crom->model_id = value; break;
			}
		}
	}

	if (textual_leaf_offset > 0)
	{
		offset = textual_leaf_offset;

		if (read_config_rom_quadlet(handle,node,offset,&quadlet) < 0) 
			return 1;
		len = quadlet >> 16;
//		fprintf(stderr,"node 0x%x: textual leaf length is %d quadlets\n",node,len);
	
// Unused?(always 0)
// Encoding for vendor text?
//		offset += 4;
//		if (read_config_rom_quadlet(handle,node,offset,&quadlet) < 0) 
//			return 1;
//		printf("language_specifier_id %x\n", quadlet >> 8);

// Unused?(always 0)
// Encoding for vendor text?
//		offset += 4;
//		if (read_config_rom_quadlet(handle,node,offset,&quadlet) < 0) 
//			return 1;
//		printf("language_id %x\n", quadlet);

		len -= 2;
		crom->vendor = (char *) malloc(len * 4);

		offset += 12; // 4 + 8 (Skip language_specifier_id and langude_id)
		if (read_config_rom(handle, node, offset, (quadlet_t *) crom->vendor, len) < 0)
			return 1;
	}

	return 0;
}
