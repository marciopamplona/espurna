// Channel
uint8_t channel = 1;

// Access point MAC
uint8_t ap[6] = {0x00,0x01,0x02,0x03,0x04,0x05};

// Client MAC
uint8_t client[6] = {0xff,0xff,0xff,0xff,0xff,0xff};

// Sequence number of a packet from AP to client
uint16_t seq_n = 0;

// Packet buffer
uint8_t packet_buffer[128];

uint8_t payload_buffer[128];
int payloadIndex = 0;

int payloadCharge(uint8_t code, int len, char *buffer){
    payload_buffer[payloadIndex] = code;
    payload_buffer[payloadIndex+1] = len;

    for (int i=0; i < len; i++){
        payload_buffer[payloadIndex+2+i] = buffer[i];
    }

    payloadIndex += len+2;
    return payloadIndex;
}

void payloadClean(){
    payloadIndex = 0;
    memset(payload_buffer,0, sizeof(payload_buffer));
}

uint16_t probe_request_packet(uint8_t *buf, uint8_t *client, uint8_t *ap, uint16_t seq, char * ssid)
{
    int i=0;

    // Type: Probe Request
    buf[0] = 0x40;
    buf[1] = 0x00;
    // Duration 0 msec, will be re-written by ESP
    buf[2] = 0x00;
    buf[3] = 0x00;
    // Destination
    for (i=0; i<6; i++) buf[i+4] = client[i];
    // Sender
    for (i=0; i<6; i++) buf[i+10] = ap[i];
    // BSSID
    for (i=0; i<6; i++) buf[i+16] = ap[i];
    // Seq_n
    buf[22] = seq % 0xFF;
    buf[23] = seq / 0xFF;    
    
    // Frame body
    int index = 24;

    payloadClean();
    payloadCharge(0,strlen(ssid), ssid);
    payloadCharge(253,strlen(ssid), ssid);

    memcpy(buf+index, (uint8_t *) payload_buffer, payloadIndex);
    index += payloadIndex;

    return (index);
}

void probeR(void)
{
    seq_n++;
    // Sequence number is increased by 16, see 802.11
    uint16_t size = probe_request_packet(packet_buffer, client, ap, seq_n, "oi");
    wifi_send_pkt_freedom(packet_buffer, size, 0);
}

void beacon_init(void)
{
    wifi_set_opmode(STATION_MODE);
    // Set up promiscuous callback
    wifi_set_channel(channel);
    // wifi_promiscuous_enable(0);
    // wifi_set_promiscuous_rx_cb(promisc_cb);
    wifi_promiscuous_enable(1);
}