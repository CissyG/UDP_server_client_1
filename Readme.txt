Assignment 1

How to compile

1. In terminal, change directory to this directory. Then run "cmake ./" (need to have cmake installed and in your PATH). 
2. Run "make" to compile

How to test: 

After compiling, there will be 2 executable files, server and client. 

First, run server in a terminal.
Then in another terminal, run client in this directory. 
The client will send a few correct packets and also other packets with errors.
The results are included in the screenshots server_screenshot.png and client_screenshot.png. 

To verify the ACK_timer of the client, shut down the server and run client again. You will notice how it waits for three seconds and re-sends three times. Open the server any time and the client will print its response. An example is included in the screenshot "ack_timer_screenshot.png".
