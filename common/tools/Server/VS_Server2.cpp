#if defined(_WIN32) // Not ported

#include "VS_Server.h"

// Some parts of this mess just had to be in a different file, because reasons...
VS_ServerComponentsInterface* VS_Server::srv_components = nullptr;
#endif
