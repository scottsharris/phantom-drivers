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
  * Phantom Library Communications: Define class to communicate using libraw1394
  */

#include "lp-com.h"
#include "libraw1394/raw1394.h"

namespace LibPhantom {
  class CommunicationLibraw1394 : public Communication
  {
  public:
    CommunicationLibraw1394();
    virtual ~CommunicationLibraw1394();

    /**
     * @returns the number of available ports on the system, or -1 if an error occurred
     */
    static int getPorts();

    /**
     * Read data from given node and address on the selected port (see setPort())
     *
     * @return 0 when there where no errors
     */
    virtual int read(unsigned int node, unsigned long address, char *buffer, unsigned int length);

    /**
     * Write data to given node and address on the selected port (see setPort())
     *
     * @return 0 when there where no errors
     */
    virtual int write(unsigned int node, unsigned long address, char *buffer, unsigned int length);

  protected:
    /**
     * @return the number of nodes connected to the selected port
     */
    virtual unsigned int getRealNumberOfNodes();

    /**
     * List with raw1394 handles, each connected to its own port
     * The list is created when getPorts() is called for the first time
     * (by then it is known how much handles needs to be created)
     */
    static raw1394handle_t *handles;
  };
}
