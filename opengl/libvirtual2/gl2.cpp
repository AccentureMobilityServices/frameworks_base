/* libs/opengles/state.cpp
 **
 ** Copyright 2006, The Android Open Source Project
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>
#include <cutils/log.h>

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "egl_context.h"
#include "AttribPointer.h"
extern "C" {
#include "crc32.h"
}

#include "gles2_emulator_constants.h"
#include "commands.h"
/* ++ Jose: Virtual device test. ++ */
// TODO: proper 32-bit assignments.
const int MAX_COMMAND_SIZE=256;

namespace android {

	// ----------------------------------------------------------------------------

	static char const * const gVendorString     = "Android";
	static char const * const gRendererString   = "Android PixelFlinger 1.4";
	static char const * const gVersionString    = "OpenGL ES-CM 1.0";
	static char const * const gExtensionsString =
		"GL_OES_byte_coordinates "              // OK
		"GL_OES_fixed_point "                   // OK
		"GL_OES_single_precision "              // OK
		"GL_OES_read_format "                   // OK
		"GL_OES_compressed_paletted_texture "   // OK
		"GL_OES_draw_texture "                  // OK
		"GL_OES_matrix_get "                    // OK
		"GL_OES_query_matrix "                  // OK
		//        "GL_OES_point_size_array "              // TODO
		//        "GL_OES_point_sprite "                  // TODO
		"GL_OES_EGL_image "                     // OK
#ifdef GL_OES_compressed_ETC1_RGB8_texture
		"GL_OES_compressed_ETC1_RGB8_texture "  // OK
#endif
		"GL_ARB_texture_compression "           // OK
		"GL_ARB_texture_non_power_of_two "      // OK
		"GL_ANDROID_user_clip_plane "           // OK
		"GL_ANDROID_vertex_buffer_object "      // OK
		"GL_ANDROID_generate_mipmap "           // OK
		;




	int getDataSize(GLenum type) 
	{
		int len = 0;
		switch(type)
		{
			case GL_BYTE:
				len=sizeof(GLbyte);
				break;
			case GL_UNSIGNED_BYTE:
				len=sizeof(GLubyte);
				break;
			case GL_SHORT:
				len=sizeof(GLshort);
				break;
			case GL_UNSIGNED_SHORT:
				len=sizeof(GLushort);
				break;
			case GL_INT:
				len=sizeof(GLint);
			case GL_UNSIGNED_INT:
				len=sizeof(GLuint);
				break;
			case GL_FLOAT:
				len=sizeof(GLfloat);
				break;
			default:
				break;
		}
		return len;
	}

	template <class T>
		void findMinMaxIndex(const T* indices, int count, int* min, int* max) {
			for (int i=0;i<count;i++) 
			{
				T val = *indices++;
				if ((int)val<*min) 
				{
					*min = val;
				}

				if ((int)val>*max)
				{
					*max = val;
				}

			}

		}

	void getMinMaxIndex(GLenum type, const GLvoid* indices, int count, int* min, int* max) 
	{
		int len = 0;
		*min = 1000000;
		*max = 0;
		switch(type)
		{
			case GL_BYTE:
				findMinMaxIndex<GLbyte>((const GLbyte*)indices, count, min, max);
				break;
			case GL_UNSIGNED_BYTE:
				findMinMaxIndex<GLubyte>((const GLubyte*)indices, count, min, max);
				break;
			case GL_SHORT:
				findMinMaxIndex<GLshort>((const GLshort*)indices, count, min, max);
				break;
			case GL_UNSIGNED_SHORT:
				findMinMaxIndex<GLushort>((const GLushort*)indices, count, min, max);
				break;
			case GL_INT:
				findMinMaxIndex<GLint>((const GLint*)indices, count, min, max);
				break;
			case GL_UNSIGNED_INT:
				findMinMaxIndex<GLuint>((const GLuint*)indices, count, min, max);
				break;
			default:
				break;
		}
	}

	void set_gl_error(ogles_context_t* c, GLenum error)
	{
		if (c->error == GL_NO_ERROR)
			c->error = error;
	}	
}

using namespace android;
GLuint glCreateProgram()
{
	LOGV("glCreateProgram command \n");
	command_control cmd;

	egl_context_t* c = getContext();
	GLuint token = c->nextToken();  

	int offset = 0;
	createGLES2Command(GLCREATEPROGRAM, sizeof(GLuint), cmd);

	char buf[MAX_COMMAND_SIZE];
	if (c->theVirtualDeviceFileDescriptor)
	{
		writeData(buf, offset, &cmd,sizeof(command_control));
		writeData(buf, offset, &token,sizeof(GLuint));
		sendCommand(buf, offset);
	}
	return token;
}

GLuint glCreateShader( GLenum type)
{

	LOGV("libvirtualGLES2 glCreateShader command comes !!!\n");
	command_control cmd;
	egl_context_t* c = getContext();
	GLuint token = c->nextToken();  
	LOGV("libvirtualGLES2 glCreateShader  token %d\n", token);
	int offset = 0;
	createGLES2Command(GLCREATESHADER, sizeof(GLenum)+sizeof(GLuint), cmd);
	char buf[MAX_COMMAND_SIZE];
	if (c->theVirtualDeviceFileDescriptor)
	{
		writeData(buf, offset, &cmd,sizeof(command_control));
		writeData(buf, offset, &type,sizeof(GLenum));
		writeData(buf, offset, &token,sizeof(GLuint));
		sendCommand(buf, offset);	
	}
	LOGV("libvirtualGLES2 glCreateShader  exit\n");
	return token;

}

void glShaderSource(GLuint shader, GLsizei count, const GLchar** strings, const GLint* length) 
{
	LOGV("libvirtualGLES2 glShaderSource cmd \n"); 
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	int stringSize = 0;
	for (int i=0;i<count;i++) {
		if (length != NULL) {
			stringSize += pad32bit(length[i]);
			LOGV("stringsize-lens %d\n", stringSize);
		} else {
			LOGV("string %s\n", strings[i]);
			stringSize += strlen(strings[i]);
			LOGV("stringsize %d\n", stringSize);
			stringSize = pad32bit(stringSize);
			LOGV("stringsize after padding %d\n", stringSize);
		}
	}
	int payloadSize = sizeof(GLuint)+sizeof(GLsizei)+count*sizeof(GLint) + stringSize; 
	createGLES2Command(GLSHADERSOURCE, payloadSize, cmd);
	// we don't know how big this command is so preallocate the buffer
	int bufferSize = payloadSize+sizeof(command_control);
	if (c->theVirtualDeviceFileDescriptor)
	{
		char* buf = new char[bufferSize];
		writeData(buf, offset, &cmd,sizeof(command_control));
		writeData(buf, offset, &shader,sizeof(GLuint));
		writeData(buf, offset, &count,sizeof(GLsizei));
		for (int i=0;i<count;i++) {
			GLint len;
			if (length != NULL) {
				len = length[i];
			} else {
				len = strlen(strings[i]);
			}
			writeData(buf, offset, &len,sizeof(GLint));
			writeData(buf, offset, (void*)strings[i],len);
			offset = pad32bit(offset); // ensure we are on a 32 bit boundary - this corresponds to the padding in the string length calculation above

		}
		LOGV("buf/offset: %d %d\n",bufferSize, offset);
		assert(offset == bufferSize);
		LOGV("after assert\n");
		sendCommand(buf, offset);	
		delete []buf;
	}
}


void glUseProgram(GLuint program)
{
	LOGV("libvirtualGLES2 glUseProgram command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLUSEPROGRAM, sizeof(GLuint), cmd);
	char buf[MAX_COMMAND_SIZE];
	if (c->theVirtualDeviceFileDescriptor)
	{
		writeData(buf, offset, &cmd,sizeof(command_control));
		writeData(buf, offset, &program,sizeof(GLuint));
		sendCommand(buf, offset);
	}
}


// we can't send the data when we get this command as we don't know how big the array is before the DrawElements, DrawArray commands have
// been issued
void glVertexAttribPointer (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr){
	egl_context_t* c = getContext();
	c->attribs[indx].size = size;
	c->attribs[indx].type = type;
	c->attribs[indx].normalized = normalized;
	c->attribs[indx].stride = stride;
	c->attribs[indx].pointer = ptr;
}


// send all the data needed for the enabled attrib pointers 
void sendVertexAttribData(int first, int count) 
{
	GLuint indx;
	egl_context_t* c = getContext();
	for (indx = 0; indx<MAX_ATTRIBS; indx++) {
		// only send enabled pointers
		if (c->attribs[indx].enabled) {
			command_control cmd;
			int offset = 0;
			int len = 0;
			switch(c->attribs[indx].type)
			{
				case GL_BYTE:
					len=sizeof(GLbyte);
					break;
				case GL_UNSIGNED_BYTE:
					len=sizeof(GLubyte);
					break;
				case GL_SHORT:
					len=sizeof(GLshort);
				case GL_UNSIGNED_SHORT:
					len=sizeof(GLushort);
					break;
				case GL_INT:
					len=sizeof(GLint);
				case GL_UNSIGNED_INT:
					len=sizeof(GLuint);
					break;
				case GL_FLOAT:
					len=sizeof(GLfloat);
					break;
				default:
					break;
			}
			len *= c->attribs[indx].size;
			LOGV("sendVertexAttribData len %d\n", len);
			GLint length = count * len;
			LOGV("sendVertexAttribData count %d length %d\n", count, length);

			// most of the time the attribute data hasn't changed, so compare the crc with the crc of the data we last sent

			bool needToSend = true;
			unsigned int crc = 0;
			if (c->attribs[indx].lastSentCount == length) 
			{
				crc = crc32((unsigned char*)c->attribs[indx].pointer, length);
				if (c->attribs[indx].lastSentCrc32 == crc) {
					needToSend = false;
				}
			}

			if (needToSend) {
				c->attribs[indx].lastSentCount = length;
				c->attribs[indx].lastSentCrc32 = crc;	
				// host interprets sending the data as enable too
				int payloadSize = sizeof(GLuint)*2+sizeof(GLint)+sizeof(GLenum)+sizeof(GLuint)+sizeof(GLsizei)+pad32bit(length);
				createGLES2Command(GLVERTEXATTRIBPOINTER, payloadSize, cmd);
				int bufferSize = payloadSize + sizeof(command_control);
				LOGV("sendVertexAttribData payloadSize %d\n", payloadSize);
				LOGV("sendVertexAttribData bufferSize %d\n", bufferSize);
				LOGV("sendVertexAttribData pointer %08x\n", c->attribs[indx].pointer);
				char buf[MAX_COMMAND_SIZE];
				if (c->theVirtualDeviceFileDescriptor)
				{
					writeData(buf, offset, &cmd,sizeof(command_control));
					writeData(buf, offset, &indx,sizeof(GLuint));
					writeData(buf, offset, &c->attribs[indx].size,sizeof(GLuint));
					writeData(buf, offset, &c->attribs[indx].type,sizeof(GLenum));
					writeData(buf, offset, &c->attribs[indx].normalized,sizeof(GLuint)); //GLboolean is a char, we want to write 32 bit words
					writeData(buf, offset, &c->attribs[indx].stride,sizeof(GLsizei));
					writeData(buf, offset, &length,sizeof(GLint));
					sendCommandDataWithHeader(buf, offset, (GLvoid*)(c->attribs[indx].pointer +(len*first)), length);
				}
			} else {
				// if data already on the host - just send enable
				int payloadSize = sizeof(GLuint);
				createGLES2Command(GLENABLEVERTEXATTRIBARRAY, payloadSize, cmd);
				int bufferSize = payloadSize + sizeof(command_control);
				char buf[MAX_COMMAND_SIZE];
				if (c->theVirtualDeviceFileDescriptor)
				{
					writeData(buf, offset, &cmd,sizeof(command_control));
					writeData(buf, offset, &indx,sizeof(GLuint));
					sendCommand(buf, offset);
				}
			}
		}
	}
}

void glEnableVertexAttribArray(GLuint index)
{

	egl_context_t* c = getContext();
	c->attribs[index].enabled = true;

}

void glAttachShader(GLuint program, GLuint shader)
{
	LOGV("libvirtualGLES2 glAttachShader command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLATTACHSHADER, sizeof(GLuint)*2, cmd);

	char buf[MAX_COMMAND_SIZE];
	if (c->theVirtualDeviceFileDescriptor)
	{
		writeData(buf, offset, &cmd, sizeof(command_control));
		writeData(buf, offset, &program, sizeof(program));
		writeData(buf, offset, &shader, sizeof(shader));
		sendCommand(buf, offset);
	}
}
void glLinkProgram(GLuint program)
{
	LOGV("libvirtualGLES2 glLinkProgram command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLLINKPROGRAM, sizeof(GLuint), cmd);
	char buf[MAX_COMMAND_SIZE];
	if (c->theVirtualDeviceFileDescriptor)
	{
		writeData(buf, offset, &cmd, sizeof(command_control));
		writeData(buf, offset, &program, sizeof(program));
		sendCommand(buf, offset);
	}
}

void glGetProgramiv(GLuint program, GLenum pname, GLint* params)
{
	params[0] = GL_TRUE;
	LOGV("libvirtualGLES2 glGetProgramiv command \n");
}
void glGetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
	LOGV("libvirtualGLES2 glGetProgramInfoLog command \n");
}
void glDeleteProgram(GLuint program)
{
	LOGV("libvirtualGLES2 glDeleteProgram command \n");
	egl_context_t* c = getContext();
	int offset = 0;
	command_control cmd;
	createGLES2Command(GLDELETEPROGRAM, sizeof(GLuint), cmd);
	char buf[MAX_COMMAND_SIZE];
	if (c->theVirtualDeviceFileDescriptor)
	{
		writeData(buf, offset, &cmd, sizeof(command_control));
		writeData(buf, offset, &program, sizeof(program));
		sendCommand(buf, offset);
	}
}

int glGetAttribLocation(GLuint program, const GLchar* name)
{
	LOGV("libvirtualGLES2 glGetAttribLocation command \n");
	command_control cmd;
	egl_context_t* c = getContext(); 
	GLint len=strlen(name)+1; // send the terminating null as well
	int size = 2*sizeof(GLuint)+pad32bit(len)+sizeof(GLuint);//token remove later
	int offset = 0;
	GLuint token = c->nextToken();  
	createGLES2Command(GLGETATTRIBLOCATION, size, cmd);
	char buf[MAX_COMMAND_SIZE];
	if (c->theVirtualDeviceFileDescriptor)
	{
		writeData(buf, offset, &cmd, sizeof(command_control));		
		writeData(buf, offset, &program, sizeof(GLuint));
		writeData(buf, offset, &token, sizeof(GLuint));
		writeData(buf, offset, &len, sizeof(GLuint));
		writeData(buf, offset, (void*)name, len);
		offset = pad32bit(offset); // correct for non-32 bit length string
		sendCommand(buf, offset);
	}
	return token;
}


void glCompileShader(GLuint shader)
{
	LOGV("libvirtualGLES2 glCompileShader command comes !!!\n");
	command_control cmd;
	egl_context_t* c = getContext(); 
	int offset = 0;
	createGLES2Command(GLCOMPILESHADER, sizeof(GLuint), cmd);

	char buf[MAX_COMMAND_SIZE];
	if (c->theVirtualDeviceFileDescriptor)
	{
		writeData(buf, offset, &cmd, sizeof(command_control));
		writeData(buf, offset, &shader, sizeof(GLuint));
		sendCommand(buf, offset);
	}
}
#if 0 
/* Implement sync mechanism between guest and host - To test*/
void glGetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
	LOGV("libvirtualGLES2 glGetShaderiv command comes !!!\n");
	command_control cmd;
	command_sync syncmsg;
	egl_context_t* c = getContext(); 
	int Size = sizeof(GLuint)+sizeof(GLenum);
	int offset = 0;
	createGLES2Command(GLGETSHADERIV, Size, cmd);
	if (c->theVirtualDeviceFileDescriptor)
	{
		writeData(buf, offset, &cmd, sizeof(command_control));
		writeData(buf, offset, &shader, sizeof(GLuint));
		writeData(buf, offset, &pname, sizeof(GLenum));
		fwrite(buf, 1, offset, c->theVirtualDeviceFileDescriptor);
		//send Sync command to get the return value of the params.

		syncmsg.syncflag=0x00;
		syncmsg.commandflag=0x00;
		syncmsg.retvalue=0x00;
		ioctl (c->theVirtualDeviceExchangeDescriptor, 68, &syncmsg);
		if(syncmsg.syncflag==0x01){
			*params=syncmsg.retvalue;
		}
		else {
			LOGV("Command sync error Device exchange Desc\n");
			*params=0;
		}
	}    

}
#endif
void glGetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
	LOGV("libvirtualGLES2 glGetShaderiv command comes !!!\n");
	command_control cmd;
	egl_context_t* c = getContext(); 
	int size = sizeof(GLuint)+sizeof(GLenum);
	int offset = 0;
	createGLES2Command(GLGETSHADERIV, size, cmd);

	switch(pname)
	{
		case GL_COMPILE_STATUS:
			//need to check the status of the of the compile status and return 
			*params=GL_TRUE;
			break;
		case GL_INFO_LOG_LENGTH:
			*params=0;
			break;
		case GL_SHADER_TYPE:
			break;
		case GL_DELETE_STATUS:
			break;
		case  GL_SHADER_SOURCE_LENGTH:
			break;
		default:
			break;
	}
	char buf[MAX_COMMAND_SIZE];
	if (c->theVirtualDeviceFileDescriptor)
	{
		writeData(buf, offset, &cmd, sizeof(command_control));
		writeData(buf, offset, &shader, sizeof(GLuint));
		writeData(buf, offset, &pname, sizeof(GLuint));
		sendCommand(buf, offset);
	}
	params[0] = GL_TRUE;

}

void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	LOGV("libvirtualGLES2 glUniformMatrix4fv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	int len=sizeof(GLint)+sizeof(GLsizei)+sizeof(GLuint)+count*16*sizeof(GLfloat);
	createGLES2Command(GLUNIFORMMATRIX4FV, len, cmd);
	GLuint transVal = (GLuint) transpose; //GLboolean is byte size, which is incovenient for the parsing which assumes 32 bit word alignment
	char buf[MAX_COMMAND_SIZE];
	if (c->theVirtualDeviceFileDescriptor)
	{
		writeData(buf, offset, &cmd, sizeof(command_control));
		writeData(buf, offset, &location, sizeof(GLuint));
		writeData(buf, offset, &count, sizeof(GLsizei));
		writeData(buf, offset, &transVal, sizeof(GLuint));
		for(int i=0;i<count;i++)
		{
			writeData(buf, offset, (void*)&value[i*16], 16*sizeof(GLfloat));
		}
		sendCommand(buf, offset);
	}
}

void glUniform3fv(GLint location, GLsizei count, const GLfloat* v)
{
	LOGV("libvirtualGLES2 glUniform3fv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	int len=sizeof(GLint)+sizeof(GLsizei)+count*3*sizeof(GLfloat);
	createGLES2Command(GLUNIFORM3FV, len, cmd);
	char buf[MAX_COMMAND_SIZE];
	if (c->theVirtualDeviceFileDescriptor)
	{
		writeData(buf, offset, &cmd, sizeof(command_control));
		writeData(buf, offset, &location, sizeof(GLuint));
		writeData(buf, offset, &count, sizeof(GLsizei));
		for(int i=0;i<count;i++)
		{
			writeData(buf, offset, (void*)&v[i*3], 3*sizeof(GLfloat));
		}
		sendCommand(buf, offset);
	}
}

int glGetUniformLocation(GLuint program, const GLchar* name)
{
	LOGV("libvirtualGLES2 glGetUniformLocation command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	GLuint token = c->nextToken();  
	int len=strlen(name)+1; // send terminating null in string
	createGLES2Command(GLGETUNIFORMLOCATION,2* sizeof(GLuint)+pad32bit(len)+sizeof(GLuint), cmd);
	char buf[MAX_COMMAND_SIZE];
	if (c->theVirtualDeviceFileDescriptor)
	{
		writeData(buf, offset, &cmd, sizeof(command_control));
		writeData(buf, offset, &program, sizeof(GLuint));
		writeData(buf, offset, &token, sizeof(GLuint));
		writeData(buf, offset, &len, sizeof(GLuint));
		writeData(buf, offset, (void*)name, len);
		offset = pad32bit(offset);
		sendCommand(buf, offset);
	}
	return token;
}

void glGetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
	LOGV("libvirtualGLES2 glGetShaderInfoLog command comes !!!\n");
}

void glViewport(GLint x, GLint y, GLsizei w, GLsizei h)
{
	LOGV("GLES2 glViewport command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLVIEWPORT, 2*sizeof(GLuint)+2*sizeof(GLsizei), cmd);
	char buf[MAX_COMMAND_SIZE];

	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &x, sizeof(GLint));
	writeData(buf, offset, &y, sizeof(GLint));
	writeData(buf, offset, &w, sizeof(GLsizei));
	writeData(buf, offset, &h, sizeof(GLsizei));
	sendCommand(buf, offset);
}

void glDeleteShader(GLuint shader)
{
	LOGV("libvirtualGLES2 glDeleteShader command \n");
	command_control cmd;
	egl_context_t* c = getContext(); 
	int offset = 0;
	createGLES2Command(GLDELETESHADER, sizeof(GLuint), cmd);
	char buf[MAX_COMMAND_SIZE];
	if (c->theVirtualDeviceFileDescriptor)
	{
		writeData(buf, offset, &cmd, sizeof(command_control));
		writeData(buf, offset, &shader, sizeof(shader));
		sendCommand(buf, offset);
	}
}


// These ones are super-easy, we're not supporting those features!
void glSampleCoverage(GLclampf value, GLboolean invert) {
}
void glSampleCoveragex(GLclampx value, GLboolean invert) {
}
void glStencilFunc(GLenum func, GLint ref, GLuint mask) {
}

// ----------------------------------------------------------------------------

void glCullFace(GLenum mode)
{
}

void glFrontFace(GLenum mode)
{
}

void glHint(GLenum target, GLenum mode)
{
}

void glEnable(GLenum cap) 
{
	LOGV("GLES2 Glenable command\n");
	command_control cmd;
	int offset = 0;
	createGLES2Command(GLENABLE, sizeof(GLenum), cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &cap, sizeof(GLenum));
	sendCommand(buf, offset);	
}

void glDisable(GLenum cap) 
{
	LOGV("GLES2 Gldisable command\n");
	command_control cmd;
	int offset = 0;
	createGLES2Command(GLDISABLE, sizeof(GLenum), cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &cap, sizeof(GLenum));
	sendCommand(buf, offset);	
}


void glFinish()
{ // nothing to do for our software implementation
}

void glFlush()
{ // nothing to do for our software implementation
}

GLenum glGetError()
{
	return GL_NO_ERROR;
}

const GLubyte* glGetString(GLenum string)
{
	switch (string) {
		case GL_VENDOR:     return (const GLubyte*)gVendorString;
		case GL_RENDERER:   return (const GLubyte*)gRendererString;
		case GL_VERSION:    return (const GLubyte*)gVersionString;
		case GL_EXTENSIONS: return (const GLubyte*)gExtensionsString;
	}
	ogles_context_t* c = ogles_context_t::get();
	set_gl_error(c, GL_INVALID_ENUM);
	return 0;
}

void glClear(GLbitfield mask) {
	LOGV("GLES2 glClear!!!");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLCLEAR, sizeof(GLbitfield), cmd);
	/* ++ Jose: Virtual device test. ++ */
	char buf[MAX_COMMAND_SIZE];
	if (c->theVirtualDeviceFileDescriptor)
	{

		writeData(buf, offset, &cmd, sizeof(command_control));
		writeData(buf, offset, &mask, sizeof(mask));
		sendCommand(buf, offset);
	}

	/* -- Jose: Virtual device test. -- */
}

void glClearColorx(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha) {
	LOGV("GLES2 glClearColorx");
	egl_context_t* c = getContext();
	command_control cmd;
	int offset = 0;
	createGLES2Command(GLCLEARCOLORX, 4*sizeof(GLclampx), cmd);


	char buf[MAX_COMMAND_SIZE];
	if (c->theVirtualDeviceFileDescriptor)
	{

		writeData(buf, offset, &cmd, sizeof(command_control));
		writeData(buf, offset, &red, sizeof(GLclampx));
		writeData(buf, offset, &green, sizeof(GLclampx));
		writeData(buf, offset, &blue, sizeof(GLclampx));
		writeData(buf, offset, &alpha, sizeof(GLclampx));
		sendCommand(buf, offset);
	}
}


void glClearColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a)
{
	LOGV("GLES2 glClearColor");

	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLCLEARCOLORF, 4*sizeof(GLclampf), cmd);
	char buf[MAX_COMMAND_SIZE];
	if (c->theVirtualDeviceFileDescriptor)
	{

		writeData(buf, offset, &cmd, sizeof(command_control));
		writeData(buf, offset, &r, sizeof(GLclampf));
		writeData(buf, offset, &g, sizeof(GLclampf));
		writeData(buf, offset, &b, sizeof(GLclampf));
		writeData(buf, offset, &a, sizeof(GLclampf));
		sendCommand(buf, offset);
	}
}

void glClearDepthx(GLclampx depth) {
	LOGV ("virtualLIB %s ", __func__);
}

void glClearDepthf(GLclampf depth)
{
	LOGV ("virtualLIB %s ", __func__);
	command_control cmd;
	int glFunction = GLCLEARDEPTHF;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLCLEARDEPTHF, sizeof(GLclampf), cmd);
	char buf[MAX_COMMAND_SIZE];
	if (c->theVirtualDeviceFileDescriptor)
	{
		writeData(buf, offset, &cmd, sizeof(command_control));
		writeData(buf, offset, &depth, sizeof(GLclampf));
		sendCommand(buf, offset);
	}

}

void glClearStencil(GLint s) {
	LOGV ("virtualLIB %s ", __func__);
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLCLEARDSTENCIL, sizeof(GLint), cmd);
	char buf[MAX_COMMAND_SIZE];
	if (c->theVirtualDeviceFileDescriptor)
	{
		writeData(buf, offset, &cmd, sizeof(command_control));
		writeData(buf, offset, &s, sizeof(GLint));
		sendCommand(buf, offset);
	}
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
	// call gl1 render
	egl_context_t* c = getContext();
	// transfer the data to the host - we can only do this now that we've been told which data to draw
	sendVertexAttribData(first, count);

	LOGV("GLES2 glDrawArrays(2) command \n");
	command_control cmd;
	int offset = 0;
	createGLES2Command(GLDRAWARRAYS, sizeof(GLenum) + sizeof(GLsizei), cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &mode, sizeof(GLenum));
	// only transfer the data count - the host will only receive data from the first index given so no need for it to know about 
	// the index 
	writeData(buf, offset, &count, sizeof(GLsizei));
	sendCommand(buf, offset);
}

void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
	int min;
	int max;
	getMinMaxIndex(type, indices, count, &min, &max); // always send from 0 for the moment - must keep aligned with index values
	//	LOGV("glDrawElements min %d max %d\n", min, max);
	sendVertexAttribData(0, max);

	//	LOGV("GLES2 glDrawArrays(2) command \n");
	command_control cmd;
	int offset = 0;
	int dataLength = pad32bit(getDataSize(type)*count);
	int payloadSize = sizeof(GLenum) + sizeof(GLsizei) + sizeof(GLenum)+dataLength;
	createGLES2Command(GLDRAWELEMENTS, payloadSize, cmd);
	char buf[MAX_COMMAND_SIZE]; 
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &mode, sizeof(GLenum));
	writeData(buf, offset, &count, sizeof(GLsizei));
	writeData(buf, offset, &type, sizeof(GLenum));
	int length = getDataSize(type)*count;
	sendCommandDataWithHeader(buf, offset, indices, dataLength);
}
