/**
 * @file x430_host_sim.c
 * @brief Host simulator: emulated X430 actuators on a serial port or a pseudo-terminal.
 *
 * Project: sfs-dynamixel-x430 - DYNAMIXEL X430 Control Table Emulator
 * Author: Anton Vynohradov
 * Email: avynohradov@systemfromscratch.com
 *
 * @copyright Copyright (c) 2026 Anton Vynohradov
 * @license Apache License 2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * @details Runs simulated devices (x430_sim) behind a POSIX serial line so that
 *          unmodified DYNAMIXEL tools talk to them: DynamixelSDK, DYNAMIXEL
 *          Wizard (through a serial adapter), ROS 2 dynamixel_hardware_interface.
 *
 *          x430_host_sim --pty --link /tmp/ttyDXL --ids 1,2,3
 *              creates a pseudo-terminal and a symlink; point the host at
 *              /tmp/ttyDXL (any baud rate).
 *          x430_host_sim --port /dev/ttyUSB0 --baud 1000000 --ids 11,12
 *              serves a real serial port (e.g. a USB-RS485 adapter wired to
 *              another computer).
 *          --nvm <dir> keeps the EEPROM area and backups in files.
 *
 *          The example uses one thread: it polls the line every millisecond,
 *          feeds the bus and steps the simulators. With threads, give the
 *          bus a port with a recursive mutex (struct x430_port).
 */

#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE 700

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "x430.h"

#define HOST_MAX_DEVICES X430_MAX_DEVICES

struct host
{
    int fd;
    const char* nvm_dir;
    struct x430_bus bus;
    struct x430_device devices[HOST_MAX_DEVICES];
    struct x430_sim sims[HOST_MAX_DEVICES];
    struct x430_hooks hooks;
    uint8_t count;
};

static volatile sig_atomic_t g_running = 1;

static void on_signal(int signal_number)
{
    (void)signal_number;
    g_running = 0;
}

/* ------------------------------------------------------------------ port */

static uint32_t host_millis(void* context)
{
    struct timespec now;

    (void)context;
    (void)clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint32_t)((uint64_t)now.tv_sec * 1000U + (uint64_t)now.tv_nsec / 1000000U);
}

static void host_delay_us(void* context, uint32_t us)
{
    const struct timespec delay = {(time_t)(us / 1000000U), (long)(us % 1000000U) * 1000L};

    (void)context;
    (void)nanosleep(&delay, NULL);
}

static int host_write(void* context, const uint8_t* data, size_t length)
{
    struct host* host = (struct host*)context;
    size_t done = 0;

    while (done < length)
    {
        const ssize_t written = write(host->fd, data + done, length - done);

        if (written < 0)
        {
            if (errno == EAGAIN || errno == EINTR)
            {
                continue;
            }
            return -1;
        }
        done += (size_t)written;
    }
    return 0;
}

/* ------------------------------------------------------- persistence */

static void nvm_path(const struct host* host, const struct x430_device* device,
                     enum x430_nvm_slot slot, char* path, size_t size)
{
    const ptrdiff_t index = device - host->devices;

    (void)snprintf(path, size, "%s/x430_dev%td_%s.bin", host->nvm_dir, index,
                   slot == X430_NVM_EEPROM ? "eeprom" : "backup");
}

static struct host* g_host = NULL;

static int file_nvm_load(struct x430_device* device, enum x430_nvm_slot slot, uint8_t* data,
                         uint16_t size, void* user_data)
{
    char path[512];
    FILE* file;
    size_t got;

    (void)user_data;
    nvm_path(g_host, device, slot, path, sizeof(path));
    file = fopen(path, "rb");
    if (file == NULL)
    {
        return -1;
    }
    got = fread(data, 1U, size, file);
    (void)fclose(file);
    return (got == size) ? 0 : -1;
}

static int file_nvm_store(struct x430_device* device, enum x430_nvm_slot slot, const uint8_t* data,
                          uint16_t size, void* user_data)
{
    char path[512];
    FILE* file;
    size_t put;

    (void)user_data;
    nvm_path(g_host, device, slot, path, sizeof(path));
    file = fopen(path, "wb");
    if (file == NULL)
    {
        return -1;
    }
    put = fwrite(data, 1U, size, file);
    (void)fclose(file);
    return (put == size) ? 0 : -1;
}

static uint8_t host_event(struct x430_device* device, enum x430_event event, void* user_data)
{
    static const char* const names[] = {"reboot",         "factory reset", "clear multi-turn",
                                        "bus watchdog",   "baud rate",     "torque shutdown",
                                        "backup restored"};
    uint8_t id = 0U;

    (void)x430_device_id(device, &id);
    (void)fprintf(stderr, "[ID %u] event: %s\n", (unsigned)id, names[(int)event]);
    return x430_sim_hooks.event(device, event, user_data);
}

/* ------------------------------------------------------------- line */

static speed_t baud_constant(long baud)
{
    switch (baud)
    {
        case 9600:
            return B9600;
        case 57600:
            return B57600;
        case 115200:
            return B115200;
        case 230400:
            return B230400;
#ifdef B460800
        case 460800:
            return B460800;
#endif
#ifdef B1000000
        case 1000000:
            return B1000000;
#endif
#ifdef B2000000
        case 2000000:
            return B2000000;
#endif
#ifdef B3000000
        case 3000000:
            return B3000000;
#endif
#ifdef B4000000
        case 4000000:
            return B4000000;
#endif
        default:
            return B0;
    }
}

static int make_raw(int fd, long baud)
{
    struct termios tio;

    if (tcgetattr(fd, &tio) != 0)
    {
        return -1;
    }
    cfmakeraw(&tio);
    tio.c_cflag |= CLOCAL | CREAD;
    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 0;
    if (baud != 0)
    {
        const speed_t speed = baud_constant(baud);

        if (speed == B0)
        {
            (void)fprintf(stderr, "unsupported baud rate %ld\n", baud);
            return -1;
        }
        (void)cfsetispeed(&tio, speed);
        (void)cfsetospeed(&tio, speed);
    }
    return tcsetattr(fd, TCSANOW, &tio);
}

static int open_pty(const char* link)
{
    const int fd = posix_openpt(O_RDWR | O_NOCTTY);
    const char* name;

    if ((fd < 0) || (grantpt(fd) != 0) || (unlockpt(fd) != 0) || ((name = ptsname(fd)) == NULL))
    {
        perror("pty");
        return -1;
    }
    {
        /* Configure the slave side raw as well, so no line discipline mangles packets. */
        const int slave = open(name, O_RDWR | O_NOCTTY);

        if (slave >= 0)
        {
            (void)make_raw(slave, 0);
            (void)close(slave);
        }
    }
    (void)make_raw(fd, 0);
    if (link != NULL)
    {
        (void)unlink(link);
        if (symlink(name, link) != 0)
        {
            perror("symlink");
        }
    }
    (void)printf("x430_host_sim: serving on %s%s%s\n", name, link ? " -> " : "", link ? link : "");
    (void)fflush(stdout);
    return fd;
}

/* ------------------------------------------------------------- main */

static void usage(const char* program)
{
    (void)fprintf(stderr,
                  "usage: %s (--pty [--link PATH] | --port DEVICE [--baud N]) [--ids 1,2,...]\n"
                  "          [--nvm DIR]\n"
                  "model: %s (Model Number %u), up to %u devices\n",
                  program, X430_MODEL_NAME, (unsigned)X430_MODEL_NUMBER,
                  (unsigned)HOST_MAX_DEVICES);
}

int main(int argc, char** argv)
{
    static struct host host;
    static const struct x430_port port = {NULL, NULL, NULL, host_millis, host_delay_us};
    const char* device_path = NULL;
    const char* link = NULL;
    const char* ids = "1";
    long baud = 1000000;
    bool use_pty = false;

    for (int arg = 1; arg < argc; arg++)
    {
        if ((strcmp(argv[arg], "--pty") == 0))
        {
            use_pty = true;
        }
        else if ((strcmp(argv[arg], "--link") == 0) && (arg + 1 < argc))
        {
            link = argv[++arg];
        }
        else if ((strcmp(argv[arg], "--port") == 0) && (arg + 1 < argc))
        {
            device_path = argv[++arg];
        }
        else if ((strcmp(argv[arg], "--baud") == 0) && (arg + 1 < argc))
        {
            baud = strtol(argv[++arg], NULL, 10);
        }
        else if ((strcmp(argv[arg], "--ids") == 0) && (arg + 1 < argc))
        {
            ids = argv[++arg];
        }
        else if ((strcmp(argv[arg], "--nvm") == 0) && (arg + 1 < argc))
        {
            host.nvm_dir = argv[++arg];
        }
        else
        {
            usage(argv[0]);
            return 2;
        }
    }
    if (use_pty == (device_path != NULL))
    {
        usage(argv[0]);
        return 2;
    }

    host.fd = use_pty ? open_pty(link) : open(device_path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if ((host.fd < 0) || (!use_pty && (make_raw(host.fd, baud) != 0)))
    {
        perror(device_path != NULL ? device_path : "pty");
        return 1;
    }

    g_host = &host;
    host.hooks = x430_sim_hooks;
    host.hooks.event = host_event;
    if (host.nvm_dir != NULL)
    {
        host.hooks.nvm_load = file_nvm_load;
        host.hooks.nvm_store = file_nvm_store;
    }

    {
        const struct x430_bus_config config = {host_write, &host, &port};

        (void)x430_bus_init(&host.bus, &config);
    }
    for (char* token = strtok((char*)ids, ","); (token != NULL) && (host.count < HOST_MAX_DEVICES);
         token = strtok(NULL, ","))
    {
        struct x430_device* device = &host.devices[host.count];
        struct x430_sim* sim = &host.sims[host.count];
        const struct x430_device_config config = {(uint8_t)atoi(token), 0U, 0U, &host.hooks, sim};

        if ((x430_sim_init(sim, device, NULL) != X430_OK) ||
            (x430_device_init(device, &config) != X430_OK) ||
            (x430_bus_attach(&host.bus, device) != X430_OK))
        {
            (void)fprintf(stderr, "cannot create device ID %s (duplicate or invalid)\n", token);
            return 1;
        }
        {
            uint8_t id = 0U;

            (void)x430_device_id(device, &id);
            (void)printf("x430_host_sim: %s ID %u\n", X430_MODEL_NAME, (unsigned)id);
        }
        host.count++;
    }
    (void)fflush(stdout);

    (void)signal(SIGINT, on_signal);
    (void)signal(SIGTERM, on_signal);

    {
        uint32_t last = host_millis(NULL);

        while (g_running)
        {
            struct pollfd fds = {host.fd, POLLIN, 0};
            uint8_t buffer[512];

            if (poll(&fds, 1, 1) > 0)
            {
                const ssize_t got = read(host.fd, buffer, sizeof(buffer));

                if (got > 0)
                {
                    (void)x430_bus_receive(&host.bus, buffer, (size_t)got);
                }
                else if ((got < 0) && (errno != EAGAIN) && (errno != EIO))
                {
                    perror("read");
                    break;
                }
                else if ((got < 0) && (errno == EIO))
                {
                    /* pty without a client yet */
                    (void)usleep(1000);
                }
            }
            {
                const uint32_t now = host_millis(NULL);

                if (now != last)
                {
                    for (uint8_t index = 0; index < host.count; index++)
                    {
                        (void)x430_sim_step(&host.sims[index], now - last);
                    }
                    (void)x430_bus_poll(&host.bus);
                    last = now;
                }
            }
        }
    }
    if (link != NULL)
    {
        (void)unlink(link);
    }
    (void)close(host.fd);
    return 0;
}
