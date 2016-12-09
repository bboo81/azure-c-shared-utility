// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef UWS_H
#define UWS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UWS_INSTANCE_TAG* UWS_HANDLE;

extern UWS_HANDLE uws_create(void);
extern void uws_destroy(UWS_HANDLE ws_io);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UWS_H */
