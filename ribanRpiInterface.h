#pragma once
#include <stdint.h> //Provides fixed length integer types
#include <string> //Provides std::string

//Library modes
#define RRPI_ENABLE_ALL     0xFFFFFFFF
#define RRPI_ENABLE_GPI     0x01

//GPI modes
#define GPI_INPUT           0x00
#define GPI_OUTPUT          0x01
#define GPI_FN0             0x04
#define GPI_FN1             0x05
#define GPI_FN2             0x06
#define GPI_FN3             0x07
#define GPI_FN4             0x03
#define GPI_FN5             0x02
#define GPI_INPUT_PULLDOWN  0x08
#define GPI_INPUT_PULLUP    0x10

/** riban Raspberry Pi Interface Library provides C++ bindings and wrapper to many Raspberry Pi interfaces<br/>
    Implemented features:
        Raspberry Pi model (string and number)
        Configure GPI pins for each function type
        GPI input with pull-up / pull down
        GPI output
        Millisecond counter (32-bit / 49 day)
    Not (yet) implemented:
        PWM
*/
class ribanRpiInterface
{
    public:
        /** @brief  Create riban Raspberry Pi interface object
        *   @param  flags Optional flags to enable / disable features
        */
        ribanRpiInterface(uint64_t flags = RRPI_ENABLE_ALL);
        virtual ~ribanRpiInterface();

        /** @brief  Get the Raspberry Pi model description
        *   @retval string Model description
        */
        static std::string GetModel();

        /** @brief  Get Raspberry Pi model number
        *   @retval uint8_t Model number [0,1,2 or 0xFF for unknown]
        */
        static uint8_t GetModelNumber();

        /** @brief  Configure a GPI pin
        *   @param  gpi GPI pin number
        *   @param  flags Configuration flags [GPI_INPUT | GPI_INPUT_PULLDOWN | GPI_INPUT_PULLUP |GPI_OUTPUT]
        *   @retval bool True on success
        */
        bool ConfigureGpi(uint8_t gpi, uint8_t flags);

        /** @brief  Get the value of a GPI input
        *   @param  gpi GPI pin number
        *   @param  debounce Quantity of milliseconds to ignore changes [Default: none]
        *   @retval bool True if GPI input asserted
        */
        bool GetGpi(uint8_t gpi, time_t debounce = 0);

        /** @brief  Set GPI output value
        *   @param  gpi GPI pin number
        *   @param  value True to assert GPI output
        */
        void SetGpi(uint8_t gpi, bool value);

        /** @brief  Is library initialised?
        *   @retval bool True if library is initialised
        */
        bool IsInit();

        /** @brief  Get the quantity of seconds since epoch
        *   @retval time_t Quantity of seconds
        */
        time_t GetSeconds();

        /** @brief  Get the quantity of milliseconds since epoch
        *   @retval uint64_t Quantity of milliseconds
        *   @note   If cast to uint32_t this will wrap at 49 days, 17 hours, 2 minutes, 47 seconds, 295ms
        */
        uint64_t GetMillis();

        /** @brief  Get the quantity of microseconds since epoch
        *   @retval uint64_t Quantity of milliseconds
        *   @note   If cast to uint32_t this will wrap at 71 minutes, 34 seconds, 967ms, 295us
        */
        uint64_t GetMicros();

    protected:

    private:
        bool initgpi(); //Initialises GPI returns true on success
        void uninitgpi(); //Uninitalises GPI
        void * m_pMap; // Memory map of GPI area
        volatile uint32_t * m_pGpiMap; //Pointer to GPI map
        bool m_bUnInit; // False when initialised
        timespec m_ts; // Time structure holds current time (updated by get time functions)
};
