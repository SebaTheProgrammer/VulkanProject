#include "AppBase.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() 
{
    AppBase app{};

    try 
    {
        app.Run();
    }
    catch ( const std::exception& e ) 
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


//Some of the youtube tutorials that I found very helpful:
//https://www.youtube.com/watch?v=W2I0DofOw9M&list=PLn3eTxaOtL2NH5nbPHMK7gE07SqhcAjmk
//https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR
//https://www.youtube.com/watch?v=dHPuU-DJoBM&list=PLv8Ddw9K0JPg1BEO-RS-0MYs423cvLVtj

