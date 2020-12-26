/*=============================================================================
  Copyright (C) 2013 - 2017 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        FrameObserver.h

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

#ifndef AVT_VMBAPI_EXAMPLES_AVCAMERA
#define AVT_VMBAPI_EXAMPLES_AVCAMERA

#include <queue>
#include "VimbaCPP/Include/VimbaCPP.h"
#include "ProgramConfig.h"
#ifdef WIN32
#include <Windows.h>
#endif //WIN32

#include "VmbTransform.h"
#include "FrameObserver.h"

#define NUM_FRAMES 3

namespace AVT {
namespace VmbAPI {
namespace Examples {


class CameraHandle
{
public:
    CameraHandle();
    ~CameraHandle();

    VmbErrorType OpenCameraHandle(const char* pID);
    VmbErrorType CloseCameraHandle();

    std::string GetCameraID();
    std::string GetSerialNumber();

    VmbErrorType QuickSnap(std::vector<VmbUchar_t> & cDestinationImageData, VmbImage & sVmbImageData);
    VmbUint32_t  GetImageSize();
    VmbUint64_t  GetFrameID();

private:

    char*               m_pStrCameraID;
    CameraPtr           m_pCamera;                  // The currently streaming camera
    FrameObserver*      m_pFrameObserver;           // Every camera has its own frame observer

    VmbErrorType PrepareCamera(); 

public:
    // Save the latest Frame Info 

    Mutex m_lock;

    //VmbUchar_t* m_imageBuffer; 
    VmbUint32_t  m_imageSize; 
    VmbUint64_t  m_frameID; 
    VmbUint64_t  m_timestamp; 

    VmbInt64_t m_imgWidth; 
    VmbInt64_t m_imgHeight; 

    std::vector<VmbUchar_t>  m_DestinationImageData;
    VmbImage m_VmbImageData;


};

}}} // namespace AVT::VmbAPI::Examples

#endif
