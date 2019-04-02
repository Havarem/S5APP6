#include "transport.h"



void send_messages(char * message, int length)
{
    int nb_messages = length / MESSAGE_MAX_LEN + (length % MESSAGE_MAX_LEN != 0);
    
    for(int i = 0; i < nb_messages-1; i++)
    {
        char trame_message[MESSAGE_MAX_LEN];
        for(int j = 0; j < MESSAGE_MAX_LEN; j++)
        {
            trame_message[j] = message[i*MESSAGE_MAX_LEN + j];
        }

        create_trame(trame_message, MESSAGE_MAX_LEN);
    }

    if(length % MESSAGE_MAX_LEN != 0)
    {
        char trame_message[MESSAGE_MAX_LEN];
        for(int j = 0; j < length % MESSAGE_MAX_LEN; j++)
        {
            trame_message[j] = message[j];
        }

        create_trame(trame_message, length % MESSAGE_MAX_LEN);
    }

}