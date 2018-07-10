/**
* sonometer.cpp
* Implementation of the Sonometer class with all the methods needed
* to request and retrieve data.
*
* Contributor: Sébastien Maes
*/
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <ctime>
#include <stdint.h>
#include <system_error>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>

#include <errno.h>
#include <SensorsDatabase.h>
#include <Sonometer.h>

// Class Constructor
Sonometer::Sonometer() : port_fd(-1) {}

bool Sonometer::openPort(string port)
{
    bool status = true;

    closePort();

    port_fd = open(port.c_str(), O_RDWR | O_NOCTTY);
    if (port_fd < 0)
    {
        throw system_error(errno, system_category());
    }

    if (!configurePort(port_fd))
    {
        closePort();
        status = false;
    }

    return status;
}

bool Sonometer::closePort(void)
{
    bool status = close(port_fd) != -1;
    port_fd = -1;
    return status;
}

bool Sonometer::isPortOpen(void) const
{
    return port_fd > -1;
}

int Sonometer::sendRequest(void)
{
    unsigned char cmd[] = {'r', '\0'};
    
    // Flush Port
    sleep(1);
    tcflush(port_fd, TCIFLUSH);

    int nb_bytes = write(port_fd, cmd, sizeof(cmd) - 1);

    if (nb_bytes < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
    {
        throw system_error(errno, system_category());
    }
    return nb_bytes;
}

bool Sonometer::isInitFrameReceived(char *buffer) const
{
    bool ret = false;
    string buffer_str;
    buffer_str = buffer;

    if (buffer_str.find("BAT") != -1)
    {
        ret = true;
    }

    return ret;
}

int Sonometer::readData(char *buffer)
{
    string buffer_str;
    
    nb_bytes = read(port_fd, buffer, BUFFER_SIZE * sizeof(char) - 1);

    // DEBUG
    buffer_str = buffer;
    cout << "==READ DEBUG==" << endl;
    cout << "Read bytes: " << nb_bytes << endl;
    cout << "Data: " << buffer_str << endl;

    if (nb_bytes < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
    {
        throw system_error(errno, system_category());
    }

    // Check if it's bad data
    if (isBadData(buffer))
    {
        cout << "BAD DATA" << endl;
    }

    // Remove \r (Carriage return)
    buffer[nb_bytes - 1] = '\0';

    return nb_bytes;
}

void Sonometer::displayData(char *buffer) const
{
    if (isInitFrameReceived(buffer) == false)
    {
        cout << "LA10:" << m_la10 << endl
             << "LA50:" << m_la50 << endl
             << "LA90:" << m_la90 << endl
             << "average LAeq:" << m_avgLaEq << endl
             << "Max Frequency:" << m_freqMax << endl
             << "Average CGS:" << m_avgCGS << endl
             << "n Event LAeq:" << m_nEvtLaEq << endl
             << "t Event LAeq:" << m_tEvtLaEq << endl
             << "n Event 60dB:" << m_nEvt60db << endl
             << "t Event 60dB:" << m_tEvt60db << endl;
    }
}

time_t Sonometer::getMonotonicTime(void) const
{
    struct timespec spec;
    if (clock_gettime(CLOCK_MONOTONIC, &spec) != 0)
    {
        throw system_error(errno, system_category());
    }
    return spec.tv_sec;
}

bool Sonometer::configurePort(int fd)
{
    struct termios options;

    if (tcgetattr(fd, &options) == -1)
    {
        throw system_error(errno, system_category());
    }

    /* Set Baud Rate */
    cfsetospeed(&options, B115200);
    cfsetispeed(&options, B115200);

    options.c_cflag &= ~PARENB; // Make 8n1
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag &= ~CRTSCTS;
    options.c_lflag = 0; // no signaling chars, no echo, no canonical processing
    options.c_oflag = 0;
    options.c_cc[VMIN] = 0;  // read doesn't block
    options.c_cc[VTIME] = 5; // 0.5 second read timeout

    options.c_cflag |= CREAD | CLOCAL; // turn on READ & ignore ctrl lines
    options.c_iflag |= INLCR | IGNCR;
    options.c_iflag &= ~(IXON | IXOFF | IXANY);         // turn off s/w flow ctrl
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    options.c_oflag &= ~OPOST;                          // make raw

    /* Flush Port, then applies attributes */
    tcflush(fd, TCIFLUSH);

    if (tcsetattr(fd, TCSANOW, &options) == -1)
    {
        throw system_error(errno, system_category());
    }

    return true;
}

bool Sonometer::isBadData(char *buffer)
{
    bool ret = false;
    string buffer_str;
    buffer_str = buffer;

    // Bad read
    if (nb_bytes <= 0)
    {
        ret = true;
    }

    // If unwanted characters are read.
    // "H#" is used only to test if the baudrate is correct
    if (buffer_str.compare("H#") == 0)
    {
        nb_bytes = 0;
        ret = true;
    }

    // Incomplete data frame
    if (nb_bytes < 21 || !isFrameComplete(buffer))
    {
        ret = true;
    }

    return ret;
}

bool Sonometer::isFrameComplete(char *buffer) const
{
    int i = 0, nb_data = 0;
    bool ret = true;

    while (buffer[i] != '\0')
    {
        if (buffer[i] == FRAME_SEPARATOR)
        {
            nb_data++;
        }

        i++;
    }

    if ((nb_data + 1) < 11)
    {
        ret = false;
    }

    return ret;
}

void Sonometer::clear(char *buffer) const
{
    memset(buffer, '\0', BUFFER_SIZE);
}


void Sonometer::processData(char *buffer, SensorsDatabase *db)
{
    char la10_str[5], la50_str[5], la90_str[5], avgLaEq_str[5], freqMax_str[5], avgCGS_str[5], nEvtLaEq_str[5], tEvtLaEq_str[5], nEvt60db_str[5], tEvt60db_str[5];
    int i = 0, j = 0, k = 0, l = 0, m = 0, n = 0, o = 0, p = 0, q = 0, r = 0, t = 0;

    /*
        FORMAT OF A FRAME SENT BY THE SONOMETER:
        BATTERY>LA10>LA50>LA90>avgLaEq>freqMax>avgCGS>nEvtLaEq>tEvtLaEq>nEvt60db>tEvt60db
    */

    if (!isBadData(buffer) && isInitFrameReceived(buffer) == false)
    {

        while (buffer[i] != FRAME_SEPARATOR && buffer[i] != '\0') // Ignore the battery field as we don't need it
        {
            i++;
        }
        i++;
        while (buffer[i] != FRAME_SEPARATOR && buffer[i] != '\0') // LA10
        {
            la10_str[j] = buffer[i];
            j++;
            la10_str[j] = '\0';
            i++;
        }
        i++;
        while (buffer[i] != FRAME_SEPARATOR && buffer[i] != '\0') // LA50
        {
            la50_str[k] = buffer[i];
            k++;
            la50_str[k] = '\0';
            i++;
        }
        i++;
        while (buffer[i] != FRAME_SEPARATOR && buffer[i] != '\0') // LA90
        {
            la90_str[l] = buffer[i];
            l++;
            la90_str[l] = '\0';
            i++;
        }
        i++;
        while (buffer[i] != FRAME_SEPARATOR && buffer[i] != '\0') // Average LAeq
        {
            avgLaEq_str[m] = buffer[i];
            m++;
            avgLaEq_str[m] = '\0';
            i++;
        }
        i++;
        while (buffer[i] != FRAME_SEPARATOR && buffer[i] != '\0') // Max frequency
        {
            freqMax_str[n] = buffer[i];
            n++;
            freqMax_str[n] = '\0';
            i++;
        }
        i++;
        while (buffer[i] != FRAME_SEPARATOR && buffer[i] != '\0') // Average CGS
        {
            avgCGS_str[o] = buffer[i];
            o++;
            avgCGS_str[o] = '\0';
            i++;
        }
        i++;
        while (buffer[i] != FRAME_SEPARATOR && buffer[i] != '\0') // Next Event LAeq
        {
            nEvtLaEq_str[p] = buffer[i];
            p++;
            nEvtLaEq_str[p] = '\0';
            i++;
        }
        i++;
        while (buffer[i] != FRAME_SEPARATOR && buffer[i] != '\0')
        {
            tEvtLaEq_str[q] = buffer[i];
            q++;
            tEvtLaEq_str[q] = '\0';
            i++;
        }
        i++;
        while (buffer[i] != FRAME_SEPARATOR && buffer[i] != '\0') // Next Event 60dB
        {
            nEvt60db_str[r] = buffer[i];
            r++;
            nEvt60db_str[r] = '\0';
            i++;
        }
        i++;
        while (buffer[i] != '\0')
        {
            tEvt60db_str[t] = buffer[i];
            t++;
            tEvt60db_str[t] = '\0';
            i++;
        }

        // STORE DATA INTO CLASS ATTRIBUTES
        // (TO DISPLAY THEM IN THE CONSOLE)
        m_la10 = atof(la10_str) / 10.0;
        m_la50 = atof(la50_str) / 10.0;
        m_la90 = atof(la90_str) / 10.0;
        m_avgLaEq = atof(avgLaEq_str) / 10.0;
        m_freqMax = atof(freqMax_str);
        m_avgCGS = atof(avgCGS_str);
        m_nEvtLaEq = atof(nEvtLaEq_str);
        m_tEvtLaEq = atof(tEvtLaEq_str);
        m_nEvt60db = atof(nEvt60db_str);
        m_tEvt60db = atof(tEvt60db_str);

        // Store data in sqlite3 database "sonometer.db"
        time_t poll_time = time(NULL);
        db->create_poll(poll_time);

        db->push_data("sound_la10", poll_time, m_la10);
        db->push_data("sound_la50", poll_time, m_la50);
        db->push_data("sound_la90", poll_time, m_la90);
        db->push_data("sound_avg_laeq", poll_time, m_avgLaEq);
        db->push_data("sound_fmax", poll_time, m_freqMax);
        db->push_data("sound_avg_cgs", poll_time, m_avgCGS);
        db->push_data("sound_n_evt_laeq", poll_time, m_nEvtLaEq);
        db->push_data("sound_t_evt_laeq", poll_time, m_tEvtLaEq);
        db->push_data("sound_n_evt_60db", poll_time, m_nEvt60db);
        db->push_data("sound_t_evt_60db", poll_time, m_tEvt60db);

        db->commit_poll(poll_time);
    }
    else if (isBadData(buffer) && isInitFrameReceived(buffer) == false)
    {
        cout << "DATA NOT PROCESSED" << endl;
    }

}
