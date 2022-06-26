#include "VS_H224Frame.h"

#ifndef VS_H281FRAME_H
#define VS_H281FRAME_H

#define VS_H281_CLIENT_ID 0x01


class VS_H281Frame : public VS_H224Frame
{
public:

  enum class eRequestType {
    IllegalRequest        = 0x00,
    StartAction           = 0x01,
    ContinueAction        = 0x02,
    StopAction            = 0x03,
    SelectVideoSource     = 0x04,
    VideoSourceSwitched   = 0x05,
    StoreAsPreset         = 0x06,
    ActivatePreset        = 0x07
  };

  enum class ePanDirection {
    NoPan        = 0x00,
    IllegalPan   = 0x40,
    PanLeft      = 0x80,
    PanRight     = 0xc0,
  };

  enum class eTiltDirection {
    NoTilt        = 0x00,
    IllegalTilt   = 0x10,
    TiltDown      = 0x20,
    TiltUp        = 0x30,
  };

  enum class eZoomDirection {
    NoZoom        = 0x00,
    IllegalZoom   = 0x04,
    ZoomOut       = 0x08,
    ZoomIn        = 0x0c
  };

  enum class eFocusDirection {
    NoFocus         = 0x00,
    IllegalFocus    = 0x01,
    FocusOut        = 0x02,
    FocusIn         = 0x03
  };

  enum class eVideoMode {
    MotionVideo                    = 0x00,
    IllegalVideoMode               = 0x01,
    NormalResolutionStillImage     = 0x02,
    DoubleResolutionStillImage     = 0x03
  };

  VS_H281Frame();
  virtual ~VS_H281Frame();

  eRequestType GetRequestType() const { return (eRequestType)(GetClientDataPtr())[0]; }
  void SetRequestType(eRequestType requestType);

  // The following methods are only valid when
  // request type is either StartAction, ContinueAction or StopAction
  ePanDirection GetPanDirection() const;
  void SetPanDirection(ePanDirection direction);

  eTiltDirection GetTiltDirection() const;
  void SetTiltDirection(eTiltDirection direction);

  eZoomDirection GetZoomDirection() const;
  void SetZoomDirection(eZoomDirection direction);

  eFocusDirection GetFocusDirection() const;
  void SetFocusDirection(eFocusDirection direction);

  // Only valid when RequestType is StartAction
  unsigned char GetTimeout() const;
  void SetTimeout(unsigned char timeout);

  // Only valid when RequestType is SelectVideoSource or VideoSourceSwitched
  unsigned char GetVideoSourceNumber() const;
  void SetVideoSourceNumber(unsigned char videoSourceNumber);

  eVideoMode GetVideoMode() const;
  void SetVideoMode(eVideoMode videoMode);

  // Only valid when RequestType is StoreAsPreset or ActivatePreset
  unsigned char GetPresetNumber() const;
  void SetPresetNumber(unsigned char presetNumber);
};


#endif // H281_H