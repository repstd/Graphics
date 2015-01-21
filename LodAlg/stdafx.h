// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once
#include "targetver.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>   


// TODO: reference additional headers your program requires here
/** 包含gl头文件 */
#include <GL/glew.h>
#include <gl/gl.h>				 
#include <gl/glu.h>
#include <tchar.h>



#define _DEBUG_ENCODE_MSG(filename,format,data) \
{\
	FILE* fp = fopen(filename, "a+"); \
	fprintf(fp, format, data);\
	fclose(fp); \
}



#define _DEBUG_LOG_INIT(filename)\
{\
	FILE* fp = fopen(filename, "w"); \
	fclose(fp); \
}

enum _LOD_STATUS
{
	_LOD_ERROR =0x1000,
	_LOD_SUCCESS = 0x0100

};