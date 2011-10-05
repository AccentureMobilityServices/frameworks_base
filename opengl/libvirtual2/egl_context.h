/* libs/opengles/context.h
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
#ifndef EGL_CONTEXT_H
#define EGL_CONTEXT_H

#include <private/opengles/gl_context.h>
#include "AttribPointer.h"
using namespace android;
using namespace android::gl;

struct egl_context_t {
    enum {
        IS_CURRENT      =   0x00010000,
        NEVER_CURRENT   =   0x00020000
    };
    uint32_t            flags;
    EGLDisplay          dpy;
    EGLConfig           config;
    EGLSurface          read;
    EGLSurface          draw;

	/* ++ libvirtual2 additions ++ */
	int token;
	AttribPointer* attribs;
	FILE *theVirtualDeviceFileDescriptor;
	FILE *theVirtualDeviceIOCTLFileDescriptor;
	int theVirtualDeviceDescriptor;
	int theVirtualDeviceIOCTLDescriptor;
	unsigned int surfaceCounter;
	unsigned int seq;

	int nextToken() {
		token += 1;
		return token;
	}
	int nextSequence() {
		seq += 1;
		return seq;
	}

    static inline egl_context_t* context(EGLContext ctx) {
        ogles_context_t* const gl = static_cast<ogles_context_t*>(ctx);
        return context(gl);
    }
    static inline egl_context_t* context(ogles_context_t* gl) {
        return static_cast<egl_context_t*>(gl->rasterizer.base);
    }
};
#endif
