# uws_frame_decoder requirements
 
## Overview

`uws_frame_decoder` is module that is responsible for decoding WebScokets frames from a stream of bytes.

## References

RFC6455 - The WebSocket Protocol.

## Exposed API

```c
    typedef struct UWS_FRAME_DECODER_TAG* UWS_FRAME_DECODER_HANDLE;

    typedef void (*ON_WS_FRAME_DECODED)(void* context, UWS_FRAME_HANDLE uws_frame);

    MOCKABLE_FUNCTION(, UWS_FRAME_DECODER_HANDLE, uws_frame_decoder_create, ON_WS_FRAME_DECODED, on_frame_decoded, void*, context);
    MOCKABLE_FUNCTION(, void, uws_frame_decoder_destroy, UWS_FRAME_DECODER_HANDLE, uws_frame_decoder);
    MOCKABLE_FUNCTION(, int, uws_frame_decoder_decode, UWS_FRAME_DECODER_HANDLE, uws_frame_decoder, const unsigned char*, bytes, size_t, size);
```

### uws_frame_decoder_create

```c
extern UWS_FRAME_DECODER_HANDLE uws_frame_decoder_create(ON_WS_FRAME_DECODED on_frame_decoded, void* context);
```

**SRS_UWS_FRAME_DECODER_01_001: [**`uws_frame_decoder_create` shall create a new `uws_frame_decoder` instance and return a non-NULL handle to it.**]**
**SRS_UWS_FRAME_DECODER_01_002: [** If the argument `on_frame_decoded` is NULL then `uws_frame_decoder_create` shall return NULL. **]**
**SRS_UWS_FRAME_DECODER_01_003: [** The argument `context` shall be allowed to be NULL. **]**
**SRS_UWS_FRAME_DECODER_01_004: [** If allocating memory for the new instance fails then `uws_frame_decoder_create` shall return NULL. **]**

### uws_frame_decoder_destroy

```c
extern void uws_frame_decoder_destroy(UWS_FRAME_DECODER_HANDLE uws_frame_decoder);
```

**SRS_UWS_FRAME_DECODER_01_005: [** `uws_frame_decoder_destroy` shall free all resources associated with the uws frame decoder instance. **]**
**SRS_UWS_FRAME_DECODER_01_006: [** If `uws_frame_Decoder` is NULL, `uws_frame_decoder_destroy` shall do nothing. **]**

### uws_frame_decoder_decode

```c
extern int uws_frame_decoder_decode(UWS_FRAME_DECODER_HANDLE uws_frame_decoder, const unsigned char* bytes, size_t size);
```
**SRS_UWS_FRAME_DECODER_01_007: [** `uws_frame_decoder_decode` shall accumulate the `size` bytes pointed to `bytes` and attempt to decode a WebSocket frame. **]**
**SRS_UWS_FRAME_DECODER_01_008: [** On success `uws_frame_decoder_decode` shall return 0. **]**
**SRS_UWS_FRAME_DECODER_01_009: [** If `uws_frame_decoder` or `bytes` is NULL, `uws_frame_decoder_decode` shall fail and return a non-zero value. **]**
**SRS_UWS_FRAME_DECODER_01_010: [** If a frame is decoded `uws_frame_decoder_decode` shall call the `on_frame_decoded` callback that was passed to `uws_frame_decoder_create`. **]**
**SRS_UWS_FRAME_DECODER_01_011: [** When `on_frame_decoded` is called the `context` value given in `uws_frame_decoder_create` shall be passed to it together with the a newly created WebSocket frame handle. **]**
**SRS_UWS_FRAME_DECODER_01_012: [** The WebSocket frame handle shall be created by calling `uws_frame_handle_create`. **]**
**SRS_UWS_FRAME_DECODER_01_013: [** If `uws_frame_handle_create` fails, `uws_frame_decoder_decode` shall fail and return a non-zero value. **]**

### RFC6455

5.  Data Framing

5.1.  Overview

   **SRS_UWS_FRAME_DECODER_01_014: [** In the WebSocket Protocol, data is transmitted using a sequence of frames. **]**
   **SRS_UWS_FRAME_DECODER_01_015: [** To avoid confusing network intermediaries (such as intercepting proxies) and for security reasons that are further discussed in Section 10.3, a client MUST mask all frames that it sends to the server (see Section 5.3 for further details). **]**
   (Note that **SRS_UWS_FRAME_DECODER_01_016: [** masking is done whether or not the WebSocket Protocol is running over TLS. **]**)
   The server MUST close the connection upon receiving a frame that is not masked.
   **SRS_UWS_FRAME_DECODER_01_017: [** In this case, a server MAY send a Close frame with a status code of 1002 (protocol error) as defined in Section 7.4.1. **]**
   **SRS_UWS_FRAME_DECODER_01_018: [** A server MUST NOT mask any frames that it sends to the client. **]**
   **SRS_UWS_FRAME_DECODER_01_019: [** A client MUST close a connection if it detects a masked frame. **]**
   **SRS_UWS_FRAME_DECODER_01_020: [** In this case, it MAY use the status code 1002 (protocol error) as defined in Section 7.4.1. (These rules might be relaxed in a future specification.) **]**

   The base framing protocol defines a frame type with an opcode, a payload length, and designated locations for "Extension data" and "Application data", which together define the "Payload data".
   Certain bits and opcodes are reserved for future expansion of the protocol.

   **SRS_UWS_FRAME_DECODER_01_021: [** A data frame MAY be transmitted by either the client or the server at any time after opening handshake completion and before that endpoint has sent a Close frame (Section 5.5.1). **]**

5.2.  Base Framing Protocol

   This wire format for the data transfer part is described by the ABNF [RFC5234] given in detail in this section.
   (Note that, unlike in other sections of this document, the ABNF in this section is operating on groups of bits.
   The length of each group of bits is indicated in a comment.
   When encoded on the wire, the most significant bit is the leftmost in the ABNF).
   A high-level overview of the framing is given in the following figure.
   In a case of conflict between the figure below and the ABNF specified later in this section, the figure is authoritative.

      0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-------+-+-------------+-------------------------------+
     |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
     |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
     |N|V|V|V|       |S|             |   (if payload len==126/127)   |
     | |1|2|3|       |K|             |                               |
     +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
     |     Extended payload length continued, if payload len == 127  |
     + - - - - - - - - - - - - - - - +-------------------------------+
     |                               |Masking-key, if MASK set to 1  |
     +-------------------------------+-------------------------------+
     | Masking-key (continued)       |          Payload Data         |
     +-------------------------------- - - - - - - - - - - - - - - - +
     :                     Payload Data continued ...                :
     + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
     |                     Payload Data continued ...                |
     +---------------------------------------------------------------+

   FIN:  1 bit

      **SRS_UWS_FRAME_DECODER_01_022: [** Indicates that this is the final fragment in a message. **]**
      **SRS_UWS_FRAME_DECODER_01_023: [** The first fragment MAY also be the final fragment. **]**

   RSV1, RSV2, RSV3:  1 bit each

      **SRS_UWS_FRAME_DECODER_01_024: [** MUST be 0 unless an extension is negotiated that defines meanings for non-zero values. **]**
      **SRS_UWS_FRAME_DECODER_01_025: [** If a nonzero value is received and none of the negotiated extensions defines the meaning of such a nonzero value, the receiving endpoint MUST _Fail the WebSocket Connection_. **]**

   Opcode:  4 bits

      Defines the interpretation of the "Payload data".
      **SRS_UWS_FRAME_DECODER_01_026: [** If an unknown opcode is received, the receiving endpoint MUST _Fail the WebSocket Connection_. **]**
      The following values are defined.

      **SRS_UWS_FRAME_DECODER_01_027: [** *  %x0 denotes a continuation frame **]**

      **SRS_UWS_FRAME_DECODER_01_028: [** *  %x1 denotes a text frame **]**

      **SRS_UWS_FRAME_DECODER_01_029: [** *  %x2 denotes a binary frame **]**

      **SRS_UWS_FRAME_DECODER_01_030: [** *  %x3-7 are reserved for further non-control frames **]**

      **SRS_UWS_FRAME_DECODER_01_031: [** *  %x8 denotes a connection close **]**

      **SRS_UWS_FRAME_DECODER_01_032: [** *  %x9 denotes a ping **]**

      **SRS_UWS_FRAME_DECODER_01_033: [** *  %xA denotes a pong **]**

      **SRS_UWS_FRAME_DECODER_01_034: [** *  %xB-F are reserved for further control frames **]**

   Mask:  1 bit

      **SRS_UWS_FRAME_DECODER_01_035: [** Defines whether the "Payload data" is masked. **]**
      **SRS_UWS_FRAME_DECODER_01_036: [** If set to 1, a masking key is present in masking-key, and this is used to unmask the "Payload data" as per Section 5.3. **]**
      **SRS_UWS_FRAME_DECODER_01_037: [** All frames sent from client to server have this bit set to 1. **]**

   Payload length:  7 bits, 7+16 bits, or 7+64 bits

      **SRS_UWS_FRAME_DECODER_01_038: [** The length of the "Payload data", in bytes: **]** **SRS_UWS_FRAME_DECODER_01_097: [** if 0-125, that is the payload length. **]**
      **SRS_UWS_FRAME_DECODER_01_039: [** If 126, the following 2 bytes interpreted as a 16-bit unsigned integer are the payload length. **]**
      **SRS_UWS_FRAME_DECODER_01_040: [** If 127, the following 8 bytes interpreted as a 64-bit unsigned integer (the most significant bit MUST be 0) are the payload length. **]**
      **SRS_UWS_FRAME_DECODER_01_041: [** Multibyte length quantities are expressed in network byte order. **]**
      **SRS_UWS_FRAME_DECODER_01_042: [** Note that in all cases, the minimal number of bytes MUST be used to encode the length, for example, the length of a 124-byte-long string can't be encoded as the sequence 126, 0, 124. **]**
      **SRS_UWS_FRAME_DECODER_01_043: [** The payload length is the length of the "Extension data" + the length of the "Application data". **]**
      **SRS_UWS_FRAME_DECODER_01_044: [** The length of the "Extension data" may be zero, in which case the payload length is the length of the "Application data". **]**

   Masking-key:  0 or 4 bytes

      **SRS_UWS_FRAME_DECODER_01_045: [** All frames sent from the client to the server are masked by a 32-bit value that is contained within the frame. **]**
      **SRS_UWS_FRAME_DECODER_01_046: [** This field is present if the mask bit is set to 1 and is absent if the mask bit is set to 0. **]**
      See Section 5.3 for further information on client-to-server masking.

   Payload data:  (x+y) bytes

      **SRS_UWS_FRAME_DECODER_01_047: [** The "Payload data" is defined as "Extension data" concatenated with "Application data". **]**

   Extension data:  x bytes

      **SRS_UWS_FRAME_DECODER_01_048: [** The "Extension data" is 0 bytes unless an extension has been negotiated. **]**
      **SRS_UWS_FRAME_DECODER_01_049: [** Any extension MUST specify the length of the "Extension data", or how that length may be calculated, and how the extension use MUST be negotiated during the opening handshake. **]**
      **SRS_UWS_FRAME_DECODER_01_050: [** If present, the "Extension data" is included in the total payload length. **]**

   Application data:  y bytes

      **SRS_UWS_FRAME_DECODER_01_051: [** Arbitrary "Application data", taking up the remainder of the frame after any "Extension data". **]**
      **SRS_UWS_FRAME_DECODER_01_052: [** The length of the "Application data" is equal to the payload length minus the length of the "Extension data". **]**

   The base framing protocol is formally defined by the following ABNF [RFC5234].
   It is important to note that the representation of this data is binary, not ASCII characters.
   As such, a field with a length of 1 bit that takes values %x0 / %x1 is represented as a single bit whose value is 0 or 1, not a full byte (octet) that stands for the characters "0" or "1" in the ASCII encoding.
   A field with a length of 4 bits with values between %x0-F again is represented by 4 bits, again NOT by an ASCII character or full byte (octet) with these values.
   [RFC5234] does not specify a character encoding: "Rules resolve into a string of terminal values, sometimes called characters. In ABNF, a character is merely a non-negative integer. In certain contexts, a specific mapping (encoding) of values into a character set (such as ASCII) will be specified."
   Here, the specified encoding is a binary encoding where each terminal value is encoded in the specified number of bits, which varies for each field.

**SRS_UWS_FRAME_DECODER_01_096: [**     ws-frame                = frame-fin           ; 1 bit in length
                              frame-rsv1          ; 1 bit in length
                              frame-rsv2          ; 1 bit in length
                              frame-rsv3          ; 1 bit in length
                              frame-opcode        ; 4 bits in length
                              frame-masked        ; 1 bit in length
                              frame-payload-length   ; either 7, 7+16,
                                                     ; or 7+64 bits in
                                                     ; length
                              [ frame-masking-key ]  ; 32 bits in length
                              frame-payload-data     ; n*8 bits in
                                                     ; length, where
                                                     ; n >= 0 **]**

    **SRS_UWS_FRAME_DECODER_01_053: [** frame-fin               = %x0 ; more frames of this message follow **]**
                            **SRS_UWS_FRAME_DECODER_01_068: [** / %x1 ; final frame of this message **]**
                                  ; 1 bit in length

    **SRS_UWS_FRAME_DECODER_01_054: [** frame-rsv1              = %x0 / %x1
                              ; 1 bit in length, MUST be 0 unless
                              ; negotiated otherwise **]**

    **SRS_UWS_FRAME_DECODER_01_055: [** frame-rsv2              = %x0 / %x1
                              ; 1 bit in length, MUST be 0 unless
                              ; negotiated otherwise **]**

    **SRS_UWS_FRAME_DECODER_01_056: [** frame-rsv3              = %x0 / %x1
                              ; 1 bit in length, MUST be 0 unless
                              ; negotiated otherwise **]**

    **SRS_UWS_FRAME_DECODER_01_057: [** frame-opcode            = frame-opcode-non-control /
                              frame-opcode-control /
                              frame-opcode-cont **]**

    **SRS_UWS_FRAME_DECODER_01_058: [** frame-opcode-cont       = %x0 ; frame continuation **]**

    **SRS_UWS_FRAME_DECODER_01_059: [** frame-opcode-non-control= %x1 ; text frame
                            / %x2 ; binary frame
                            / %x3-7
                            ; 4 bits in length,
                            ; reserved for further non-control frames **]**

    **SRS_UWS_FRAME_DECODER_01_060: [** frame-opcode-control    = %x8 ; connection close
                            / %x9 ; ping
                            / %xA ; pong
                            / %xB-F ; reserved for further control
                                    ; frames
                                    ; 4 bits in length **]**

    **SRS_UWS_FRAME_DECODER_01_061: [** frame-masked            = %x0
                            ; frame is not masked, no frame-masking-key
                            / %x1
                            ; frame is masked, frame-masking-key present
                            ; 1 bit in length **]**

    **SRS_UWS_FRAME_DECODER_01_062: [** frame-payload-length    = ( %x00-7D )
                            / ( %x7E frame-payload-length-16 )
                            / ( %x7F frame-payload-length-63 )
                            ; 7, 7+16, or 7+64 bits in length,
                            ; respectively **]**

    **SRS_UWS_FRAME_DECODER_01_063: [** frame-payload-length-16 = %x0000-FFFF ; 16 bits in length **]**

    **SRS_UWS_FRAME_DECODER_01_064: [** frame-payload-length-63 = %x0000000000000000-7FFFFFFFFFFFFFFF
                            ; 64 bits in length **]**

    **SRS_UWS_FRAME_DECODER_01_065: [** frame-masking-key       = 4( %x00-FF )
                              ; present only if frame-masked is 1
                              ; 32 bits in length **]**

    **SRS_UWS_FRAME_DECODER_01_066: [** frame-payload-data      = (frame-masked-extension-data
                               frame-masked-application-data)
                            ; when frame-masked is 1
                              / (frame-unmasked-extension-data
                                frame-unmasked-application-data)
                            ; when frame-masked is 0 **]**

    **SRS_UWS_FRAME_DECODER_01_067: [** frame-masked-extension-data     = *( %x00-FF )
                            ; reserved for future extensibility
                            ; n*8 bits in length, where n >= 0 **]**

    **SRS_UWS_FRAME_DECODER_01_069: [** frame-masked-application-data   = *( %x00-FF )
                            ; n*8 bits in length, where n >= 0 **]**

    **SRS_UWS_FRAME_DECODER_01_070: [** frame-unmasked-extension-data   = *( %x00-FF )
                            ; reserved for future extensibility
                            ; n*8 bits in length, where n >= 0 **]**

    **SRS_UWS_FRAME_DECODER_01_071: [** frame-unmasked-application-data = *( %x00-FF )
                            ; n*8 bits in length, where n >= 0 **]**

5.3.  Client-to-Server Masking

   **SRS_UWS_FRAME_DECODER_01_072: [** A masked frame MUST have the field frame-masked set to 1, as defined in Section 5.2. **]**

   **SRS_UWS_FRAME_DECODER_01_073: [** The masking key is contained completely within the frame, as defined in Section 5.2 as frame-masking-key. **]**
   **SRS_UWS_FRAME_DECODER_01_074: [** It is used to mask the "Payload data" defined in the same section as frame-payload-data, which includes "Extension data" and "Application data". **]**

   **SRS_UWS_FRAME_DECODER_01_075: [** The masking key is a 32-bit value chosen at random by the client. **]**
   **SRS_UWS_FRAME_DECODER_01_076: [** When preparing a masked frame, the client MUST pick a fresh masking key from the set of allowed 32-bit values. **]**
   **SRS_UWS_FRAME_DECODER_01_077: [** The masking key needs to be unpredictable; thus, the masking key MUST be derived from a strong source of entropy, and the masking key for a given frame MUST NOT make it simple for a server/proxy to predict the masking key for a subsequent frame. **]**
   The unpredictability of the masking key is essential to prevent authors of malicious applications from selecting the bytes that appear on the wire.
   RFC 4086 [RFC4086] discusses what entails a suitable source of entropy for security-sensitive applications.

   The masking does not affect the length of the "Payload data".
   **SRS_UWS_FRAME_DECODER_01_078: [** To convert masked data into unmasked data, or vice versa, the following algorithm is applied. **]**
   **SRS_UWS_FRAME_DECODER_01_079: [** The same algorithm applies regardless of the direction of the translation, e.g., the same steps are applied to mask the data as to unmask the data. **]**

   **SRS_UWS_FRAME_DECODER_01_080: [** Octet i of the transformed data ("transformed-octet-i") is the XOR of octet i of the original data ("original-octet-i") with octet at index i modulo 4 of the masking key ("masking-key-octet-j"): **]**

     j                   = i MOD 4
     transformed-octet-i = original-octet-i XOR masking-key-octet-j

   **SRS_UWS_FRAME_DECODER_01_081: [** The payload length, indicated in the framing as frame-payload-length, does NOT include the length of the masking key. **]**
   It is the length of the "Payload data", e.g., the number of bytes following the masking key.

5.5.  Control Frames

   **SRS_UWS_FRAME_DECODER_01_082: [** Control frames are identified by opcodes where the most significant bit of the opcode is 1. **]**
   **SRS_UWS_FRAME_DECODER_01_083: [** Currently defined opcodes for control frames include 0x8 (Close), 0x9 (Ping), and 0xA (Pong). **]**
   **SRS_UWS_FRAME_DECODER_01_084: [** Opcodes 0xB-0xF are reserved for further control frames yet to be defined. **]**

   **SRS_UWS_FRAME_DECODER_01_085: [** Control frames are used to communicate state about the WebSocket. **]**
   **SRS_UWS_FRAME_DECODER_01_086: [** Control frames can be interjected in the middle of a fragmented message. **]**

   **SRS_UWS_FRAME_DECODER_01_087: [** All control frames MUST have a payload length of 125 bytes or less and MUST NOT be fragmented. **]**

