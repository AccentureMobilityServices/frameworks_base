#ifndef __COMMANDS_H__
#define __COMMANDS_H__
#include "gles2_emulator_constants.h"

egl_context_t* getContext();

void createGLES2Command(int glFunction, command_control& cmd);
void createEGLCommand(int glFunction, command_control& cmd);
void createEGLCommand(egl_context_t* c, int glFunction, command_control& cmd);

int pad32bit(int length);
int pad32strlen(const char* str);

// writes the given amount of data to the buffer, and increments the offset to the next location to write in the buffer
void writeData(char* buffer, int& offset, const void* ptr, int size);
void writeUint32(char* buffer, int& offset, unsigned int val);

// writes the command to the virtual device
int sendCommand(egl_context_t* c, const char* buffer, int size);
int sendCommand(const char* buffer, int size);
int sendCommandSync(const char* buffer, int size, void* retBuffer, int retSize);
// special case, single int return value
int sendCommandSync(const char* buffer, int size);
// writes the command followed by a buffer of data
int sendCommandDataWithHeader(const void* header, int headerSize, const void* data, int dataSize);
//int sendCommandDataWithHeaderSync(const void* header, int headerSize, const void* data, int dataSize);
void GetReturnValue(char* buffer,int offset, int length);
#endif
