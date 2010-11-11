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
 * Test application to test the isochronous channel functionality
 */

#include <stdio.h>

#include "Communication.h"
#include "DeviceIterator.h"
#include "Phantom.h"

int main()
{
  using namespace LibPhantom;

  try
  {
    // These tests assume that findChannel finds the first free channel available
    // if it just find any free channel available the test might not pass

    DeviceIterator *it = DeviceIterator::createInstance();
    FirewireDevice *dev = it->next();

    delete it;
    if (dev == 0)
    {
      printf("Could not find any firewire devices...\n");
      return 1;
    }

    unsigned int channel = dev->getFreeChannel();
    printf("Found channel: %d\n", channel);

    dev->claimChannel(channel);
    printf("Claimed channel %d:  ", channel);

    unsigned int channel2 = dev->getFreeChannel();
    // channel2 should be unequal to channel, since we claimed that channel and it is not free anymore
    // channel2 = channel + 1 (assuming that no channels are in use before the test started)
    if (channel == channel2)
    {
      printf("Channel %d is still marked as unused...", channel);
      return 1;
    }
    printf("Found next free channel: %d\n", channel2);

    dev->releaseChannel(channel);
    printf("Released channel %d: ", channel);

    unsigned int channel3 = dev->getFreeChannel();
    if (channel != channel3)
    {
      printf("Channel %d is still marked as claimed...\n", channel);
      return 1;
    }
    printf("Found next free channel: %d\n", channel3);

    // Delete dev to be able to find it as a Phantom device if only one is connected (and it would be claimed by dev)
    delete dev;

    // Now enable isochronous communication for a moment
    Phantom *p = Phantom::findPhantom();
    if(p == 0)
    {
      printf("Error: could not find a Phantom...\n");
      return 1;
    }

    p->startPhantom();
    printf("Successfully started isochronous communication with a Phantom\n");
    p->stopPhantom();
    printf("Stopped isochronous communication\n");

    delete p;
  }
  catch (char const* str)
  {
    printf("Exception raised: %s\n", str);
    return 1;
  }

  printf("Tests succeeded!\n");
  return 0;
}
