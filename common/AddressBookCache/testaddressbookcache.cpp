#include "transport/Client/VS_TransportClient.h"

#include "VSClient/VSTrClientProc.h"
#include "VSClient/VS_Dmodule.h"

#include "VS_AddressBookManager.h"
#include "VS_LoginManager.h"
#include "VS_CorePinAnalyser.h"
#include "VS_CoreStateMachine.h"

#include "testcache.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

VS_AddressBookManager*	g_abm = 0;

VS_TestAbInterface::~VS_TestAbInterface()
{
}

bool VS_TestAbInterface::GetAddressBook() {
	return g_abm->GetAddressBook();
}
bool VS_TestAbInterface::GetBanList() {
	return g_abm->GetBanList();
}
bool VS_TestAbInterface::GetUserDetailes(const char *call_id){
	return g_abm->GetUserDetailes(call_id);
}
bool VS_TestAbInterface::GetUserPicture(const char *call_id){
	return g_abm->GetUserPicture(call_id);
}
long VS_TestAbInterface::GetStatus(const char *call_id){
	return g_abm->GetStatus(call_id);
}
bool VS_TestAbInterface::SetDisplayName(const char *call_id, const wchar_t *dm){
	return g_abm->SetDisplayName(call_id, (wchar_t*)dm);
}
bool VS_TestAbInterface::SetAvatar(const void *pic_buf, const unsigned long sz){
	return g_abm->SetAvatar(pic_buf, sz);
}
bool VS_TestAbInterface::AddToAddressBook(const char * call_id){
	return g_abm->AddToAddressBook(call_id);
}
bool VS_TestAbInterface::AddToBanList(const char * call_id){
	return g_abm->AddToBanList(call_id);
}
bool VS_TestAbInterface::RemoveFromAddressBook(const char *call_id){
	return g_abm->RemoveFromAddressBook(call_id);
}
bool VS_TestAbInterface::RemoveFromBanList(const char *call_id){
	return g_abm->RemoveFromBanList(call_id);
}
bool VS_TestAbInterface::ChatSend(wchar_t *Message, char * To) {
	return g_abm->ChatSend((wchar_t*)Message, To);
}
bool VS_TestAbInterface::UserSearch(const char *query) {
	return g_abm->UserSearch(query);
}
