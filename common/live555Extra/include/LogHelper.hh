#ifndef _LOG_HELPER_H_
#define _LOG_HELPER_H_

#define MEDIUM_LOG(cl)        this->envir() << #cl "(" << this->name() << "): "

#define FRAMED_FILTER_LOG(cl) this->envir() << #cl "(" << this->name() << "<-" << (this->fInputSource ? this->fInputSource->name() : "(NULL)") << "): "

#endif
