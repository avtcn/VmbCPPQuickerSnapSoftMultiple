/*=============================================================================
  Copyright (C) 2013 - 2017 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        program.cpp

  Description: Implementation of main entry point of AsynchronousGrabConsole
               example of VimbaCPP.

-------------------------------------------------------------------------------

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
  NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
  DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#include <string>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <conio.h>


#include "VimbaCPP/Include/VimbaCPP.h"
#include "ApiController.h"

#include "CameraHandle.h"


/*
CHAR getch() {
    DWORD mode, cc;
    HANDLE h = GetStdHandle( STD_INPUT_HANDLE );

    if (h == NULL) {
        return 0; // console not found
    }

    GetConsoleMode( h, &mode );
    SetConsoleMode( h, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT) );
    TCHAR c = 0;
    ReadConsole( h, &c, 1, &cc, NULL );
    SetConsoleMode( h, mode );
    return c;
}
*/

int main( int argc, char* argv[] )
{
    VmbErrorType err = VmbErrorSuccess;

    std::cout<<"///////////////////////////////////////////\n";
    std::cout<<"/// Vimba API Asynchronous Grab Example ///\n";
    std::cout<<"///////////////////////////////////////////\n\n";

    //////////////////////
    //Parse command line//
    //////////////////////
    AVT::VmbAPI::Examples::ProgramConfig Config;
    err = Config.ParseCommandline( argc, argv);
    //Write out an error if we could not parse the command line
    if ( VmbErrorBadParameter == err )
    {
        std::cout<< "Invalid parameters!\n\n" ;
        Config.setPrintHelp( true );
    }

    //Print out help and end program
    if ( Config.getPrintHelp() )
    {
        Config.PrintHelp( std::cout );
    }
    else
    {
        AVT::VmbAPI::Examples::ApiController apiController;
        
        // Print out version of Vimba
        std::cout<<"Vimba C++ API Version "<<apiController.GetVersion()<<"\n";

#if 0
        // Startup Vimba
        err = apiController.StartUp();        
        if ( VmbErrorSuccess == err )
        {
            if( Config.getCameraID().empty() )
            {
                AVT::VmbAPI::CameraPtrVector cameras = apiController.GetCameraList();
                if( cameras.empty() )
                {
                    err = VmbErrorNotFound;
                }
                else
                {
                    std::string strCameraID;
                    err = cameras[0]->GetID( strCameraID );
                    if( VmbErrorSuccess == err )
                    {
                        Config.setCameraID( strCameraID );
                    }
                }
            }
            if ( VmbErrorSuccess == err )
            {
                std::cout<<"Opening camera with ID: "<<Config.getCameraID()<<"\n";

                err = apiController.StartContinuousImageAcquisition( Config );

                if ( VmbErrorSuccess == err )
                {
                    std::cout<< "Press <enter> to stop acquisition...\n" ;
                    getchar();

                    apiController.StopContinuousImageAcquisition();
                }
            }

            apiController.ShutDown();
        }

        if ( VmbErrorSuccess == err )
        {
            std::cout<<"\nAcquisition stopped.\n" ;
        }
        else
        {
            std::string strError = apiController.ErrorCodeToMessage( err );
            std::cout<<"\nAn error occurred: " << strError << "\n";
        }
#else
        // Startup Vimba
        err = apiController.StartUp();        

        AVT::VmbAPI::Examples::CameraHandle camera1;
        //err = apiController.OpenCamera("DEV_000F314CA646", camera1);
        err = apiController.OpenCamera("DEV_1AB22C0019F9", camera1);
        if ( VmbErrorSuccess != err )
        {
            std::cout<<"\nFailed to open camera !!!\n" ;
        }
        else
        {
            std::cout << "\nSucessfully opened camera done !" << "\n";
        }


        std::cout << "Press <q> to exit ...\n";

        while(1)
        {
            char pressed = _getch();

            if (pressed == ' ') { 
                std::string strID = apiController.GetCameraID(camera1);
                std::string strSN = apiController.GetSerialNumber(camera1);
                std::cout << strID << ", " << strSN << std::endl;

                std::vector<VmbUchar_t> imageData;
                apiController.QuickSnap(camera1, imageData);

                char old_fill_char = std::cout.fill('0');
                std::cout << std::hex << "R = 0x" << std::setw(2) << (int)imageData[0] << " "
                    << "G = 0x" << std::setw(2) << (int)imageData[1] << " "
                    << "B = 0x" << std::setw(2) << (int)imageData[2] << std::dec << "\n";
                std::cout.fill(old_fill_char);
            }
            else {
            }

            if (pressed == 'q'|| pressed == 'Q')
                break;
        } 
        std::cout << std::endl;




        apiController.CloseCamera(camera1);

        apiController.ShutDown();
#endif

    }

    return err;
}
