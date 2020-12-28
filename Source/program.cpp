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

#define _AFXDLL

#include <afxwin.h>

#include "VimbaCPP/Include/VimbaCPP.h"
#include "ApiController.h"

#include "CameraHandle.h"

#include "Bitmap.h"

using AVT::VmbAPI::Examples::CameraHandle;


UINT CameraProc(LPVOID pParam)
{
    CameraHandle* pCamera = (CameraHandle*)pParam;

    if (pCamera == NULL)
        return 1;   // if pCamera is not valid

    LARGE_INTEGER m_liPerfFreq = { 0 };
    // CPU Performance Tick
    QueryPerformanceFrequency(&m_liPerfFreq);

    // do something with this camera
    while (1)
    {
        Sleep(2000);
        std::string strID = pCamera->GetCameraID();
        std::string strSN = pCamera->GetSerialNumber();
        std::cout << strID << ", " << strSN << std::endl;

        VmbErrorType err;
        std::vector<VmbUchar_t> imageData;
        VmbImage sVmbImageData;
        DWORD dwStart = GetTickCount();

        LARGE_INTEGER m_liPerfStart = { 0 };
        QueryPerformanceCounter(&m_liPerfStart);

        err = pCamera->QuickSnap(imageData, sVmbImageData);
        if (err != VmbErrorSuccess)
        {
            std::cout << "Failed to get QuickSnap()!!!!!!!!!!!!!!! SN: " << strSN << std::endl;
        }
        else
        {
            DWORD dwEnd = GetTickCount();
            LARGE_INTEGER liPerfNow = { 0 };
            QueryPerformanceCounter(&liPerfNow);
            double time = (((liPerfNow.QuadPart - m_liPerfStart.QuadPart) * 1000) * 1.00000f / m_liPerfFreq.QuadPart);

            char old_fill_char = std::cout.fill('0');
            std::cout << std::hex << "R = 0x" << std::setw(2) << (int)imageData[0] << " "
                << "G = 0x" << std::setw(2) << (int)imageData[1] << " "
                << "B = 0x" << std::setw(2) << (int)imageData[2] << std::dec 
                << ", time = " << std::setw(8) << (time) << " ms" << "\n";
            std::cout.fill(old_fill_char);

            // Save current snap photo to disk
            {
                AVTBitmap bitmap;

                bitmap.colorCode = ColorCodeRGB24;
                bitmap.bufferSize = pCamera->GetImageSize() * 3; // TODO: Mono8 also saved in RGB24 format
                bitmap.width = pCamera->m_imgWidth;
                bitmap.height = pCamera->m_imgHeight;

                // Create the bitmap
                if (0 == AVTCreateBitmap(&bitmap, &*imageData.begin()))
                {
                    std::cout << "Could not create bitmap.\n";
                    err = VmbErrorResources;
                }
                else
                {
                    char pFileName[256];
                    sprintf(pFileName, "%s_%010d_CameraProc.bmp", strSN.c_str(), pCamera->GetFrameID());

                    // Save the bitmap
                    int ret = 1;
                    // If you do not want to save image into disk, please comment the line below.
                    ret = AVTWriteBitmapToFile(&bitmap, pFileName);
                    if (0 == ret)
                    {
                        std::cout << "Could not write bitmap to file.\n";
                        err = VmbErrorOther;
                    }
                    else
                    {
                        std::cout << "Bitmap successfully written to file \"" << pFileName << "\"\n";
                        // Release the bitmap's buffer
                        if (0 == AVTReleaseBitmap(&bitmap))
                        {
                            std::cout << "Could not release the bitmap.\n";
                            err = VmbErrorInternalFault;
                        }
                    }
                }

            }
        }
    }

    return 0;   // thread completed successfully
}


int StartNewCameraThread(AVT::VmbAPI::Examples::ApiController & sysController, CameraHandle & camera, const char * strCameraID)
{

    VmbErrorType err = VmbErrorSuccess;
    err = sysController.OpenCamera(strCameraID, camera);
    if (VmbErrorSuccess != err)
    {
        std::cout << "\nFailed to open camera !!!"<< strCameraID <<"\n";
        return 0;
    }
    else
    {
        std::cout << "\nSucessfully opened camera done !" << strCameraID << "\n";
    }

    AfxBeginThread(CameraProc, (LPVOID)&camera);


    return 1; 
}

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
        system("del_bitmps.cmd");

        // Startup Vimba
        err = apiController.StartUp();        


        AVT::VmbAPI::Examples::CameraHandle camera2;
        StartNewCameraThread(apiController, camera2, "DEV_1AB22D01BBB8"); // Model: 1800 U-500m, S/N: A2114

#if 1
        // TODO: add two more cameras.
        AVT::VmbAPI::Examples::CameraHandle camera3;
        StartNewCameraThread(apiController, camera3, "DEV_000F314CA646"); // Manta_G-125B (E0020002)

        AVT::VmbAPI::Examples::CameraHandle camera4;
        StartNewCameraThread(apiController, camera4, "DEV_000F314D5B52"); // Manta G-895B (E0622706)
#endif


        AVT::VmbAPI::Examples::CameraHandle camera1;
#if 1
        StartNewCameraThread(apiController, camera1, "DEV_1AB22C0019F9"); // Model: 1800 U-319m, S/N: 0054P
#else
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
                err = apiController.QuickSnap(camera1, imageData);
                if (err != VmbErrorSuccess)
                {
                    std::cout << "Failed to get QuickSnap()!!!!!!!!!!!!!!!" << std::endl; 
                } 
                else
                {

                    char old_fill_char = std::cout.fill('0');
                    std::cout << std::hex << "R = 0x" << std::setw(2) << (int)imageData[0] << " "
                        << "G = 0x" << std::setw(2) << (int)imageData[1] << " "
                        << "B = 0x" << std::setw(2) << (int)imageData[2] << std::dec << "\n";
                    std::cout.fill(old_fill_char);

                    // Save current snap photo to disk
                    {
                        AVTBitmap bitmap;

                        bitmap.colorCode = ColorCodeRGB24;
                        bitmap.bufferSize = camera1.GetImageSize() * 3; // TODO: Mono8 also saved in RGB24 format
                        bitmap.width = camera1.m_imgWidth;
                        bitmap.height = camera1.m_imgHeight;

                        // Create the bitmap
                        if (0 == AVTCreateBitmap(&bitmap, &*imageData.begin()))
                        {
                            std::cout << "Could not create bitmap.\n";
                            err = VmbErrorResources;
                        }
                        else
                        {
                            char pFileName[256];
                            sprintf(pFileName, "%s_%020d_main_thread.bmp", strSN.c_str(), camera1.GetFrameID());

                            // Save the bitmap
                            if (0 == AVTWriteBitmapToFile(&bitmap, pFileName))
                            {
                                std::cout << "Could not write bitmap to file.\n";
                                err = VmbErrorOther;
                            }
                            else
                            {
                                std::cout << "Bitmap successfully written to file \"" << pFileName << "\"\n";
                                // Release the bitmap's buffer
                                if (0 == AVTReleaseBitmap(&bitmap))
                                {
                                    std::cout << "Could not release the bitmap.\n";
                                    err = VmbErrorInternalFault;
                                }
                            }
                        }

                    }
                }
            }
            else {
            }

            if (pressed == 'q' || pressed == 'Q')
                break;
        }
        std::cout << std::endl; 
        apiController.CloseCamera(camera1);
#endif

        std::cout << "\n\nPress Enter to quit the application ...\n";
        _getch();
        apiController.ShutDown();
#endif

    }

    return err;
}
