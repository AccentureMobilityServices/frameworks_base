LOCAL_PATH:= $(call my-dir)

#
# Build the Libvirtual2 library, this library allows OpenGLES2 commands to be executed on QEMU emulator via host access
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	egl.cpp                     \
	commands.cpp				\
	../libagl/state.cpp		            \
	../libagl/texture.cpp		            \
    ../libagl/Tokenizer.cpp               \
    ../libagl/TokenManager.cpp            \
    ../libagl/TextureObjectManager.cpp    \
    ../libagl/BufferObjectManager.cpp     \
	../libagl/array.cpp.arm		        \
	../libagl/fp.cpp.arm		            \
	../libagl/light.cpp.arm		        \
	../libagl/matrix.cpp.arm		        \
	../libagl/mipmap.cpp.arm		        \
	../libagl/primitives.cpp.arm	        \
	../libagl/vertex.cpp.arm

LOCAL_CFLAGS += -DLOG_TAG=\"libvirtual2\" 
LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES
LOCAL_CFLAGS += -fvisibility=hidden
LOCAL_CFLAGS += -I$(LOCAL_PATH)/../libagl/

LOCAL_SHARED_LIBRARIES := libcutils libhardware libutils libpixelflinger libETC1
LOCAL_LDLIBS := -lpthread -ldl

ifeq ($(TARGET_ARCH),arm)
	LOCAL_SRC_FILES += ../libagl/fixed_asm.S ../libagl/iterators.S
	LOCAL_CFLAGS += -fstrict-aliasing
endif

ifeq ($(ARCH_ARM_HAVE_TLS_REGISTER),true)
    LOCAL_CFLAGS += -DHAVE_ARM_TLS_REGISTER
endif

ifneq ($(TARGET_SIMULATOR),true)
    # we need to access the private Bionic header <bionic_tls.h>
    # on ARM platforms, we need to mirror the ARCH_ARM_HAVE_TLS_REGISTER
    # behavior from the bionic Android.mk file
    ifeq ($(TARGET_ARCH)-$(ARCH_ARM_HAVE_TLS_REGISTER),arm-true)
        LOCAL_CFLAGS += -DHAVE_ARM_TLS_REGISTER
    endif
    LOCAL_C_INCLUDES += bionic/libc/private
endif

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/egl
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_C_INCLUDES += vendor/accenture/opengles2emulator/include
LOCAL_MODULE:= libEGL_virtual2

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	gl2.cpp		            \
	crc32.c				\
	commands.cpp		\

LOCAL_CFLAGS += -DLOG_TAG=\"libvirtual2\"
LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES
LOCAL_CFLAGS += -fvisibility=hidden

LOCAL_SHARED_LIBRARIES := libcutils libhardware libutils libpixelflinger libETC1
LOCAL_LDLIBS := -lpthread -ldl

ifeq ($(TARGET_ARCH),arm)
	LOCAL_SRC_FILES += ../libagl/fixed_asm.S ../libagl/iterators.S
	LOCAL_CFLAGS += -fstrict-aliasing
endif

ifeq ($(ARCH_ARM_HAVE_TLS_REGISTER),true)
    LOCAL_CFLAGS += -DHAVE_ARM_TLS_REGISTER
endif

ifneq ($(TARGET_SIMULATOR),true)
    # we need to access the private Bionic header <bionic_tls.h>
    # on ARM platforms, we need to mirror the ARCH_ARM_HAVE_TLS_REGISTER
    # behavior from the bionic Android.mk file
    ifeq ($(TARGET_ARCH)-$(ARCH_ARM_HAVE_TLS_REGISTER),arm-true)
        LOCAL_CFLAGS += -DHAVE_ARM_TLS_REGISTER
    endif
    LOCAL_C_INCLUDES += bionic/libc/private
endif


LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/egl
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_C_INCLUDES += vendor/accenture/opengles2emulator/include
LOCAL_MODULE:= libGLESv2_virtual2

include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := egl.cfg
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib/egl
LOCAL_SRC_FILES := egl.cfg 
include $(BUILD_PREBUILT)

# make sure we depend on egl.cfg, so it gets installed
$(installed_libEGL): | egl.cfg

# include the src files from the libagl software implementation
include $(CLEAR_VARS)
LOCAL_PATH := $(LOCAL_PATH)/../libagl
LOCAL_SRC_FILES:= \
	state.cpp		            \
	texture.cpp		            \
    Tokenizer.cpp               \
    TokenManager.cpp            \
    TextureObjectManager.cpp    \
    BufferObjectManager.cpp     \
	array.cpp.arm		        \
	fp.cpp.arm		            \
	light.cpp.arm		        \
	matrix.cpp.arm		        \
	mipmap.cpp.arm		        \
	primitives.cpp.arm	        \
	vertex.cpp.arm

LOCAL_CFLAGS += -DLOG_TAG=\"libagl\"
LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES
LOCAL_CFLAGS += -fvisibility=hidden

LOCAL_SHARED_LIBRARIES := libcutils libhardware libutils libpixelflinger libETC1
LOCAL_LDLIBS := -lpthread -ldl

ifeq ($(TARGET_ARCH),arm)
	LOCAL_SRC_FILES += fixed_asm.S iterators.S
	LOCAL_CFLAGS += -fstrict-aliasing
endif

ifeq ($(ARCH_ARM_HAVE_TLS_REGISTER),true)
    LOCAL_CFLAGS += -DHAVE_ARM_TLS_REGISTER
endif

ifneq ($(TARGET_SIMULATOR),true)
    # we need to access the private Bionic header <bionic_tls.h>
    # on ARM platforms, we need to mirror the ARCH_ARM_HAVE_TLS_REGISTER
    # behavior from the bionic Android.mk file
    ifeq ($(TARGET_ARCH)-$(ARCH_ARM_HAVE_TLS_REGISTER),arm-true)
        LOCAL_CFLAGS += -DHAVE_ARM_TLS_REGISTER
    endif
    LOCAL_C_INCLUDES += bionic/libc/private
endif

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/egl
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false 
LOCAL_MODULE:= libGLESv1_CM_virtual2

include $(BUILD_SHARED_LIBRARY)

