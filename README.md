RelayVideoStreamThe server designed for retransmission of video channels is characterized by low memory consumption.

In total, about 160 kilobytes of memory is used to cache the video stream

USE:

relay2 port path no_client_timeout_sec url1 url2 url3 ....

path - path string, no_client_timeout_sec - terminated if no clent (0 - no terminated), url1, url2, url3 - url to alternative video stream

EXAMPLE:

relay2 8000 1BEFC98754AM 60 http://host1.com/video http://host2.com/video

use video stream on http://localhost:8000/1BEFC98754AM


If the broadcast from any video stream is interrupted, the broadcast resumes from the next alternative stream.

Build with Makefile on Linux