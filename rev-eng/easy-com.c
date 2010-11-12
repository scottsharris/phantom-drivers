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

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "libraw1394/raw1394.h"

#include "easy-com.h"

int write_data_char(raw1394handle_t h, nodeid_t node, nodeaddr_t address, unsigned char buf)
{
  int ret;
  while((ret = raw1394_write(h, node, address, 1, (quadlet_t *) &buf)) && errno == EAGAIN)
  {
    printf("waiting to write data\n");
    usleep(100000); // wait 100ms, device is not ready, since this is part of the initialisation it is no problem (from a real-time perspective) to retry
  }

  if(ret)
    fprintf(stderr, "Failed to write char: (%d) %s\n", errno, strerror(errno));
  return ret;
}

int write_data(raw1394handle_t h, nodeid_t node, nodeaddr_t address, size_t length, quadlet_t *buf)
{
  int ret;
  while((ret = raw1394_write(h ,node, address, length, buf)) && errno == EAGAIN)
    usleep(100000); // wait 100ms, device is not ready, since this is part of the initialisation it is no problem (from a real-time perspective) to retry

  if(ret)
    fprintf(stderr, "Failed to write data: (%d) %s\n", errno, strerror(errno));
  return ret;
}


/**
 * @return false when expected_response did not match received response (or an error occurred)
 */
int get_expected_char(raw1394handle_t h, nodeid_t node, nodeaddr_t address, const unsigned char expected_response, unsigned char *buf)
{
  int ret;
  while((ret = raw1394_read(h, node, address, 1, (quadlet_t *) buf)) && errno == EAGAIN)
    usleep(100000); // wait 100ms, device is not ready, since this is part of the initialisation it is no problem (from a real-time perspective) to retry

  if(ret)
  {
    fprintf(stderr, "Failed to read value: (%d) %s\n", errno, strerror(errno));
    return 1;
  }
  return *buf != expected_response;
}

/**
 * @return false when expected_response did not match received response (or an error occurred)
 */
int get_expected_quadlet(raw1394handle_t h, nodeid_t node, nodeaddr_t address, const quadlet_t expected_response, quadlet_t *buf)
{
  int ret;
  while((ret = raw1394_read(h, node, address, sizeof(quadlet_t), buf)) && errno == EAGAIN)
    usleep(100000); // wait 100ms, device is not ready, since this is part of the initialisation it is no problem (from a real-time perspective) to retry)

  if(ret)
  {
    fprintf(stderr, "Failed to read value: (%d) %s\n", errno, strerror(errno));
    return 1;
  }
  return *buf != expected_response;
}

/**
 * Tries to read data from given handle, node and address. When it fails with an EAGAIN it will wait some time and tries again.
 *
 * @return 1 when an error occurred (see errno)
 */
int read_data_char(raw1394handle_t h, nodeid_t node, nodeaddr_t address, unsigned char *buf)
{
  int ret;
  while((ret = raw1394_read(h, node, address, 1, (quadlet_t *) buf)) && errno == EAGAIN)
    usleep(100000); // wait 100ms, device is not ready, since this is part of the initialisation it is no problem (from a real-time perspective) to retry)

  if(ret)
    fprintf(stderr, "Failed to read data: (%d) %s\n", errno, strerror(errno));
  return ret;
}

/**
 * Tries to read data from given handle, node and address. When it fails with an EAGAIN it will wait some time and tries again.
 *
 * @return 1 when an error occurred (see errno)
 */
int read_data(raw1394handle_t h, nodeid_t node, nodeaddr_t address, size_t length, unsigned char *buf)
{
  int ret;
  while((ret = raw1394_read(h, node, address, length, (quadlet_t *) buf)) && errno == EAGAIN)
    usleep(100000); // wait 100ms, device is not ready, since this is part of the initialisation it is no problem (from a real-time perspective) to retry)

  if(ret)
    fprintf(stderr, "Failed to read data: (%d) %s\n", errno, strerror(errno));
  return ret;
}

