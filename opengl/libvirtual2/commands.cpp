#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "egl_context.h"
#include "gles2_emulator_constants.h"
#include <cutils/log.h>
#include "commands.h"



	// --------
egl_context_t* getContext()
{
	ogles_context_t* c = ogles_context_t::get();
	return egl_context_t::context(c);
}

void createEGLCommand(egl_context_t* c, int eglFunction, command_control& cmd) {
	cmd.virtualDeviceMagicNumber = GLES2_DEVICE_HEADER;
	cmd.length = -1; // sendCommand will now fill in the length based on the data it has been given to write
	cmd.sequenceNumber = -1;
	cmd.command = EGL14;
	cmd.context = (int)c;
	cmd.returnVal = 0;
	cmd.glFunction = eglFunction;

}

void createEGLCommand(int eglFunction, command_control& cmd) {
	egl_context_t* c = getContext();
	createEGLCommand(c, eglFunction, cmd);
}


void createGLES2Command(int glFunction, command_control& cmd) {
	egl_context_t* c = getContext();

	cmd.virtualDeviceMagicNumber = GLES2_DEVICE_HEADER;
	cmd.length = -1;  // sendCommand will now fill in the length
	cmd.sequenceNumber = -1; 
	cmd.command = GLES20;
	cmd.context = (int)c;
	cmd.returnVal = 0;
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
		command_control* header = (command_control*)buffer;
		header->length = size - sizeof(command_control);
		header->sequenceNumber = c->nextSequence();
		return fwrite(buffer, 1, size, c->theVirtualDeviceFileDescriptor);
	} else {
		LOGV(" --> non-theVirtualDeviceFileDescriptor on sendCommand!");
		return -1;
	}
}
int sendCommand(const char* buffer, int size)
{
	egl_context_t* c = getContext();
	return sendCommand(c, buffer, size);
}

int sendCommandSync(const char* buffer, int size)
{
	int retVal;
	sendCommandSync(buffer, size, &retVal, sizeof(int));
	LOGV("SendCommandSync received %d\n", retVal);
	return retVal;
}

int sendCommandSync(const char* buffer, int size, void* retValue, int retSize)
{
	int ret = 0;
	int retaddr = 0;
	int length = sizeof(GLuint) *2;
	command_control cmd;


	egl_context_t* c = getContext();


	if (c->theVirtualDeviceFileDescriptor)
	{

		command_control* header = (command_control*)buffer;
		header->returnVal = (int)c->physaddress_start;
		ret = sendCommand(c, buffer, size);

		fflush(c->theVirtualDeviceFileDescriptor);

		retaddr = ioctl (c->theVirtualDeviceIOCTLDescriptor, VIRTUALDEVICE_HOST_COMMAND_REGION_WRITE_DONE, 0);
		if (retaddr == (int)c->physaddress_start)
		{
			memcpy(retValue, c->virtaddr, retSize);
		}

	}

	return ret;
}

int sendCommandDataWithHeader(const void* header, int headerSize, const void* data, int dataSize)
{
	egl_context_t* c = getContext();
	if (c->theVirtualDeviceFileDescriptor) {
		command_control* hdr = (command_control*)header;
		hdr->length = (headerSize - sizeof(command_control)) + dataSize;
		hdr->sequenceNumber = c->nextSequence();
		int dataWritten = fwrite(header, 1, pad32bit(headerSize), c->theVirtualDeviceFileDescriptor);
		if (dataWritten > 0) {
			dataWritten += fwrite(data, 1, pad32bit(dataSize), c->theVirtualDeviceFileDescriptor);
		}
		return dataWritten;
	} else {
		return -1;
	}
}

void GetReturnValue(char* buffer,int offset, int length)
{
        egl_context_t* c = getContext();
		memcpy(buffer, (char*)(c->virtaddr)+offset, length);
}

