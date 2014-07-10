#ifndef LIB_ARCUS_MESSAGE_TYPES_H
#define LIB_ARCUS_MESSAGE_TYPES_H

namespace arc
{
	enum ECommand
	{
		CMD_REQUEST_ID = 0x00100000;		//VOID
			// Request the unique identifier of the other side. To identify/verify what we are talking with.
		CMD_REPLY_ID = 0x00100001;			//UTF8_String identifier
			// Reply to CMD_REQUEST_ID, I'm a tea pot (If applicable)
		CMD_REQUEST_VERSION = 0x00100002;	//VOID
			// Request the version number of the application at the other end.
		CMD_REPLY_VERSION = 0x00100003;		//INT32 major, INT32 minor, INT32 revision
			// Request the version number of the application at the other end.
	};
}

#endif//LIB_ARCUS_MESSAGE_TYPES_H
