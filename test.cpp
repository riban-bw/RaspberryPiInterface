/*  This is a test program demonstrating and validating the functionality of ribanRpiInterface
    Copyright riban 2019
    Author: Brian Walton brian@riban.co.uk
*/

#include "ribanRpiInterface.h"
#include <cstdio>
#include <unistd.h>

#define BUTTON 4

int main(int argc, char* argv[])
{
    printf("%s\n", ribanRpiInterface::GetModel().c_str());

    ribanRpiInterface rpi;
    if(rpi.IsInit())
        printf("Raspberry Pi Interface intialised\n");
    else
    {
        printf("Raspberry Pi Interface failed to intialise\n");
        return -1;
    }
    rpi.ConfigureGpi(4, GPI_INPUT_PULLUP);
    bool bPressed = rpi.GetGpi(BUTTON);
    while(true)
    {
        if(rpi.GetGpi(BUTTON, 50) != bPressed)
        {
            bPressed = rpi.GetGpi(BUTTON);
            printf("GPI %d: %s\n", BUTTON, bPressed?"ON":"OFF");
        }
        usleep(1000);
    }
    return 0;
}
