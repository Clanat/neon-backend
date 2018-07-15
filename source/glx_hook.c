//
// Created by clanat on 07.07.18.
//

#define _GNU_SOURCE
#include <dlfcn.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include "glx_hook.h"

static void*(*dlsym_real)(void*, const char*) = NULL;
static __GLXextFuncPtr (*glXGetProcAddressARB_real)(const GLubyte*) = NULL;
static __GLXextFuncPtr (*glXSwapBuffers_real)(Display* dpy, GLXDrawable drawable) = NULL;

void glXSwapBuffers_hooked(Display* dpy, GLXDrawable drawable) {
    glXSwapBuffers_real(dpy, drawable);

    if (glx_hook) {
        glx_hook();
    }
}

__GLXextFuncPtr glXGetProcAddressARB_hooked(const GLubyte* procName) {
    if (!glXGetProcAddressARB_real) {
        glXGetProcAddressARB_real = dlsym_real(RTLD_NEXT, "glXGetProcAddressARB");
    }

    const char* error = dlerror();
    if (error) {
        fprintf(stderr, "glXGetProcAddressARB hooking failed: %s\n", error);
        exit(1);
    }

    __GLXextFuncPtr extention_func = glXGetProcAddressARB_real(procName);
    if (strcmp((const char*)procName, "glXSwapBuffers") == 0) {
        glXSwapBuffers_real = (void *)extention_func;
        return (__GLXextFuncPtr)glXSwapBuffers_hooked;
    }

    return extention_func;
}

void* dlsym(void* handle, const char* symbol) {
    if (!dlsym_real) {
        dlsym_real = dlvsym(RTLD_DEFAULT, "dlsym", "GLIBC_2.0");
    }

    if (strcmp(symbol, "glXGetProcAddressARB") == 0) {
        return glXGetProcAddressARB_hooked;
    }

    return dlsym_real(handle, symbol);
}