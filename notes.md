
extern osThreadId_t defaultTaskHandle;

void StartDefaultTask(void *argument) {
    uint8_t rx_buffer[ICMP_BUFFER_SIZE];
    uint8_t remote_ip[4];
    uint16_t remote_port;

    // Init W5500 already before this point (SPI + IP config)
    int icmp_socket = socket(1, Sn_MR_IPRAW, 0, 0);  // RAW IP mode
    if (icmp_socket < 0) {
        // Handle error
        while (1);
    }

    for (;;) {
        int len = recvfrom(icmp_socket, rx_buffer, ICMP_BUFFER_SIZE, remote_ip, &remote_port);
        if (len > 0) {
            process_icmp_request(rx_buffer, len, remote_ip, icmp_socket);
        }

        osDelay(10);  // Avoid tight loop
    }
}
