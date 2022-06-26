#include "VS_ChatBotMessages.h"

const std::string bot::CHAT_CTRLS_HELP_RTF_TEMPLATE =
R"s({\rtf1\fbidis\ansi\ansicpg1251\deff0\nouicompat\deflang1049{\fonttbl{\f0\fmodern\fprq1\fcharset204 Courier New;}{\f1\fnil\fcharset0 Times New Roman;}}
{\colortbl ;\red0\green0\blue0;}
{\*\generator Riched20 16.0.4600}\viewkind4\uc1
\pard\widctlpar\sb40\sa40\tx916\tx1832\tx2748\tx3664\tx4580\tx5496\tx6412\tx7328\tx8244\tx9160\tx10076\tx10992\tx11908\tx12824\tx13740\tx14656\cf1\protect\outl\f0\fs22 This is TrueConf chat bot of\~{{\field{\*\fldinst{HYPERLINK %s }}{\fldrslt{%s\ul0\cf0}}}}\f0\fs20\line
Use the following commands here. \par
To call user with TrueConf username \par
/call username \par
\par
To call SIP or H.323 terminal with IP\par
/call #sip:@IP\par
/call #sip:SIPUSER@IP\par
/call #h323:@IP\par
\par
To create conference\par
/conf\par
\par
To create conference, inviting username1,username2. SIP/H323 terminals could be invited in format #sip:@IP or #h323:@IP\par
/conf username1[,username2[,...]] ["Topic"]\par
\par
To invite user into group conference or webinar\par
/invite username1[,username2[,...]]\par
\par
To remove user from group conference or webinar\par
/remove username\par
\par
To create webinar\par
/webinar username1[,username2[,...]] ["Topic"]\par
\par
Send request to take webinar's podium\par
/take\par
\par
Leave webinar's podium\par
/leave\par
\par
To invite user to webinar's podium\par
/podium username\par
\par
To remove user from webinar's podium\par

\protect0\pard\protect /free username\protect0\outl0\f1\fs20\lang1033  \f2\lang1049\par
{\*\lyncflags<rtf=1>}})s";

const std::string bot::CHAT_CTRLS_HELP_PLAIN_TEXT_TEMPLATE = R"(This is TrueConf chat bot of %s
Use the following commands here.


To call user with TrueConf username
/call username

To call SIP or H.323 terminal with IP
/call #sip:@IP
/call #sip:SIPUSER@IP
/call #h323:@IP

To create conference
conf

To create conference, inviting username1,username2. SIP/H323 terminals could be invited in format #sip:@IP or #h323:@IP
/conf username1[,username2[,...]] ["Topic"]

To invite user into group conference or webinar
/invite username1[,username2[,...]]

To remove user from group conference or webinar
/remove username

To create webinar
/webinar username1[,username2[,...]] ["Topic"]

Send request to take webinar's podium
/take

Leave webinar's podium
/leave

For webinar owner and moderators:
To invite user to webinar's podium
/podium username

To remove user from webinar's podium
/free username)";

const char bot::NOT_RECOGNIZED_ANSWER[] = "Command not recognized\n";
const char bot::ALLOWED_ONLY_IN_WEBINAR_ANSWER[] = "Not supported in this type of conference. This command is for webinar only.";
const char bot::YOU_MUST_BE_MODERATOR_ANSWER[] = "You do not have rights for that in this conference. Moderator rights are required!\n";
const char bot::USER_NOT_RECOGNIZED_ANSWER[] = "User not recognized. Please, enter correct username.";