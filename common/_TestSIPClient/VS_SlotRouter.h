#ifndef VS_SLOTROUTER_H_HEADER_INCLUDED_BB93ABDC
#define VS_SLOTROUTER_H_HEADER_INCLUDED_BB93ABDC

#include "VS_SlotNetworkLayer.h"
#include "VS_SlotRouterInterface.h"
#include "VS_SlotServiceContainer.h"

//##ModelId=446C49D2025F
class VS_SlotRouter : public VS_SlotRouterInterface
{
 public:

 protected:
    //##ModelId=446C54C6038B
    VS_SlotNetworkLayer *m_netLayer;

    //##ModelId=446C561E00BD
    VS_SlotServiceContainer m_serviceCont;

};



#endif /* VS_SLOTROUTER_H_HEADER_INCLUDED_BB93ABDC */
