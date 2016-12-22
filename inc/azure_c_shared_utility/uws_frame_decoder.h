#ifndef UWS_FRAME_DECODER_H
#define UWS_FRAME_DECODER_H

#include "uws_frame.h"

typedef (*ON_WS_FRAME_DECODED)(void* context, UWS_FRAME_HANDLE uws_frame)

extern UWS_FRAME_DECODER_HANDLE uws_frame_decoder_create(ON_WS_FRAME_DECODED on_frame_decoded, void* context);
extern void uws_frame_decoder_destroy(UWS_FRAME_DECODER_HANDLE uws_frame_decoder);

#endif /* UWS_FRAME_DECODER_H */
