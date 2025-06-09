#!/usr/bin/env python3
"""
STM32 Device Discovery Tool
--------------------------
Simple Python script to discover STM32 devices on the local network.
Uses UDP broadcast to find devices that implement the discovery protocol.

Usage:
python discover_stm32.py 

Dependencies:
- Python 3.x
"""

import socket
import json
import time
import sys
import threading
import argparse

# Discovery configuration
DISCOVERY_PORT = 5678
DISCOVERY_MESSAGE = "DISCOVER_STM32"
BROADCAST_ADDR = "255.255.255.255"
TIMEOUT = 5  # seconds

discovered_devices = []
discovery_lock = threading.Lock()

def print_device_info(device):
    """Print formatted device information"""
    print("\n=== STM32 Device Found ===")
    print(f"Hostname: {device.get('hostname', 'Unknown')}")
    print(f"IP Address: {device.get('ip', 'Unknown')}")
    print(f"Device Type: {device.get('type', 'Unknown')}")
    if 'version' in device:
        print(f"Firmware Version: {device.get('version', 'Unknown')}")
    print("========================\n")

def listen_for_responses(timeout=TIMEOUT):
    """Listen for discovery responses from devices"""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.settimeout(timeout)
    sock.bind(('', DISCOVERY_PORT))
    
    start_time = time.time()
    
    print(f"Listening for STM32 devices for {timeout} seconds...\n")
    
    while time.time() - start_time < timeout:
        try:
            data, addr = sock.recvfrom(1024)
            if data:
                try:
                    device_info = json.loads(data.decode('utf-8'))
                    with discovery_lock:
                        if addr[0] not in [d.get('ip') for d in discovered_devices]:
                            discovered_devices.append(device_info)
                            print_device_info(device_info)
                except json.JSONDecodeError:
                    print(f"Received non-JSON response from {addr[0]}")
                except Exception as e:
                    print(f"Error processing response from {addr[0]}: {e}")
        except socket.timeout:
            pass
    
    sock.close()

def send_discovery_broadcast():
    """Send discovery broadcast to find STM32 devices"""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    
    try:
        sock.sendto(DISCOVERY_MESSAGE.encode('utf-8'), (BROADCAST_ADDR, DISCOVERY_PORT))
        print("Sent discovery broadcast")
    except Exception as e:
        print(f"Error sending discovery broadcast: {e}")
    
    sock.close()

def discover_devices(timeout=TIMEOUT):
    """Main discovery function"""
    # Create and start listener thread
    listener_thread = threading.Thread(target=listen_for_responses, args=(timeout,))
    listener_thread.daemon = True
    listener_thread.start()
    
    # Send discovery broadcast
    send_discovery_broadcast()
    
    # Wait for listener to finish
    listener_thread.join()
    
    # Print summary
    with discovery_lock:
        if not discovered_devices:
            print("\nNo STM32 devices found on the network.")
        else:
            print(f"\nFound {len(discovered_devices)} STM32 device(s) on the network.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Discover STM32 devices on the network')
    parser.add_argument('--timeout', type=int, default=TIMEOUT, 
                        help=f'Discovery timeout in seconds (default: {TIMEOUT})')
    args = parser.parse_args()
    
    try:
        discover_devices(args.timeout)
    except KeyboardInterrupt:
        print("\nDiscovery cancelled.")
        sys.exit(0)
