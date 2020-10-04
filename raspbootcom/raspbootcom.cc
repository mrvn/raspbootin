/* raspbootcom.cc - upload kernel.img via serial port to the RPi */
/* Copyright (C) 2013 Goswin von Brederlow <goswin-v-b@web.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define _DEFAULT_SOURCE             /* See feature_test_macros(7) */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <termios.h>
#include <signal.h>

#include "scope.h"

#include <string>
#include <system_error>

enum {
      BUF_SIZE = 65536,
};

volatile bool keep_running = true;

// handler invoked by SIGINT or SIGTERM
void stop_running(int) {
  keep_running = false;
}

struct UnixError : public std::system_error::system_error {
  UnixError(const std::string& what_arg)
    : std::system_error::system_error(errno, std::generic_category(), what_arg) { }
  UnixError(const std::string& what_arg, int err_)
    : std::system_error::system_error(err_, std::generic_category(), what_arg) { }

  template<typename CB>
  static void maybe_raise(const std::string& what_arg, CB on_error) {
    if (std::uncaught_exceptions() > 0) {
      // throwing an exception while unwinding would terminate
      perror(what_arg.c_str());
      on_error();
    } else {
      int err = errno;
      on_error();
      throw UnixError(what_arg, err);
    }
  }

  static void maybe_raise(const std::string& what_arg) {
    maybe_raise(what_arg, [](){});
  }

  template<typename CB>
  static ssize_t check(const std::string& what_arg, ssize_t res, CB on_error) {
    // fprintf(stderr, "+++ %s\n\r", what_arg.c_str());
    if (res == -1) {
      maybe_raise(what_arg, on_error);
    }
    return res;
  }

  static ssize_t check(const std::string& what_arg, ssize_t res) {
    return check(what_arg, res, [](){});
  }
};

// send kernel to rpi
void send_kernel(int fd, const char *file) {
  // Open file
  int file_fd = UnixError::check("open kernel",
                                 open(file, O_RDONLY));
  SCOPE_EXIT {
    close(file_fd);
  };

  // Get kernel size
  ssize_t size = UnixError::check("probe kernel size",
                                  lseek(file_fd, 0L, SEEK_END));
  if (size > 0x200000) {
    throw UnixError("kernel too big", 0);
  }
  UnixError::check("rewind kernel",
                   lseek(file_fd, 0L, SEEK_SET));
  fprintf(stderr, "\n\r### sending kernel %s [%zd byte]\n\r", file, size);

  // send kernel size to RPi
  for (int i = 0; keep_running & (i < 4); ++i) {
    char c = (size >> 8 * i) & 0xFF;
    UnixError::check("sending kernel size",
                     write(fd, &c, 1));
  }
  // wait for OK
  char ok_buf[2] = {0};
  for (int i = 0; keep_running && (i < 2); ++i) {
    UnixError::check("reading kernel size response",
                     read(fd, &ok_buf[i], 1));
    if (ok_buf[i] == 0) --i; // retry
  }
  if (ok_buf[0] != 'O' || ok_buf[1] != 'K') {
    fprintf(stderr, "error after sending size, got '%c%c' [0x%02x 0x%02x]\n\r",
            ok_buf[0], ok_buf[1], uint8_t(ok_buf[0]), uint8_t(ok_buf[1]));
    return;
  }

  while(keep_running && (size > 0)) {
    char buf[BUF_SIZE];
    ssize_t pos = 0;
    ssize_t len = UnixError::check("reading kernel",
                                   read(file_fd, buf, BUF_SIZE));
    size -= len;
    while(keep_running && (len > 0)) {
      ssize_t len2 = UnixError::check("sending kernel",
                                      write(fd, &buf[pos], len));
      len -= len2;
      pos += len2;
    }
  }

  fprintf(stderr, "### finished sending\n\r");
}

int main(int argc, char *argv[]) {
  try {
    int serial_fd, max_fd = STDIN_FILENO;
    fd_set rfds, wfds, efds;
    int breaks = 0;
    int exit_code = 0;

    printf("Raspbootcom V1.0\n");

    if (argc != 3) {
      printf("USAGE: %s <dev> <file>\n", argv[0]);
      printf("Example: %s /dev/ttyUSB0 kernel/kernel.img\n", argv[0]);
      exit(EXIT_FAILURE);
    }

    struct termios old_tio, new_tio;
    if (isatty(STDIN_FILENO)) {
      // get the terminal settings for stdin
      UnixError::check("get terminal settings",
                       tcgetattr(STDIN_FILENO, &old_tio));
    }
    SCOPE_EXIT {
      if (isatty(STDIN_FILENO)) {
        // undo settings at exit
        UnixError::check("restoring terminal settings",
                         tcsetattr(STDIN_FILENO, TCSANOW, &old_tio));
      }
    };
    if (isatty(STDIN_FILENO)) {
      new_tio = old_tio;
      // disable canonical mode (buffered i/o) and local echo
      new_tio.c_lflag &= (~ICANON & ~ECHO);

      // set the new settings immediately
      UnixError::check("set terminal settings",
                       tcsetattr(STDIN_FILENO, TCSANOW, &new_tio));
    }

    // add signal handlers to stop running when interrupted
    signal(SIGINT, stop_running);
    signal(SIGTERM, stop_running);

    while(keep_running) {
      // Open serial device
      if ((serial_fd = open(argv[1], O_RDWR | O_NOCTTY | O_NONBLOCK)) == -1) {
        // udev takes a while to change ownership
        // so sometimes one gets EACCESS
        if (errno == ENOENT || errno == ENODEV || errno == EACCES) {
          fprintf(stderr, "\r### Waiting for %s...\r", argv[1]);
          sleep(1);
          continue;
        } else
          throw UnixError("open serial");
      }
      SCOPE_EXIT {
        close(serial_fd);
      };

      // The termios structure, to be configured for serial interface.
      struct termios termios;

      // must be a tty
      if (!isatty(serial_fd)) {
        fprintf(stderr, "%s is not a tty\n\r", argv[1]);
        keep_running = false;
        break;
      }

      // Get the attributes.
      UnixError::check("get attributes",
                       tcgetattr(serial_fd, &termios));

      // So, we poll.
      termios.c_cc[VTIME] = 0;
      termios.c_cc[VMIN] = 0;

      // 8N1 mode, no input/output/line processing masks.
      termios.c_iflag = 0;
      termios.c_oflag = 0;
      termios.c_cflag = CS8 | CREAD | CLOCAL;
      termios.c_lflag = 0;

      // Set the baud rate.
      UnixError::check("set BAUD rate (in)",
                       cfsetispeed(&termios, B115200));
      UnixError::check("set BAUD rate (out)",
                       cfsetospeed(&termios, B115200));

      // Write the attributes.
      UnixError::check("set attributes",
                       tcsetattr(serial_fd, TCSAFLUSH, &termios));

      // Ready to listen
      fprintf(stderr, "### Listening on %s     \n\r", argv[1]);

      // select needs the largeds FD + 1
      if (serial_fd > STDIN_FILENO) {
        max_fd = serial_fd + 1;
      } else {
        max_fd = STDIN_FILENO + 1;
      }

      while(keep_running) {
        // Watch stdin and serial for input.
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(serial_fd, &rfds);

        // Watch stdin and dev for error.
        FD_ZERO(&efds);
        FD_SET(STDIN_FILENO, &efds);
        FD_SET(serial_fd, &efds);

        // Wait for something to happend
        ssize_t num_fds = UnixError::check("select",
                                           select(max_fd, &rfds, &wfds, &efds,
                                                  NULL));
        (void)num_fds;
        //fprintf(stderr, "==> %zd\n\r", num_fds);
        // check for errors
        if (FD_ISSET(STDIN_FILENO, &efds)) {
          fprintf(stderr, "error on STDIN\n");
          keep_running = false;
          exit_code = 1;
          break;
        }
        if (FD_ISSET(serial_fd, &efds)) {
          fprintf(stderr, "error on device\n");
          keep_running = false;
          exit_code = 1;
          break;
        }
        // input from the user, copy to RPi
        if (FD_ISSET(STDIN_FILENO, &rfds)) {
          char c;
          ssize_t len = UnixError::check("read from stdin",
                                         read(STDIN_FILENO, &c, 1));
          if (len == 0) {
            keep_running = false;
            break;
          }
          //fprintf(stderr, "stdin: '%c' [%02x]\n\r", c, uint8_t(c));
          len = UnixError::check("write to serial",
                                 write(serial_fd, &c, 1));
          if (len == 0) {
            break;
          }
        }
        // output from the RPi, copy to STDOUT
        if (FD_ISSET(serial_fd, &rfds)) {
          char c;
          ssize_t len = UnixError::check("read from serial",
                                         read(serial_fd, &c, 1));
          if (len == 0) {
            break;
          }
          //fprintf(stderr, "serial: '%c' [%02x]\n\r", c, uint8_t(c));
          // scan output for tripple break (^C^C^C)
          // send kernel on tripple break, otherwise output text
          if (c == '\x03') {
            ++breaks;
            if (breaks == 3) {
              send_kernel(serial_fd, argv[2]);
              breaks = 0;
            }
          } else {
            while (breaks > 0) {
              ssize_t len = UnixError::check("write to stdout",
                                             write(STDOUT_FILENO,
                                                   "\x03\x03\x03",
                                                   breaks));
              if (len == 0) {
                keep_running = false;
                break;
              }
              breaks -= len;
            }
            if (keep_running) {
              ssize_t len = UnixError::check("write to stdout",
                                             write(STDOUT_FILENO, &c, 1));
              if (len == 0) {
                keep_running = false;
                break;
              }
            }
          }
        }
      }
    } 
    return exit_code;
  } catch(std::exception&) {
    throw;
  }
}
