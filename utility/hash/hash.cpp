/*
 * Author	: JiangHongjun
 * Date		: 2016/9/30
 * Reference	: http://blog.csdn.net/sangyongjia/article/details/33397109
 * Hash Table
 * An implement about MPQ HASH, and you can use <prepareCryptTable>, <HashString>, <GetHashTablePos_MPQ> directly. 
 * You have better do changes to <insert_string> according to your project before use it.
 */

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct  
{  
    int nHashA;  
    int nHashB;  
    char bExists;  
} SOMESTRUCTRUE,MPQHASHTABLE; 


//crytTable[]里面保存的是HashString函数里面将会用到的一些数据，在prepareCryptTable
//函数里面初始化
unsigned long cryptTable[0x500];

//函数prepareCryptTable以下的函数生成一个长度为0x500（合10进制数：1280）的cryptTable[0x500]  
void prepareCryptTable()  
{   
    unsigned long seed = 0x00100001, index1 = 0, index2 = 0, i;  
  
    for( index1 = 0; index1 < 0x100; index1++ )  
    {   
        for( index2 = index1, i = 0; i < 5; i++, index2 += 0x100 )  
        {   
            unsigned long temp1, temp2;  
  
            seed = (seed * 125 + 3) % 0x2AAAAB;  
            temp1 = (seed & 0xFFFF) << 0x10;  
  
            seed = (seed * 125 + 3) % 0x2AAAAB;  
            temp2 = (seed & 0xFFFF);  
  
            cryptTable[index2] = ( temp1 | temp2 );   
        }   
    }   
}

//以下函数计算lpszFileName 字符串的hash值，其中dwHashType 为hash的类型，
//在下面GetHashTablePos函数里面调用本函数，其可以取的值为0、1、2；该函数
//返回lpszFileName 字符串的hash值；  
unsigned long HashString(const char *lpszkeyName, unsigned long dwHashType )  
{  
    unsigned char *key  = (unsigned char *)lpszkeyName;  
    unsigned long seed1 = 0x7FED7FED;  
    unsigned long seed2 = 0xEEEEEEEE;  
    int ch;  
  
    while( *key != 0 )  
    {  ch = *key++;  
        seed1 = cryptTable[(dwHashType<<8) + ch] ^ (seed1 + seed2);  
        seed2 = ch + seed1 + seed2 + (seed2<<5) + 3;  
    }  
    return seed1;  
}

//函数GetHashTablePos下述函数为在Hash表中查找是否存在目标字符串，有则返回要查找字符串的Hash值，无则，return -1.  
//lpszString要在Hash表中查找的字符串，lpTable为存储字符串Hash值的Hash表
int GetHashTablePos(char *lpszString, SOMESTRUCTURE *lpTable )  
{   
    int nHash = HashString(lpszString);  //调用上述函数HashString，返回要查找字符串lpszString的Hash值。  
    int nHashPos = nHash % nTableSize;  
   
    if ( lpTable[nHashPos].bExists  &&  !strcmp( lpTable[nHashPos].pString, lpszString ) )   
    {  
	//如果找到的Hash值在表中存在，且要查找的字符串与表中对应位置的字符串相同，  
        return nHashPos;    //返回找到的Hash值  
    }   
    else  
    {  
        return -1;    
    }   
}

//函数GetHashTablePos_MPQ(暴雪的hash算法)中，lpszString 为要在hash表中查找的字符串；lpTable 为存储字符串hash值的hash表；nTableSize 为hash表的长度：   
int GetHashTablePos_MPQ( char *lpszString, MPQHASHTABLE *lpTable, int nTableSize )  
{  
    const int  HASH_OFFSET = 0, HASH_A = 1, HASH_B = 2;  
   
    int  nHash = HashString( lpszString, HASH_OFFSET );  
    int  nHashA = HashString( lpszString, HASH_A );  
    int  nHashB = HashString( lpszString, HASH_B );  
    int  nHashStart = nHash % nTableSize;  
    int  nHashPos = nHashStart;  
   
    while ( lpTable[nHashPos].bExists )  
   {  
//     如果仅仅是判断在该表中时候存在这个字符串，就比较这两个hash值就可以了，不用对结构体中的字符串进行比较。  
//         这样会加快运行的速度？减少hash表占用的空间？这种方法一般应用在什么场合？  
        if ( 　 lpTable[nHashPos].nHashA == nHashA  
        &&  lpTable[nHashPos].nHashB == nHashB )  
       {  
            return nHashPos;  
       }  
       else  
       {  
            nHashPos = (nHashPos + 1) % nTableSize;  
       }  
   
        if (nHashPos == nHashStart)  
              break;  
    }  
     return -1;  
}

//直接调用上面的hashstring，nHashPos就是对应的HASH值。
/*
 * 此方法针对具体要插入数据时使用，所以需要响应的数据结构
 * 也需要包含更多的头文件，具体使用时请添加
 * 此处给出使用方法，当前文件如果直接编译会出现问题
 * 
 */
typedef struct
{
    int	weight;
    char *pkey;
}KEYNODE;
MPQHASHTABLE TestHashTable[nTableSize];  
int TestHashCTable[nTableSize];  
int TestHashDTable[nTableSize];  
list<KEYNODE> test_data[nTableSize];  

int insert_string(const char *string_in)  
{  
    const int HASH_OFFSET = 0, HASH_C = 1, HASH_D = 2;  
    unsigned int nHash = HashString(string_in, HASH_OFFSET);  
    unsigned int nHashC = HashString(string_in, HASH_C);  
    unsigned int nHashD = HashString(string_in, HASH_D);  
    unsigned int nHashStart = nHash % nTableSize;  
    unsigned int nHashPos = nHashStart;  
    int ln, ires = 0;  
  
    while (TestHashTable[nHashPos].bExists)  
    {
	//判断hash表中是否有位置可以存放，确定存放位置
	nHashPos = (nHashPos + 1) % nTableSize;  
  
        if (nHashPos == nHashStart)  
            break;  
    }  
  
    ln = strlen(string_in);  
    if (!TestHashTable[nHashPos].bExists && (ln < nMaxStrLen))//nMaxStrLen，字符串最大长度限制
    {
	;
	//保存数据
	//将校验数据分别保存如两个数组中
        /*
	 * TestHashCTable[nHashPos] = nHashC;  
         * TestHashDTable[nHashPos] = nHashD;  
	*/

  	/* 分配内存，用来保存插入字符串的内容和对应的hash值
        test_data[nHashPos] = (KEYNODE *) malloc (sizeof(KEYNODE) * 1);  
        if(test_data[nHashPos] == NULL)  
        {  
            printf("10000 EMS ERROR !!!!\n");  
            return 0;  
        }  
  
        test_data[nHashPos]->pkey = (char *)malloc(ln+1);  
        if(test_data[nHashPos]->pkey == NULL)  
        {  
            printf("10000 EMS ERROR !!!!\n");  
            return 0;  
        }  
  
        memset(test_data[nHashPos]->pkey, 0, ln+1);  
        strncpy(test_data[nHashPos]->pkey, string_in, ln);  
        *((test_data[nHashPos]->pkey)+ln) = 0;  
        test_data[nHashPos]->weight = nHashPos;  
  	*/
	/*
	 * 将存放位置设置为true，表明这个位置已经存放数据
        TestHashTable[nHashPos].bExists = 1;  
	*/
    }  
    else  
    {
	//无法保存
	;
	/*
	 * 错误输出
        if(TestHashTable[nHashPos].bExists)  
            printf("30000 in the hash table %s !!!\n", string_in);  
        else  
            printf("90000 strkey error !!!\n"); 
	*/
    }  
    return nHashPos;  
} 


 //在main中测试argv[1]的三个hash值：
 //./hash  "arr/units.dat"
 //./hash  "unit\neutral/acritter.grp"
 int main( int argc, char **argv )
 {
     char *test1 = "arr/units.dat";
     char *test2 = "unit/neutral/acritter.grp";
     
     unsigned long ulHashValue;
     int i = 0;
     /*初始化数组：crytTable[0x500]*/
     prepareCryptTable();

     ulHashValue = HashString(test1, 0 );
     printf("----%X ----\n", ulHashValue );

     ulHashValue = HashString(test2, 1 );
     printf("----%X ----\n", ulHashValue );

     system("PAUSE");
     return 0;
 }
