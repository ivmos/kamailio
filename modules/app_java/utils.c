/**
 * $Id$
 *
 * Copyright (C) 2013 Konstantin Mosesov
 *
 * This file is part of Kamailio, a free SIP server.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../sr_module.h"

#include <jni.h>

#include "global.h"
#include "utils.h"
#include "java_mod.h"
#include "java_iface.h"
#include "java_support.h"
#include "java_native_methods.h"
#include "java_sig_parser.h"


char **split(char *str, char *sep)
{
    char **buf = NULL;
    char *token = NULL;
    char *saveptr = NULL;
    int i;

    buf = (char **)pkg_malloc(sizeof(char *));
    if (!buf)
    {
	LM_ERR("pkg_malloc() has failed. Not enough memory!\n");
	return NULL;
    }
    memset(&buf, 0, sizeof(char *));

    if (str == NULL)
	return buf;

    if (strncmp(str, sep, strlen(sep)) <= 0)
    {
	// string doesn't contains a separator
	buf[0] = strdup(str);
	return buf;
    }

    token = strdup(str);
    for (i=0; token != NULL; token = saveptr, i++)
    {
        token = strtok_r(token, (const char *)sep, &saveptr);

        if (token == NULL || !strcmp(token, ""))
            break;

	buf = (char **)pkg_realloc(buf, (i+1) * sizeof(char *));
	if (!buf)
	{
	    LM_ERR("pkg_realloc() has failed. Not enough memory!\n");
	    return NULL;
	}
        buf[i] = strdup(token);
    }
    buf[i] = NULL;

    free(token);

    return buf;
}

void ThrowNewException(JNIEnv *env, char *fmt, ...)
{
    va_list ap;
    char buf[1024];

    memset(buf, 0, sizeof(char));

    va_start(ap, fmt);
    vsnprintf(buf, 1024, fmt, ap);
    va_end(ap);

    (*env)->ThrowNew(env, (*env)->FindClass(env, "java/lang/Exception"), buf);
}


struct sip_msg *get_struct_sip_msg(JNIEnv *jenv)
{
    jfieldID fid;
    int msgptr;
    jclass cls;

    cls = (*jenv)->GetObjectClass(jenv, KamailioClassInstance);
    fid = (*jenv)->GetFieldID(jenv, cls, "mop", "I");
    if (!fid)
    {
        (*jenv)->ExceptionClear(jenv);
        LM_ERR("Failed to find protected field org.siprouter.NativeMethods.mop\n");
        return NULL;
    }
    (*jenv)->DeleteLocalRef(jenv, cls);

    msgptr = (int)(*jenv)->GetIntField(jenv, KamailioClassInstance, fid);
    if ((*jenv)->ExceptionCheck(jenv))
    {
        handle_exception();
        return NULL;
    }

    if (msgptr == 0x0)
    {
        LM_ERR("app_java: KamExec(): Unable to execute. Internal Error: msgptr is NULL\n");
        return NULL;
    }

    return FORCE_CAST_O2P(msgptr, struct sip_msg *);
}

