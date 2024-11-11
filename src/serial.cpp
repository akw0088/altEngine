//=============================================================================
// This file is part of the altEngine distribution
// (https://github.com/akw0088/altEngine/)
// Copyright (c) 2018 Alexander Wright All Rights Reserved.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON - INFRINGEMENT.IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//=============================================================================

#include "serial.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef WIN32
int serial_init(char *port, handle_t *handle)
{
	HANDLE hSerial;
	DCB dcb;

	hSerial = CreateFile(port, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hSerial == INVALID_HANDLE_VALUE)
	{
		printf("Unable to open com port %s\n", port);
		return -1;
	}

	GetCommState(hSerial, &dcb);
	dcb.DCBlength = sizeof(DCB);
	dcb.BaudRate = CBR_115200; // CBR_2400, CBR_4800 CBR_9600 CBR_14400 CBR_19200 CBR_38400
	dcb.ByteSize = 8;
	dcb.StopBits = ONESTOPBIT;
	dcb.Parity = NOPARITY;

	if (SetCommState(hSerial, &dcb) == 0)
	{
		printf("Unable to set port params\n");
		return -1;
	}


	COMMTIMEOUTS timeouts;

	// read timeout
	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;
	if (!SetCommTimeouts(hSerial, &timeouts))
	{
		printf("Unable to set port params\n");
		return -1;
	}

	*handle = hSerial;

	return 0;
}


int serial_write(handle_t handle, char *data, int size)
{
	HANDLE hSerial = handle;
	DWORD nWrote = 0;

	WriteFile(hSerial, data, size, &nWrote, NULL);

	return nWrote;
}

int serial_read(handle_t handle, char *data, int size)
{
	HANDLE hSerial = handle;
	DWORD nRead = 0;


	ReadFile(hSerial, data, size, &nRead, NULL);

	return nRead;
}

void serial_close(handle_t handle)
{
	HANDLE hSerial = handle;

	CloseHandle(hSerial);
}
#endif

#ifdef __linux__
int serial_init(char *port, handle_t *handle)
{
	struct termios options;
	int fd;

	fd = open(port, O_RDWR | O_NOCTTY);

	if (fd == -1)
	{
		perror("open failed");
		return -1;
	}

	fcntl(fd, F_SETFL, 0);
	tcgetattr(fd, &options);
	options.c_iflag = 0;
	options.c_iflag |= (IGNBRK);
	options.c_oflag = 0;
	cfsetispeed(&options, B115200);
	cfsetospeed(&options, B115200);
	options.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
	options.c_cflag |= (CS8 | CLOCAL | CREAD);
	options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	options.c_cc[VTIME] = 10;
	options.c_cc[VMIN] = 0;
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &options);

	*handle = fd;

	return 0;
}

int serial_write(handle_t handle, char *data, int size)
{
	int fd = handle;

	return write(fd, data, size);
}

int serial_read(handle_t handle, char *data, int size)
{
	int fd = handle;

	return read(fd, data, size);
}

void serial_close(handle_t handle)
{
	int fd = handle;

	close(fd);
}

#endif


#ifdef __APPLE__
int serial_init(char *port, handle_t *handle)
{
    int fd;

    fd = open(port, O_RDWR | O_NOCTTY);

    if (fd == -1)
    {
        perror("open failed");
        return -1;
    }

    // set baud rate in apple fashion here

    *handle = fd;

    return 0;
}

int serial_write(handle_t handle, char *data, int size)
{
    int fd = handle;

    return write(fd, data, size);
}

int serial_read(handle_t handle, char *data, int size)
{
    int fd = handle;

    return read(fd, data, size);
}

void serial_close(handle_t handle)
{
    int fd = handle;

    close(fd);
}
#endif
