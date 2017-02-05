#What is it?
	
It a simple message passing application that is written in C using TCP connection. 

#How to run it?
	
1. run 'make' under the root directory.
2. run 'server' with one args, the port it is listening.
3. run 'client' with two args, first is server IP address, second is port in step 2.
4. run another 'client'.
5. start chatting.

#Features
	
1.The clients can send and receive multiple messages to other.
2.Both clients can send as well as receive data at any time.
3.If one host, say A, leave the chat, B won't get terminated. Another host C can join the chat.
4.No authentication.

Author:subo. sbzhuang@gmail.com
