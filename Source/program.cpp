﻿/*=============================================================================
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

    std::string strModel = pCamera->GetCameraModel();
    strModel = strModel.replace(strModel.find(" "), 1, "_");

    std::string strID = pCamera->GetCameraID();
    std::string strSN = pCamera->GetSerialNumber();
    std::string strTag = strModel + "_" + strID + "_" + strSN + ": ";
    std::string strFile = "log_" + strModel + "_" + strID + "_" + strSN + ".csv";

    std::ofstream fileLog;
    fileLog.open(strFile);

    // do something with this camera
    while (1)
    {
        Sleep(2000);

        VmbErrorType err;
        std::vector<VmbUchar_t> imageData;
        VmbImage sVmbImageData;
        DWORD dwStart = GetTickCount();

        LARGE_INTEGER m_liPerfStart = { 0 };
        QueryPerformanceCounter(&m_liPerfStart);

        err = pCamera->QuickSnap(imageData, sVmbImageData);
        if (err != VmbErrorSuccess)
        {
            std::cout << strTag << "Failed to get QuickSnap()!!!!!!!!!!!!!!! SN: " << strSN << std::endl;
        }
        else
        {
            DWORD dwEnd = GetTickCount();
            LARGE_INTEGER liPerfNow = { 0 };
            QueryPerformanceCounter(&liPerfNow);
            double time = (((liPerfNow.QuadPart - m_liPerfStart.QuadPart) * 1000) * 1.00000f / m_liPerfFreq.QuadPart);

            char old_fill_char = std::cout.fill('0');
            std::cout << strTag << std::hex << "R = 0x" << std::setw(2) << (int)imageData[0] << " "
                << "G = 0x" << std::setw(2) << (int)imageData[1] << " "
                << "B = 0x" << std::setw(2) << (int)imageData[2] << std::dec
                << std::endl;
            std::cout.fill(old_fill_char);
            std::cout << strTag << "time consumption: " << (time) << " ms" << std::endl;

            // Save current snap photo to disk
            {
                AVTBitmap bitmap;

                //bitmap.colorCode = ColorCodeRGB24;
                //bitmap.bufferSize = pCamera->GetImageSize() * 3; // TODO: Mono8 also saved in RGB24 format
                bitmap.colorCode = ColorCodeMono8;
                bitmap.bufferSize = pCamera->GetImageSize() * 1; // Mono8 
                bitmap.width = (unsigned long)pCamera->m_imgWidth;
                bitmap.height = (unsigned long)pCamera->m_imgHeight;

                // Create the bitmap
                if (0 == AVTCreateBitmap(&bitmap, &*imageData.begin()))
                {
                    std::cout << strTag << "Could not create bitmap.\n";
                    err = VmbErrorResources;
                }
                else
                {
                    char pFileName[256];
                    sprintf(pFileName, "%s_%010d_CameraProc.bmp", strSN.c_str(), pCamera->GetFrameID());

                    // Save log with frame id and time consumption in Snap() function
                    fileLog << std::setw(10) << pCamera->GetFrameID() << ", " << std::setw(10) << std::fixed << std::setprecision(6) << time << std::endl;
                    fileLog.flush();

                    // Save the bitmap
                    int ret = 1;
                    // If you do not want to save image into disk, please comment the line below.
                    ret = AVTWriteBitmapToFile(&bitmap, pFileName);
                    if (0 == ret)
                    {
                        std::cout << strTag << "Could not write bitmap to file.\n";
                        err = VmbErrorOther;
                    }
                    else
                    {
                        std::cout << strTag << "Bitmap successfully written to file \"" << pFileName << "\"\n";
                        // Release the bitmap's buffer
                        if (0 == AVTReleaseBitmap(&bitmap))
                        {
                            std::cout << strTag << "Could not release the bitmap.\n";
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




int SnapAndSave(AVT::VmbAPI::Examples::ApiController & sysController, CameraHandle & camera, std::ofstream & fileLog)
{
    LARGE_INTEGER m_liPerfFreq = { 0 };
    // CPU Performance Tick
    QueryPerformanceFrequency(&m_liPerfFreq);

    std::string strModel = camera.GetCameraModel();
    strModel = strModel.replace(strModel.find(" "), 1, "_");

    std::string strID = camera.GetCameraID();
    std::string strSN = camera.GetSerialNumber();
    std::string strTag = strModel + "_" + strID + "_" + strSN + ": ";
    std::string strFile = "log_" + strModel + "_" + strID + "_" + strSN + ".csv";

    VmbErrorType err = VmbErrorSuccess;
    std::vector<VmbUchar_t> imageData;
    VmbImage sVmbImageData;
    DWORD dwStart = GetTickCount();

    LARGE_INTEGER m_liPerfStart = { 0 };
    QueryPerformanceCounter(&m_liPerfStart);

    err = camera.QuickSnap(imageData, sVmbImageData);
    if (err != VmbErrorSuccess)
    {
        std::cout << strTag << "Failed to get QuickSnap()!!!!!!!!!!!!!!! SN: " << strSN << std::endl;
    }
    else
    {
        DWORD dwEnd = GetTickCount();
        LARGE_INTEGER liPerfNow = { 0 };
        QueryPerformanceCounter(&liPerfNow);
        double time = (((liPerfNow.QuadPart - m_liPerfStart.QuadPart) * 1000) * 1.00000f / m_liPerfFreq.QuadPart);

        char old_fill_char = std::cout.fill('0');
        std::cout << strTag << std::hex << "R = 0x" << std::setw(2) << (int)imageData[0] << " "
            << "G = 0x" << std::setw(2) << (int)imageData[1] << " "
            << "B = 0x" << std::setw(2) << (int)imageData[2] << std::dec
            << std::endl;
        std::cout.fill(old_fill_char);
        std::cout << strTag << "time consumption: " << (time) << " ms" << std::endl;

        // Save current snap photo to disk
        {
            AVTBitmap bitmap;

            //bitmap.colorCode = ColorCodeRGB24;
            //bitmap.bufferSize = camera.GetImageSize() * 3; // TODO: Mono8 also saved in RGB24 format
            bitmap.colorCode = ColorCodeMono8;
            bitmap.bufferSize = camera.GetImageSize() * 1; // TODO: Mono8 also saved in RGB24 format
            bitmap.width = (unsigned long)camera.m_imgWidth;
            bitmap.height = (unsigned long)camera.m_imgHeight;

            // Create the bitmap
            if (0 == AVTCreateBitmap(&bitmap, &*imageData.begin()))
            {
                std::cout << strTag << "Could not create bitmap.\n";
                err = VmbErrorResources;
            }
            else
            {
                char pFileName[256];
                sprintf(pFileName, "%s_%010ld_CameraProc.bmp", strSN.c_str(),(long long) camera.GetFrameID());

                // Save log with frame id and time consumption in Snap() function
                fileLog << std::setw(10) << camera.GetFrameID() << ", " << std::setw(10) << std::fixed << std::setprecision(6) << time << std::endl;
                fileLog.flush();

                // Save the bitmap
                int ret = 1;
                // If you do not want to save image into disk, please comment the line below.
                //ret = AVTWriteBitmapToFile(&bitmap, pFileName);
                if (0 == ret)
                {
                    std::cout << strTag << "Could not write bitmap to file.\n";
                    err = VmbErrorOther;
                }
                else
                {
                    std::cout << strTag << "Bitmap successfully written to file \"" << pFileName << "\"\n";
                    // Release the bitmap's buffer
                    if (0 == AVTReleaseBitmap(&bitmap))
                    {
                        std::cout << strTag << "Could not release the bitmap.\n";
                        err = VmbErrorInternalFault;
                    }
                }
            }

        }
    }


    return 1;
}

int TestMultipleCamerasSnap(AVT::VmbAPI::Examples::ApiController& sysController, const char* strCamera1, const char* strCamera2, const char* strCamera3, const char* strCamera4)
{
    AVT::VmbAPI::Examples::CameraHandle camera1;
    AVT::VmbAPI::Examples::CameraHandle camera2;
    AVT::VmbAPI::Examples::CameraHandle camera3;
    AVT::VmbAPI::Examples::CameraHandle camera4;

    VmbErrorType err = VmbErrorSuccess;
    err = sysController.OpenCamera(strCamera1, camera1);
    if (VmbErrorSuccess != err)
    {
        std::cout << "\nFailed to open camera !!!"<< strCamera1 <<"\n";
        return 0;
    }
    else
    {
        std::cout << "\nSucessfully opened camera done !" << strCamera1 << "\n";
    }
    err = sysController.OpenCamera(strCamera2, camera2);
    if (VmbErrorSuccess != err)
    {
        std::cout << "\nFailed to open camera !!!"<< strCamera2 <<"\n";
        return 0;
    }
    else
    {
        std::cout << "\nSucessfully opened camera done !" << strCamera2 << "\n";
    }
    err = sysController.OpenCamera(strCamera3, camera3);
    if (VmbErrorSuccess != err)
    {
        std::cout << "\nFailed to open camera !!!"<< strCamera3 <<"\n";
        return 0;
    }
    else
    {
        std::cout << "\nSucessfully opened camera done !" << strCamera3 << "\n";
    }
    err = sysController.OpenCamera(strCamera4, camera4);
    if (VmbErrorSuccess != err)
    {
        std::cout << "\nFailed to open camera !!!"<< strCamera4 <<"\n";
        return 0;
    }
    else
    {
        std::cout << "\nSucessfully opened camera done !" << strCamera4 << "\n";
    }
    

    std::string strModel = camera1.GetCameraModel();
    strModel = strModel.replace(strModel.find(" "), 1, "_"); 
    std::string strID = camera1.GetCameraID();
    std::string strSN = camera1.GetSerialNumber();
    std::string strTag = strModel + "_" + strID + "_" + strSN + ": ";
    std::string strFile = "log_" + strModel + "_" + strID + "_" + strSN + ".csv"; 
    std::ofstream fileLog1;
    fileLog1.open(strFile);

    strModel = camera2.GetCameraModel();
    strModel = strModel.replace(strModel.find(" "), 1, "_");
    strID = camera2.GetCameraID();
    strSN = camera2.GetSerialNumber();
    strTag = strModel + "_" + strID + "_" + strSN + ": ";
    strFile = "log_" + strModel + "_" + strID + "_" + strSN + ".csv"; 
    std::ofstream fileLog2;
    fileLog2.open(strFile);

    strModel = camera3.GetCameraModel();
    strModel = strModel.replace(strModel.find(" "), 1, "_");
    strID = camera3.GetCameraID();
    strSN = camera3.GetSerialNumber();
    strTag = strModel + "_" + strID + "_" + strSN + ": ";
    strFile = "log_" + strModel + "_" + strID + "_" + strSN + ".csv"; 
    std::ofstream fileLog3;
    fileLog3.open(strFile);

    strModel = camera4.GetCameraModel();
    strModel = strModel.replace(strModel.find(" "), 1, "_");
    strID = camera4.GetCameraID();
    strSN = camera4.GetSerialNumber();
    strTag = strModel + "_" + strID + "_" + strSN + ": ";
    strFile = "log_" + strModel + "_" + strID + "_" + strSN + ".csv"; 
    std::ofstream fileLog4;
    fileLog4.open(strFile);

    std::cout << "Press <q> to exit ...\n";

    while (1)
    {
        if (_kbhit()) {
            char pressed = _getch();
            if (pressed == 'q' || pressed == 'Q')
                break;
        } 

        Sleep(1000);
        SnapAndSave(sysController, camera1, fileLog1);
        Sleep(100);
        SnapAndSave(sysController, camera2, fileLog2);
        Sleep(100);
        SnapAndSave(sysController, camera3, fileLog3);
        Sleep(100);
        SnapAndSave(sysController, camera4, fileLog4);

    }

    return 1;
}




int main( int argc, char* argv[] )
{
    VmbErrorType err = VmbErrorSuccess;

    std::cout<<"/////////////////////////////////////////////////////////////////////////\n";
    std::cout<<"/// Based on Vimba API Asynchronous Grab Example CPP Console          ///\n";
    std::cout<<"///                                                                   ///\n";
    std::cout<<"/// Implement quicker software snap function for multiple cameras     ///\n";
    std::cout<<"///                                                                   ///\n";
    std::cout<<"///                                                                   ///\n";
    std::cout<<"///                                            By Joe 2020 DEC 28     ///\n";
    std::cout<<"///                                            By Joe 2023 JUN 21     ///\n";
    std::cout<<"/////////////////////////////////////////////////////////////////////////\n\n";

    //////////////////////
    //Parse command line//
    //////////////////////
    AVT::VmbAPI::Examples::ProgramConfig Config;
    err = Config.ParseCommandline(argc, argv);
    //Write out an error if we could not parse the command line
    if (VmbErrorBadParameter == err)
    {
        std::cout << "Invalid parameters!\n\n";
        Config.setPrintHelp(true);
    }

    AVT::VmbAPI::Examples::ApiController apiController;

    // Print out version of Vimba
    std::cout << "Vimba C++ API Version " << apiController.GetVersion() << "\n";

    // Delete all local bitmap files
    system("del_bitmps.cmd");

    // Startup Vimba
    err = apiController.StartUp();

    // Comment below to use test in seperate thread for different camera.
//#define TEST_SNAP_IN_MAIN_THREAD

#ifdef TEST_SNAP_IN_MAIN_THREAD
    TestMultipleCamerasSnap(apiController, "DEV_1AB22D01BBB8", "DEV_000F314CA646", "DEV_000F314D5B52", "DEV_1AB22C0019F9");

#else
        // TEST_SNAP_IN_SEPERATE_THREAD 
    AVT::VmbAPI::Examples::CameraHandle camera1;
    AVT::VmbAPI::Examples::CameraHandle camera2;
    StartNewCameraThread(apiController, camera1, "DEV_000F314D5B52"); // Manta G-895B (E0622706)
    StartNewCameraThread(apiController, camera2, "DEV_000A470003BF"); // Alvium G1-507m (DEV_000A470003BF) 

#endif

    std::cout << "\n\nPress Enter to quit the application ...\n";
    _getch();
    
    // Quit Vimba SDK
    apiController.ShutDown();


    return err;
}
