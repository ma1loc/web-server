#!/bin/bash

python3 - << 'EOF'
import socket
import time

body = b'r' * 100000000
host = '192.168.1.7'
port = 8080

request = (
    'POST //directory/youpi.bla HTTP/1.1\r\n'
    'Host: 192.168.1.7:8080\r\n'
    'Content-Type: test/file\r\n'
    'Content-Length: ' + str(len(body)) + '\r\n'
    '\r\n'
)

print("Connecting...")
s = socket.socket()
s.connect((host, port))

print("Sending headers...")
s.sendall(request.encode())
time.sleep(0.5)

print("Sending body in chunks...")
chunk_size = 8192
sent = 0
try:
    while sent < len(body):
        end = min(sent + chunk_size, len(body))
        s.sendall(body[sent:end])
        sent += (end - sent)
        if sent % 1000000 == 0:
            print(f"Sent {sent // 1000000} MB")
except Exception as e:
    print(f"Send failed at {sent} bytes: {e}")

print("Waiting for response...")
response = b''
s.settimeout(120)
try:
    while True:
        chunk = s.recv(4096)
        if not chunk:
            break
        response += chunk
except Exception as e:
    print("Timeout/error:", e)

s.close()

print("\n========== RAW RESPONSE ==========")
print(response[:1000].decode('utf-8', errors='replace'))
print("===================================")
print("Total bytes received:", len(response))
EOF
