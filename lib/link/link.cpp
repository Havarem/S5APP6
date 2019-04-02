#include "link.h"
#include "trame.h"


void create_trame(char content[MESSAGE_MAX_LEN], int length)
{
    trame_t trame;
    trame.message[0] = 0x55; //preambule
    trame.message[1] = 0x7E; //start
    trame.message[2] = 0x00; //Flags
    trame.message[3] = length; //lenght
    for (int i = 0; i < length; i++)
    {
        trame.message[i + 4] = content[i];
    }
    unsigned short crc = crc16(content, length);

    trame.message[length+4] =  crc & 0xFF; //CRC
    trame.message[length+5] =  (crc >> 8) & 0xFF; //CRC
    //printf("CRC: 0x%02x 0x%02x\r\n", crc & 0xFF, (crc >> 8) & 0xFF);
    trame.message[length+6] = 0x7E; //end

    trame.length = length + 7;

    //send message to physic layer
    send_message_to_phy(trame);
}