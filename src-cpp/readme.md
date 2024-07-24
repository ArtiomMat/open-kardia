# C++ Source code

 This is the source code for the C++ version of the engine for Xalurtia.

# Technicals

 The engine is compiled with -ffast-math -O3, this can be disabled by just removing it from the makefile, I haven't tested the effect of such operations on the speed of the engine. From what I have read, you are able to compile your binary that uses the engine without -ffast-math. With that being said, you must be enformed that -ffast-math makes assumptions, one of them being that there are no infinities, no division by 0, etc. And it will not care if you make it divide by 0 in floats, it will keep going.

# Protocols

 Every packet starts with a header containing a single i8, designating what the rest of the data means.
 NOTE: EVERY PACKET, can have extra information that he protocol of the game itself takes care of, after all the data that the server writes the game can handle writing and reading extra info on the client and server sides. So even if `EMPTY` is written, the game may still add extra data. For `str[x]`, `str` is limited to be the size of `x` at most(including null terminator).

## For the server

* `SERVER_B_TICK`: server tick, a global message sent to all clients connected.\
 `EMPTY`

* `SERVER_B_REPLY`: a reply to a request made by the client, a request does not include JOIN, rather it's CLI_B_REQUEST.\
 `EMPTY`

* `SERVER_B_JOIN`: the server accepted the client join request.\
 `i8`: index of client, if rejected -1.

* `SERVER_B_INFO`: (for joined clients) general info about the connecting.\
 `u8`: how many connected.

* `SERVER_B_INFO`: (for unjoined clients) general info about server before joining.\
 `str[MAX_SERVER_ALIAS]`: alias.\
 `str[MAX_SERVER_DESC]`: description(not optional but can be just left null terminated at start).\
 `u8`: how many connected

## For the client

* `CLI_B_INFO`: (for joined clients) client wants info.\
 `i8`: index, -1 for info as if disjoined(whether joined or not).

* `CLI_B_REQUEST`: request for the server, for which it sends SERVER_B_REPLY.\
 `i8`: index.

* `CLI_B_JOIN`: request to join\
 `i8`: index, but is meaningless and ignored, just has to be there because of how the server works.\
 `str[MAX_CLIENT_ALIAS]:` alias.
 
* `CLI_B_EXIT`: notification of exiting the connecting.\
 `i8`: index.

* `CLI_B_GOT_ACCEPT`: the client got the accept of the server.
 `i8`: index that the client got.

* `CLI_B_ALIVE`: the client has nothing to send so it's just alive.

# Compiling

 Run make, you can cross compile by changine the PLATFORM variable, read the comment in the makefile for more info.
