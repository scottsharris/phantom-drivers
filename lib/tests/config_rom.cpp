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
  * Test application to test the Communication::getRonfigRom() functionality
  */

#include <stdio.h>
#include <sys/types.h>

#include "lp-com-libraw1394.h"

int main()
{
  using namespace LibPhantom;

  Communication *com = Communication::createInstance();

  if(com->setPort(0))
  {
    printf("Unable to select port 0, are there any firewire ports available?!\n");
    return 1;
  }

  printf("%u Node(s) available on port 0\n", com->getNumberOfNodes());

  for(unsigned int node = 0; node < com->getNumberOfNodes(); node++)
    printf("Node %u -> vendor: 0x%6.6x %s\n", node, com->getVendorId(node), com->getVendorName(node)?com->getVendorName(node):"");
}
