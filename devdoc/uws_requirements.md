# uws requirements
 
## Overview

uws is module that provides the public API for uWS, a Websocket C library geared for small devices.

## References

RFC6455 - The WebSocket Protocol.

## Exposed API

```c
extern UWS_HANDLE uws_create(const char* hostname, unsigned int port, bool use_ssl);
extern void uws_destroy(UWS_HANDLE uws);
extern int uws_open(UWS_HANDLE uws, ON_UW_OPEN_COMPLETE on_uw_open_complete, ON_WS_FRAME_RECEIVED on_ws_frame_received, ON_WS_ERROR on_ws_error, void* callback_context);
extern int uws_close(UWS_HANDLE uws);
extern int uws_send(UWS_HANDLE uws, const unsigned char* buffer, size_t size, ON_WS_SEND_COMPLETE on_ws_send_complete, void* callback_context);
extern void uws_dowork(UWS_HANDLE uws);
```

### uws_create

```c
extern UWS_HANDLE uws_create(const char* hostname, unsigned int port, bool use_ssl);
```

**SRS_UWS_01_001: \[**`uws_create` shall create an instance of uws and return a non-NULL handle to it.**\]**
**SRS_UWS_01_002: [** If the argument `hostname` is NULL then `uws_create` shall return NULL. **]**
**SRS_UWS_01_003: [** If allocating memory for the new uws instance fails then `uws_create` shall return NULL. **]**
**SRS_UWS_01_004: [** The argument `hostname` shall be copied for later use (they are needed when the IO is opened). **]**
**SRS_UWS_01_005: [** If `use_ssl` is 0 then `uws_create` shall obtain the interface used to create a socketio instance by calling `socketio_get_interface_description`. **]**
**SRS_UWS_01_006: [** If `use_ssl` is 1 then `uws_create` shall obtain the interface used to create a tlsio instance by calling `platform_get_default_tlsio`. **]**
**SRS_UWS_01_007: [** If obtaining the underlying IO interface fails, then `uws_create` shall fail and return NULL. **]** 
**SRS_UWS_01_008: [** The obtained interface shall be used to create the IO used as underlying IO by the newly created uws instance. **]**
**SRS_UWS_01_009: [** The underlying IO shall be created by calling `uws_create`. **]**
**SRS_UWS_01_010: [** The create arguments for the socket IO (when `use_ssl` is 0) shall have: **]**
**SRS_UWS_01_011: [** - `host` set to the `host` field in the `io_create_parameters` passed to `uws_create`. **]**
**SRS_UWS_01_012: [** - `port` set to 80. **]**
**SRS_UWS_01_013: [** The create arguments for the tls IO (when `use_ssl` is 1) shall have: **]**
**SRS_UWS_01_014: [** - `host` set to the `host` field in the `io_create_parameters` passed to `uws_create`. **]**
**SRS_UWS_01_015: [** - `port` set to 443. **]**
**SRS_UWS_01_016: [** If `xio_create` fails, then `uws_create` shall fail and return NULL. **]**
**SRS_UWS_01_017: [** `uws_create` shall create a pending send IO list that is to be used to queue send packets by calling `singlylinkedlist_create`. **]**
**SRS_UWS_01_018: [** If `singlylinkedlist_create` fails then `uws_create` shall fail and return NULL. **]**

### uws_destroy

```c
extern void uws_destroy(UWS_HANDLE uws);
```

**SRS_UWS_01_019: [** `uws_destroy` shall free all resources associated with the uws instance. **]**
**SRS_UWS_01_020: [** If `uws` is NULL, `uws_destroy` shall do nothing. **]** 
**SRS_UWS_01_021: [** `uws_destroy` shall perform a close action if the uws instance has already been open. **]**
**SRS_UWS_01_022: [** `uws_destroy` shall execute a close action if an open is in progress. **]**
**SRS_UWS_01_023: [** `uws_destroy` shall destroy the underlying IO created in `uws_create` by calling `xio_destroy`. **]**
**SRS_UWS_01_024: [** `uws_destroy` shall free the list used to track the pending sends by calling `singlylinkedlist_destroy`. **]**

### uws_open

```c
extern int uws_open(UWS_HANDLE uws, ON_UWS_OPEN_COMPLETE on_ws_open_complete, ON_WS_FRAME_RECEIVED on_ws_frame_received, ON_WS_ERROR on_ws_error, void* callback_context);
```

**SRS_UWS_01_025: [** `uws_open` shall open the underlying IO by calling `xio_open` and providing the IO handle created in `uws_create` as argument. **]**
**SRS_UWS_01_026: [** On success, `uws_open` shall return 0. **]**
**SRS_UWS_01_027: [** If `on_ws_open_complete` or `on_ws_error` is NULL, `uws_open` shall fail and return a non-zero value. **]**
**SRS_UWS_01_028: [** If opening the underlyion IO fails then `uws_open` shall fail and return a non-zero value. **]**

### uws_close

```c
extern int uws_close(UWS_HANDLE uws);
```

**SRS_UWS_01_029: [** `uws_close` shall close the uws instance connection if an open action is either pending or has completed successfully (if the IO is open). **]** 
**SRS_UWS_01_030: [** if `uws` is NULL, `uws_close` shall return a non-zero value. **]** 
**SRS_UWS_01_031: [** `uws_close` shall close the connection by calling `xio_close` while passing as argument the IO handle created in `uws_create`. **]** 
**SRS_UWS_01_032: [** `uws_close` when no open action has been issued shall fail and return a non-zero value. **]**
**SRS_UWS_01_033: [** `uws_close` after a `uws_close` shall fail and return a non-zero value. **]** 
**SRS_UWS_01_034: [** `uws_close` shall obtain all the pending IO items by repetitively querying for the head of the pending IO list and freeing that head item. **]**
**SRS_UWS_01_035: [** Obtaining the head of the pending IO list shall be done by calling `singlylinkedlist_get_head_item`. **]**
**SRS_UWS_01_036: [** For each pending item the send complete callback shall be called with `UWS_SEND_CANCELLED`. **]**
**SRS_UWS_01_037: [** The callback context passed to the `on_ws_send_complete` callback shall be the context given to `uws_send`. **]** 

### uws_send

```c
extern int uws_send(UWS_HANDLE uws, const unsigned char* buffer, size_t size, ON_WS_SEND_COMPLETE on_ws_send_complete, void* callback_context);
```

**SRS_UWS_01_038: [** `uws_send` shall create and queue a structure that contains: **]**
**SRS_UWS_01_039: [** - the websocket frame containing the `size` bytes pointed by `buffer`, so that the frame can be later sent when `uws_dowork` is called **]**
**SRS_UWS_01_040: [** - the send complete callback `on_ws_send_complete` **]**
**SRS_UWS_01_041: [** - the send complete callback context `on_send_complete_context` **]**
**SRS_UWS_01_042: [** On success, `uws_send` shall return 0. **]**
**SRS_UWS_01_043: [** If the uws instance is not OPEN (open has not been called or is still in progress) then `uws_send` shall fail and return a non-zero value. **]**
**SRS_UWS_01_044: [** If any of the arguments `uws` or `buffer` are NULL, `uws_send` shall fail and return a non-zero value. **]**
**SRS_UWS_01_045: [** If `size` is zero then `uws_send` shall fail and return a non-zero value. **]**
**SRS_UWS_01_046: [** `uws_send` shall allocate enough memory to hold the websocket frame that contains `size` bytes. **]**
**SRS_UWS_01_047: [** If allocating memory for the newly queued item fails, `uws_send` shall fail and return a non-zero value. **]**
**SRS_UWS_01_048: [** Queueing shall be done by calling `singlylinkedlist_add`. **]**
**SRS_UWS_01_049: [** If `singlylinkedlist_add` fails, `uws_send` shall fail and return a non-zero value. **]**
**SRS_UWS_01_050: [** The argument on_ws_send_complete shall be optional, if NULL is passed by the caller then no send complete callback shall be triggered. **]**

### uws_dowork

```c
extern void uws_dowork(UWS_HANDLE uws);
```

**SRS_UWS_01_051: [** `uws_dowork` shall iterate through all the pending sends and for each one of them: **]**
**SRS_UWS_01_052: [** `uws_dowork` shall call `xio_send` to send the websocket frame with the following arguments: **]**
**SRS_UWS_01_053: [** - the io handle shall be the underlyiong IO handle created in `uws_create`. **]**
**SRS_UWS_01_054: [** - the `buffer` argument shall point to the complete websocket frame to be sent. **]**
**SRS_UWS_01_055: [** - the `size` argument shall indicate the websocket frame length. **]**
**SRS_UWS_01_056: [** - the `send_complete` callback shall be the `uws_send_complete` function. **]**
**SRS_UWS_01_057: [** - the `send_complete_context` argument shall identify the pending send. **]**
**SRS_UWS_01_058: [** If `xio_send` fails, `uws_dowork` shall indicate that by calling the `on_ws_send_complete` callback associated with the pending send with the `UWS_SEND_ERROR` code. **]** 
**SRS_UWS_01_059: [** If the `uws` argument is NULL, `uws_dowork` shall do nothing. **]**
**SRS_UWS_01_060: [** If the IO is not yet open, `uws_dowork` shall do nothing. **]**

### on_underlying_io_error

### on_underlying_io_bytes_received

### on_underlying_io_open_complete

### on_underlying_io_send_complete

### RFC6455

3.  WebSocket URIs


   This specification defines two URI schemes, using the ABNF syntax defined in RFC 5234 [RFC5234], and terminology and ABNF productions defined by the URI specification RFC 3986 [RFC3986].

          ws-URI = "ws:" "//" host [ ":" port ] path [ "?" query ]
          wss-URI = "wss:" "//" host [ ":" port ] path [ "?" query ]

          host = <host, defined in [RFC3986], Section 3.2.2>
          port = <port, defined in [RFC3986], Section 3.2.3>
          path = <path-abempty, defined in [RFC3986], Section 3.3>
          query = <query, defined in [RFC3986], Section 3.4>

   The port component is OPTIONAL; the default for "ws" is port 80, while the default for "wss" is port 443.

   The URI is called "secure" (and it is said that "the secure flag is set") if the scheme component matches "wss" case-insensitively.

   The "resource-name" (also known as /resource name/ in Section 4.1) can be constructed by concatenating the following:

   -  "/" if the path component is empty

   -  the path component

   -  "?" if the query component is non-empty

   -  the query component

   Fragment identifiers are meaningless in the context of WebSocket URIs and MUST NOT be used on these URIs.
   As with any URI scheme, the character "#", when not indicating the start of a fragment, MUST be escaped as %23.

4.  Opening Handshake

4.1.  Client Requirements

   **SRS_UWS_01_061: [** To _Establish a WebSocket Connection_, a client opens a connection and sends a handshake as defined in this section. **]**
   **SRS_UWS_01_062: [** A connection is defined to initially be in a CONNECTING state. **]**
   **SRS_UWS_01_063: [** A client will need to supply a /host/, /port/, /resource name/, and a /secure/ flag, which are the components of a WebSocket URI as discussed in Section 3, along with a list of /protocols/ and /extensions/ to be used. **]**
   **SRS_UWS_01_064: [** Additionally, if the client is a web browser, it supplies /origin/. **]**

   Clients running in controlled environments, e.g., browsers on mobile handsets tied to specific carriers, MAY offload the management of the connection to another agent on the network.
   In such a situation, the client for the purposes of this specification is considered to include both the handset software and any such agents.

   **SRS_UWS_01_065: [** When the client is to _Establish a WebSocket Connection_ given a set of (/host/, /port/, /resource name/, and /secure/ flag), along with a list of /protocols/ and /extensions/ to be used, and an /origin/ in the case of web browsers, it MUST open a connection, send an opening handshake, and read the server's handshake in response. **]**
   The exact requirements of how the connection should be opened, what should be sent in the opening handshake, and how the server's response should be interpreted are as follows in this section.
   In the following text, we will use terms from Section 3, such as "/host/" and "/secure/ flag" as defined in that section.

   1.  **SRS_UWS_01_066: [** The components of the WebSocket URI passed into this algorithm (/host/, /port/, /resource name/, and /secure/ flag) MUST be valid according to the specification of WebSocket URIs specified in Section 3. **]** 
   	   **SRS_UWS_01_067: [** If any of the components are invalid, the client MUST _Fail the WebSocket Connection_ and abort these steps. **]**

   2.  **SRS_UWS_01_068: [** If the client already has a WebSocket connection to the remote host (IP address) identified by /host/ and port /port/ pair, even if the remote host is known by another name, the client MUST wait until that connection has been established or for that connection to have failed. **]**
       **SRS_UWS_01_069: [** There MUST be no more than one connection in a CONNECTING state. **]**
	   **SRS_UWS_01_070: [** If multiple connections to the same IP address are attempted simultaneously, the client MUST serialize them so that there is no more than one connection at a time running through the following steps. **]**

       **SRS_UWS_01_071: [** If the client cannot determine the IP address of the remote host (for example, because all communication is being done through a proxy server that performs DNS queries itself), then the client MUST assume for the purposes of this step that each host name refers to a distinct remote host, and instead the client SHOULD limit the total number of simultaneous pending connections to a reasonably low number (e.g., the client might allow simultaneous pending connections to a.example.com and b.example.com, but if thirty simultaneous connections to a single host are requested, that may not be allowed). **]**
       For example, in a web browser context, the client needs to consider the number of tabs the user has open in setting a limit to the number of simultaneous pending connections.

       NOTE: This makes it harder for a script to perform a denial-of-service attack by just opening a large number of WebSocket connections to a remote host.
       A server can further reduce the load on itself when attacked by pausing before closing the connection, as that will reduce the rate at which the client reconnects.

       NOTE: There is no limit to the number of established WebSocket connections a client can have with a single remote host.
       Servers can refuse to accept connections from hosts/IP addresses with an excessive number of existing connections or disconnect resource-hogging connections when suffering high load.

   3.  **SRS_UWS_01_072: [** _Proxy Usage_: If the client is configured to use a proxy when using the WebSocket Protocol to connect to host /host/ and port /port/, then the client SHOULD connect to that proxy and ask it to open a TCP connection to the host given by /host/ and the port given by /port/. **]**

          EXAMPLE: For example, if the client uses an HTTP proxy for all traffic, then if it was to try to connect to port 80 on server example.com, it might send the following lines to the proxy server:

              CONNECT example.com:80 HTTP/1.1
              Host: example.com

          If there was a password, the connection might look like:

              CONNECT example.com:80 HTTP/1.1
              Host: example.com
              Proxy-authorization: Basic ZWRuYW1vZGU6bm9jYXBlcyE=

       **SRS_UWS_01_073: [** If the client is not configured to use a proxy, then a direct TCP connection SHOULD be opened to the host given by /host/ and the port given by /port/. **]**

       NOTE: Implementations that do not expose explicit UI for selecting a proxy for WebSocket connections separate from other  proxies are encouraged to use a SOCKS5 [RFC1928] proxy for WebSocket connections, if available, or failing that, to prefer the proxy configured for HTTPS connections over the proxy configured for HTTP connections.

       **SRS_UWS_01_074: [** For the purpose of proxy autoconfiguration scripts, the URI to pass the function MUST be constructed from /host/, /port/, /resource name/, and the /secure/ flag using the definition of a WebSocket URI as given in Section 3. **]**

       NOTE: The WebSocket Protocol can be identified in proxy autoconfiguration scripts from the scheme ("ws" for unencrypted connections and "wss" for encrypted connections).

   4.  **SRS_UWS_01_075: [** If the connection could not be opened, either because a direct connection failed or because any proxy used returned an error, then the client MUST _Fail the WebSocket Connection_ and abort the connection attempt. **]**

   5.  **SRS_UWS_01_076: [** If /secure/ is true, the client MUST perform a TLS handshake over the connection after opening the connection and before sending the handshake data [RFC2818]. **]**
       **SRS_UWS_01_077: [** If this fails (e.g., the server's certificate could not be verified), then the client MUST _Fail the WebSocket Connection_ and abort the connection. **]**
       **SRS_UWS_01_078: [** Otherwise, all further communication on this channel MUST run through the encrypted tunnel [RFC5246]. **]**

       **SRS_UWS_01_079: [** Clients MUST use the Server Name Indication extension in the TLS handshake [RFC6066]. **]**

   **SRS_UWS_01_080: [** Once a connection to the server has been established (including a connection via a proxy or over a TLS-encrypted tunnel), the client MUST send an opening handshake to the server. **]**
   **SRS_UWS_01_081: [** The handshake consists of an HTTP Upgrade request, along with a list of required and optional header fields. **]**
   The requirements for this handshake are as follows.

   1.   **SRS_UWS_01_082: [** The handshake MUST be a valid HTTP request as specified by [RFC2616]. **]**

   2.   **SRS_UWS_01_083: [** The method of the request MUST be GET, and the HTTP version MUST be at least 1.1. **]**

        For example, if the WebSocket URI is "ws://example.com/chat", the first line sent should be "GET /chat HTTP/1.1".

   3.   **SRS_UWS_01_084: [** The "Request-URI" part of the request MUST match the /resource name/ defined in Section 3 (a relative URI) or be an absolute http/https URI that, when parsed, has a /resource name/, /host/, and /port/ that match the corresponding ws/wss URI. **]**

   4.   **SRS_UWS_01_085: [** The request MUST contain a |Host| header field whose value contains /host/ plus optionally ":" followed by /port/ (when not using the default port). **]**

   5.   **SRS_UWS_01_086: [** The request MUST contain an |Upgrade| header field whose value MUST include the "websocket" keyword. **]**

   6.   **SRS_UWS_01_087: [** The request MUST contain a |Connection| header field whose value MUST include the "Upgrade" token. **]**

   7.   **SRS_UWS_01_088: [** The request MUST include a header field with the name |Sec-WebSocket-Key|. **]**
        **SRS_UWS_01_089: [** The value of this header field MUST be a nonce consisting of a randomly selected 16-byte value that has been base64-encoded (see Section 4 of [RFC4648]). **]**
        **SRS_UWS_01_090: [** The nonce MUST be selected randomly for each connection. **]**

        NOTE: As an example, if the randomly selected value was the sequence of bytes 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0a 0x0b 0x0c 0x0d 0x0e 0x0f 0x10, the value of the header field would be "AQIDBAUGBwgJCgsMDQ4PEC=="

   8.   **SRS_UWS_01_091: [** The request MUST include a header field with the name |Origin| [RFC6454] if the request is coming from a browser client. **]**
        **SRS_UWS_01_092: [** If the connection is from a non-browser client, the request MAY include this header field if the semantics of that client match the use-case described here for browser clients. **]**
        **SRS_UWS_01_093: [** The value of this header field is the ASCII serialization of origin of the context in which the code establishing the connection is running. **]**
        See [RFC6454] for the details of how this header field value is constructed.

        As an example, if code downloaded from www.example.com attempts to establish a connection to ww2.example.com, the value of the header field would be "http://www.example.com".

   9.   **SRS_UWS_01_094: [** The request MUST include a header field with the name |Sec-WebSocket-Version|. **]**
        **SRS_UWS_01_095: [** The value of this header field MUST be 13. **]**

        NOTE: Although draft versions of this document (-09, -10, -11, and -12) were posted (they were mostly comprised of editorial changes and clarifications and not changes to the wire protocol), values 9, 10, 11, and 12 were not used as valid values for Sec-WebSocket-Version.
        These values were reserved in the IANA registry but were not and will not be used.

   10.  **SRS_UWS_01_096: [** The request MAY include a header field with the name |Sec-WebSocket-Protocol|. **]**
        **SRS_UWS_01_097: [** If present, this value indicates one or more comma-separated subprotocol the client wishes to speak, ordered by preference. **]**
        **SRS_UWS_01_098: [** The elements that comprise this value MUST be non-empty strings with characters in the range U+0021 to U+007E not including separator characters as defined in [RFC2616] **]** **SRS_UWS_01_099: [** and MUST all be unique strings **]**.
        The ABNF for the value of this header field is 1#token, where the definitions of constructs and rules are as given in [RFC2616].

   11.  **SRS_UWS_01_100: [** The request MAY include a header field with the name |Sec-WebSocket-Extensions|. **]**
        If present, this value indicates the protocol-level extension(s) the client wishes to speak.
        The interpretation and format of this header field is described in Section 9.1.

   12.  **SRS_UWS_01_101: [** The request MAY include any other header fields, for example, cookies [RFC6265] and/or authentication-related header fields such as the |Authorization| header field [RFC2616], which are processed according to documents that define them. **]**

   **SRS_UWS_01_102: [** Once the client's opening handshake has been sent, the client MUST wait for a response from the server before sending any further data. **]**
   **SRS_UWS_01_103: [** The client MUST validate the server's response as follows: **]**

   1.  **SRS_UWS_01_104: [** If the status code received from the server is not 101, the client handles the response per HTTP [RFC2616] procedures. **]**
       In particular, **SRS_UWS_01_105: [** the client might perform authentication if it receives a 401 status code; **]** **SRS_UWS_01_106: [** the server might redirect the client using a 3xx status code **]** **SRS_UWS_01_107: [** (but clients are not required to follow them) **]**, etc.
       Otherwise, proceed as follows.

   2.  **SRS_UWS_01_108: [** If the response lacks an |Upgrade| header field or the |Upgrade| header field contains a value that is not an ASCII case-insensitive match for the value "websocket", the client MUST _Fail the WebSocket Connection_. **]**

   3.  **SRS_UWS_01_109: [** If the response lacks a |Connection| header field or the |Connection| header field doesn't contain a token that is an ASCII case-insensitive match for the value "Upgrade", the client MUST _Fail the WebSocket Connection_. **]**

   4.  **SRS_UWS_01_110: [** If the response lacks a |Sec-WebSocket-Accept| header field or the |Sec-WebSocket-Accept| contains a value other than the base64-encoded SHA-1 of the concatenation of the |Sec-WebSocket-Key| (as a string, not base64-decoded) with the string "258EAFA5-E914-47DA-95CA-C5AB0DC85B11" but ignoring     any leading and trailing whitespace, the client MUST _Fail the WebSocket Connection_. **]**

   5.  **SRS_UWS_01_111: [** If the response includes a |Sec-WebSocket-Extensions| header field and this header field indicates the use of an extension that was not present in the client's handshake (the server has indicated an extension not requested by the client), the client MUST _Fail the WebSocket Connection_. **]** (The parsing of this header field to determine which extensions are requested is discussed in Section 9.1.)

   6.  **SRS_UWS_01_112: [** If the response includes a |Sec-WebSocket-Protocol| header field and this header field indicates the use of a subprotocol that was not present in the client's handshake (the server has indicated a subprotocol not requested by the client), the client MUST _Fail the WebSocket Connection_. **]**

   **SRS_UWS_01_113: [** If the server's response does not conform to the requirements for the server's handshake as defined in this section and in Section 4.2.2, the client MUST _Fail the WebSocket Connection_. **]**

   **SRS_UWS_01_114: [** Please note that according to [RFC2616], all header field names in both HTTP requests and HTTP responses are case-insensitive. **]**

   **SRS_UWS_01_115: [** If the server's response is validated as provided for above, it is said that _The WebSocket Connection is Established_ and that the WebSocket Connection is in the OPEN state. **]**
   **SRS_UWS_01_116: [** The _Extensions In Use_ is defined to be a (possibly empty) string, the value of which is equal to the value of the |Sec-WebSocket-Extensions| header field supplied by the server's handshake or the null value if that header field was not present in the server's handshake. **]**
   **SRS_UWS_01_117: [** The _Subprotocol In Use_ is defined to be the value of the |Sec-WebSocket-Protocol| header field in the server's handshake or the null value if that header field was not present in the server's handshake. **]**
   **SRS_UWS_01_118: [** Additionally, if any header fields in the server's handshake indicate that cookies should be set (as defined by [RFC6265]), these cookies are referred to as _Cookies Set During the Server's Opening Handshake_. **]**

4.2.  Server-Side Requirements

   Servers MAY offload the management of the connection to other agents on the network, for example, load balancers and reverse proxies.
   In such a situation, the server for the purposes of this specification is considered to include all parts of the server-side infrastructure from the first device to terminate the TCP connection all the way to the server that processes requests and sends responses.

   EXAMPLE: A data center might have a server that responds to WebSocket requests with an appropriate handshake and then passes the connection to another server to actually process the data frames.
   For the purposes of this specification, the "server" is the combination of both computers.

4.2.1.  Reading the Client's Opening Handshake

   When a client starts a WebSocket connection, it sends its part of the opening handshake.
   The server must parse at least part of this handshake in order to obtain the necessary information to generate the server part of the handshake.

   The client's opening handshake consists of the following parts.
   If the server, while reading the handshake, finds that the client did not send a handshake that matches the description below (note that as per [RFC2616], the order of the header fields is not important), including but not limited to any violations of the ABNF grammar specified for the components of the handshake, the server MUST stop processing the client's handshake and return an HTTP response with an appropriate error code (such as 400 Bad Request).

   1.   An HTTP/1.1 or higher GET request, including a "Request-URI" [RFC2616] that should be interpreted as a /resource name/ defined in Section 3 (or an absolute HTTP/HTTPS URI containing the /resource name/).

   2.   A |Host| header field containing the server's authority.

   3.   An |Upgrade| header field containing the value "websocket", treated as an ASCII case-insensitive value.

   4.   A |Connection| header field that includes the token "Upgrade", treated as an ASCII case-insensitive value.

   5.   A |Sec-WebSocket-Key| header field with a base64-encoded (see Section 4 of [RFC4648]) value that, when decoded, is 16 bytes in length.

   6.   A |Sec-WebSocket-Version| header field, with a value of 13.

   7.   Optionally, an |Origin| header field.  This header field is sent by all browser clients.
        A connection attempt lacking this header field SHOULD NOT be interpreted as coming from a browser client.

   8.   Optionally, a |Sec-WebSocket-Protocol| header field, with a list of values indicating which protocols the client would like to speak, ordered by preference.

   9.   Optionally, a |Sec-WebSocket-Extensions| header field, with a list of values indicating which extensions the client would like to speak.
        The interpretation of this header field is discussed in Section 9.1.

   10.  Optionally, other header fields, such as those used to send cookies or request authentication to a server.
        Unknown header fields are ignored, as per [RFC2616].

4.2.2.  Sending the Server's Opening Handshake

   When a client establishes a WebSocket connection to a server, the server MUST complete the following steps to accept the connection and send the server's opening handshake.

   1.  If the connection is happening on an HTTPS (HTTP-over-TLS) port, perform a TLS handshake over the connection.
       If this fails (e.g., the client indicated a host name in the extended client hello "server_name" extension that the server does not host), then close the connection; otherwise, all further communication for the connection (including the server's handshake) MUST run through the encrypted tunnel [RFC5246].

   2.  The server can perform additional client authentication, for example, by returning a 401 status code with the corresponding |WWW-Authenticate| header field as described in [RFC2616].

   3.  The server MAY redirect the client using a 3xx status code [RFC2616].
       Note that this step can happen together with, before, or after the optional authentication step described above.

   4.  Establish the following information:

       /origin/
          The |Origin| header field in the client's handshake indicates the origin of the script establishing the connection.
          The origin is serialized to ASCII and converted to lowercase.
          The server MAY use this information as part of a determination of whether to accept the incoming connection.
          If the server does not validate the origin, it will accept connections from anywhere.
          If the server does not wish to accept this connection, it MUST return an appropriate HTTP error code (e.g., 403 Forbidden) and abort the WebSocket handshake described in this section.
          For more detail, refer to Section 10.

       /key/
          The |Sec-WebSocket-Key| header field in the client's handshake includes a base64-encoded value that, if decoded, is 16 bytes in length.
          This (encoded) value is used in the creation of the server's handshake to indicate an acceptance of the connection.
          It is not necessary for the server to base64-decode the |Sec-WebSocket-Key| value.

       /version/
          The |Sec-WebSocket-Version| header field in the client's handshake includes the version of the WebSocket Protocol with which the client is attempting to communicate.
          If this version does not match a version understood by the server, the server MUST abort the WebSocket handshake described in this section and instead send an appropriate HTTP error code (such as 426 Upgrade Required) and a |Sec-WebSocket-Version| header field indicating the version(s) the server is capable of understanding.

       /resource name/
          An identifier for the service provided by the server.
          If the server provides multiple services, then the value should be derived from the resource name given in the client's handshake in the "Request-URI" [RFC2616] of the GET method.
          If the requested service is not available, the server MUST send an appropriate HTTP error code (such as 404 Not Found) and abort the WebSocket handshake.

       /subprotocol/
          Either a single value representing the subprotocol the server is ready to use or null.
          The value chosen MUST be derived from the client's handshake, specifically by selecting one of the values from the |Sec-WebSocket-Protocol| field that the server is willing to use for this connection (if any).
          If the client's handshake did not contain such a header field or if the server does not agree to any of the client's requested subprotocols, the only acceptable value is null.
          The absence of such a field is equivalent to the null value (meaning that if the server does not wish to agree to one of the suggested subprotocols, it MUST NOT send back a |Sec-WebSocket-Protocol| header field in its response).
          The empty string is not the same as the null value for these purposes and is not a legal value for this field.
          The ABNF for the value of this header field is (token), where the definitions of constructs and rules are as given in [RFC2616].

       /extensions/
          A (possibly empty) list representing the protocol-level extensions the server is ready to use.
          If the server supports multiple extensions, then the value MUST be derived from the client's handshake, specifically by selecting one or more of the values from the |Sec-WebSocket-Extensions| field.
          The absence of such a field is equivalent to the null value.
          The empty string is not the same as the null value for these purposes.
          Extensions not listed by the client MUST NOT be listed.
          The method by which these values should be selected and interpreted is discussed in Section 9.1.

   5.  If the server chooses to accept the incoming connection, it MUST reply with a valid HTTP response indicating the following.

       1.  A Status-Line with a 101 response code as per RFC 2616 [RFC2616].
           Such a response could look like "HTTP/1.1 101 Switching Protocols".

       2.  An |Upgrade| header field with value "websocket" as per RFC 2616 [RFC2616].

       3.  A |Connection| header field with value "Upgrade".

       4.  A |Sec-WebSocket-Accept| header field.  The value of this header field is constructed by concatenating /key/, defined above in step 4 in Section 4.2.2, with the string "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", taking the SHA-1 hash of this concatenated value to obtain a 20-byte value and base64-encoding (see Section 4 of [RFC4648]) this 20-byte hash.

           The ABNF [RFC2616] of this header field is defined as follows:

           Sec-WebSocket-Accept     = base64-value-non-empty
           base64-value-non-empty = (1*base64-data [ base64-padding ]) |
                                    base64-padding
           base64-data      = 4base64-character
           base64-padding   = (2base64-character "==") |
                              (3base64-character "=")
           base64-character = ALPHA | DIGIT | "+" | "/"

   NOTE: As an example, if the value of the |Sec-WebSocket-Key| header field in the client's handshake were "dGhlIHNhbXBsZSBub25jZQ==", the server would append the string "258EAFA5-E914-47DA-95CA-C5AB0DC85B11" to form the string "dGhlIHNhbXBsZSBub25jZQ==258EAFA5-E914-47DA-95CA-C5AB0DC85B11".
   The server would then take the SHA-1 hash of this string, giving the value 0xb3 0x7a 0x4f 0x2c 0xc0 0x62 0x4f 0x16 0x90 0xf6 0x46 0x06 0xcf 0x38 0x59 0x45 0xb2 0xbe 0xc4 0xea.
   This value is then base64-encoded, to give the value "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=", which would be returned in the |Sec-WebSocket-Accept| header field.

       5.  Optionally, a |Sec-WebSocket-Protocol| header field, with a value /subprotocol/ as defined in step 4 in Section 4.2.2.

       6.  Optionally, a |Sec-WebSocket-Extensions| header field, with a value /extensions/ as defined in step 4 in Section 4.2.2.
           If multiple extensions are to be used, they can all be listed in a single |Sec-WebSocket-Extensions| header field or split between multiple instances of the |Sec-WebSocket-Extensions| header field.

   This completes the server's handshake.  If the server finishes these steps without aborting the WebSocket handshake, the server considers the WebSocket connection to be established and that the WebSocket connection is in the OPEN state.
   At this point, the server may begin sending (and receiving) data.

4.3.  Collected ABNF for New Header Fields Used in Handshake

   This section is using ABNF syntax/rules from Section 2.1 of [RFC2616], including the "implied *LWS rule".

   Note that the following ABNF conventions are used in this section.
   Some names of the rules correspond to names of the corresponding header fields.
   Such rules express values of the corresponding header fields, for example, the Sec-WebSocket-Key ABNF rule describes syntax of the |Sec-WebSocket-Key| header field value.
   ABNF rules with the "-Client" suffix in the name are only used in requests sent by the client to the server; ABNF rules with the "-Server" suffix in the name are only used in responses sent by the server to the client.
   For example, the ABNF rule Sec-WebSocket-Protocol-Client describes syntax of the |Sec-WebSocket-Protocol| header field value sent by the client to the server.

   The following new header fields can be sent during the handshake from the client to the server:

      **SRS_UWS_01_119: [** Sec-WebSocket-Key = base64-value-non-empty **]**
      **SRS_UWS_01_120: [** Sec-WebSocket-Extensions = extension-list **]**
      **SRS_UWS_01_121: [** Sec-WebSocket-Protocol-Client = 1#token **]**
      **SRS_UWS_01_122: [** Sec-WebSocket-Version-Client = version **]**

      **SRS_UWS_01_123: [** base64-value-non-empty = (1*base64-data [ base64-padding ]) |
                                base64-padding
      base64-data      = 4base64-character
      base64-padding   = (2base64-character "==") |
                         (3base64-character "=")
      base64-character = ALPHA | DIGIT | "+" | "/" **]**
      **SRS_UWS_01_124: [** extension-list = 1#extension **]**
      **SRS_UWS_01_125: [** extension = extension-token *( ";" extension-param ) **]**
      **SRS_UWS_01_126: [** extension-token = registered-token **]**
      **SRS_UWS_01_127: [** registered-token = token **]**
      **SRS_UWS_01_128: [** extension-param = token [ "=" (token | quoted-string) ]
           ; When using the quoted-string syntax variant, the value
           ; after quoted-string unescaping MUST conform to the
           ; 'token' ABNF. **]**
      **SRS_UWS_01_130: [** NZDIGIT       =  "1" | "2" | "3" | "4" | "5" | "6" |
                       "7" | "8" | "9" **]**
      **SRS_UWS_01_129: [** version = DIGIT | (NZDIGIT DIGIT) |
                ("1" DIGIT DIGIT) | ("2" DIGIT DIGIT)
                ; Limited to 0-255 range, with no leading zeros **]**

   The following new header fields can be sent during the handshake from the server to the client:

      **SRS_UWS_01_131: [** Sec-WebSocket-Extensions = extension-list **]**
      **SRS_UWS_01_132: [** Sec-WebSocket-Accept     = base64-value-non-empty **]**
      **SRS_UWS_01_133: [** Sec-WebSocket-Protocol-Server = token **]**
      **SRS_UWS_01_134: [** Sec-WebSocket-Version-Server = 1#version **]**

4.4.  Supporting Multiple Versions of WebSocket Protocol

   This section provides some guidance on supporting multiple versions of the WebSocket Protocol in clients and servers.

   **SRS_UWS_01_135: [** Using the WebSocket version advertisement capability (the |Sec-WebSocket-Version| header field), a client can initially request the version of the WebSocket Protocol that it prefers (which doesn't necessarily have to be the latest supported by the client). **]**
   **SRS_UWS_01_136: [** If the server supports the requested version and the handshake message is otherwise valid, the server will accept that version. **]**
   **SRS_UWS_01_137: [** If the server doesn't support the requested version, it MUST respond with a |Sec-WebSocket-Version| header field (or multiple |Sec-WebSocket-Version| header fields) containing all versions it is willing to use. **]**
   **SRS_UWS_01_138: [** At this point, if the client supports one of the advertised versions, it can repeat the WebSocket handshake using a new version value. **]**

   The following example demonstrates version negotiation described above:

      GET /chat HTTP/1.1
      Host: server.example.com
      Upgrade: websocket
      Connection: Upgrade
      ...
      Sec-WebSocket-Version: 25

   The response from the server might look as follows:

      HTTP/1.1 400 Bad Request
      ...
      Sec-WebSocket-Version: 13, 8, 7

   Note that the last response from the server might also look like:

      HTTP/1.1 400 Bad Request
      ...
      Sec-WebSocket-Version: 13
      Sec-WebSocket-Version: 8, 7

   The client now repeats the handshake that conforms to version 13:

      GET /chat HTTP/1.1
      Host: server.example.com
      Upgrade: websocket
      Connection: Upgrade
      ...
      Sec-WebSocket-Version: 13

5.  Data Framing

5.1.  Overview

   **SRS_UWS_01_139: [** In the WebSocket Protocol, data is transmitted using a sequence of frames. **]**
   **SRS_UWS_01_140: [** To avoid confusing network intermediaries (such as intercepting proxies) and for security reasons that are further discussed in Section 10.3, a client MUST mask all frames that it sends to the server (see Section 5.3 for further details). **]**
   (Note that **SRS_UWS_01_141: [** masking is done whether or not the WebSocket Protocol is running over TLS. **]**)
   The server MUST close the connection upon receiving a frame that is not masked.
   **SRS_UWS_01_142: [** In this case, a server MAY send a Close frame with a status code of 1002 (protocol error) as defined in Section 7.4.1. **]**
   **SRS_UWS_01_143: [** A server MUST NOT mask any frames that it sends to the client. **]**
   **SRS_UWS_01_144: [** A client MUST close a connection if it detects a masked frame. **]**
   **SRS_UWS_01_145: [** In this case, it MAY use the status code 1002 (protocol error) as defined in Section 7.4.1. (These rules might be relaxed in a future specification.) **]**

   The base framing protocol defines a frame type with an opcode, a payload length, and designated locations for "Extension data" and "Application data", which together define the "Payload data".
   Certain bits and opcodes are reserved for future expansion of the protocol.

   **SRS_UWS_01_146: [** A data frame MAY be transmitted by either the client or the server at any time after opening handshake completion and before that endpoint has sent a Close frame (Section 5.5.1). **]**

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

      **SRS_UWS_01_147: [** Indicates that this is the final fragment in a message. **]**
      **SRS_UWS_01_148: [** The first fragment MAY also be the final fragment. **]**

   RSV1, RSV2, RSV3:  1 bit each

      **SRS_UWS_01_149: [** MUST be 0 unless an extension is negotiated that defines meanings for non-zero values. **]**
      **SRS_UWS_01_150: [** If a nonzero value is received and none of the negotiated extensions defines the meaning of such a nonzero value, the receiving endpoint MUST _Fail the WebSocket Connection_. **]**

   Opcode:  4 bits

      Defines the interpretation of the "Payload data".
      **SRS_UWS_01_151: [** If an unknown opcode is received, the receiving endpoint MUST _Fail the WebSocket Connection_. **]**
      The following values are defined.

      **SRS_UWS_01_152: [** *  %x0 denotes a continuation frame **]**

      **SRS_UWS_01_153: [** *  %x1 denotes a text frame **]**

      **SRS_UWS_01_154: [** *  %x2 denotes a binary frame **]**

      **SRS_UWS_01_155: [** *  %x3-7 are reserved for further non-control frames **]**

      **SRS_UWS_01_156: [** *  %x8 denotes a connection close **]**

      **SRS_UWS_01_157: [** *  %x9 denotes a ping **]**

      **SRS_UWS_01_158: [** *  %xA denotes a pong **]**

      **SRS_UWS_01_159: [** *  %xB-F are reserved for further control frames **]**

   Mask:  1 bit

      **SRS_UWS_01_160: [** Defines whether the "Payload data" is masked. **]**
      **SRS_UWS_01_161: [** If set to 1, a masking key is present in masking-key, and this is used to unmask the "Payload data" as per Section 5.3. **]**
      **SRS_UWS_01_162: [** All frames sent from client to server have this bit set to 1. **]**

   Payload length:  7 bits, 7+16 bits, or 7+64 bits

      **SRS_UWS_01_163: [** The length of the "Payload data", in bytes: **]** **SRS_UWS_01_164: [** if 0-125, that is the payload length. **]**
      **SRS_UWS_01_165: [** If 126, the following 2 bytes interpreted as a 16-bit unsigned integer are the payload length. **]**
      **SRS_UWS_01_166: [** If 127, the following 8 bytes interpreted as a 64-bit unsigned integer (the most significant bit MUST be 0) are the payload length. **]**
      **SRS_UWS_01_167: [** Multibyte length quantities are expressed in network byte order. **]**
      **SRS_UWS_01_168: [** Note that in all cases, the minimal number of bytes MUST be used to encode the length, for example, the length of a 124-byte-long string can't be encoded as the sequence 126, 0, 124. **]**
      **SRS_UWS_01_169: [** The payload length is the length of the "Extension data" + the length of the "Application data". **]**
      **SRS_UWS_01_170: [** The length of the "Extension data" may be zero, in which case the payload length is the length of the "Application data". **]**

   Masking-key:  0 or 4 bytes

      **SRS_UWS_01_171: [** All frames sent from the client to the server are masked by a 32-bit value that is contained within the frame. **]**
      **SRS_UWS_01_172: [** This field is present if the mask bit is set to 1 and is absent if the mask bit is set to 0. **]**
      See Section 5.3 for further information on client-to-server masking.

   Payload data:  (x+y) bytes

      **SRS_UWS_01_173: [** The "Payload data" is defined as "Extension data" concatenated with "Application data". **]**

   Extension data:  x bytes

      **SRS_UWS_01_174: [** The "Extension data" is 0 bytes unless an extension has been negotiated. **]**
      **SRS_UWS_01_175: [** Any extension MUST specify the length of the "Extension data", or how that length may be calculated, and how the extension use MUST be negotiated during the opening handshake. **]**
      **SRS_UWS_01_176: [** If present, the "Extension data" is included in the total payload length. **]**

   Application data:  y bytes

      **SRS_UWS_01_177: [** Arbitrary "Application data", taking up the remainder of the frame after any "Extension data". **]**
      **SRS_UWS_01_178: [** The length of the "Application data" is equal to the payload length minus the length of the "Extension data". **]**

   The base framing protocol is formally defined by the following ABNF [RFC5234].
   It is important to note that the representation of this data is binary, not ASCII characters.
   As such, a field with a length of 1 bit that takes values %x0 / %x1 is represented as a single bit whose value is 0 or 1, not a full byte (octet) that stands for the characters "0" or "1" in the ASCII encoding.
   A field with a length of 4 bits with values between %x0-F again is represented by 4 bits, again NOT by an ASCII character or full byte (octet) with these values.
   [RFC5234] does not specify a character encoding: "Rules resolve into a string of terminal values, sometimes called characters. In ABNF, a character is merely a non-negative integer. In certain contexts, a specific mapping (encoding) of values into a character set (such as ASCII) will be specified."
   Here, the specified encoding is a binary encoding where each terminal value is encoded in the specified number of bits, which varies for each field.

**SRS_UWS_01_179: [**     ws-frame                = frame-fin           ; 1 bit in length
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

    **SRS_UWS_01_180: [** frame-fin               = %x0 ; more frames of this message follow **]**
                            **SRS_UWS_01_181: [** / %x1 ; final frame of this message **]**
                                  ; 1 bit in length

    **SRS_UWS_01_182: [** frame-rsv1              = %x0 / %x1
                              ; 1 bit in length, MUST be 0 unless
                              ; negotiated otherwise **]**

    **SRS_UWS_01_183: [** frame-rsv2              = %x0 / %x1
                              ; 1 bit in length, MUST be 0 unless
                              ; negotiated otherwise **]**

    **SRS_UWS_01_184: [** frame-rsv3              = %x0 / %x1
                              ; 1 bit in length, MUST be 0 unless
                              ; negotiated otherwise **]**

    **SRS_UWS_01_185: [** frame-opcode            = frame-opcode-non-control /
                              frame-opcode-control /
                              frame-opcode-cont **]**

    **SRS_UWS_01_186: [** frame-opcode-cont       = %x0 ; frame continuation **]**

    **SRS_UWS_01_187: [** frame-opcode-non-control= %x1 ; text frame
                            / %x2 ; binary frame
                            / %x3-7
                            ; 4 bits in length,
                            ; reserved for further non-control frames **]**

    **SRS_UWS_01_188: [** frame-opcode-control    = %x8 ; connection close
                            / %x9 ; ping
                            / %xA ; pong
                            / %xB-F ; reserved for further control
                                    ; frames
                                    ; 4 bits in length **]**

    **SRS_UWS_01_189: [** frame-masked            = %x0
                            ; frame is not masked, no frame-masking-key
                            / %x1
                            ; frame is masked, frame-masking-key present
                            ; 1 bit in length **]**

    **SRS_UWS_01_190: [** frame-payload-length    = ( %x00-7D )
                            / ( %x7E frame-payload-length-16 )
                            / ( %x7F frame-payload-length-63 )
                            ; 7, 7+16, or 7+64 bits in length,
                            ; respectively **]**

    **SRS_UWS_01_191: [** frame-payload-length-16 = %x0000-FFFF ; 16 bits in length **]**

    **SRS_UWS_01_192: [** frame-payload-length-63 = %x0000000000000000-7FFFFFFFFFFFFFFF
                            ; 64 bits in length **]**

    **SRS_UWS_01_193: [** frame-masking-key       = 4( %x00-FF )
                              ; present only if frame-masked is 1
                              ; 32 bits in length **]**

    **SRS_UWS_01_194: [** frame-payload-data      = (frame-masked-extension-data
                               frame-masked-application-data)
                            ; when frame-masked is 1
                              / (frame-unmasked-extension-data
                                frame-unmasked-application-data)
                            ; when frame-masked is 0 **]**

    **SRS_UWS_01_195: [** frame-masked-extension-data     = *( %x00-FF )
                            ; reserved for future extensibility
                            ; n*8 bits in length, where n >= 0 **]**

    **SRS_UWS_01_196: [** frame-masked-application-data   = *( %x00-FF )
                            ; n*8 bits in length, where n >= 0 **]**

    **SRS_UWS_01_197: [** frame-unmasked-extension-data   = *( %x00-FF )
                            ; reserved for future extensibility
                            ; n*8 bits in length, where n >= 0 **]**

    **SRS_UWS_01_198: [** frame-unmasked-application-data = *( %x00-FF )
                            ; n*8 bits in length, where n >= 0 **]**

5.3.  Client-to-Server Masking

   **SRS_UWS_01_199: [** A masked frame MUST have the field frame-masked set to 1, as defined in Section 5.2. **]**

   **SRS_UWS_01_200: [** The masking key is contained completely within the frame, as defined in Section 5.2 as frame-masking-key. **]**
   **SRS_UWS_01_201: [** It is used to mask the "Payload data" defined in the same section as frame-payload-data, which includes "Extension data" and "Application data". **]**

   **SRS_UWS_01_202: [** The masking key is a 32-bit value chosen at random by the client. **]**
   **SRS_UWS_01_203: [** When preparing a masked frame, the client MUST pick a fresh masking key from the set of allowed 32-bit values. **]**
   **SRS_UWS_01_204: [** The masking key needs to be unpredictable; thus, the masking key MUST be derived from a strong source of entropy, and the masking key for a given frame MUST NOT make it simple for a server/proxy to predict the masking key for a subsequent frame. **]**
   The unpredictability of the masking key is essential to prevent authors of malicious applications from selecting the bytes that appear on the wire.
   RFC 4086 [RFC4086] discusses what entails a suitable source of entropy for security-sensitive applications.

   The masking does not affect the length of the "Payload data".
   **SRS_UWS_01_205: [** To convert masked data into unmasked data, or vice versa, the following algorithm is applied. **]**
   **SRS_UWS_01_206: [** The same algorithm applies regardless of the direction of the translation, e.g., the same steps are applied to mask the data as to unmask the data. **]**

   **SRS_UWS_01_207: [** Octet i of the transformed data ("transformed-octet-i") is the XOR of octet i of the original data ("original-octet-i") with octet at index i modulo 4 of the masking key ("masking-key-octet-j"): **]**

     j                   = i MOD 4
     transformed-octet-i = original-octet-i XOR masking-key-octet-j

   **SRS_UWS_01_208: [** The payload length, indicated in the framing as frame-payload-length, does NOT include the length of the masking key. **]**
   It is the length of the "Payload data", e.g., the number of bytes following the masking key.

5.4.  Fragmentation

   The primary purpose of fragmentation is to allow sending a message that is of unknown size when the message is started without having to buffer that message.
   If messages couldn't be fragmented, then an endpoint would have to buffer the entire message so its length could be counted before the first byte is sent.
   With fragmentation, a server or intermediary may choose a reasonable size buffer and, when the buffer is full, write a fragment to the network.

   A secondary use-case for fragmentation is for multiplexing, where it is not desirable for a large message on one logical channel to monopolize the output channel, so the multiplexing needs to be free to split the message into smaller fragments to better share the output channel.
   (Note that the multiplexing extension is not described in this document.)

   Unless specified otherwise by an extension, frames have no semantic meaning.
   **SRS_UWS_01_209: [** An intermediary might coalesce and/or split frames, if no extensions were negotiated by the client and the server **]** or **SRS_UWS_01_210: [** if some extensions were negotiated, but the intermediary understood all the extensions negotiated and knows how to coalesce and/or split frames in the presence of these extensions. **]**
   **SRS_UWS_01_211: [** One implication of this is that in absence of extensions, senders and receivers must not depend on the presence of specific frame boundaries. **]**

   The following rules apply to fragmentation:

   o  **SRS_UWS_01_212: [** An unfragmented message consists of a single frame with the FIN bit set (Section 5.2) and an opcode other than 0. **]**

   o  **SRS_UWS_01_213: [** A fragmented message consists of a single frame with the FIN bit clear and an opcode other than 0, followed by zero or more frames with the FIN bit clear and the opcode set to 0, and terminated by a single frame with the FIN bit set and an opcode of 0. **]**
      A fragmented message is conceptually equivalent to a single larger message whose payload is equal to the concatenation of the payloads of the fragments in order; however, in the presence of extensions, this may not hold true as the extension defines the interpretation of the "Extension data" present.
      For instance, "Extension data" may only be present at the beginning of the first fragment and apply to subsequent fragments, or there may be "Extension data" present in each of the fragments that applies only to that particular fragment.
      In the absence of "Extension data", the following example demonstrates how fragmentation works.

      EXAMPLE: For a text message sent as three fragments, the first fragment would have an opcode of 0x1 and a FIN bit clear, the second fragment would have an opcode of 0x0 and a FIN bit clear, and the third fragment would have an opcode of 0x0 and a FIN bit that is set.

   o  **SRS_UWS_01_214: [** Control frames (see Section 5.5) MAY be injected in the middle of a fragmented message. **]**
      **SRS_UWS_01_215: [** Control frames themselves MUST NOT be fragmented. **]**

   o  **SRS_UWS_01_216: [** Message fragments MUST be delivered to the recipient in the order sent by the sender. **]**

   o  **SRS_UWS_01_217: [** The fragments of one message MUST NOT be interleaved between the fragments of another message unless an extension has been negotiated that can interpret the interleaving. **]**

   o  **SRS_UWS_01_218: [** An endpoint MUST be capable of handling control frames in the middle of a fragmented message. **]**

   o  **SRS_UWS_01_219: [** A sender MAY create fragments of any size for non-control messages. **]**

   o  **SRS_UWS_01_220: [** Clients and servers MUST support receiving both fragmented and unfragmented messages. **]**

   o  **SRS_UWS_01_221: [** As control frames cannot be fragmented, an intermediary MUST NOT attempt to change the fragmentation of a control frame. **]**

   o  **SRS_UWS_01_222: [** An intermediary MUST NOT change the fragmentation of a message if any reserved bit values are used and the meaning of these values is not known to the intermediary. **]**

   o  **SRS_UWS_01_223: [** An intermediary MUST NOT change the fragmentation of any message in the context of a connection where extensions have been negotiated and the intermediary is not aware of the semantics of the negotiated extensions. **]**
      **SRS_UWS_01_224: [** Similarly, an intermediary that didn't see the WebSocket handshake (and wasn't notified about its content) that resulted in a WebSocket connection MUST NOT change the fragmentation of any message of such connection. **]**

   o  **SRS_UWS_01_225: [** As a consequence of these rules, all fragments of a message are of the same type, as set by the first fragment's opcode. **]**
      **SRS_UWS_01_226: [** Since control frames cannot be fragmented, the type for all fragments in a message MUST be either text, binary, or one of the reserved opcodes. **]**

   NOTE: If control frames could not be interjected, the latency of a ping, for example, would be very long if behind a large message.
   Hence, the requirement of handling control frames in the middle of a fragmented message.

   IMPLEMENTATION NOTE: **SRS_UWS_01_227: [** In the absence of any extension, a receiver doesn't have to buffer the whole frame in order to process it. **]**
   For example, if a streaming API is used, a part of a frame can be delivered to the application.
   However, note that this assumption might not hold true for all future WebSocket extensions.

5.5.  Control Frames

   **SRS_UWS_01_228: [** Control frames are identified by opcodes where the most significant bit of the opcode is 1. **]**
   **SRS_UWS_01_229: [** Currently defined opcodes for control frames include 0x8 (Close), 0x9 (Ping), and 0xA (Pong). **]**
   **SRS_UWS_01_230: [** Opcodes 0xB-0xF are reserved for further control frames yet to be defined. **]**

   **SRS_UWS_01_231: [** Control frames are used to communicate state about the WebSocket. **]**
   **SRS_UWS_01_232: [** Control frames can be interjected in the middle of a fragmented message. **]**

   **SRS_UWS_01_233: [** All control frames MUST have a payload length of 125 bytes or less and MUST NOT be fragmented. **]**

5.5.1.  Close

   **SRS_UWS_01_234: [** The Close frame contains an opcode of 0x8. **]**

   **SRS_UWS_01_235: [** The Close frame MAY contain a body (the "Application data" portion of the frame) that indicates a reason for closing, such as an endpoint shutting down, an endpoint having received a frame too large, or an endpoint having received a frame that does not conform to the format expected by the endpoint. **]**
   **SRS_UWS_01_236: [** If there is a body, the first two bytes of the body MUST be a 2-byte unsigned integer (in network byte order) representing a status code with value /code/ defined in Section 7.4. **]**
   **SRS_UWS_01_237: [** Following the 2-byte integer, the body MAY contain UTF-8-encoded data with value /reason/, the interpretation of which is not defined by this specification. **]**
   This data is not necessarily human readable but may be useful for debugging or passing information relevant to the script that opened the connection.
   **SRS_UWS_01_238: [** As the data is not guaranteed to be human readable, clients MUST NOT show it to end users. **]**

   **SRS_UWS_01_239: [** Close frames sent from client to server must be masked as per Section 5.3. **]**

   **SRS_UWS_01_240: [** The application MUST NOT send any more data frames after sending a Close frame. **]**

   **SRS_UWS_01_241: [** If an endpoint receives a Close frame and did not previously send a Close frame, the endpoint MUST send a Close frame in response. **]**
   (When sending a Close frame in response, the endpoint typically echos the status code it received.)
   **SRS_UWS_01_242: [** It SHOULD do so as soon as practical. **]**
   **SRS_UWS_01_243: [** An endpoint MAY delay sending a Close frame until its current message is sent (for instance, if the majority of a fragmented message is already sent, an endpoint MAY send the remaining fragments before sending a Close frame). **]**
   However, there is no guarantee that the endpoint that has already sent a Close frame will continue to process data.

   **SRS_UWS_01_244: [** After both sending and receiving a Close message, an endpoint considers the WebSocket connection closed and MUST close the underlying TCP connection. **]**
   The server MUST close the underlying TCP connection immediately; **SRS_UWS_01_245: [** the client SHOULD wait for the server to close the connection but MAY close the connection at any time after sending and receiving a Close message **]**, e.g., if it has not received a TCP Close from the server in a reasonable time period.

   **SRS_UWS_01_246: [** If a client and server both send a Close message at the same time, both endpoints will have sent and received a Close message and should consider the WebSocket connection closed and close the underlying TCP connection. **]**

5.5.2.  Ping

   **SRS_UWS_01_247: [** The Ping frame contains an opcode of 0x9. **]**

   **SRS_UWS_01_248: [** A Ping frame MAY include "Application data". **]**

   **SRS_UWS_01_249: [** Upon receipt of a Ping frame, an endpoint MUST send a Pong frame in response, unless it already received a Close frame. **]**
   **SRS_UWS_01_250: [** It SHOULD respond with Pong frame as soon as is practical. **]**
   Pong frames are discussed in Section 5.5.3.

   **SRS_UWS_01_251: [** An endpoint MAY send a Ping frame any time after the connection is established and before the connection is closed. **]**

   NOTE: A Ping frame may serve either as a keepalive or as a means to verify that the remote endpoint is still responsive.

5.5.3.  Pong

   **SRS_UWS_01_252: [** The Pong frame contains an opcode of 0xA. **]**

   Section 5.5.2 details requirements that apply to both Ping and Pong frames.

   **SRS_UWS_01_253: [** A Pong frame sent in response to a Ping frame must have identical "Application data" as found in the message body of the Ping frame being replied to. **]**

   **SRS_UWS_01_254: [** If an endpoint receives a Ping frame and has not yet sent Pong frame(s) in response to previous Ping frame(s), the endpoint MAY elect to send a Pong frame for only the most recently processed Ping frame. **]**

   **SRS_UWS_01_255: [** A Pong frame MAY be sent unsolicited. **]**
   This serves as a unidirectional heartbeat.
   **SRS_UWS_01_256: [** A response to an unsolicited Pong frame is not expected. **]**

5.6.  Data Frames

   **SRS_UWS_01_257: [** Data frames (e.g., non-control frames) are identified by opcodes where the most significant bit of the opcode is 0. **]**
   **SRS_UWS_01_258: [** Currently defined opcodes for data frames include 0x1 (Text), 0x2 (Binary). **]**
   **SRS_UWS_01_259: [** Opcodes 0x3-0x7 are reserved for further non-control frames yet to be defined. **]**

   Data frames carry application-layer and/or extension-layer data.  **SRS_UWS_01_260: [** The opcode determines the interpretation of the data: **]**

   Text

      **SRS_UWS_01_261: [** The "Payload data" is text data encoded as UTF-8. **]**  Note that **SRS_UWS_01_262: [** a particular text frame might include a partial UTF-8 sequence; however, the whole message MUST contain valid UTF-8. **]**
      **SRS_UWS_01_263: [** Invalid UTF-8 in reassembled messages is handled as described in Section 8.1. **]**

   Binary

      **SRS_UWS_01_264: [** The "Payload data" is arbitrary binary data whose interpretation is solely up to the application layer. **]**

5.7.  Examples

   o  A single-frame unmasked text message

      *  0x81 0x05 0x48 0x65 0x6c 0x6c 0x6f (contains "Hello")

   o  A single-frame masked text message

      *  0x81 0x85 0x37 0xfa 0x21 0x3d 0x7f 0x9f 0x4d 0x51 0x58 (contains "Hello")

   o  A fragmented unmasked text message

      *  0x01 0x03 0x48 0x65 0x6c (contains "Hel")

      *  0x80 0x02 0x6c 0x6f (contains "lo")

   o  Unmasked Ping request and masked Ping response

      *  0x89 0x05 0x48 0x65 0x6c 0x6c 0x6f (contains a body of "Hello", but the contents of the body are arbitrary)

      *  0x8a 0x85 0x37 0xfa 0x21 0x3d 0x7f 0x9f 0x4d 0x51 0x58 (contains a body of "Hello", matching the body of the ping)

   o  256 bytes binary message in a single unmasked frame

      *  0x82 0x7E 0x0100 [256 bytes of binary data]

   o  64KiB binary message in a single unmasked frame

      *  0x82 0x7F 0x0000000000010000 [65536 bytes of binary data]

5.8.  Extensibility

   The protocol is designed to allow for extensions, which will add capabilities to the base protocol.
   **SRS_UWS_01_265: [** The endpoints of a connection MUST negotiate the use of any extensions during the opening handshake. **]**
   **SRS_UWS_01_266: [** This specification provides opcodes 0x3 through 0x7 and 0xB through 0xF, the "Extension data" field, and the frame-rsv1, frame-rsv2, and frame-rsv3 bits of the frame header for use by extensions. **]**
   The negotiation of extensions is discussed in further detail in Section 9.1.
   Below are some anticipated uses of extensions.
   This list is neither complete nor prescriptive.

   o  "Extension data" may be placed in the "Payload data" before the "Application data".

   o  Reserved bits can be allocated for per-frame needs.

   o  Reserved opcode values can be defined.

   o  Reserved bits can be allocated to the opcode field if more opcode values are needed.

   o  A reserved bit or an "extension" opcode can be defined that allocates additional bits out of the "Payload data" to define larger opcodes or more per-frame bits.

6.  Sending and Receiving Data

6.1.  Sending Data

   **SRS_UWS_01_267: [** To _Send a WebSocket Message_ comprising of /data/ over a WebSocket connection, an endpoint MUST perform the following steps. **]**

   1.  **SRS_UWS_01_268: [** The endpoint MUST ensure the WebSocket connection is in the OPEN state **]** (cf. Sections 4.1 and 4.2.2.)
       **SRS_UWS_01_269: [** If at any point the state of the WebSocket connection changes, the endpoint MUST abort the following steps. **]**

   2.  **SRS_UWS_01_270: [** An endpoint MUST encapsulate the /data/ in a WebSocket frame as defined in Section 5.2. **]**
       **SRS_UWS_01_271: [** If the data to be sent is large or if the data is not available in its entirety at the point the endpoint wishes to begin sending the data, the endpoint MAY alternately encapsulate the data in a series of frames as defined in Section 5.4. **]**

   3.  **SRS_UWS_01_272: [** The opcode (frame-opcode) of the first frame containing the data MUST be set to the appropriate value from Section 5.2 for data that is to be interpreted by the recipient as text or binary data. **]**

   4.  **SRS_UWS_01_273: [** The FIN bit (frame-fin) of the last frame containing the data MUST be set to 1 as defined in Section 5.2. **]**

   5.  **SRS_UWS_01_274: [** If the data is being sent by the client, the frame(s) MUST be masked as defined in Section 5.3. **]**

   6.  **SRS_UWS_01_275: [** If any extensions (Section 9) have been negotiated for the WebSocket connection, additional considerations may apply as per the definition of those extensions. **]**

   7.  **SRS_UWS_01_276: [** The frame(s) that have been formed MUST be transmitted over the underlying network connection. **]**

6.2.  Receiving Data

   **SRS_UWS_01_277: [** To receive WebSocket data, an endpoint listens on the underlying network connection. **]**
   **SRS_UWS_01_278: [** Incoming data MUST be parsed as WebSocket frames as defined in Section 5.2. **]**
   **SRS_UWS_01_279: [** If a control frame (Section 5.5) is received, the frame MUST be handled as defined by Section 5.5. **]**
   **SRS_UWS_01_280: [** Upon receiving a data frame (Section 5.6), the endpoint MUST note the /type/ of the data as defined by the opcode (frame-opcode) from Section 5.2. **]**
   **SRS_UWS_01_281: [** The "Application data" from this frame is defined as the /data/ of the message. **]**
   **SRS_UWS_01_282: [** If the frame comprises an unfragmented message (Section 5.4), it is said that _A WebSocket Message Has Been Received_ with type /type/ and data /data/. **]**
   **SRS_UWS_01_283: [** If the frame is part of a fragmented message, the "Application data" of the subsequent data frames is concatenated to form the /data/. **]**
   **SRS_UWS_01_284: [** When the last fragment is received as indicated by the FIN bit (frame-fin), it is said that _A WebSocket Message Has Been Received_ with data /data/ (comprised of the concatenation of the "Application data" of the fragments) and type /type/ (noted from the first frame of the fragmented message). **]**
   **SRS_UWS_01_285: [** Subsequent data frames MUST be interpreted as belonging to a new WebSocket message. **]**

   **SRS_UWS_01_286: [** Extensions (Section 9) MAY change the semantics of how data is read, specifically including what comprises a message boundary. **]**
   **SRS_UWS_01_287: [** Extensions, in addition to adding "Extension data" before the "Application data" in a payload, MAY also modify the "Application data" (such as by compressing it). **]**

   A server MUST remove masking for data frames received from a client as described in Section 5.3.

7.  Closing the Connection

7.1.  Definitions

7.1.1.  Close the WebSocket Connection

   **SRS_UWS_01_288: [** To _Close the WebSocket Connection_, an endpoint closes the underlying TCP connection. **]**
   **SRS_UWS_01_289: [** An endpoint SHOULD use a method that cleanly closes the TCP connection, as well as the TLS session, if applicable, discarding any trailing bytes that may have been received. **]**
   **SRS_UWS_01_290: [** An endpoint MAY close the connection via any means available when necessary, such as when under attack. **]**

   **SRS_UWS_01_291: [** The underlying TCP connection, in most normal cases, SHOULD be closed first by the server **]**, so that it holds the TIME_WAIT state and not the client (as this would prevent it from re-opening the connection for 2 maximum segment lifetimes (2MSL), while there is no corresponding server impact as a TIME_WAIT connection is immediately reopened upon a new SYN with a higher seq number).
   **SRS_UWS_01_292: [** In abnormal cases (such as not having received a TCP Close from the server after a reasonable amount of time) a client MAY initiate the TCP Close. **]**
   As such, when a server is instructed to _Close the WebSocket Connection_ it SHOULD initiate a TCP Close immediately, and **SRS_UWS_01_293: [** when a client is instructed to do the same, it SHOULD wait for a TCP Close from the server. **]**

   As an example of how to obtain a clean closure in C using Berkeley sockets, one would call shutdown() with SHUT_WR on the socket, call recv() until obtaining a return value of 0 indicating that the peer has also performed an orderly shutdown, and finally call close() on the socket.

7.1.2.  Start the WebSocket Closing Handshake

   **SRS_UWS_01_294: [** To _Start the WebSocket Closing Handshake_ with a status code (Section 7.4) /code/ and an optional close reason (Section 7.1.6) /reason/, an endpoint MUST send a Close control frame, as described in Section 5.5.1, whose status code is set to /code/ and whose close reason is set to /reason/. **]**
   **SRS_UWS_01_295: [** Once an endpoint has both sent and received a Close control frame, that endpoint SHOULD _Close the WebSocket Connection_ as defined in Section 7.1.1. **]**

7.1.3.  The WebSocket Closing Handshake is Started

   **SRS_UWS_01_296: [** Upon either sending or receiving a Close control frame, it is said that _The WebSocket Closing Handshake is Started_ and that the WebSocket connection is in the CLOSING state. **]**

7.1.4.  The WebSocket Connection is Closed

   **SRS_UWS_01_297: [** When the underlying TCP connection is closed, it is said that _The WebSocket Connection is Closed_ and that the WebSocket connection is in the CLOSED state. **]**
   **SRS_UWS_01_298: [** If the TCP connection was closed after the WebSocket closing handshake was completed, the WebSocket connection is said to have been closed _cleanly_. **]**

   **SRS_UWS_01_299: [** If the WebSocket connection could not be established, it is also said that _The WebSocket Connection is Closed_, but not _cleanly_. **]**

7.1.5.  The WebSocket Connection Close Code

   **SRS_UWS_01_300: [** As defined in Sections 5.5.1 and 7.4, a Close control frame may contain a status code indicating a reason for closure. **]**
   **SRS_UWS_01_301: [** A closing of the WebSocket connection may be initiated by either endpoint, potentially simultaneously. **]**
   **SRS_UWS_01_302: [** _The WebSocket Connection Close Code_ is defined as the status code (Section 7.4) contained in the first Close control frame received by the application implementing this protocol. **]**
   **SRS_UWS_01_303: [** If this Close control frame contains no status code, _The WebSocket Connection Close Code_ is considered to be 1005. **]**
   **SRS_UWS_01_304: [** If _The WebSocket Connection is Closed_ and no Close control frame was received by the endpoint (such as could occur if the underlying transport connection is lost), _The WebSocket Connection Close Code_ is considered to be 1006. **]**

   NOTE: Two endpoints may not agree on the value of _The WebSocket Connection Close Code_.
   As an example, if the remote endpoint sent a Close frame but the local application has not yet read the data containing the Close frame from its socket's receive buffer, and the local application independently decided to close the connection and send a Close frame, both endpoints will have sent and received a Close frame and will not send further Close frames.
   Each endpoint will see the status code sent by the other end as _The WebSocket Connection Close Code_.
   As such, it is possible that the two endpoints may not agree on the value of _The WebSocket Connection Close Code_ in the case that both endpoints _Start the WebSocket Closing Handshake_ independently and at roughly the same time.

7.1.6.  The WebSocket Connection Close Reason

   **SRS_UWS_01_305: [** As defined in Sections 5.5.1 and 7.4, a Close control frame may contain a status code indicating a reason for closure, followed by UTF-8-encoded data, the interpretation of said data being left to the endpoints and not defined by this protocol. **]**
   **SRS_UWS_01_308: [** A closing of the WebSocket connection may be initiated by either endpoint, potentially simultaneously. **]**
   **SRS_UWS_01_307: [** _The WebSocket Connection Close Reason_ is defined as the UTF-8-encoded data following the status code (Section 7.4) contained in the first Close control frame received by the application implementing this protocol. **]**
   **SRS_UWS_01_306: [** If there is no such data in the Close control frame, _The WebSocket Connection Close Reason_ is the empty string. **]**

   NOTE: Following the same logic as noted in Section 7.1.5, two endpoints may not agree on _The WebSocket Connection Close Reason_.

7.1.7.  Fail the WebSocket Connection

   Certain algorithms and specifications require an endpoint to _Fail the WebSocket Connection_.
   **SRS_UWS_01_309: [** To do so, the client MUST _Close the WebSocket Connection_, and MAY report the problem to the user (which would be especially useful for developers) in an appropriate manner. **]**
   Similarly, to do so, the server MUST _Close the WebSocket Connection_, and SHOULD log the problem.

   **SRS_UWS_01_310: [** If _The WebSocket Connection is Established_ prior to the point where the endpoint is required to _Fail the WebSocket Connection_, the endpoint SHOULD send a Close frame with an appropriate status code (Section 7.4) before proceeding to _Close the WebSocket Connection_. **]**
   **SRS_UWS_01_311: [** An endpoint MAY omit sending a Close frame if it believes the other side is unlikely to be able to receive and process the Close frame, due to the nature of the error that led the WebSocket connection to fail in the first place. **]**
   **SRS_UWS_01_312: [** An endpoint MUST NOT continue to attempt to process data (including a responding Close frame) from the remote endpoint after being instructed to _Fail the WebSocket Connection_. **]**

   Except as indicated above or as specified by the application layer (e.g., a script using the WebSocket API), clients SHOULD NOT close the connection.

7.2.  Abnormal Closures

7.2.1.  Client-Initiated Closure

   **SRS_UWS_01_313: [** Certain algorithms, in particular during the opening handshake, require the client to _Fail the WebSocket Connection_. **]**
   **SRS_UWS_01_314: [** To do so, the client MUST _Fail the WebSocket Connection_ as defined in Section 7.1.7. **]**

   **SRS_UWS_01_315: [** If at any point the underlying transport layer connection is unexpectedly lost, the client MUST _Fail the WebSocket Connection_. **]**

   **SRS_UWS_01_316: [** Except as indicated above or as specified by the application layer (e.g., a script using the WebSocket API), clients SHOULD NOT close the connection. **]**

7.2.2.  Server-Initiated Closure

   Certain algorithms require or recommend that the server _Abort the WebSocket Connection_ during the opening handshake. To do so, the server MUST simply _Close the WebSocket Connection_ (Section 7.1.1).

7.2.3.  Recovering from Abnormal Closure

   Abnormal closures may be caused by any number of reasons.
   Such closures could be the result of a transient error, in which case reconnecting may lead to a good connection and a resumption of normal operations.
   Such closures may also be the result of a nontransient problem, in which case if each deployed client experiences an abnormal closure and immediately and persistently tries to reconnect, the server may experience what amounts to a denial-of-service attack by a large number of clients trying to reconnect.
   The end result of such a scenario could be that the service is unable to recover in a timely manner or recovery is made much more difficult.

   To prevent this, clients SHOULD use some form of backoff when trying to reconnect after abnormal closures as described in this section.

   The first reconnect attempt SHOULD be delayed by a random amount of time.
   The parameters by which this random delay is chosen are left to the client to decide; a value chosen randomly between 0 and 5 seconds is a reasonable initial delay though clients MAY choose a different interval from which to select a delay length based on implementation experience and particular application.

   Should the first reconnect attempt fail, subsequent reconnect attempts SHOULD be delayed by increasingly longer amounts of time, using a method such as truncated binary exponential backoff.

7.3.  Normal Closure of Connections

   **SRS_UWS_01_318: [** Servers MAY close the WebSocket connection whenever desired. **]**
   **SRS_UWS_01_317: [** Clients SHOULD NOT close the WebSocket connection arbitrarily. **]**
   **SRS_UWS_01_319: [** In either case, an endpoint initiates a closure by following the procedures to _Start the WebSocket Closing Handshake_ (Section 7.1.2). **]**

7.4.  Status Codes

   **SRS_UWS_01_320: [** When closing an established connection (e.g., when sending a Close frame, after the opening handshake has completed), an endpoint MAY indicate a reason for closure. **]**
   **SRS_UWS_01_321: [** The interpretation of this reason by an endpoint, and the action an endpoint should take given this reason, are left undefined by this specification. **]**
   **SRS_UWS_01_322: [** This specification defines a set of pre-defined status codes and specifies which ranges may be used by extensions, frameworks, and end applications. **]**
   **SRS_UWS_01_323: [** The status code and any associated textual message are optional components of a Close frame. **]**

7.4.1.  Defined Status Codes

   Endpoints MAY use the following pre-defined status codes when sending a Close frame.

   1000

      **SRS_UWS_01_324: [** 1000 indicates a normal closure, meaning that the purpose for which the connection was established has been fulfilled. **]**

   1001

      **SRS_UWS_01_325: [** 1001 indicates that an endpoint is "going away", such as a server going down or a browser having navigated away from a page. **]**

   1002

      **SRS_UWS_01_326: [** 1002 indicates that an endpoint is terminating the connection due to a protocol error. **]**

   1003

      **SRS_UWS_01_327: [** 1003 indicates that an endpoint is terminating the connection because it has received a type of data it cannot accept (e.g., an endpoint that understands only text data MAY send this if it receives a binary message). **]**

   1004

      **SRS_UWS_01_328: [** Reserved.  The specific meaning might be defined in the future. **]**

   1005

      **SRS_UWS_01_329: [** 1005 is a reserved value and MUST NOT be set as a status code in a Close control frame by an endpoint. **]**
      It is designated for use in applications expecting a status code to indicate that no status code was actually present.

   1006

      **SRS_UWS_01_330: [** 1006 is a reserved value and MUST NOT be set as a status code in a Close control frame by an endpoint. **]**
      It is designated for use in applications expecting a status code to indicate that the connection was closed abnormally, e.g., without sending or receiving a Close control frame.

   1007

      **SRS_UWS_01_331: [** 1007 indicates that an endpoint is terminating the connection because it has received data within a message that was not consistent with the type of the message (e.g., non-UTF-8 [RFC3629] data within a text message). **]**

   1008

      **SRS_UWS_01_332: [** 1008 indicates that an endpoint is terminating the connection because it has received a message that violates its policy. **]**
      This is a generic status code that can be returned when there is no other more suitable status code (e.g., 1003 or 1009) or if there is a need to hide specific details about the policy.

   1009

      **SRS_UWS_01_333: [** 1009 indicates that an endpoint is terminating the connection because it has received a message that is too big for it to process. **]**

   1010

      **SRS_UWS_01_334: [** 1010 indicates that an endpoint (client) is terminating the connection because it has expected the server to negotiate one or more extension, but the server didn't return them in the response message of the WebSocket handshake. **]**
      **SRS_UWS_01_335: [** The list of extensions that are needed SHOULD appear in the /reason/ part of the Close frame. **]**
      Note that this status code is not used by the server, because it can fail the WebSocket handshake instead.

   1011

      **SRS_UWS_01_336: [** 1011 indicates that a server is terminating the connection because it encountered an unexpected condition that prevented it from fulfilling the request. **]**

   1015

      **SRS_UWS_01_337: [** 1015 is a reserved value and MUST NOT be set as a status code in a Close control frame by an endpoint. **]**
      It is designated for use in applications expecting a status code to indicate that the connection was closed due to a failure to perform a TLS handshake (e.g., the server certificate can't be verified).

7.4.2.  Reserved Status Code Ranges

   0-999

      **SRS_UWS_01_338: [** Status codes in the range 0-999 are not used. **]**

   1000-2999

      **SRS_UWS_01_339: [** Status codes in the range 1000-2999 are reserved for definition by this protocol, its future revisions, and extensions specified in a permanent and readily available public specification. **]**

   3000-3999

      **SRS_UWS_01_340: [** Status codes in the range 3000-3999 are reserved for use by libraries, frameworks, and applications. **]**
      These status codes are registered directly with IANA.
      The interpretation of these codes is undefined by this protocol.

   4000-4999

      **SRS_UWS_01_341: [** Status codes in the range 4000-4999 are reserved for private use and thus can't be registered. **]**
      Such codes can be used by prior agreements between WebSocket applications.
      The interpretation of these codes is undefined by this protocol.

8.  Error Handling

8.1.  Handling Errors in UTF-8-Encoded Data

   **SRS_UWS_01_342: [** When an endpoint is to interpret a byte stream as UTF-8 but finds that the byte stream is not, in fact, a valid UTF-8 stream, that endpoint MUST _Fail the WebSocket Connection_. **]**
   **SRS_UWS_01_343: [** This rule applies both during the opening handshake and during subsequent data exchange. **]**

9.  Extensions

   **SRS_UWS_01_344: [** WebSocket clients MAY request extensions to this specification, and WebSocket servers MAY accept some or all extensions requested by the client. **]**
   **SRS_UWS_01_345: [** A server MUST NOT respond with any extension not requested by the client. **]**
   **SRS_UWS_01_346: [** If extension parameters are included in negotiations between the client and the server, those parameters MUST be chosen in accordance with the specification of the extension to which the parameters apply. **]**

9.1.  Negotiating Extensions

   **SRS_UWS_01_347: [** A client requests extensions by including a |Sec-WebSocket-Extensions| header field, which follows the normal rules for HTTP header fields (see [RFC2616], Section 4.2) and the value of the header field is defined by the following ABNF [RFC2616]. **]**
   Note that this section is using ABNF syntax/rules from [RFC2616], including the "implied *LWS rule".
   **SRS_UWS_01_348: [** If a value is received by either the client or the server during negotiation that does not conform to the ABNF below, the recipient of such malformed data MUST immediately _Fail the WebSocket Connection_. **]**

         Sec-WebSocket-Extensions = extension-list
         extension-list = 1#extension
         extension = extension-token *( ";" extension-param )
         extension-token = registered-token
         registered-token = token
         extension-param = token [ "=" (token | quoted-string) ]
             ;When using the quoted-string syntax variant, the value
             ;after quoted-string unescaping MUST conform to the
             ;'token' ABNF.

   **SRS_UWS_01_349: [** Note that like other HTTP header fields, this header field MAY be split or combined across multiple lines. **]**
   Ergo, the following are equivalent:

         Sec-WebSocket-Extensions: foo
         Sec-WebSocket-Extensions: bar; baz=2

   is exactly equivalent to

         Sec-WebSocket-Extensions: foo, bar; baz=2

   **SRS_UWS_01_350: [** Any extension-token used MUST be a registered token (see Section 11.4). **]**
   **SRS_UWS_01_351: [** The parameters supplied with any given extension MUST be defined for that extension. **]**
   **SRS_UWS_01_352: [** Note that the client is only offering to use any advertised extensions and MUST NOT use them unless the server indicates that it wishes to use the extension. **]**

   **SRS_UWS_01_353: [** Note that the order of extensions is significant. **]**
   Any interactions between multiple extensions MAY be defined in the documents defining the extensions.
   **SRS_UWS_01_354: [** In the absence of such definitions, the interpretation is that the header fields listed by the client in its request represent a preference of the header fields it wishes to use, with the first options listed being most preferable. **]**
   **SRS_UWS_01_355: [** The extensions listed by the server in response represent the extensions actually in use for the connection. **]**
   **SRS_UWS_01_356: [** Should the extensions modify the data and/or framing, the order of operations on the data should be assumed to be the same as the order in which the extensions are listed in the server's response in the opening handshake. **]**

   For example, if there are two extensions "foo" and "bar" and if the header field |Sec-WebSocket-Extensions| sent by the server has the value "foo, bar", then operations on the data will be made as bar(foo(data)), be those changes to the data itself (such as compression) or changes to the framing that may "stack".

   Non-normative examples of acceptable extension header fields (note that long lines are folded for readability):

         Sec-WebSocket-Extensions: deflate-stream
         Sec-WebSocket-Extensions: mux; max-channels=4; flow-control,
          deflate-stream
         Sec-WebSocket-Extensions: private-extension

   **SRS_UWS_01_357: [** A server accepts one or more extensions by including a |Sec-WebSocket-Extensions| header field containing one or more extensions that were requested by the client. **]**
   **SRS_UWS_01_358: [** The interpretation of any extension parameters, and what constitutes a valid response by a server to a requested set of parameters by a client, will be defined by each such extension. **]**

9.2.  Known Extensions

   Extensions provide a mechanism for implementations to opt-in to additional protocol features.
   This document doesn't define any extension, but implementations MAY use extensions defined separately.

10.  Security Considerations

   This section describes some security considerations applicable to the WebSocket Protocol.
   Specific security considerations are described in subsections of this section.

10.1.  Non-Browser Clients

   The WebSocket Protocol protects against malicious JavaScript running inside a trusted application such as a web browser, for example, by checking of the |Origin| header field (see below).
   See Section 1.6 for additional details.
   Such assumptions don't hold true in the case of a more-capable client.

   While this protocol is intended to be used by scripts in web pages, it can also be used directly by hosts.
   Such hosts are acting on their own behalf and can therefore send fake |Origin| header fields, misleading the server.
   Servers should therefore be careful about assuming that they are talking directly to scripts from known origins and must consider that they might be accessed in unexpected ways.
   In particular, a server should not trust that any input is valid.

   EXAMPLE: If the server uses input as part of SQL queries, all input text should be escaped before being passed to the SQL server, lest the server be susceptible to SQL injection.

10.2.  Origin Considerations

   Servers that are not intended to process input from any web page but only for certain sites SHOULD verify the |Origin| field is an origin they expect.
   If the origin indicated is unacceptable to the server, then it SHOULD respond to the WebSocket handshake with a reply containing HTTP 403 Forbidden status code.

   The |Origin| header field protects from the attack cases when the untrusted party is typically the author of a JavaScript application that is executing in the context of the trusted client.
   The client itself can contact the server and, via the mechanism of the |Origin| header field, determine whether to extend those communication privileges to the JavaScript application.
   The intent is not to prevent non-browsers from establishing connections but rather to ensure that trusted browsers under the control of potentially malicious JavaScript cannot fake a WebSocket handshake.

10.3.  Attacks On Infrastructure (Masking)

   In addition to endpoints being the target of attacks via WebSockets, other parts of web infrastructure, such as proxies, may be the subject of an attack.

   As this protocol was being developed, an experiment was conducted to demonstrate a class of attacks on proxies that led to the poisoning of caching proxies deployed in the wild [TALKING].
   The general form of the attack was to establish a connection to a server under the "attacker's" control, perform an UPGRADE on the HTTP connection similar to what the WebSocket Protocol does to establish a connection, and subsequently send data over that UPGRADEd connection that looked like a GET request for a specific known resource (which in an attack would likely be something like a widely deployed script for tracking hits or a resource on an ad-serving network).
   The remote server would respond with something that looked like a response to the fake GET request, and this response would be cached by a nonzero percentage of deployed intermediaries, thus poisoning the cache.
   The net effect of this attack would be that if a user could be convinced to visit a website the attacker controlled, the attacker could potentially poison the cache for that user and other users behind the same cache and run malicious script on other origins, compromising the web security model.

   To avoid such attacks on deployed intermediaries, it is not sufficient to prefix application-supplied data with framing that is not compliant with HTTP, as it is not possible to exhaustively discover and test that each nonconformant intermediary does not skip such non-HTTP framing and act incorrectly on the frame payload.
   Thus, the defense adopted is to mask all data from the client to the server, so that the remote script (attacker) does not have control over how the data being sent appears on the wire and thus cannot construct a message that could be misinterpreted by an intermediary as an HTTP request.

   Clients MUST choose a new masking key for each frame, using an algorithm that cannot be predicted by end applications that provide data.
   For example, each masking could be drawn from a cryptographically strong random number generator.
   If the same key is used or a decipherable pattern exists for how the next key is chosen, the attacker can send a message that, when masked, could appear to be an HTTP request (by taking the message the attacker wishes to see on the wire and masking it with the next masking key to be used, the masking key will effectively unmask the data when the client applies it).

   It is also necessary that once the transmission of a frame from a client has begun, the payload (application-supplied data) of that frame must not be capable of being modified by the application.
   Otherwise, an attacker could send a long frame where the initial data was a known value (such as all zeros), compute the masking key being used upon receipt of the first part of the data, and then modify the data that is yet to be sent in the frame to appear as an HTTP request when masked.  
   (This is essentially the same problem described in the previous paragraph with using a known or predictable masking key.)
   If additional data is to be sent or data to be sent is somehow changed, that new or changed data must be sent in a new frame and thus with a new masking key.
   In short, once transmission of a frame begins, the contents must not be modifiable by the remote script (application).

   The threat model being protected against is one in which the client sends data that appears to be an HTTP request.
   As such, the channel that needs to be masked is the data from the client to the server.
   The data from the server to the client can be made to look like a response, but to accomplish this request, the client must also be able to forge a request.
   As such, it was not deemed necessary to mask data in both directions (the data from the server to the client is not masked).

   Despite the protection provided by masking, non-compliant HTTP proxies will still be vulnerable to poisoning attacks of this type by clients and servers that do not apply masking.

10.4.  Implementation-Specific Limits

   **SRS_UWS_01_359: [** Implementations that have implementation- and/or platform-specific limitations regarding the frame size or total message size after reassembly from multiple frames MUST protect themselves against exceeding those limits. **]**
   (For example, a malicious endpoint can try to exhaust its peer's memory or mount a denial-of-service attack by sending either a single big frame (e.g., of size 2**60) or by sending a long stream of small frames that are a part of a fragmented message.)
   Such an implementation SHOULD impose a limit on frame sizes and the total message size after reassembly from multiple frames.

10.5.  WebSocket Client Authentication

   This protocol doesn't prescribe any particular way that servers can authenticate clients during the WebSocket handshake.
   The WebSocket server can use any client authentication mechanism available to a generic HTTP server, such as cookies, HTTP authentication, or TLS authentication.

10.6.  Connection Confidentiality and Integrity

   **SRS_UWS_01_360: [** Connection confidentiality and integrity is provided by running the WebSocket Protocol over TLS (wss URIs). **]**
   **SRS_UWS_01_361: [** WebSocket implementations MUST support TLS and SHOULD employ it when communicating with their peers. **]**

   For connections using TLS, the amount of benefit provided by TLS depends greatly on the strength of the algorithms negotiated during the TLS handshake.
   For example, some TLS cipher mechanisms don't provide connection confidentiality.
   **SRS_UWS_01_362: [** To achieve reasonable levels of protection, clients should use only Strong TLS algorithms. **]**
   "Web Security Context: User Interface Guidelines" [W3C.REC-wsc-ui-20100812] discusses what constitutes Strong TLS algorithms.
   [RFC5246] provides additional guidance in Appendix A.5 and Appendix D.3.

10.7.  Handling of Invalid Data

   **SRS_UWS_01_363: [** Incoming data MUST always be validated by both clients and servers. **]**
   **SRS_UWS_01_364: [** If, at any time, an endpoint is faced with data that it does not understand or that violates some criteria by which the endpoint determines safety of input, or when the endpoint sees an opening handshake that does not correspond to the values it is expecting (e.g., incorrect path or origin in the client request), the endpoint MAY drop the TCP connection. **]**
   **SRS_UWS_01_365: [** If the invalid data was received after a successful WebSocket handshake, the endpoint SHOULD send a Close frame with an appropriate status code (Section 7.4) before proceeding to _Close the WebSocket Connection_. **]**
   Use of a Close frame with an appropriate status code can help in diagnosing the problem.
   **SRS_UWS_01_366: [** If the invalid data is sent during the WebSocket handshake, the server SHOULD return an appropriate HTTP [RFC2616] status code. **]**

   A common class of security problems arises when sending text data using the wrong encoding.
   This protocol specifies that messages with a Text data type (as opposed to Binary or other types) contain UTF-8-encoded data.
   Although the length is still indicated and applications implementing this protocol should use the length to determine where the frame actually ends, sending data in an improper encoding may still break assumptions that applications built on top of this protocol may make, leading to anything from misinterpretation of data to loss of data or potential security bugs.

10.8.  Use of SHA-1 by the WebSocket Handshake

   The WebSocket handshake described in this document doesn't depend on any security properties of SHA-1, such as collision resistance or resistance to the second pre-image attack (as described in [RFC4270]).