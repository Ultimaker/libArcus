#ifndef LIB_ARCUS_MESSAGE_TYPES_H
#define LIB_ARCUS_MESSAGE_TYPES_H

namespace arc
{
	// Enum that holds all types of messages. The Types(s) as a comment behind the command indicate which data is sent with it. 
	// Comment below the messageType gives more info in how the Type is to be used
	enum EMessageType
	{
		// GENERAL COMMANDS
		MSG_NO_MESSAGE = 0x00000000;			// VOID
			// There is no message
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
		MSG_PROGESS = 0x00100009;			// float32 progress_amount
			// Report processing in float values 0.0 to 1.0
		MSG_REQUEST_BIT_COLOR = 0x00100020; 		// VOID
			// As what current bit color is. Client will reply with a 0x00100021 to indicate of which color the bits are now. Or will reply with 0xEEE00000 "I am a tea pot"; as only tea-potsdo not support bit color.
		MSG_REPLY_BIT_COLOR = 0x00100021;		// INT24 hex-color
			// Tell the receiver that the following bits are send in the given color. Verification if the bits are received without color modification lays with the client

		// MESH 
		MSG_OBJECT_LIST = 0x00200000;			// INT32 numMeshlists
			// Following data is a list of meshes that belong to the same object.
		MSG_MESH_LIST = 0x00200001;			// INT32 numDataLists
			// Following data is part of a single mesh.
		MSG_VERTEX_LIST = 0x00200002;			// float32[] vertex
			// 3D vertex list.
		MSG_NORMAL_LIST = 0x00200003;			// float32[] normal
			// 3D normal list.
		MSG_INDEX_LIST = 0x00200004;			// INT32[] index
			// Index list for 3D vertex data (to make triangles / polies).

		// ERROR
		MSG_ERROR_UNDEFINED = 0xEEE00000; 		// UTF8_STRING
			// I'm a tea pot.
		MSG_ERROR_UNKNOWN_COMMAND = 0xEEE00001; 	// INT32 command
			// Previous command was not recognised.
		
		// CURA SPECIFIC 
		MSG_CONV_OBJ_TO_GCODE = 0x00300000;		// VOID	
			// Start processing the provided objects to g-code.
		MSG_SET_TRANSFOMATION_MATRIX = 0x00300001;	// float32[3*3] transformation_matrix
			// Set the 3x3 transformation matrix used by Cura.
		MSG_SET_OBJECT_COUNT = 0x00300002;		// INT32 count
			// This amount of [object list](0x00200000) commands will be send.
		MSG_REPORT_OBJ_PINT_TIME = 0x00300003;		// INT32 object_index, FLOAT32 print_time.
			// Report the print time in seconds for the object with index [object_index]
		MSG_REPORT_OBJ_MATERIAL = 0x00300004;		// INT32 object_index, INT32 extruder_nr, FLOAT32 material_amount.
			// Report the material amount in mm for the object with index [object_index] for [extruder nr]	
	};
}

#endif//LIB_ARCUS_MESSAGE_TYPES_H
