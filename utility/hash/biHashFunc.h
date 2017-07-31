/* the famous DJB Hash Function for strings */    
unsigned int DJBHash(char *str)    
{    
    unsigned int hash = 5381;    
     
    while (*str){    
        hash = ((hash << 5) + hash) + (*str++); /* times 33 */    
    }    
    hash &= ~(1 << 31); /* strip the highest bit */    
    return hash;    
}

unsigned int ngx_murmur_hash2(unsigned char *data, size_t len)  
{  
    unsigned int h, k;  
  
    h = 0 ^ len;  
  
    while (len >= 4) {  
        k  = data[0];  
        k |= data[1] << 8;  
        k |= data[2] << 16;  
        k |= data[3] << 24;  
  
        k *= 0x5bd1e995;  
        k ^= k >> 24;  
        k *= 0x5bd1e995;  
  
        h *= 0x5bd1e995;  
        h ^= k;  
  
        data += 4;  
        len -= 4;  
    }  
  
    switch (len) {  
    case 3:  
        h ^= data[2] << 16;  
    case 2:  
        h ^= data[1] << 8;  
    case 1:  
        h ^= data[0];  
        h *= 0x5bd1e995;  
    }  
  
    h ^= h >> 13;  
    h *= 0x5bd1e995;  
    h ^= h >> 15;  
  
    return h;  
}
