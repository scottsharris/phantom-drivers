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
 * Defines some functions which will retry reading/writing when an EAGAIN is encountered
 */

#ifndef __easy_com_h
#define __easy_com_h

#include "libraw1394/raw1394.h"

int write_data_char(raw1394handle_t h, nodeid_t node, nodeaddr_t address, unsigned char buf);
int write_data(raw1394handle_t h, nodeid_t node, nodeaddr_t address, size_t length, quadlet_t *buf);

/**
 * @return false when expected_response did not match received response (or an error occurred)
 */
int get_expected_char(raw1394handle_t h, nodeid_t node, nodeaddr_t address, const unsigned char expected_response, unsigned char *buf);

/**
 * @return false when expected_response did not match received response (or an error occurred)
 */
int get_expected_quadlet(raw1394handle_t h, nodeid_t node, nodeaddr_t address, const quadlet_t expected_response, quadlet_t *buf);

/**
 * Tries to read data from given handle, node and address. When it fails with an EAGAIN it will wait some time and tries again.
 *
 * @return 1 when an error occurred (see errno)
 */
int read_data_char(raw1394handle_t h, nodeid_t node, nodeaddr_t address, unsigned char *buf);

/**
 * Tries to read data from given handle, node and address. When it fails with an EAGAIN it will wait some time and tries again.
 *
 * @return 1 when an error occurred (see errno)
 */
int read_data(raw1394handle_t h, nodeid_t node, nodeaddr_t address, size_t length, unsigned char *buf);

#endif
