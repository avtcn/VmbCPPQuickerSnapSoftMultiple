/*=============================================================================
  Copyright (C) 2013 - 2017 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        AVCamera.cpp

  Description: The frame observer that is used for notifications from VimbaCPP
               regarding the arrival of a newly acquired frame.

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

#include <iostream>
#include <iomanip>
#ifdef WIN32
#include <Windows.h>
#else
#include <time.h>
#endif //WIN32

#include "CameraHandle.h"

namespace AVT {
namespace VmbAPI {
namespace Examples {

//
// We pass the camera that will deliver the frames to the constructor
//
// Parameters:
//  [in]    pCamera             The camera the frame was queued at
//  [in]    eFrameInfos         Indicates how the frame will be displayed
//  [in]    eColorProcessing    Indicates how color processing is applied
//
CameraHandle::CameraHandle()
{
    m_pStrCameraID = NULL;
}

CameraHandle::~CameraHandle()
{
    if (m_pStrCameraID)
    {
        delete m_pStrCameraID;
        m_pStrCameraID = NULL;
    }

    VmbErrorType res = m_pCamera->StopContinuousImageAcquisition();

    // Needed? Not needed!!!
    //if (m_pFrameObserver) {
    //    delete m_pFrameObserver;
    //    m_pFrameObserver = NULL;
    //} 

}

VmbErrorType CameraHandle::OpenCameraHandle(const char* pID)
{
    // Open the desired camera by its ID
    VmbErrorType res = VimbaSystem::GetInstance().OpenCameraByID( pID, VmbAccessModeFull, m_pCamera );
    if ( VmbErrorSuccess == res )
    {
        // Set the GeV packet size to the highest possible value
        // (In this example we do not test whether this cam actually is a GigE cam)
        FeaturePtr pCommandFeature;
        if ( VmbErrorSuccess == m_pCamera->GetFeatureByName( "GVSPAdjustPacketSize", pCommandFeature ))
        {
            if ( VmbErrorSuccess == pCommandFeature->RunCommand() )
            {
                bool bIsCommandDone = false;
                do
                {
                    if ( VmbErrorSuccess != pCommandFeature->IsCommandDone( bIsCommandDone ))
                    {
                        break;
                    }
                } while ( false == bIsCommandDone );
            }
        }

        FeaturePtr pWidthFeature;
        if (VmbErrorSuccess == m_pCamera->GetFeatureByName("Width", pWidthFeature))
        {
            pWidthFeature->GetValue(m_imgWidth);
        }
        FeaturePtr pHeightFeature;
        if (VmbErrorSuccess == m_pCamera->GetFeatureByName("Height", pHeightFeature))
        {
            pHeightFeature->GetValue(m_imgHeight);
        }

        if ( VmbErrorSuccess == res )
        {
            // set camera so that transform algorithms will never fail
            //res = PrepareCamera(); // BUG in this function
            res = VmbErrorSuccess;
            if ( VmbErrorSuccess == res )
            {
                AVT::VmbAPI::Examples::ProgramConfig Config;
                Config.m_FrameInfos = FrameInfos_Show;

                // Create a frame observer for this camera (This will be wrapped in a shared_ptr so we don't delete it)
                m_pFrameObserver = new FrameObserver( this, m_pCamera, Config.getFrameInfos(), Config.getColorProcessing(), Config.getRGBValue() );
                if (!m_pFrameObserver)
                {
                    std::cout << "Failed to create m_pFrameObserver" << std::endl;
                }
                else {
                    // Start streaming
                    res = m_pCamera->StartContinuousImageAcquisition(NUM_FRAMES, IFrameObserverPtr(m_pFrameObserver));
                    if (res != VmbErrorSuccess)
                    {
                        std::cout << "Failed to call StartContinuousImageAcquisition" << std::endl; 
                    }
                } 
            }
            else
            {
                std::cout << "Failed to call PrepareCamera" << std::endl; 
            }
        }
        if ( VmbErrorSuccess != res )
        {
            // If anything fails after opening the camera we close it
            m_pCamera->Close();
            return VmbErrorDeviceNotOpen;
        }
    }
    return VmbErrorSuccess;
}


VmbErrorType CameraHandle::CloseCameraHandle()
{
    m_pCamera->Close();

    return VmbErrorSuccess;
}

/**setting a feature to maximum value that is a multiple of 2 and a multiple of the increment*/
VmbErrorType SetIntFeatureValueModulo2b( const CameraPtr &pCamera, const char* const& Name )
{
    VmbErrorType    result;
    FeaturePtr      feature;
    VmbInt64_t      value_min = 0;
    VmbInt64_t      value_max = 0;
    VmbInt64_t      value_increment = 0;

    result = SP_ACCESS( pCamera )->GetFeatureByName( Name, feature );
    if( VmbErrorSuccess != result )
    {
        std::cout << "Failed to GetFeatureByName()" << std::endl;
        return result;
    }

    result = SP_ACCESS( feature )->GetRange( value_min, value_max );
    if( VmbErrorSuccess != result )
    {
        std::cout << "Failed to GetRange()" << std::endl;
        return result;
    }

    result = SP_ACCESS( feature )->GetIncrement( value_increment );
    if( VmbErrorSuccess != result )
    {
        std::cout << "Failed to GetIncrement()" << std::endl;
        return result;
    }

    value_max = value_max - ( value_max % value_increment);
    if( value_max % 2 != 0)
    {
        value_max -= value_increment;
    }

    result = SP_ACCESS( feature )->SetValue ( value_max );
    if( VmbErrorSuccess != result )
    {
        std::cout << "Failed to SetValue( ), value_max = " << value_max << std::endl;
        return result;
    }
    return result;
}



/**prepare camera so that the delivered image will not fail in image transform*/
VmbErrorType CameraHandle::PrepareCamera()
{
    VmbErrorType result;
    result = SetIntFeatureValueModulo2b( m_pCamera, "Width" );
    if( VmbErrorSuccess != result )
    {
        return result;
    }
    result = SetIntFeatureValueModulo2b( m_pCamera, "Height" );
    if( VmbErrorSuccess != result )
    {
        return result;
    }
    return result;
}


std::string CameraHandle::GetCameraID()
{
    std::string strID;
    m_pCamera->GetID(strID);

    return strID; 
}



std::string CameraHandle::GetSerialNumber()
{
    std::string strSN;
    m_pCamera->GetSerialNumber(strSN);

    return strSN; 
}


VmbErrorType CameraHandle::QuickSnap(std::vector<VmbUchar_t>& cDestinationImageData, VmbImage& sVmbImageData)
{
    m_lock.Lock();

    //cDestinationImageData.swap(m_DestinationImageData);
    cDestinationImageData = m_DestinationImageData;

    m_lock.Unlock();

    if (cDestinationImageData.size() <= 0)
    {
        return VmbErrorIncomplete;
    }

    return VmbErrorSuccess;
}

VmbUint32_t  CameraHandle::GetImageSize()
{
    VmbUint32_t size = 0;

    m_lock.Lock();

    size = m_imageSize;

    m_lock.Unlock();

    return size;
}

VmbUint64_t  CameraHandle::GetFrameID()
{
    VmbUint64_t id = 0;

    m_lock.Lock();

    id = m_frameID;

    m_lock.Unlock();

    return id;
}





}}} // namespace AVT::VmbAPI::Examples
