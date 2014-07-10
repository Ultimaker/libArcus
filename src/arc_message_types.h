#ifndef LIB_ARCUS_MESSAGE_TYPES_H
#define LIB_ARCUS_MESSAGE_TYPES_H

namespace arc
{
	enum EMessageType
	{
		//GENERAL COMMANDS
		MSG_REQUEST_ID = 0x00100000;			// VOID
			// Request the unique identifier of the other side. To identify/verify what we are talking with.
		MSG_REPLY_ID = 0x00100001;			// UTF8_String identifier
			// Reply to CMD_REQUEST_ID, I'm a tea pot (If applicable)
		MSG_REQUEST_VERSION = 0x00100002;		// VOID
			// Request the version number of the application at the other end.
		MSG_REPLY_VERSION = 0x00100003;			// INT32 major, INT32 minor, INT32 revision
			// Request the version number of the application at the other end.
		MSG_SET_CONFIG_SETTING = 0x00100004;		// UTF8_String Key, UTF8_String Value
			// Set a configuration setting.
		MSG_REQUEST_ALL_CONFIG_SETTINGS = 0x00100005; 	// VOID
			// Request all configuration settings known to application
		MSG_REPLY_ALL_CONFIG_SETTINGS = 0x00100006; 	// INT32 num_settings, UTF8_String[] Keys, UTF8_String[] Values
			// Reply with all configuration settings known to application.
		MSG_REQUEST_CONFIGURATION_SETTING = 0x00100007;	// UTF8_String Key
			// Request value of a setting defined by key
		MSG_REPLY_CONFIGURATION_SETTING = 0x00100008;	// UTF8_String value
			// Value of setting as requested (response to 0x00100007)
		MSG_REQUEST_BIT_COLOR = 0x00100020; 		// VOID
			// As what current bit color is. Client will reply with a 0x00100021 to indicate of which color the bits are now. Or will reply with 0xEEE00000 "I am a tea pot"; as only tea-potsdo not support bit color.
		MSG_REPLY_BIT_COLOR = 0x00100021;		// INT24 hex-color
			// Tell the receiver that the following bits are send in the given color. Verification if the bits are received without color modification lays with the client

	
		

	
	};
}

#endif//LIB_ARCUS_MESSAGE_TYPES_H
