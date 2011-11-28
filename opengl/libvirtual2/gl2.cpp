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

//#define LOG_NDEBUG 0
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
				LOGV("Unknown pixel data format\n");
				break;
		}
		return len;
	}
int getDataFormatSize(GLenum type)
	{
		int len = 0;
		switch(type)
		{
			case GL_ALPHA:
			case GL_LUMINANCE:
				len=1;
				break;
			case GL_LUMINANCE_ALPHA:
				len=2;
				break;
			case GL_RGB:
				len=3;
				break;
			case GL_RGBA:
				len=4;
				break;
			default:
				LOGV("Unknown pixel format\n");
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
	int offset = 0;
	createGLES2Command(GLCREATEPROGRAM, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd,sizeof(command_control));
	return sendCommandSync(buf, offset);
}

GLuint glCreateShader( GLenum type)
{

	LOGV("libvirtualGLES2 glCreateShader command comes !!!\n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLCREATESHADER, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd,sizeof(command_control));
	writeData(buf, offset, &type,sizeof(GLenum));
	return sendCommandSync(buf, offset);
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
	createGLES2Command(GLSHADERSOURCE, cmd);
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
	int offset = 0;
	createGLES2Command(GLUSEPROGRAM, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd,sizeof(command_control));
	writeData(buf, offset, &program,sizeof(GLuint));
	sendCommand(buf, offset);
}


// we can't send the data when we get this command as we don't know how big the array is before the DrawElements, DrawArray commands have
// been issued
void glVertexAttribPointer (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr){
	LOGV("(%s)\n", __FUNCTION__);
	LOGV("index %d\n", indx);
	egl_context_t* c = getContext();
	c->attribs[indx].arrayBuffer = c->arrayBuffer; // store whether bound to a buffer for all arrays
	if (c->arrayBuffer ==0) { // only store the rest of the parameters if not bound to a buffer 
		LOGV("not bound to buffer\n");
		c->attribs[indx].size = size;
		c->attribs[indx].type = type;
		c->attribs[indx].normalized = normalized;
		c->attribs[indx].stride = stride;
		c->attribs[indx].pointer = ptr;
	} else {
		// send through pointer if bound to a buffer
		LOGV("bound to buffer\n");
		command_control cmd;
		int offset =0;
		createGLES2Command(GLVERTEXATTRIBPOINTER, cmd);
		char buf[MAX_COMMAND_SIZE];
		writeData(buf, offset, &cmd,sizeof(command_control));
		writeData(buf, offset, &indx,sizeof(GLuint));
		writeData(buf, offset, &size,sizeof(GLuint));
		writeData(buf, offset, &type,sizeof(GLenum));
		writeData(buf, offset, &normalized,sizeof(GLuint)); //GLboolean is a char, we want to write 32 bit words
		writeData(buf, offset, &stride,sizeof(GLsizei));
		writeData(buf, offset, &ptr,sizeof(GLint));
		sendCommand(buf, offset);
	}
}


// send all the data needed for the enabled attrib pointers
void sendVertexAttribData(int first, int count)
{
	GLuint indx;
	egl_context_t* c = getContext();
	for (indx = 0; indx<MAX_ATTRIBS; indx++) {
		// only send enabled pointers and ones not bound to a buffer
		if (c->attribs[indx].enabled && c->attribs[indx].arrayBuffer ==0) {
			LOGV("Sending index %d\n", indx);
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
			LOGV("sendVertexAttribData pointer 0x%08x\n", c->attribs[indx].pointer);

			// most of the time the attribute data hasn't changed, so compare the crc with the crc of the data we last sent

			bool needToSend = true;
			unsigned int crc = 0;
			if (c->attribs[indx].lastSentCount == length)
			{
				crc = calcCRC32((unsigned char*)c->attribs[indx].pointer, length);
				if (c->attribs[indx].lastSentCrc32 == crc) {
					needToSend = false;
				}
			}

			if (needToSend) {
				c->attribs[indx].lastSentCount = length;
				c->attribs[indx].lastSentCrc32 = crc;
				// host interprets sending the data as enable too
				createGLES2Command(SENDVERTEXATTRIBPOINTERDATA, cmd);
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
					sendCommandDataWithHeader(buf, offset, (GLvoid*)((char*)c->attribs[indx].pointer +(len*first)), length);
				}
			}
		}
	}
}

void glEnableVertexAttribArray(GLuint index)
{

	egl_context_t* c = getContext();
	c->attribs[index].enabled = true;
	command_control cmd;
	int offset =0;
	createGLES2Command(GLENABLEVERTEXATTRIBARRAY, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd,sizeof(command_control));
	writeData(buf, offset, &index,sizeof(GLuint));
	sendCommand(buf, offset);

}

void glAttachShader(GLuint program, GLuint shader)
{
	LOGV("libvirtualGLES2 glAttachShader command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLATTACHSHADER, cmd);

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
	createGLES2Command(GLLINKPROGRAM, cmd);
	char buf[MAX_COMMAND_SIZE];
	if (c->theVirtualDeviceFileDescriptor)
	{
		writeData(buf, offset, &cmd, sizeof(command_control));
		writeData(buf, offset, &program, sizeof(program));
		sendCommand(buf, offset);
	}
}


void glDeleteProgram(GLuint program)
{
	LOGV("libvirtualGLES2 glDeleteProgram command \n");
	egl_context_t* c = getContext();
	int offset = 0;
	command_control cmd;
	createGLES2Command(GLDELETEPROGRAM, cmd);
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
	int offset = 0;
	createGLES2Command(GLGETATTRIBLOCATION, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &program, sizeof(GLuint));
	writeData(buf, offset, &len, sizeof(GLuint));
	writeData(buf, offset, (void*)name, len);
	offset = pad32bit(offset); // correct for non-32 bit length string
	return sendCommandSync(buf, offset);
}


void glCompileShader(GLuint shader)
{
	LOGV("libvirtualGLES2 glCompileShader command comes !!!\n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLCOMPILESHADER, cmd);

	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &shader, sizeof(GLuint));
	sendCommand(buf, offset);
}


void glGetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
	LOGV("libvirtualGLES2 glGetShaderiv command comes !!!\n");
	command_control cmd;
	egl_context_t* c = getContext();
	int size = sizeof(GLuint)+sizeof(GLenum);
	int offset = 0;
	createGLES2Command(GLGETSHADERIV, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &shader, sizeof(GLuint));
        writeData(buf, offset, &pname, sizeof(GLenum));
	sendCommandSync(buf, offset, params, sizeof(GLint));

}

void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	LOGV("libvirtualGLES2 glUniformMatrix4fv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	int len=sizeof(GLint)+sizeof(GLsizei)+sizeof(GLuint)+count*16*sizeof(GLfloat);
	createGLES2Command(GLUNIFORMMATRIX4FV, cmd);
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
	createGLES2Command(GLUNIFORM3FV, cmd);
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

GLint glGetUniformLocation(GLuint program, const GLchar* name)
{
	LOGV("libvirtualGLES2 glGetUniformLocation command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	int len=strlen(name)+1; // send terminating null in string
	createGLES2Command(GLGETUNIFORMLOCATION, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &program, sizeof(GLuint));
	writeData(buf, offset, &len, sizeof(GLuint));
	writeData(buf, offset, (void*)name, len);
	offset = pad32bit(offset);
	return sendCommandSync(buf, offset);
}


void glViewport(GLint x, GLint y, GLsizei w, GLsizei h)
{
	LOGV("GLES2 glViewport command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLVIEWPORT, cmd);
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
	createGLES2Command(GLDELETESHADER, cmd);
	char buf[MAX_COMMAND_SIZE];
	if (c->theVirtualDeviceFileDescriptor)
	{
		writeData(buf, offset, &cmd, sizeof(command_control));
		writeData(buf, offset, &shader, sizeof(shader));
		sendCommand(buf, offset);
	}
}


// These ones are super-easy, we're not supporting those features!
void glSampleCoverage(GLclampf value, GLboolean invert)
{
	LOGV("GLES2 command glSampleCoverage\n");
	command_control cmd;
	int offset = 0;
	GLuint invertval = (GLuint) invert; //GLboolean is byte size, which is incovenient for the parsing which assumes 32 bit word alignment
	createGLES2Command(GLSAMPLECOVERAGE, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &value, sizeof(GLclampf));
	writeData(buf, offset, &invertval, sizeof(GLuint));
	sendCommand(buf, offset);
}
void glSampleCoveragex(GLclampx value, GLboolean invert)
{
	LOGV("GLES2 command glSampleCoveragex\n");
	command_control cmd;
	int offset = 0;
	GLuint invertval = (GLuint) invert; //GLboolean is byte size, which is incovenient for the parsing which assumes 32 bit word alignment
	createGLES2Command(GLSAMPLECOVERAGEX, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &value, sizeof(GLclampx));
	writeData(buf, offset, &invert, sizeof(GLuint));
	sendCommand(buf, offset);
}
void glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
	LOGV("GLES2 command glSampleCoverage\n");
	command_control cmd;
	int offset = 0;
	createGLES2Command(GLSTENCILFUNC, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &func, sizeof(GLenum));
	writeData(buf, offset, &ref, sizeof(GLint));
	writeData(buf, offset, &mask, sizeof(GLint));
	sendCommand(buf, offset);
}

// ----------------------------------------------------------------------------

void glCullFace(GLenum mode)
{
	LOGV("GLES2 command glCullFace\n");
	command_control cmd;
	int offset = 0;
	createGLES2Command(GLCULLFACE, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &mode, sizeof(GLenum));
	sendCommand(buf, offset);
}

void glFrontFace(GLenum mode)
{
	LOGV("GLES2 command glFrontFace\n");
	command_control cmd;
	int offset = 0;
	createGLES2Command(GLFRONTFACE, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &mode, sizeof(GLenum));
	sendCommand(buf, offset);
}

void glHint(GLenum target, GLenum mode)
{
	LOGV("GLES2 command glHint\n");
	command_control cmd;
	int offset = 0;
	createGLES2Command(GLHINT, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &target, sizeof(GLenum));
	writeData(buf, offset, &mode, sizeof(GLenum));
	sendCommand(buf, offset);
}

void glEnable(GLenum cap)
{
	LOGV("GLES2 Glenable command\n");
	command_control cmd;
	int offset = 0;
	createGLES2Command(GLENABLE, cmd);
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
	createGLES2Command(GLDISABLE, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &cap, sizeof(GLenum));
	sendCommand(buf, offset);
}


void glFinish()
{
       LOGV("GLES2  command glFinish\n");
	command_control cmd;
	int offset = 0;
	createGLES2Command(GLFINISH, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	sendCommand(buf, offset);
}

void glFlush()
{
       LOGV("GLES2 command glFlush\n");
	command_control cmd;
	int offset = 0;
	createGLES2Command(GLFLUSH, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	sendCommand(buf, offset);
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
	createGLES2Command(GLCLEAR, cmd);
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
	createGLES2Command(GLCLEARCOLORX, cmd);


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
	createGLES2Command(GLCLEARCOLORF, cmd);
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
	command_control cmd;
	int glFunction = GLCLEARDEPTHX;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLCLEARDEPTHX, cmd);
	char buf[MAX_COMMAND_SIZE];
	if (c->theVirtualDeviceFileDescriptor)
	{
		writeData(buf, offset, &cmd, sizeof(command_control));
		writeData(buf, offset, &depth, sizeof(GLclampf));
		sendCommand(buf, offset);
	}

}

void glClearDepthf(GLclampf depth)
{
	LOGV ("virtualLIB %s ", __func__);
	command_control cmd;
	int glFunction = GLCLEARDEPTHF;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLCLEARDEPTHF, cmd);
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
	createGLES2Command(GLCLEARDSTENCIL, cmd);
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
	createGLES2Command(GLDRAWARRAYS, cmd);
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
	createGLES2Command(GLDRAWELEMENTS, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &mode, sizeof(GLenum));
	writeData(buf, offset, &count, sizeof(GLsizei));
	writeData(buf, offset, &type, sizeof(GLenum));
	int length = getDataSize(type)*count;
	sendCommandDataWithHeader(buf, offset, indices, dataLength);
}
void glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
	command_control cmd;
	egl_context_t* c = getContext();
        int offset = 0;
	createGLES2Command(GLBLENDCOLOR, cmd);
        char buf[MAX_COMMAND_SIZE];
        writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &red, sizeof(GLclampf));
	writeData(buf, offset, &green, sizeof(GLclampf));
	writeData(buf, offset, &blue, sizeof(GLclampf));
	writeData(buf, offset, &alpha, sizeof(GLclampf));
 	sendCommand(buf, offset);
}

void glBlendEquation( GLenum mode )
{
	command_control cmd;
	egl_context_t* c = getContext();
        int offset = 0;
	createGLES2Command(GLBLENDEQUATION, cmd);
        char buf[MAX_COMMAND_SIZE];
        writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &mode, sizeof(GLenum));
	sendCommand(buf, offset);
}

void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
	command_control cmd;
	egl_context_t* c = getContext();
        int offset = 0;
	createGLES2Command(GLBLENDEQUATIONSEPARATE, cmd);
        char buf[MAX_COMMAND_SIZE];
        writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &modeRGB, sizeof(GLenum));
	writeData(buf, offset, &modeAlpha, sizeof(GLenum));
	sendCommand(buf, offset);
}
void glBlendFunc(GLenum sfactor, GLenum dfactor)
{
	command_control cmd;
	egl_context_t* c = getContext();
        int offset = 0;
	createGLES2Command(GLBLENDFUNC, cmd);
        char buf[MAX_COMMAND_SIZE];
        writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &sfactor, sizeof(GLenum));
	writeData(buf, offset, &dfactor, sizeof(GLenum));
	sendCommand(buf, offset);
}

void glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
	command_control cmd;
	egl_context_t* c = getContext();
        int offset = 0;
	createGLES2Command(GLBLENDFUNCSEPARATE, cmd);
        char buf[MAX_COMMAND_SIZE];
        writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &srcRGB, sizeof(GLenum));
	writeData(buf, offset, &dstRGB, sizeof(GLenum));
	writeData(buf, offset, &srcAlpha, sizeof(GLenum));
	writeData(buf, offset, &dstAlpha, sizeof(GLenum));
 	sendCommand(buf, offset);
}
void glBindAttribLocation(GLuint program, GLuint index, const GLchar* name)
{
	command_control cmd;
	egl_context_t* c = getContext();
        int offset = 0;
        GLint len=strlen(name)+1;
	int size = 3*sizeof(GLuint)+pad32bit(len);
	createGLES2Command(GLBINDATTRIBLOCATION, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &program, sizeof(GLuint));
	writeData(buf, offset, &index, sizeof(GLuint));
	writeData(buf, offset, &len, sizeof(GLuint));
	writeData(buf, offset, (void*)name, len);
	offset = pad32bit(offset);
	sendCommand(buf, offset);
}
void glGenTextures(GLsizei n, GLuint* textures)
{
        LOGV("GLES2 glGenTextures command \n");
	command_control cmd;
	egl_context_t* c = getContext();
        int offset = 0;
	createGLES2Command(GLGENTEXTURES, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &n, sizeof(GLsizei));
	sendCommandSync(buf, offset, textures, sizeof(GLuint)*n);
}

void glActiveTexture(GLenum texture)
{
 	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLACTIVETEXTURE, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &texture, sizeof(GLenum));
        sendCommand(buf, offset);
}
void glBindTexture(GLenum target, GLuint texture)
{
 	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLBINDTEXTURE, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &target, sizeof(GLenum));
	writeData(buf, offset, &texture, sizeof(GLuint));
        sendCommand(buf, offset);

}
void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
        command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLFRAMEBUFFERTEXTURE2D, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &target, sizeof(GLenum));
	writeData(buf, offset, &attachment, sizeof(GLenum));
	writeData(buf, offset, &textarget, sizeof(GLenum));
	writeData(buf, offset, &texture, sizeof(GLuint));
	writeData(buf, offset, &level, sizeof(GLuint));
        sendCommand(buf, offset);
}
GLboolean glIsTexture(GLuint texture)
{
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	GLboolean retval;
	createGLES2Command(GLISTEXTURE, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &texture, sizeof(GLenum));
        sendCommandSync(buf, offset, &retval, sizeof(GLboolean));
   	return retval;

}
void glDeleteTextures(GLsizei n, const GLuint* textures)
{
        command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLDELETETEXTURES, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &n, sizeof(GLsizei));
	for(int i=0;i<n;i++)
	{
		writeData(buf, offset, (void*)&textures[i], sizeof(GLuint));
	}

        sendCommand(buf, offset);
}
void glBufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)
{
    //To-Do
    LOGV("GLES2 glBufferData command \n");
    command_control cmd;
    egl_context_t* c = getContext();
    int offset = 0;
    createGLES2Command(GLBUFFERDATA, cmd);
    char buf[MAX_COMMAND_SIZE];
    writeData(buf, offset, &cmd, sizeof(command_control));
    writeData(buf, offset, &target, sizeof(GLenum));
    writeData(buf, offset, &size, sizeof(GLsizeiptr));
    writeData(buf, offset, &usage, sizeof(GLenum));
    if(size!=0){
      sendCommandDataWithHeader(buf, offset, data, size);
    }
     else{
           sendCommand(buf, offset);     
     }
}

void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data)
{
    LOGV("GLES2 glBufferSubData command \n");
    command_control cmd;
    egl_context_t* c = getContext();
    int lenoffset = 0;
    createGLES2Command(GLBUFFERSUBDATA, cmd);
    char buf[MAX_COMMAND_SIZE];
    writeData(buf, lenoffset, &cmd, sizeof(command_control));
    writeData(buf, lenoffset, &target, sizeof(GLenum));
    writeData(buf, lenoffset, &offset, sizeof(GLintptr));
    writeData(buf, lenoffset, &size, sizeof(GLsizeiptr));
    if(size!=0){
        sendCommandDataWithHeader(buf, lenoffset, data, size);
    }
     else{
           sendCommand(buf, lenoffset);     
     }
  
}

GLenum glCheckFramebufferStatus(GLenum target)
{
	LOGV("GLES2 glBufferSubData command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	GLenum framebuffstatus;
	int offset = 0;
	createGLES2Command(GLCHECKFRAMEBUFFERSTATUS, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
    	writeData(buf, offset, &target, sizeof(GLenum));
	sendCommandSync(buf, offset, &framebuffstatus, sizeof(GLint));
        return framebuffstatus;
}
void glBindBuffer(GLenum target, GLuint buffer)
{
	LOGV("(%s)\n", __FUNCTION__);
    command_control cmd;
    egl_context_t* c = getContext();
	LOGV("target 0x%04x  buffer %d \n",target, buffer);
	switch(target){
		case GL_ARRAY_BUFFER: 
			c->arrayBuffer = buffer;
			LOGV("%d array buffer bound\n",buffer);
			break;
	}
    int offset = 0;
    createGLES2Command(GLBINDBUFFER, cmd);
    char buf[MAX_COMMAND_SIZE];
    writeData(buf, offset, &cmd, sizeof(command_control));
    writeData(buf, offset, &target, sizeof(GLenum));
    writeData(buf, offset, &buffer, sizeof(GLuint));
    sendCommand(buf, offset);
}
void glBindFramebuffer(GLenum target, GLuint framebuffer)
{
    command_control cmd;
    egl_context_t* c = getContext();
    int offset = 0;
    createGLES2Command(GLBINDFRAMEBUFFER, cmd);
    char buf[MAX_COMMAND_SIZE];
    writeData(buf, offset, &cmd, sizeof(command_control));
    writeData(buf, offset, &target, sizeof(GLenum));
    writeData(buf, offset, &framebuffer, sizeof(GLuint));
    sendCommand(buf, offset);
}
void glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
    command_control cmd;
    egl_context_t* c = getContext();
    int offset = 0;
    createGLES2Command(GLBINDRENDERBUFFER, cmd);
    char buf[MAX_COMMAND_SIZE];
    writeData(buf, offset, &cmd, sizeof(command_control));
    writeData(buf, offset, &target, sizeof(GLenum));
    writeData(buf, offset, &renderbuffer, sizeof(GLuint));
    sendCommand(buf, offset);
}

void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    command_control cmd;
    egl_context_t* c = getContext();
    int offset = 0;
    GLuint redval = (GLuint) red;
    GLuint greenval = (GLuint) green;
    GLuint blueval = (GLuint) blue;
    GLuint alphaval = (GLuint) alpha;
    createGLES2Command(GLCOLORMASK, cmd);
    char buf[MAX_COMMAND_SIZE];
    writeData(buf, offset, &cmd, sizeof(command_control));
    writeData(buf, offset, &redval, sizeof(GLuint));
    writeData(buf, offset, &greenval, sizeof(GLuint));
    writeData(buf, offset, &blueval, sizeof(GLuint));
    writeData(buf, offset, &alphaval, sizeof(GLuint));
    sendCommand(buf, offset);
}
void glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data)
{
    LOGV("GLES2 glCompressedTexImage2D command \n");
    command_control cmd;
    egl_context_t* c = getContext();
    int offset = 0;
    createGLES2Command(GLCOMPRESSEDTEXIMAGE2D, cmd);
    char buf[MAX_COMMAND_SIZE];
    writeData(buf, offset, &cmd, sizeof(command_control));
    writeData(buf, offset, &target, sizeof(GLenum));
    writeData(buf, offset, &level, sizeof(GLint));
    writeData(buf, offset, &internalformat, sizeof(GLenum));
    writeData(buf, offset, &width, sizeof(GLsizei));
    writeData(buf, offset, &height, sizeof(GLsizei));
    writeData(buf, offset, &border, sizeof(GLint));
    writeData(buf, offset, &imageSize, sizeof(GLsizei));
    if (imageSize>0) {
		sendCommandDataWithHeader(buf, offset, data, imageSize);
	} else {
		sendCommand(buf, offset);
       }
}

void glCompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data)
{
   LOGV("GLES2 glCompressedTexSubImage2D command \n");
    command_control cmd;
    egl_context_t* c = getContext();
    int offset = 0;
    createGLES2Command(GLCOMPRESSEDTEXSUBIMAGE2D, cmd);
    char buf[MAX_COMMAND_SIZE+imageSize];
    writeData(buf, offset, &cmd, sizeof(command_control));
    writeData(buf, offset, &target, sizeof(GLenum));
    writeData(buf, offset, &level, sizeof(GLint));
    writeData(buf, offset, &xoffset, sizeof(GLint));
    writeData(buf, offset, &yoffset, sizeof(GLint));
    writeData(buf, offset, &width, sizeof(GLsizei));
    writeData(buf, offset, &height, sizeof(GLsizei));
    writeData(buf, offset, &format, sizeof(GLenum));
    writeData(buf, offset, &imageSize, sizeof(GLsizei));
    if (imageSize>0) {
		sendCommandDataWithHeader(buf, offset, data, imageSize);
	} else {
		sendCommand(buf, offset);
       }
}

void glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    command_control cmd;
    egl_context_t* c = getContext();
    int offset = 0;
    int size=2*sizeof(GLenum)+2*sizeof(GLsizei)+4*sizeof(GLint);
    createGLES2Command(GLCOPYTEXIMAGE2D, cmd);
    char buf[MAX_COMMAND_SIZE];
    writeData(buf, offset, &cmd, sizeof(command_control));
    writeData(buf, offset, &target, sizeof(GLenum));
    writeData(buf, offset, &level, sizeof(GLint));
    writeData(buf, offset, &internalformat, sizeof(GLenum));
    writeData(buf, offset, &x, sizeof(GLint));
    writeData(buf, offset, &y, sizeof(GLint));
    writeData(buf, offset, &width, sizeof(GLsizei));
    writeData(buf, offset, &height, sizeof(GLsizei));
    writeData(buf, offset, &border, sizeof(GLenum));
    sendCommand(buf, offset);
}
void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    command_control cmd;
    egl_context_t* c = getContext();
    int offset = 0;
    int size=sizeof(GLenum)+2*sizeof(GLsizei)+5*sizeof(GLint);
    createGLES2Command(GLCOPYTEXSUBIMAGE2D, cmd);
    char buf[MAX_COMMAND_SIZE];
    writeData(buf, offset, &cmd, sizeof(command_control));
    writeData(buf, offset, &target, sizeof(GLenum));
    writeData(buf, offset, &level, sizeof(GLint));
    writeData(buf, offset, &xoffset, sizeof(GLint));
    writeData(buf, offset, &yoffset, sizeof(GLint));
    writeData(buf, offset, &x, sizeof(GLint));
    writeData(buf, offset, &y, sizeof(GLint));
    writeData(buf, offset, &width, sizeof(GLsizei));
    writeData(buf, offset, &height, sizeof(GLsizei));
    sendCommand(buf, offset);
}
void glDeleteBuffers(GLsizei n, const GLuint* buffers)
{
        command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLDELETEBUFFERS, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &n, sizeof(GLsizei));
	for(int i=0;i<n;i++)
	{
		writeData(buf, offset, (void*)&buffers[i], sizeof(GLuint));
	}

        sendCommand(buf, offset);
}
void glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers)
{
        command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLDELETEFRAMEBUFFERS, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &n, sizeof(GLsizei));
	for(int i=0;i<n;i++)
	{
		writeData(buf, offset, (void*)&framebuffers[i], sizeof(GLuint));
	}

        sendCommand(buf, offset);
}
void glDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers)
{
        command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLDELETERENDERBUFFERS, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &n, sizeof(GLsizei));
	for(int i=0;i<n;i++)
	{
		writeData(buf, offset, (void*)&renderbuffers[i], sizeof(GLuint));
	}

        sendCommand(buf, offset);
}
void glDepthFunc (GLenum func)
{
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLDEPTHFUNC, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &func, sizeof(GLenum));
	sendCommand(buf, offset);
}
void glDepthMask(GLboolean flag)
{
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	GLuint flagval = (GLuint) flag;
        createGLES2Command(GLDEPTHMASK, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &flagval, sizeof(GLuint));
	sendCommand(buf, offset);
}
void glDepthRangef(GLclampf zNear, GLclampf zFar)
{
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLDEPTHRANGEF, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &zNear, sizeof(GLclampf));
	writeData(buf, offset, &zFar, sizeof(GLclampf));
	sendCommand(buf, offset);
}
void glDetachShader(GLuint program, GLuint shader)
{
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLDETACHSHADER, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &program, sizeof(GLuint));
	writeData(buf, offset, &shader, sizeof(GLuint));
	sendCommand(buf, offset);
}
void glDisableVertexAttribArray(GLuint index)
{

	egl_context_t* c = getContext();
	c->attribs[index].enabled = false;
	command_control cmd;
	int offset = 0;
	createGLES2Command(GLDISABLEVERTEXATTRIBARRAY, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd,sizeof(command_control));
	writeData(buf, offset, &index,sizeof(GLuint));
	sendCommand(buf, offset);
}
void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLFRAMEBUFFERRENDERBUFFER, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &target, sizeof(GLenum));
	writeData(buf, offset, &attachment, sizeof(GLenum));
	writeData(buf, offset, &renderbuffertarget, sizeof(GLenum));
	writeData(buf, offset, &renderbuffer, sizeof(GLuint));
	sendCommand(buf, offset);
}

void glGetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
	LOGV("libvirtualGLES2 glGetShaderInfoLog command comes !!!\n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLGETSHADERINFOLOG,cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &shader, sizeof(GLuint));
	writeData(buf, offset, &bufsize, sizeof(GLsizei));
  	GLsizei len;	
	sendCommandSync(buf, offset, &len, sizeof(GLsizei));
	GetReturnValue(infolog,sizeof(GLsizei),len);
	if (length != NULL) {
		*length = len;
	}
}

void glGetProgramiv(GLuint program, GLenum pname, GLint* params)
{

	LOGV("libvirtualGLES2 glGetProgramiv command \n");
     	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLGETPROGRAMIV,cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &program, sizeof(GLuint));
       	writeData(buf, offset, &pname, sizeof(GLenum));
	sendCommandSync(buf, offset, params, sizeof(GLint));
}
void glGetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
	LOGV("libvirtualGLES2 glGetProgramInfoLog command \n");
       	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        int lenflag=(length==NULL)?false:true;
	createGLES2Command(GLGETPROGRAMINFOLOG,cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &program, sizeof(GLuint));
       	writeData(buf, offset, &bufsize, sizeof(GLenum));
 	writeData(buf, offset, &lenflag, sizeof(int));
        if(lenflag==true)
	{
           sendCommandSync(buf, offset, length, sizeof(GLsizei));
           GetReturnValue(infolog,sizeof(GLsizei),(*length)+1);
	}
	else
	{
	      sendCommandSync(buf, offset,infolog,bufsize);
        }
}

void glGenerateMipmap(GLenum target)
{
	LOGV("glGenerateMipmap command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLGENERATEMIPMAP, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &target, sizeof(GLenum));
	sendCommand(buf, offset);
}

void glGenBuffers(GLsizei n, GLuint* buffers)
{
	LOGV("glGenBuffers command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLGENBUFFERS, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &n, sizeof(GLsizei));
	sendCommandSync(buf, offset, buffers, sizeof(GLuint)*n);
}

void glGenFramebuffers(GLsizei n, GLuint* framebuffers)
{
	LOGV("glGenFramebuffers command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLGENFRAMEBUFFERS, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &n, sizeof(GLsizei));
	sendCommandSync(buf, offset, framebuffers, sizeof(GLuint)*n);
}
void glGenRenderbuffers(GLsizei n, GLuint* renderbuffers)
{
	LOGV("glGenRenderbuffers command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLGENRENDERBUFFERS, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &n, sizeof(GLsizei));
	sendCommandSync(buf, offset, renderbuffers, sizeof(GLuint)*n);
}
GLenum glGetError()
{
	LOGV("glGetError command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	GLenum Errorval;
	createGLES2Command(GLGETERROR, cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	sendCommandSync(buf, offset, &Errorval, sizeof(GLenum));

//	return Errorval;
	return GL_NO_ERROR;

}

void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
	LOGV("glGetActiveAttrib command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	int lenflag=(length==NULL)?false:true; //To check length field is NULL or not
       	createGLES2Command(GLGETACTIVEATTRIB,cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &program, sizeof(GLuint));
	writeData(buf, offset, &index, sizeof(GLuint));
	writeData(buf, offset, &bufsize, sizeof(GLsizei));
	writeData(buf, offset, &lenflag, sizeof(int));//pass the length to the host. Added extra field ..Host should take car
	if(lenflag==true)
	{
           sendCommandSync(buf, offset, length, sizeof(GLsizei));
           GetReturnValue((char*)size,sizeof(GLsizei),sizeof(GLint));
           GetReturnValue((char*)type,sizeof(GLsizei)+sizeof(GLint),sizeof(GLenum));
           GetReturnValue(name,sizeof(GLsizei)+sizeof(GLint)+sizeof(GLenum),(*length)+1);
	}
	else
	{
           sendCommandSync(buf, offset,size,sizeof(GLint));
   	   GetReturnValue((char*)type,sizeof(GLint),sizeof(GLenum));
           GetReturnValue(name,sizeof(GLint)+sizeof(GLenum),bufsize);
        }
}
void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
	LOGV("glGetActiveUniform command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	int lenflag=(length==NULL)?false:true; //To check length field is NULL or not
       	createGLES2Command(GLGETACTIVEUNIFORM,cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &program, sizeof(GLuint));
	writeData(buf, offset, &index, sizeof(GLuint));
	writeData(buf, offset, &bufsize, sizeof(GLsizei));
	writeData(buf, offset, &lenflag, sizeof(int));//pass the length to the host. Added extra field ..Host should take care
	if(lenflag==true)
	{
           sendCommandSync(buf, offset, length, sizeof(GLsizei));
           GetReturnValue((char*)size,sizeof(GLsizei),sizeof(GLint));
           GetReturnValue((char*)type,sizeof(GLsizei)+sizeof(GLint),sizeof(GLenum));
           GetReturnValue(name,sizeof(GLsizei)+sizeof(GLint)+sizeof(GLenum),(*length)+1);
	}
	else
	{
           sendCommandSync(buf, offset,size,sizeof(GLint));
   	   GetReturnValue((char*)type,sizeof(GLint),sizeof(GLenum));
           GetReturnValue(name,sizeof(GLint)+sizeof(GLenum),bufsize);
        }
}
void glGetAttachedShaders(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders)
{

	LOGV("glGetAttachedShaders command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLGETATTACHEDSHADERS,cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &program, sizeof(GLuint));
       	writeData(buf, offset, &maxcount, sizeof(GLsizei));
        sendCommandSync(buf, offset, count, sizeof(GLsizei));
        if(*count!=0)
	{
           GetReturnValue((char*)shaders,sizeof(GLsizei),*count*sizeof(GLuint));
	}
}

void glGetBooleanv(GLenum pname, GLboolean* params)
{
	LOGV("glGetBooleanv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        int paramscount;
	createGLES2Command(GLGETBOOLEANV,cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
       	writeData(buf, offset, &pname, sizeof(GLenum));
	sendCommandSync(buf, offset, &paramscount, sizeof(int));//pass paramscount to find how many return params available.
	if(paramscount>0)
	{
           GetReturnValue((char*)params,sizeof(int),paramscount*sizeof(GLboolean));//copy the return value or values
	}
}

void glGetBufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
	LOGV("glGetBufferParameteriv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        int paramscount;
	createGLES2Command(GLGETBUFFERPARAMETERIV,cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
       	writeData(buf, offset, &pname, sizeof(GLenum));
	sendCommandSync(buf, offset, &paramscount, sizeof(int));//pass paramscount to find how many return params available.
	if(paramscount>0)
	{
           GetReturnValue((char*)params,sizeof(int),paramscount*sizeof(GLint));//copy the return value or values
	}
}


void glGetFloatv(GLenum pname, GLfloat* params)
{
	LOGV("glGetFloatv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        int paramscount;
	createGLES2Command(GLGETFLOATV,cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
       	writeData(buf, offset, &pname, sizeof(GLenum));
	sendCommandSync(buf, offset, &paramscount, sizeof(int));//pass paramscount to find how many return params available.
	if(paramscount>0)
	{
           GetReturnValue((char*)params,sizeof(int),paramscount*sizeof(GLfloat));//copy the return value or values
	}
}
void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
	LOGV("glGetFramebufferAttachmentParameteriv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLGETFRAMEBUFFERATTACHMENTPARAMETERIV,cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
       	writeData(buf, offset, &target, sizeof(GLenum));
	writeData(buf, offset, &attachment, sizeof(GLenum));
	writeData(buf, offset, &pname, sizeof(GLenum));
	sendCommandSync(buf, offset, params, sizeof(GLint));
}

void glGetIntegerv(GLenum pname, GLint* params)
{
	LOGV("glGetIntegerv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        int paramscount;
	createGLES2Command(GLGETINTEGERRV,cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
       	writeData(buf, offset, &pname, sizeof(GLenum));
	sendCommandSync(buf, offset, &paramscount, sizeof(int));//pass paramscount to find how many return params available.
	if(paramscount>0)
	{
           GetReturnValue((char*)params,sizeof(int),paramscount*sizeof(GLint));//copy the return value or values
	}
}

void glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
	LOGV("glGetRenderbufferParameteriv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLGETRENDERBUFFERPARAMETERIV,cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
       	writeData(buf, offset, &target, sizeof(GLenum));
	writeData(buf, offset, &pname, sizeof(GLenum));
	sendCommandSync(buf, offset, params, sizeof(GLint));
}

void glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
	LOGV("glGetShaderPrecisionFormat command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	createGLES2Command(GLGETSHADERPRECISIONFORMAT,cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
       	writeData(buf, offset, &shadertype, sizeof(GLenum));
	writeData(buf, offset, &precisiontype, sizeof(GLenum));
	sendCommandSync(buf, offset, range, 2*sizeof(GLint));
	GetReturnValue((char*)precision,2*sizeof(GLint),sizeof(GLint));
}
void glGetShaderSource(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source)
{
	LOGV("glGetShaderSource command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	int lenflag=(length==NULL)?false:true;
        createGLES2Command(GLGETSHADERSOURCE,cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &shader, sizeof(GLuint));
	writeData(buf, offset, &bufsize, sizeof(GLuint));
	writeData(buf, offset, &lenflag, sizeof(int));
	if(lenflag==true)
	{
           sendCommandSync(buf, offset, length, sizeof(GLsizei));
           GetReturnValue(source,sizeof(GLsizei),(*length)+1);
	}
	else
	{
	  sendCommandSync(buf, offset, source, bufsize);
        }
}

void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params)
{
	return;
	LOGV("glGetTexParameterfv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        int paramscount;
	createGLES2Command(GLGETTEXPARAMETERFV,cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &target, sizeof(GLenum));
       	writeData(buf, offset, &pname, sizeof(GLenum));
	sendCommandSync(buf, offset, &paramscount, sizeof(int));//pass paramscount to find how many return params available.
	if(paramscount>0)
	{
           GetReturnValue((char*)params,sizeof(int),paramscount*sizeof(GLfloat));//copy the return value or values
	}
}

void glGetTexParameteriv(GLenum target, GLenum pname, GLint* params)
{
	LOGV("glGetTexParameteriv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        int paramscount;
	createGLES2Command(GLGETTEXPARAMETERIV,cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &target, sizeof(GLenum));
       	writeData(buf, offset, &pname, sizeof(GLenum));
	sendCommandSync(buf, offset, &paramscount, sizeof(int));//pass paramscount to find how many return params available.
	if(paramscount>0)
	{
           GetReturnValue((char*)params,sizeof(int),paramscount*sizeof(GLint));//copy the return value or values
	}
}

void glGetUniformfv(GLuint program, GLint location, GLfloat* params)
{
	LOGV("glGetUniformfv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        int paramscount;
	createGLES2Command(GLGETUNIFORMFV,cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &program, sizeof(GLuint));
       	writeData(buf, offset, &location, sizeof(GLint));
	sendCommandSync(buf, offset, &paramscount, sizeof(int));//pass paramscount to find how many return params available.
	if(paramscount>0)
	{
           GetReturnValue((char*)params,sizeof(int),paramscount*sizeof(GLfloat));//copy the return value or values
	}
}
void glGetUniformiv(GLuint program, GLint location, GLint* params)
{
	LOGV("glGetUniformiv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        int paramscount;
	createGLES2Command(GLGETUNIFORMIV,cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &program, sizeof(GLuint));
       	writeData(buf, offset, &location, sizeof(GLint));
	sendCommandSync(buf, offset, &paramscount, sizeof(int));//pass paramscount to find how many return params available.
	if(paramscount>0)
	{
           GetReturnValue((char*)params,sizeof(int),paramscount*sizeof(GLint));//copy the return value or values
	}
}

void glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params)
{
	LOGV("glGetVertexAttribfv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        int paramscount;
        createGLES2Command(GLGETVERTEXATTRIBFV,cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &index, sizeof(GLuint));
       	writeData(buf, offset, &pname, sizeof(GLenum));
	sendCommandSync(buf, offset, &paramscount, sizeof(int));//pass paramscount to find how many return params available.
	if(paramscount>0)
	{
           GetReturnValue((char*)params,sizeof(int),paramscount*sizeof(GLfloat));//copy the return value or values
	}
}
void glGetVertexAttribiv(GLuint index, GLenum pname, GLint* params)
{
	LOGV("glGetVertexAttribiv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        int paramscount;
       	createGLES2Command(GLGETVERTEXATTRIBIV,cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &index, sizeof(GLuint));
       	writeData(buf, offset, &pname, sizeof(GLenum));
	sendCommandSync(buf, offset, &paramscount, sizeof(int));//pass paramscount to find how many return params available.
	if(paramscount>0)
	{
           GetReturnValue((char*)params,sizeof(int),paramscount*sizeof(GLint));//copy the return value or values
	}
}
void glGetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid** pointer)
{
 	 LOGV("glGetVertexAttribPointerv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLGETVERTEXATTRIBPOINTERRV,cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &index, sizeof(GLuint));
       	writeData(buf, offset, &pname, sizeof(GLenum));
	sendCommandSync(buf, offset, *pointer, sizeof(int));//pass paramscount to find how many return params available.
}

GLboolean glIsBuffer(GLuint buffer)
{
	LOGV("glIsBuffer command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        GLboolean retval;
        createGLES2Command(GLISBUFFER, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &buffer, sizeof(GLuint));
	sendCommandSync(buf, offset, &retval, sizeof(GLboolean));
        return retval;
}
GLboolean glIsEnabled(GLenum cap)
{
        LOGV("glIsEnabled command \n");
    	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	GLboolean retval;
        createGLES2Command(GLISENABLED, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &cap, sizeof(GLenum));
	sendCommandSync(buf, offset, &retval, sizeof(GLboolean));
        return retval;
}

GLboolean glIsFramebuffer(GLuint framebuffer)
{
        LOGV("glIsFramebuffer command \n");
        command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        GLboolean retval;
        createGLES2Command(GLISFRAMEBUFFER, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &framebuffer, sizeof(GLuint));
	sendCommandSync(buf, offset, &retval, sizeof(GLboolean));
        return retval;

}
GLboolean glIsProgram(GLuint program)
{
        LOGV("glIsProgram command \n");
        command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        GLboolean retval;
        createGLES2Command(GLISPROGRAM, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &program, sizeof(GLuint));
	sendCommandSync(buf, offset, &retval, sizeof(GLboolean));
        return retval;

}
GLboolean glIsRenderbuffer(GLuint renderbuffer)
{
        LOGV("glIsRenderbuffer command \n");
        command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        GLboolean retval;
        createGLES2Command(GLISRENDERBUFFER, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &renderbuffer, sizeof(GLuint));
	sendCommandSync(buf, offset, &retval, sizeof(GLboolean));
        return retval;
}
GLboolean glIsShader(GLuint shader)
{
        LOGV("glIsShader command \n");
        command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	GLboolean retval;
        createGLES2Command(GLISSHADER, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &shader, sizeof(GLuint));
	sendCommandSync(buf, offset, &retval, sizeof(GLboolean));
        return retval;
}

void glLineWidth(GLfloat width)
{
        LOGV("glLineWidth command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLLINEWIDTH, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &width, sizeof(GLfloat));
	sendCommand(buf, offset);
}

void glPixelStorei(GLenum pname, GLint param)
{
       LOGV("glPixelStorei command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLPIXELSTOREI, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &pname, sizeof(GLenum));
	writeData(buf, offset, &param, sizeof(GLint));
	sendCommand(buf, offset);
}
void glPolygonOffset(GLfloat factor, GLfloat units)
{
        LOGV("glPolygonOffset command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLPOLYGONOFFSET, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &factor, sizeof(GLfloat));
	writeData(buf, offset, &units, sizeof(GLfloat));
	sendCommand(buf, offset);

}

void glReleaseShaderCompiler(void)
{
        LOGV("glReleaseShaderCompiler command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLRELEASESHADERCOMPILER, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	sendCommand(buf, offset);
}
void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
        LOGV("glRenderbufferStorage command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLRENDERBUFFERSTORAGE, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &target, sizeof(GLenum));
	writeData(buf, offset, &internalformat, sizeof(GLenum));
	writeData(buf, offset, &width, sizeof(GLsizei));
	writeData(buf, offset, &height, sizeof(GLsizei));
	sendCommand(buf, offset);
}

void glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
        LOGV("glScissor command \n");
 	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLSCISSOR, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &x, sizeof(GLint));
	writeData(buf, offset, &y, sizeof(GLint));
	writeData(buf, offset, &width, sizeof(GLsizei));
	writeData(buf, offset, &height, sizeof(GLsizei));
	sendCommand(buf, offset);
}
void glShaderBinary(GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length)
{
       LOGV("glShaderBinary command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        GLuint shaderlen=n*sizeof(GLuint);
        createGLES2Command(GLSHADERBINARY, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &n, sizeof(GLsizei));
	writeData(buf, offset, &shaderlen, sizeof(GLuint));
	writeData(buf, offset, (void*)shaders, shaderlen);
	writeData(buf, offset, &binaryformat, sizeof(GLenum));
	writeData(buf, offset, &length, sizeof(GLsizei)); //length should be passed before the data.
	writeData(buf, offset, (void*)binary, length);
	offset=pad32bit(offset);
	sendCommand(buf, offset);
}

void glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
       LOGV("glStencilFuncSeparate command \n");
 	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLSTENCILFUNCSEPARATE, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &face, sizeof(GLenum));
	writeData(buf, offset, &func, sizeof(GLenum));
	writeData(buf, offset, &ref, sizeof(GLint));
	writeData(buf, offset, &mask, sizeof(GLuint));
	sendCommand(buf, offset);
}
void glStencilMask(GLuint mask)
{
        LOGV("glStencilMask command \n");
 	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLSTENCILMASK, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &mask, sizeof(GLint));
	sendCommand(buf, offset);
}
void glStencilMaskSeparate(GLenum face, GLuint mask)
{
          LOGV("glStencilMaskSeparate command \n");
 	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLSTENCILMASKSEPARATE, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &face, sizeof(GLenum));
	writeData(buf, offset, &mask, sizeof(GLuint));
	sendCommand(buf, offset);
}
void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
        LOGV("glStencilOp command \n");
 	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLSTENCILOP, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &fail, sizeof(GLenum));
	writeData(buf, offset, &zfail, sizeof(GLenum));
	writeData(buf, offset, &zpass, sizeof(GLenum));
	sendCommand(buf, offset);
}
void glStencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass)
{
        LOGV("glStencilOpSeparate command \n");
 	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLSTENCILOPSEPARATE, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &face, sizeof(GLenum));
	writeData(buf, offset, &fail, sizeof(GLenum));
	writeData(buf, offset, &zfail, sizeof(GLenum));
	writeData(buf, offset, &zpass, sizeof(GLenum));
	sendCommand(buf, offset);
}

void glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
        LOGV("glTexParameterf command \n");
 	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLTEXPARAMETERF, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &target, sizeof(GLenum));
	writeData(buf, offset, &pname, sizeof(GLenum));
	writeData(buf, offset, &param, sizeof(GLfloat));
	sendCommand(buf, offset);
}

void glTexParameteri(GLenum target, GLenum pname, GLint param)
{
        LOGV("glTexParameteri command \n");
 	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLTEXPARAMETERI, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &target, sizeof(GLenum));
	writeData(buf, offset, &pname, sizeof(GLenum));
	writeData(buf, offset, &param, sizeof(GLint));
	sendCommand(buf, offset);
}
void glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
 	LOGV("glTexImage2D command \n");
 	command_control cmd;
	int offset = 0;
	int size = 0;
	if (pixels != NULL) {
 		size = width*height*getDataSize(type)*getDataFormatSize(format);
	}
	createGLES2Command(GLTEXIMAGE2D,cmd);
	char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeUint32(buf, offset, target);
	writeUint32(buf, offset, level);
	writeUint32(buf, offset, internalformat);
	writeUint32(buf, offset, width);
	writeUint32(buf, offset, height);
	writeUint32(buf, offset, border);
	writeUint32(buf, offset, format);
	writeUint32(buf, offset, type);
	writeUint32(buf, offset, size); // write the size of the data
	if (size>0) {
		sendCommandDataWithHeader(buf, offset, pixels, size);
	} else {
		sendCommand(buf, offset);
	}
}

void glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params)
{
	LOGV("glTexParameterfv command \n");
 	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLTEXPARAMETERFV, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &target, sizeof(GLenum));
	writeData(buf, offset, &pname, sizeof(GLenum));
	writeData(buf, offset, params, sizeof(GLfloat));
	sendCommand(buf, offset);
}
void glTexParameteriv(GLenum target, GLenum pname, const GLint* params)
{
	LOGV("glTexParameteriv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLTEXPARAMETERIV, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &target, sizeof(GLenum));
	writeData(buf, offset, &pname, sizeof(GLenum));
	writeData(buf, offset, params, sizeof(GLint));
	sendCommand(buf, offset);
}
void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
	LOGV("glTexSubImage2D command \n");
 	command_control cmd;
	int offset = 0;
	int size = 0;
	if (pixels != NULL) {
 		size = width*height*getDataSize(type)*getDataFormatSize(format);
	}
        createGLES2Command(GLTEXSUBIMAGE2D,cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeUint32(buf, offset, target);
	writeUint32(buf, offset, level);
	writeUint32(buf, offset, xoffset);
	writeUint32(buf, offset, yoffset);
	writeUint32(buf, offset, width);
	writeUint32(buf, offset, height);
	writeUint32(buf, offset, format);
	writeUint32(buf, offset, type);
	writeUint32(buf, offset, size); // write the size of the data
	if (size>0) {
		sendCommandDataWithHeader(buf, offset, pixels, size);
	} else {
		sendCommand(buf, offset);
	}
}

void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels)
{
	LOGV("glReadPixels command \n");
        command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	int size=width*height*getDataSize(type)*getDataFormatSize(format);;
        createGLES2Command(GLREADPIXELS,cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &x, sizeof(GLint));
	writeData(buf, offset, &y, sizeof(GLint));
	writeData(buf, offset, &width, sizeof(GLsizei));
	writeData(buf, offset, &height, sizeof(GLsizei));
	writeData(buf, offset, &format, sizeof(GLenum));
	writeData(buf, offset, &type, sizeof(GLenum));
	sendCommandSync(buf, offset, pixels,size);

}

void glUniform1f(GLint location, GLfloat x)
{
        LOGV("glUniform1f command \n");
 	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLUNIFORM1F, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &location, sizeof(GLint));
	writeData(buf, offset, &x, sizeof(GLfloat));
	sendCommand(buf, offset);
}

void glUniform1fv(GLint location, GLsizei count, const GLfloat* v)
{
        LOGV("glUniform1fv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	int size=sizeof(GLsizei)+sizeof(GLint)+count*sizeof(GLfloat);
        createGLES2Command(GLUNIFORM1FV, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &location, sizeof(GLint));
	writeData(buf, offset, &count, sizeof(GLsizei));
	writeData(buf, offset, (void*)v, count*sizeof(GLfloat));
	sendCommand(buf, offset);
}

void glUniform1i(GLint location, GLint x)
{
        LOGV("glUniform1i command \n");
 	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLUNIFORM1I, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &location, sizeof(GLint));
	writeData(buf, offset, &x, sizeof(GLint));
	sendCommand(buf, offset);
}
void glUniform1iv(GLint location, GLsizei count, const GLint* v)
{
        LOGV("glUniform1iv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	int size=sizeof(GLsizei)+sizeof(GLint)+count*sizeof(GLint);
        createGLES2Command(GLUNIFORM1IV, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &location, sizeof(GLint));
	writeData(buf, offset, &count, sizeof(GLsizei));
	writeData(buf, offset, (void*)v, count*sizeof(GLint));
	sendCommand(buf, offset);
}
void glUniform2f(GLint location, GLfloat x, GLfloat y)
{
        LOGV("glUniform2f command \n");
 	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLUNIFORM2F, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &location, sizeof(GLint));
	writeData(buf, offset, &x, sizeof(GLfloat));
	writeData(buf, offset, &y, sizeof(GLfloat));
	sendCommand(buf, offset);
}
void glUniform2fv(GLint location, GLsizei count, const GLfloat* v)
 {
        LOGV("glUniform2fv command \n");
 	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	int size=sizeof(GLsizei)+sizeof(GLint)+count*2*sizeof(GLfloat);
        createGLES2Command(GLUNIFORM2FV, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &location, sizeof(GLint));
	writeData(buf, offset, &count, sizeof(GLsizei));
	for(int i=0;i<count;i++)
		{
			writeData(buf, offset, (void*)&v[i*2], 2*sizeof(GLfloat));
		}
	sendCommand(buf, offset);
}

void glUniform2i(GLint location, GLint x, GLint y)
{
        LOGV("glUniform2i command \n");
 	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLUNIFORM2I, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &location, sizeof(GLint));
	writeData(buf, offset, &x, sizeof(GLint));
	writeData(buf, offset, &y, sizeof(GLint));
	sendCommand(buf, offset);
}
void glUniform2iv(GLint location, GLsizei count, const GLint* v)
{
        LOGV("glUniform2iv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	int size=sizeof(GLsizei)+sizeof(GLint)+count*2*sizeof(GLint);
        createGLES2Command(GLUNIFORM2IV, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &location, sizeof(GLint));
	writeData(buf, offset, &count, sizeof(GLsizei));
	for(int i=0;i<count;i++)
	{
		writeData(buf, offset, (void*)&v[i*2], 2*sizeof(GLint));
	}
	sendCommand(buf, offset);
}
void glUniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z)
{
        LOGV("glUniform3f command \n");
 	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLUNIFORM3F, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &location, sizeof(GLint));
	writeData(buf, offset, &x, sizeof(GLfloat));
	writeData(buf, offset, &y, sizeof(GLfloat));
	writeData(buf, offset, &z, sizeof(GLfloat));
	sendCommand(buf, offset);
}

void glUniform3i(GLint location, GLint x, GLint y, GLint z)
{
        LOGV("glUniform3i command \n");
 	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLUNIFORM3I, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &location, sizeof(GLint));
	writeData(buf, offset, &x, sizeof(GLint));
	writeData(buf, offset, &y, sizeof(GLint));
	writeData(buf, offset, &z, sizeof(GLint));
	sendCommand(buf, offset);
}
void glUniform3iv(GLint location, GLsizei count, const GLint* v)
{
        LOGV("glUniform3iv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	int size=sizeof(GLsizei)+sizeof(GLint)+3*count*sizeof(GLint);
        createGLES2Command(GLUNIFORM3IV, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &location, sizeof(GLint));
	writeData(buf, offset, &count, sizeof(GLsizei));
	for(int i=0;i<count;i++)
	{
		writeData(buf, offset, (void*)&v[i*3], 3*sizeof(GLint));
	}
	sendCommand(buf, offset);
}
void glUniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
        LOGV("glUniform4f command \n");
 	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLUNIFORM4F, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &location, sizeof(GLint));
	writeData(buf, offset, &x, sizeof(GLfloat));
	writeData(buf, offset, &y, sizeof(GLfloat));
	writeData(buf, offset, &z, sizeof(GLfloat));
	writeData(buf, offset, &w, sizeof(GLfloat));
	sendCommand(buf, offset);
}
void glUniform4fv(GLint location, GLsizei count, const GLfloat* v)
{
        LOGV("glUniform4fv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	int size=sizeof(GLsizei)+sizeof(GLint)+4*count*sizeof(GLint);
        createGLES2Command(GLUNIFORM4FV, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &location, sizeof(GLint));
	writeData(buf, offset, &count, sizeof(GLsizei));
	for(int i=0;i<count;i++)
	{
		writeData(buf, offset, (void*)&v[i*4], 4*sizeof(GLfloat));
	}
	sendCommand(buf, offset);
}
void glUniform4i(GLint location, GLint x, GLint y, GLint z, GLint w)
{
        LOGV("glUniform4i command \n");
 	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLUNIFORM4I, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &location, sizeof(GLint));
	writeData(buf, offset, &x, sizeof(GLint));
	writeData(buf, offset, &y, sizeof(GLint));
	writeData(buf, offset, &z, sizeof(GLint));
	writeData(buf, offset, &w, sizeof(GLint));
	sendCommand(buf, offset);
}
void glUniform4iv(GLint location, GLsizei count, const GLint* v)
{
        LOGV("glUniform4iv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	int size=sizeof(GLsizei)+sizeof(GLint)+4*count*sizeof(GLint);
        createGLES2Command(GLUNIFORM4IV, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &location, sizeof(GLint));
	writeData(buf, offset, &count, sizeof(GLsizei));
	for(int i=0;i<count;i++)
	{
		writeData(buf, offset, (void*)&v[i*4], 4*sizeof(GLint));
	}
	sendCommand(buf, offset);
}
void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
        LOGV("glUniformMatrix2fv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	GLuint transVal = (GLuint) transpose; //GLboolean is byte size, which is incovenient for the parsing which assumes 32 bit word alignment
	int size=sizeof(GLsizei)+2*sizeof(GLint)+4*count*sizeof(GLfloat);
        createGLES2Command(GLUNIFORMMATRIX2FV, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &location, sizeof(GLint));
	writeData(buf, offset, &count, sizeof(GLsizei));
        writeData(buf, offset, &transVal, sizeof(GLuint));
	for(int i=0;i<count;i++)
	{
		writeData(buf, offset, (void*)&value[i*4], 4*sizeof(GLfloat));
	}

	sendCommand(buf, offset);
}
void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
        LOGV("glUniformMatrix3fv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
	GLuint transVal = (GLuint) transpose;
	int size=sizeof(GLsizei)+2*sizeof(GLint)+9*count*sizeof(GLint);
        createGLES2Command(GLUNIFORMMATRIX3FV, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &location, sizeof(GLint));
	writeData(buf, offset, &count, sizeof(GLsizei));
	writeData(buf, offset, &transVal, sizeof(GLuint));
	for(int i=0;i<count;i++)
	{
		writeData(buf, offset, (void*)&value[i*9], 9*sizeof(GLfloat));
	}
	sendCommand(buf, offset);
}


void glValidateProgram(GLuint program)
{
        LOGV("glValidateProgram command \n");
    	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLVALIDATEPROGRAM, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &program, sizeof(GLint));
	sendCommand(buf, offset);
}
void glVertexAttrib1f(GLuint indx, GLfloat x)
{
        LOGV("glVertexAttrib1f command \n");
    	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLVERTEXATTRIB1F, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &indx, sizeof(GLuint));
	writeData(buf, offset, &x, sizeof(GLfloat));
	sendCommand(buf, offset);
}
void glVertexAttrib1fv(GLuint indx, const GLfloat* values)
{
        LOGV("glVertexAttrib1fv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLVERTEXATTRIB1FV, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &indx, sizeof(GLuint));
	writeData(buf, offset, (void*)values, sizeof(GLfloat));
	sendCommand(buf, offset);
}

void glVertexAttrib2f(GLuint indx, GLfloat x, GLfloat y)
{
        LOGV("glVertexAttrib2f command \n");
    	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLVERTEXATTRIB2F, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &indx, sizeof(GLuint));
	writeData(buf, offset, &x, sizeof(GLfloat));
	writeData(buf, offset, &y, sizeof(GLfloat));
	sendCommand(buf, offset);
}
void glVertexAttrib2fv(GLuint indx, const GLfloat* values)
{
        LOGV("glVertexAttrib2fv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLVERTEXATTRIB2FV, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &indx, sizeof(GLuint));
	writeData(buf, offset, (void*)values, 2*sizeof(GLfloat));
	sendCommand(buf, offset);
}
void glVertexAttrib3f(GLuint indx, GLfloat x, GLfloat y, GLfloat z)
{
        LOGV("glVertexAttrib3f command \n");
    	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLVERTEXATTRIB3F, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &indx, sizeof(GLuint));
	writeData(buf, offset, &x, sizeof(GLfloat));
	writeData(buf, offset, &y, sizeof(GLfloat));
	writeData(buf, offset, &z, sizeof(GLfloat));
	sendCommand(buf, offset);
}
void glVertexAttrib3fv(GLuint indx, const GLfloat* values)
{
       LOGV("glVertexAttrib3fv command \n");
	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLVERTEXATTRIB3FV, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &indx, sizeof(GLuint));
	writeData(buf, offset, (void*)values, 3*sizeof(GLfloat));
	sendCommand(buf, offset);

}
void glVertexAttrib4f(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
       LOGV("glVertexAttrib4f command \n");
    	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLVERTEXATTRIB4F, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &indx, sizeof(GLuint));
	writeData(buf, offset, &x, sizeof(GLfloat));
	writeData(buf, offset, &y, sizeof(GLfloat));
	writeData(buf, offset, &z, sizeof(GLfloat));
	writeData(buf, offset, &w, sizeof(GLfloat));
	sendCommand(buf, offset);
}

void glVertexAttrib4fv(GLuint indx, const GLfloat* values)
{
	LOGV("glVertexAttrib4fv command \n");
 	command_control cmd;
	egl_context_t* c = getContext();
	int offset = 0;
        createGLES2Command(GLVERTEXATTRIB4FV, cmd);
        char buf[MAX_COMMAND_SIZE];
	writeData(buf, offset, &cmd, sizeof(command_control));
	writeData(buf, offset, &indx, sizeof(GLuint));
	writeData(buf, offset, (void*)values, 4*sizeof(GLfloat));
	sendCommand(buf, offset);
}


