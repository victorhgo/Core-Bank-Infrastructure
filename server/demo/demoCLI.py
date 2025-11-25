"""
A Simple Python CLI for testing a C++ Server

"""
import argparse
import socket
import sys

def sendCommand(host: str, port: int, command: str) -> str:
    """
    Open a TCP connection to host, port, send a command,
    and return the server response as a string.
    """
    try:

        with socket.create_connection((host, port), timeout=5.0) as sock:
            # Send command and newline
            sock.sendall((command + "\n").encode("utf-8"))

            # Read up to 4KB for the response (this is more than enough for our simple protocol)
            data = sock.recv(4096)

            if not data:
                return ""

            return data.decode("utf-8").strip()

    except ConnectionRefusedError:
        print(f"ERROR: Could not connect to {host}:{port}. Connection refused. "
              f"Please check if the Transaction Server is running", file=sys.stderr)
        sys.exit(1)

    except socket.timeout:
        print(f"ERROR: Connection to {host}:{port} timed out.", file=sys.stderr)
        sys.exit(1)
        
    except OSError as error:
        print(f"ERROR: Socket error: {error}", file=sys.stderr)
        sys.exit(1)

def test():
    for _ in range(1, 100):
        response = sendCommand('127.0.0.1', 8080, 'CLI Test')

        print(response)

if __name__ == "__main__":
    print("= Server response =")
    print(sendCommand('127.0.0.1', 8080 , 'CLI Test'))
