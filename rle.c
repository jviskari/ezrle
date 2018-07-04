#include <stdint.h>
#include <stdio.h>

//#define TRACE

#ifdef TRACE
#define PRINT printf
#else
#define PRINT(...)
#endif


void PRINT_BYTES(uint8_t * src, uint8_t buflen)
{
    uint8_t i = 0;
    while(i < buflen)
    {
        printf("%2x ", src[i]);
        i++;
    }
    printf("\n");
}

void unpack(uint8_t * dst, uint8_t * src, uint8_t buflen)
{
    uint8_t byte1;
    uint8_t byte2;

    uint8_t len;
    uint8_t si=0,di=0;
    uint8_t end_di; 
    uint8_t rle_mode = 1; //start in SEQ mode

    len = src[si++];

 
    while( di < len)
    {
        PRINT("UNPACK: src[%d]:",si);
        byte1 = src[si++];
        PRINT("%2x mode:%d\n",byte1,rle_mode);

        if (byte1 == 0) //escape character
        {
            if( src[si+1] == 0 )
            {
                PRINT("UNPACK: Handle Special byte:%2x\n",byte1);
                si++; //skip extra special character
                dst[di++] = byte1; //store special character
            }
            else
            {
                rle_mode ^= 1;  //change mode 
                PRINT("UNPACK: Change RLE mode:%d\n",rle_mode);
            }

            continue;   
        }

        if(rle_mode == 0)
        {
            PRINT("UNPACK: BYTE:dst[%d]:%2x\n",di,byte1);
            dst[di++] = byte1;            
            continue;  
        }
        else
        {
            byte2 = src[si++];
            end_di = di+byte1;
            PRINT("UNPACK: SEQ From:%d To:%d LEN:%d\n",di,end_di-1,byte1); 

            do
            {
                PRINT("UNPACK: SEQ DATA:dst[%d]:%2x\n",di,byte2); 
                dst[di++] = byte2;
            }while(di < end_di);

        }
      
    }

    PRINT("UNPACKED:len=%d\n",len);
#ifdef TRACE   
    PRINT_BYTES(&dst[0], len);
#endif
}


uint8_t pack(uint8_t * dst, uint8_t * src, uint8_t buflen)
{
    uint8_t byte1;
    uint8_t start_si;
    uint8_t si=0,di=0;
    uint8_t rle_mode;

    //set len
    dst[di] = buflen;
    di++;

    PRINT("PACK:len=%d\n",buflen);
 #ifdef TRACE   
    PRINT_BYTES(&src[0], buflen);
 #endif   

    while(si < buflen)
    {
        uint8_t bcompare;

        byte1 = src[si];
        bcompare = src[si+1];

        PRINT("PACK:state si:%2x:di:%2x:byte1:%2x:RLE:%d\n", si,di,byte1,rle_mode);

        if (byte1 == bcompare)
        {
            //handle mode
            if(rle_mode == 0)
            {
                rle_mode = 1; //enter rle_mode
                if(si != 0)
                {
                    //first byte is never escape character
                    dst[di++] = 0; //end single byte mode
                }

                PRINT("PACK: Enter RLE mode SEQ:%2X DI:%2X SI:%2X\n",byte1, di, si);
                continue; 
            }

            start_si = si;
            PRINT("PACK: Start count at SI:%2x\n", start_si);
            //count bytes
            while(src[si] == bcompare)
            {
                PRINT("PACK BUF[%d]:%2x\n", si,src[si]);
                si++;
            }

            PRINT("PACK: End count at SI:%2x\n", si);

            PRINT("PACK:Store SEQ:%2x len:%2x si:%2x di:%2x\n",byte1, si-start_si, si, di);

            dst[di++] = si-start_si;
            dst[di++] = byte1;

        }
        else
        {
            if(rle_mode == 1)
            {
                rle_mode = 0;  //enter rle_mode
                dst[di++] = 0; //end single byte mode
                continue;  
            }


            if(si == 0)
            {
                PRINT("PACK:Store Dummy Counter for First Byte:%2x di:%2x\n",byte1, di);
                dst[di++] = 1;
                rle_mode = 1; //enter RLE mode
            }

            if (byte1 == 0)
            {
                PRINT("PACK:Store Escape Byte:dst[%d]:%2x\n",di,byte1);
                dst[di++] = byte1;
            }

            PRINT("PACK:Store Byte:dst[%d]:%2x\n",di,byte1);
            dst[di++] = byte1;
            si++;
        }
        PRINT("PACK: index-state: si:%2x di:%2x buflen: %2x\n",si,di,buflen);
    }

    PRINT("END BUFFER\n");

#ifdef TRACE
    PRINT_BYTES(&dst[0], di);
#endif
    return di;    
}

int main(int argc, char const *argv[])
{
    uint8_t buff_in[] = {1,1,1,1,2,2,2,2,1,1,1,1,2,2,2,2};
    uint8_t buff_pack[128];
    uint8_t buff_res[128];
    uint8_t packed_len = 0;

    packed_len = pack(buff_pack, buff_in, sizeof(buff_in));
    unpack(buff_res, buff_pack, sizeof(buff_in));

    printf("IN:\n");
    PRINT_BYTES(buff_in,sizeof(buff_in));

    printf("PACKED:\n");
    PRINT_BYTES(&buff_pack[0], packed_len);

    printf("UNPACKED:\n");
    PRINT_BYTES(buff_res, sizeof(buff_in)); 
   
    return 0;

}
