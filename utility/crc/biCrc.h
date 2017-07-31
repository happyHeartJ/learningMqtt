typedef union                         
{
    unsigned char  tbyte[2];          // T_byte[1] is Tword's high uint8
    unsigned short tword;             //T_word[0] is Tword's low uint8
}word_to_byte_t, *p_word_to_byte;

unsigned short cal_crc(unsigned char *start_addr, unsigned short lenth)
{
    word_to_byte_t tmpCRC;
    unsigned char temp;
    unsigned char i;

    tmpCRC.tword = 0xffff;
    while(lenth > 0)
    {
     lenth--;
     tmpCRC.tbyte[0] ^= *start_addr++;
        for( i = 0; i < 8; i++ )
        {
           temp = tmpCRC.tbyte[0];
           tmpCRC.tword >>= 1;
           if( (temp & 0x01) != 0 )
           {
            tmpCRC.tword ^= 0xa001;
           }
        }
     }
     return(tmpCRC.tword);
}
