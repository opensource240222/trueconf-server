
/////////////////////////////////////////////////////////////////////////////////////////

#include "VS_RasMessages.h"

/////////////////////////////////////////////////////////////////////////////////////////

VS_RasMessage::operator VS_RasGatekeeperRequest *( )
{	return dynamic_cast<VS_RasGatekeeperRequest *>(choice);		}
// end VS_RasMessage::operator VS_RasGatekeeperRequest *

VS_RasMessage::operator VS_RasGatekeeperConfirm *( void )
{	return dynamic_cast<VS_RasGatekeeperConfirm *>(choice);	}
// end VS_RasMessage::operator VS_RasGatekeeperConfirm *

VS_RasMessage::operator VS_RasGatekeeperReject *( void )
{	return dynamic_cast<VS_RasGatekeeperReject *>(choice);	}
// end VS_RasMessage::operator VS_RasGatekeeperReject *

VS_RasMessage::operator VS_RasRegistrationRequest *( void )
{	return dynamic_cast<VS_RasRegistrationRequest *>(choice);	}
// end VS_RasMessage::operator VS_RasRegistrationRequest *

VS_RasMessage::operator VS_RasRegistrationConfirm *( void )
{	return dynamic_cast<VS_RasRegistrationConfirm *>(choice);	}
// end VS_RasMessage::operator VS_RasRegistrationConfirm *

VS_RasMessage::operator VS_RasRegistrationReject *( void )
{	return dynamic_cast<VS_RasRegistrationReject *>(choice);	}
// end VS_RasMessage::operator VS_RasRegistrationReject *

VS_RasMessage::operator VS_RasUnregistrationRequest *( void )
{	return dynamic_cast<VS_RasUnregistrationRequest *>(choice);	}
// end VS_RasMessage::operator VS_RasUnregistrationRequest *

VS_RasMessage::operator VS_RasUnregistrationConfirm *( void )
{	return dynamic_cast<VS_RasUnregistrationConfirm *>(choice);	}
// end VS_RasMessage::operator VS_RasUnregistrationConfirm *

VS_RasMessage::operator VS_RasUnregistrationReject *( void )
{	return dynamic_cast<VS_RasUnregistrationReject *>(choice);	}
// end VS_RasMessage::operator VS_RasUnregistrationReject *

VS_RasMessage::operator VS_RasAdmissionRequest *( void )
{	return dynamic_cast<VS_RasAdmissionRequest *>(choice);	}
// end VS_RasMessage::operator VS_RasAdmissionRequest *

VS_RasMessage::operator VS_RasAdmissionConfirm *( void )
{	return dynamic_cast<VS_RasAdmissionConfirm *>(choice);	}
// end VS_RasMessage::operator VS_RasAdmissionConfirm *

VS_RasMessage::operator VS_RasAdmissionReject *( void )
{	return dynamic_cast<VS_RasAdmissionReject *>(choice);	}
// end VS_RasMessage::operator VS_RasAdmissionReject *

VS_RasMessage::operator VS_RasBandwidthRequest *( void )
{	return dynamic_cast<VS_RasBandwidthRequest *>(choice);	}
// end VS_RasMessage::operator VS_RasBandwidthRequest *

VS_RasMessage::operator VS_RasBandwidthConfirm *( void )
{	return dynamic_cast<VS_RasBandwidthConfirm *>(choice);	}
// end VS_RasMessage::operator VS_RasBandwidthConfirm *

VS_RasMessage::operator VS_RasBandwidthReject *( void )
{	return dynamic_cast<VS_RasBandwidthReject *>(choice);	}
// end VS_RasMessage::operator VS_RasBandwidthReject *

VS_RasMessage::operator VS_RasDisengageRequest *( void )
{	return dynamic_cast<VS_RasDisengageRequest *>(choice);	}
// end VS_RasMessage::operator VS_RasDisengageRequest *

VS_RasMessage::operator VS_RasDisengageConfirm *( void )
{	return dynamic_cast<VS_RasDisengageConfirm *>(choice);	}
// end VS_RasMessage::operator VS_RasDisengageConfirm *

VS_RasMessage::operator VS_RasDisengageReject *( void )
{	return dynamic_cast<VS_RasDisengageReject *>(choice);	}
// end VS_RasMessage::operator VS_RasDisengageReject *

VS_RasMessage::operator VS_RasLocationRequest *( void )
{	return dynamic_cast<VS_RasLocationRequest *>(choice);	}
// end VS_RasMessage::operator VS_RasLocationRequest *

VS_RasMessage::operator VS_RasLocationConfirm *( void )
{	return dynamic_cast<VS_RasLocationConfirm *>(choice);	}
// end VS_RasMessage::operator VS_RasLocationConfirm *

VS_RasMessage::operator VS_RasLocationReject *( void )
{	return dynamic_cast<VS_RasLocationReject *>(choice);	}
// end VS_RasMessage::operator VS_RasLocationReject *

VS_RasMessage::operator VS_RasInfoRequest *( void )
{	return dynamic_cast<VS_RasInfoRequest *>(choice);	}
// end VS_RasMessage::operator VS_RasInfoRequest *

VS_RasMessage::operator VS_RasInfoRequestResponse *( void )
{	return dynamic_cast<VS_RasInfoRequestResponse *>(choice);	}
// end VS_RasMessage::operator VS_RasInfoRequestResponse *

VS_RasMessage::operator VS_RasNonStandardMessage *( void )
{	return dynamic_cast<VS_RasNonStandardMessage *>(choice);	}
// end VS_RasMessage::operator VS_RasNonStandardMessage *

VS_RasMessage::operator VS_RasUnknownMessageResponse *( void )
{	return dynamic_cast<VS_RasUnknownMessageResponse *>(choice);	}
// end VS_RasMessage::operator VS_RasUnknownMessageResponse *

VS_RasMessage::operator VS_RasRequestInProgress *( void )
{	return dynamic_cast<VS_RasRequestInProgress *>(choice);	}
// end VS_RasMessage::operator VS_RasRequestInProgress *

VS_RasMessage::operator VS_RasResourcesAvailableIndicate *( void )
{	return dynamic_cast<VS_RasResourcesAvailableIndicate *>(choice);	}
// end VS_RasMessage::operator VS_RasResourcesAvailableIndicate *

VS_RasMessage::operator VS_RasResourcesAvailableConfirm *( void )
{	return dynamic_cast<VS_RasResourcesAvailableConfirm *>(choice);	}
// end VS_RasMessage::operator VS_RasResourcesAvailableConfirm *

VS_RasMessage::operator VS_RasInfoRequestAck *( void )
{	return dynamic_cast<VS_RasInfoRequestAck *>(choice);	}
// end VS_RasMessage::operator VS_RasInfoRequestAck *

VS_RasMessage::operator VS_RasInfoRequestNak *( void )
{	return dynamic_cast<VS_RasInfoRequestNak *>(choice);	}
// end VS_RasMessage::operator VS_RasInfoRequestNak *

VS_RasMessage::operator VS_RasServiceControlIndication *( void )
{	return dynamic_cast<VS_RasServiceControlIndication *>(choice);	}
// end VS_RasMessage::operator VS_RasServiceControlIndication *

VS_RasMessage::operator VS_RasServiceControlResponse *( void )
{	return dynamic_cast<VS_RasServiceControlResponse *>(choice);	}
// end VS_RasMessage::operator VS_RasServiceControlResponse *

/////////////////////////////////////////////////////////////////////////////////////////
