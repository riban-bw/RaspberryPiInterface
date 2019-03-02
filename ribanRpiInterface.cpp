#include "ribanRpiInterface.h"
#include <unistd.h> //provide usleep
#include <poll.h> //provides poll
#include <fcntl.h> //provides file open constants
#include <cstdio> //provides printf (enable for debug)
#include <pthread.h> //provides threading
#include <time.h> //provides clock
#include <sys/mman.h> //provides mmap

#define MAX_GPI     54
#define BLOCK_SIZE  (1024 * 4)

#define GPSET0      7
#define GPCLR0      10
#define GPLEV0      13
#define GPEDS0      16
#define GPREN0      19
#define GPFEN0      22
#define GPHEN0      25
#define GPLEN0      28
#define GPAREN0     31
#define GPAFEN0     34
#define GPPUD       37
#define GPPUDCLK0   38

static const int8_t anUnavailableGpi[MAX_GPI] = {1,1,0,0,0,0,0,0,0,0,
                                                0,0,0,0,0,0,0,0,0,0,
                                                0,0,0,0,0,0,0,0,0,0,
                                                0,0,1,1,1,1,1,1,1,1,
                                                1,1,1,1,1,1,1,1,1,1,
                                                1,1,1,1}; //Table of unavailable GPI pins

class Debounce
{
    public:
        Debounce() :
            nextTriggerTime(0),
            value(false)
        {};
    time_t  nextTriggerTime;
    bool    value;
};

ribanRpiInterface::ribanRpiInterface(uint64_t flags)
{
    m_bUnInit = true;
    if(flags & RRPI_ENABLE_GPI)
        initgpi();
}

ribanRpiInterface::~ribanRpiInterface()
{
    uninitgpi();
}

std::string ribanRpiInterface::GetModel()
{
    int fd;
    std::string sModel = "";
    if((fd = open("/proc/device-tree/model", O_RDONLY) ) < 0)
    {
        return std::string("Unknown");
    }
    char c;
    while(read(fd, &c, 1) > 0)
        sModel += c;
    close(fd);
    return sModel;
}

uint8_t ribanRpiInterface::GetModelNumber()
{
    /* Model strings:
        Raspberry Pi Model B Rev 2
        Raspberry Pi 2 Model B Rev 1.1
    */
    std::string sModel = GetModel();
    if(sModel.length() > 13)
    {
        uint8_t nModel = sModel[13] - 48; //!@todo Validate GetModelNumber works for each type of Raspberry Pi
        if(nModel < 10)
            return nModel;
        else
            return 1; // Raspberry Pi 1 does not have its number so assume non-numeric digit to be RPi 1
    }
    return -1;
}

bool ribanRpiInterface::GetGpi(uint8_t gpi, time_t debounce)
{
    if(m_bUnInit || gpi > MAX_GPI || anUnavailableGpi[gpi])
        return false;
    if(debounce)
    {
        static Debounce anDebounce[MAX_GPI]; // Only instantiate array of debounce structures if debounce is used
        if(GetMillis() > anDebounce[gpi].nextTriggerTime)
        {
            anDebounce[gpi].nextTriggerTime = GetMillis() + debounce;
            anDebounce[gpi].value = GetGpi(gpi);
        }
        else return anDebounce[gpi].value;
    }
    return(((*(m_pGpiMap + GPLEV0 + gpi / 32)) & (1 << (gpi % 32))) != 0);
}

void ribanRpiInterface::SetGpi(uint8_t gpi, bool value)
{
    if(m_bUnInit || gpi > MAX_GPI || anUnavailableGpi[gpi])
        return;
    if(value)
        *(m_pGpiMap + GPSET0) = 1 << gpi;
    else
        *(m_pGpiMap + GPCLR0) = 1 << gpi;
}

bool ribanRpiInterface::ConfigureGpi(uint8_t gpi, uint8_t flags)
{
    if(m_bUnInit || gpi > MAX_GPI || anUnavailableGpi[gpi])
        return false;
    /*  There are 10 GPI configurations per register. Registers start at GPIO_BASE
        Each configuration consists of three bits defining mode
    */
    //Clear configuration bits
    *(m_pGpiMap + (gpi / 10)) &= ~(7 << ((gpi % 10) * 3)); //reset 3 flags for this gpi
    //Set configuration bits to match requested mode
    *(m_pGpiMap + (gpi / 10)) |= ((flags & 0x07) << ((gpi % 10) * 3)); //Configure for function
    //Set pull-up/down flags then clock into selected pin
    *(m_pGpiMap + GPPUD) = (flags & 0x18) >> 3;
    usleep(1); //Need to wait 150 cycles which is 0.6us on the slowest RPi so let's wait 1us
    *(m_pGpiMap + GPPUDCLK0 + (gpi / 32)) = 1 << (gpi % 32);
    usleep(1); //Need to wait 150 cycles which is 0.6us on the slowest RPi so let's wait 1us
    *(m_pGpiMap + GPPUD) = 0;
    *(m_pGpiMap + GPPUDCLK0 + (gpi /32)) = 0;
    return true;
}

bool ribanRpiInterface::IsInit()
{
    return !m_bUnInit;
}

time_t ribanRpiInterface::GetSeconds()
{
    clock_gettime(CLOCK_MONOTONIC_RAW, &m_ts);
    return m_ts.tv_sec;
}

uint64_t ribanRpiInterface::GetMillis()
{
    clock_gettime(CLOCK_MONOTONIC_RAW, &m_ts);
    return (uint64_t)m_ts.tv_sec * 1000 + (uint64_t)m_ts.tv_nsec / 1000000;
}

uint64_t ribanRpiInterface::GetMicros()
{
    clock_gettime(CLOCK_MONOTONIC_RAW, &m_ts);
    return (uint64_t)m_ts.tv_sec * 1000000 + (uint64_t)m_ts.tv_nsec / 1000;
}

bool ribanRpiInterface::initgpi()
{
    if(!m_bUnInit)
        return true;
    int fd;
    if((fd = open("/dev/gpiomem", O_RDWR|O_SYNC) ) < 0)
        return false;
    m_pMap = mmap(NULL, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd); //Don't need the file open after memory map
    if(m_pMap != MAP_FAILED)
    {
        m_pGpiMap = (volatile uint32_t *)m_pMap;
        m_bUnInit = false;
    }
    return IsInit();
}

void ribanRpiInterface::uninitgpi()
{
    if(m_bUnInit)
        return;
    munmap(m_pMap, BLOCK_SIZE);
    m_bUnInit = true;
}
