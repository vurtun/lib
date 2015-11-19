from __future__ import print_function
import websocket

if __name__=="__main__":
    websocket.enableTrace(True)
    ws = websocket.create_connection("ws://localhost:8888/wstest");
    print("Sending 'Hello World'...")
    ws.send("Hello World");
    print("Send")
    print("Receiving...");
    result = ws.recv()
    print("Received '%s'" % result)
    ws.close()

