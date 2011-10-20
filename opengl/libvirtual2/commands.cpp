#include <stdio.h>
#include "egl_context.h"
#include "gles2_emulator_constants.h"

	// --------
egl_context_t* getContext() 
{
	ogles_context_t* c = ogles_context_t::get();
	return egl_context_t::context(c);
}

void createEGLCommand(egl_context_t* c, int eglFunction, int length, command_control& cmd) {
	cmd.virtualDeviceMagicNumber = GLES2_DEVICE_HEADER;
	cmd.length = length; 
	cmd.command = EGL14;
	cmd.context = (int)c;
	cmd.glFunction = eglFunction;
}

void createEGLCommand(int eglFunction, int length, command_control& cmd) {
	egl_context_t* c = getContext();
	createEGLCommand(c, eglFunction, length, cmd);
}


void createGLES2Command(int glFunction, int length, command_control& cmd) {
	egl_context_t* c = getContext();

	cmd.virtualDeviceMagicNumber = GLES2_DEVICE_HEADER;
	cmd.length = length; 
	cmd.command = GLES20;
	cmd.context = (int)c;
	cmd.glFunction = glFunction;

}

int pad32bit(int length){
	return ((length+3)>>2)<<2;
}

int pad32strlen(const char* str) {
	return pad32bit(strlen(str));
}


void writeData(char* buffer, int& offset, const void* ptr, int size) {
	memcpy(buffer+offset, ptr, size);
	offset+=size;
}

void writeUint32(char* buffer, int& offset, unsigned int val) {
	unsigned int size = sizeof(unsigned int);
	memcpy(buffer+offset, &val, size);
	offset+=size;
}

int sendCommand(egl_context_t* c, const char* buffer, int size)
{
	if (c->theVirtualDeviceFileDescriptor) {
		return fwrite(buffer, 1, size, c->theVirtualDeviceFileDescriptor);
	} else {
		return -1;
	}	
}
int sendCommand(const char* buffer, int size)
{
	egl_context_t* c = getContext();
	return sendCommand(c, buffer, size);
}
int sendCommandDataWithHeader(const void* header, int headerSize, const void* data, int dataSize)
{
	egl_context_t* c = getContext();
	if (c->theVirtualDeviceFileDescriptor) {
		int dataWritten = fwrite(header, 1, pad32bit(headerSize), c->theVirtualDeviceFileDescriptor);
		if (dataWritten > 0) {
			dataWritten += fwrite(data, 1, pad32bit(dataSize), c->theVirtualDeviceFileDescriptor);
		}
		return dataWritten;
	} else {
		return -1;
	}
}

