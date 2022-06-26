#ifndef VS_SLOTSERVICECONTAINER_H_HEADER_INCLUDED_BB93B7E7
#define VS_SLOTSERVICECONTAINER_H_HEADER_INCLUDED_BB93B7E7

#include "VS_SlotServiceBase.h"

//##ModelId=446C49030075
class VS_SlotServiceContainer
{
  public:
    //##ModelId=446C4A5D0261
    virtual int AddService( VS_SlotServiceBase * aService);

    //##ModelId=446C4B0201D5
    int RemoveAll();

};



#endif /* VS_SLOTSERVICECONTAINER_H_HEADER_INCLUDED_BB93B7E7 */
