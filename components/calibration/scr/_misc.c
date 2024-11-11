#include "_misc.h"
#include "_debug.h"


void cipher_TEA_encrypt( U32 *v, U32 *k )
{
    test_param( NULL == v );
    test_param( NULL == k );

    /* set up */
    U32 v0 = v[0];
    U32 v1 = v[1];
    U32 sum = 0;
    U32 i;

    /* a key schedule constant */
    U32 delta = 0x9e3779b9;

    /* cache key */
    U32 k0 = k[0];
    U32 k1 = k[1];
    U32 k2 = k[2];
    U32 k3 = k[3];

    /* basic cycle start */
    for (i = 0; i < 32; i++)
    {
        sum += delta;
        v0 += ((v1 << 4) + k0) ^ (v1 + sum) ^ ((v1 >> 5) + k1);
        v1 += ((v0 << 4) + k2) ^ (v0 + sum) ^ ((v0 >> 5) + k3);
    }
    /* end cycle */

    v[0] = v0;
    v[1] = v1;
}

void cipher_TEA_decrypt( U32 *v, U32 *k )
{
    test_param( NULL == v );
    test_param( NULL == k );

    /* set up */
    U32 v0 = v[0];
    U32 v1 = v[1];
    U32 sum = 0xC6EF3720;
    U32 i;

    /* a key schedule constant */
    U32 delta = 0x9E3779B9;

    /* cache key */
    U32 k0 = k[0];
    U32 k1 = k[1];
    U32 k2 = k[2];
    U32 k3 = k[3];

    /* basic cycle start */
    for( i = 0; i < 32; i++ )
    {
        v1 -= ((v0 << 4) + k2) ^ (v0 + sum) ^ ((v0 >> 5) + k3);
        v0 -= ((v1 << 4) + k0) ^ (v1 + sum) ^ ((v1 >> 5) + k1);
        sum -= delta;
    }
    /* end cycle */

    v[0] = v0;
    v[1] = v1;
}

// RTEA ========================================================================
void cipher_RTEA_encrypt( U32 *value, U32 *key )
{
    U32 a, b;
    INT r;

    test_param( NULL == value );
    test_param( NULL == key );

    a = *value;
    b = *(value + 1);
    for( r = 0; r < 64; r++ )
    {
        b += a + ((a << 6) ^ (a >> 8)) + (key[r % 8] + r);
        r++;
        a += b + ((b << 6) ^ (b >> 8)) + (key[r % 8] + r);
    }
    *value = a;
    *(value + 1) = b;
}


void cipher_RTEA_decrypt( U32 *value, U32 *key )
{
    U32 a, b;
    INT r;

    test_param( NULL == value );
    test_param( NULL == key );

    a = *value;
    b = *(value + 1);
    for( r = 63; r >= 0; r--)
    {
        a -= b + ((b << 6) ^ (b >> 8)) + (key[r % 8] + r);
        r--;
        b -= a + ((a << 6) ^ (a >> 8)) + (key[r % 8] + r);
    }
    *value = a;
    *(value + 1) = b;
}


// Hash ========================================================================
U32 hash_Ly( char *str )
{
	test_param( NULL == str );
    U32 hash = 0;

    for(; *str; str++)
    {
        hash = (hash * 1664525) + (U8)(*str) + 1013904223;
    }
    return hash;
}

U32 hash_Rs( const char *str )
{
	test_param( NULL == str );

    static const U32 b = 378551;
    unsigned int a = 63689;
    unsigned int hash = 0;

    for(; *str; str++)
    {
        hash = hash * a + (unsigned char)(*str);
        a *= b;
    }

    return hash;
}

U32 hash_Rot13( const char *str )
{
	test_param( NULL == str );

    U32 hash = 0;

    for(; *str; str++ )
    {
        hash += (unsigned char)(*str);
        hash -= (hash << 13) | (hash >> 19);
    }

    return hash;
}

U32 hash_FAQ6( const char *str )
{
	test_param( NULL == str );

    UINT hash = 0;

    for( ; *str; str++ )
    {
        hash += (unsigned char)(*str);
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}


// Gray code ===================================================================
U32 coder_Gray_encode( U32 x )
{
    return x ^ (x >> 1);
}

U32 coder_Gray_decode( U32 x )
{
   return x ^ ((x & 0x88888888) >> 3) ^ ((x & 0xCCCCCCCC) >> 2) ^ ((x & 0xEEEEEEEE) >> 1);
}

//==============================================================================
S32 sqrt_fixed( S32 x )
{
    S32 guess = 1 * 100000;
    U32 lim = 40;

    while( (_abs( guess * guess - x) >= 1 ) && ( lim-- > 0 ))
    {
        guess = ((x / guess ) + guess ) / 2;
    }
    return ( guess );
}

S32 dir_correct( S32 dir )
{
    return dir % 360;
}

// sorting algorithms ==========================================================
void sort_Shell_16bit( U16 *p, U32 n )
{
    U32 i, j, k;

    U16 t;

    for( k = n / 2; k > 0; k /= 2 )
    {
        for( i = k; i < n; i += 1 )
        {
            t = p[ i ];
            for( j = i; j >= k; j -= k )
            {
                if( t < p[ j - k ] )
                    p[ j ] = p[ j - k ];
                else
                    break;
            }
            p[ j ] = t;
        }
    }
}


void sort_Shell_32bit( S32 *p, U32 n )
{
    U32 i, j, k;

    S32 t;

    for( k = n / 2; k > 0; k /= 2 )
    {
        for( i = k; i < n; i += 1 )
        {
            t = p[ i ];
            for( j = i; j >= k; j -= k )
            {
                if( t < p[ j - k ] )
                    p[ j ] = p[ j - k ];
                else
                    break;
            }
            p[ j ] = t;
        }
    }
}


U32 bit_32_reverse( U32 value )
{
    U32 left = (U32)1 << 31;
    U32 right = 1;
    U32 result = 0;

    for (int i = 31; i >= 1; i -= 2)
    {
        result |= (value & left) >> i;
        result |= (value & right) << i;
        left >>= 1;
        right <<= 1;
    }
    return result;
}


U32 bit_reverse( U32 bits_num, U32 value )
{
    //U32 result = __RBIT( value );
    //result =  value >> ((32 - bits_num) -0 );
    U32 result = 0;
    for( U32 i = 0; i < bits_num; i++)
    {
        result <<= 1;
        result |= (value & 1);
        value >>= 1;
    }
    return result;
}


U8 bcd8_to_dec8( U8 val )
{
    return (((val / 16) * 10) + (val % 16));
}


U8 dec8_to_bcd8( U8 val )
{
    return (((val / 10) * 16) + (val % 10));
}


U32 dec16_to_bcd32( U16 dec )
{
    U32 result = 0;
    int shift = 0;

    while( dec )
    {
        result +=  (dec % 10) << shift;
        dec = dec / 10;
        shift += 4;
    }
    return result;
}


U8 char_to_hex( char c )
{
    U8 v;

    if( c >= '0' && c <= '9' )
        v = c - '0';
    else
    if( c >= 'a' && c <= 'f' )
        v = c - 'a'+ 10;
    else
    if( c >= 'A' && c <= 'F' )
        v = c - 'A'+ 10;
    else
        v = 0xFF; //not a hex
    return v;
}


FAST_U8 U32_to_str( U32 value, char *pstring )//, U32 hgh )
{
    U8 ms[ 10 ];
    U8 size, i;
    U32 temp;
    BOOL test = FALSE;

    i = 10;
    while( i-- )
    {
        //ms[i] = 0; // nill all
        temp = value % 10;
        ms[i] = temp;
        value /= 10;
    }
    size = 0;
    for( i = 0; i < 10; i++ )
    {
    	if( 0 != ms[i] && !test )
		{
			test = TRUE;
		}
		if( test )
		{
			pstring[size] = ms[i] + '0';
			size++;
		}
    }
    pstring[ size ] = '\0'; // null terminate
    return size;
}


U32 _atoi( char *pstr )
{
    U8 ch;
    uint32_t result = 0;

    ch = *pstr++;
    while ( ch >= '0' && ch <= '9' )
    {
        result += ch - '0';
        result *= 10;
        ch = *pstr++;
    }
    result /= 10;

    return result;
}


void _itoa( U32 num, char *pstr )
{
    U8 i = 7;

    // TODO memcpy(filename,LOG_FILENAME_TEMPLATE,sizeof(LOG_FILENAME_TEMPLATE));
    while( i > 2 && num > 0 )
    {
    	pstr[i--] = (num % 10) + '0';
        num /= 10;
    }
}


S32 FLOAT_to_S32( char *ptr, FLOAT number, U8 zeros ) //@todo
{
    U8 i;
    //BOOL sign = FALSE;
    FLOAT fvalue;

    if (number < 0)
    { /** Признак отрицательного числа **/
        //sign = TRUE ;
        number *= -1;
    }
    fvalue = number;
    for (i = 0; i < zeros; i++)
    {
        fvalue *= 10;
    }
    // 0.5 - округление в большую сторону
    return (S32)(fvalue + 0.5);
}


INT str_UTF_to_ANSI( char *src )
{
    U32 i, k;
    U8 n;
    U32 n1;

    test_param( NULL == src );
    k = 0;
    i = 0;
    while( src[ i ] != 0 )
    {
        n = src[ i ];
        if( n > 0xC0 )
        {
            n1 = n & 0x1F;
            i++;
            n = src[ i ];
            test_param( 0x80 != (n & 0xC0) );
            n1 = n1 << 6;
            n1 += n & 0x3F;
            n1 = n1 - 0x350; //correct, for ANSI-1251 - 192 start for crlic 'А'
            n = (U8)n1;
        }
        src[ k ] = n;
        i++;
        k++;
    }
    src[ k ] = 0; //null terminate

    return k;
}

U32 _strlen( const char *str ) // TODO max size string is 0xFFFF - 65535
{
    U32 i = 0;

    if( NULL != str )
    {
        while( '\0' != str[i] )
        {
            i++;
        }
    }

    return i;
}

U32 _strnlen( const char *str, U32 maxlen )
{
    U32 i = 0;

    if( NULL != str )
        while( '\0' != str[i] )
        {
            i++;
            if( i > maxlen )
            {
                return maxlen;
            }
        }

    return i;
}

S32 _strcmp( const char *strA, const char *strB )
{
    if( NULL == strA ) // pointer checking
    {
        return -2;
    }
    if( NULL == strB )
    {
        return 2;
    }
    while( 1 )
    {
        if( *strA != *strB )
        {
            return 3;//TODO
        }
        strA++;
        strB++;
        if (( *strA == 0 ) && ( *strB == 0 ))
        {
            return 0;
        }
        if(*strA == 0 )
        {
            return -1;
        }
        if (*strB == 0 ) //if strlen(strA) > strlen(strB)
        {
            return 1;
        }
    }
}


char *_strncmp( const char *strA, const char *strB, U32 size ) //TODO not work
{
    U32 i, j;
    U32 m, n;

    if ( NULL == strA ) // pointer checking
    {
        return (char *)NULL;
    }
    if ( NULL == strB )
    {
        return (char *)NULL;
    }
    m = _strlen( strA );
    if ( m >= 0xFFFF )  // max size checking
    {
        return (char *)NULL;
    }
    m = _strlen( strB );
    if (m >= 0xFFFF)
    {
        return (char *)NULL;
    }
    n = m;
    j = 0;
    while( '\0' != strA[j] )
    {
        if (strA[j] == strB[0])
        {
            m = n; //_strlen (strB);
            i = 0;
            while (('\0' != strB[i]) || ('\0' != strA[j + i]))
            {
                if (strA[j + i] == strB[i]) //
                    m--;
                else
                    break;
                i++;
            }
            if (0 == m)
            {
                return (char *)&strA[j];
            }
        }
        j++;
        if( j > size )
        {
            break;
        }
    }

    return (char *)NULL;
}


char char_to_lower_case( char ch )
{
    if ((ch >= 'A') && (ch <= 'Z'))
    {
        ch = (U8)ch + 32;
    }
    return ch;
}


char char_to_upper_case( char ch )
{
    if ((ch >= 'a') && (ch <= 'z'))
    {
        ch = (U8)ch - 32; //ch &= 0x5F;
    }
    /*
    else
    {//conversion Windows-1251
        if ((U8)ch >= (U8)('а'))// &&
            //((U8)(ch) <= (U8)(0xFF))) //'я'
        {
            ch -= ('А' - 'а');
        }
    }
    */
    return ch;
}


void str_to_lower_case( char *str )
{
   while (*str != 0)
   {
       *str = char_to_lower_case( *str );
       str++;
   }
}

void str_to_upper_case( char *str )
{
   while (*str != 0)
   {
       *str = char_to_upper_case( *str );
       str++;
   }
}

void str_remove_spaces( char *str )
{
   char *tp = str;
   while (*str != 0)
   {
       if (*str == ' ')
       {
           tp = str;
           do
           {
               *tp = *(tp+1);
               tp++;
           }
           while (*tp != 0);
       }
       str++;
   }
}

char *_strtok_r( char *str, const char *delim, char **last )
{
   U32 i;
   char *str_p;

   if (str == NULL)
   {
       if (*last == NULL)
       {
           return (char *)NULL;
       }
       str = *last;
   }
   str_p = str;
   while (*str_p != 0)
   {
       i = 0;
       while (delim[i] != 0)
       {
           if (*str_p == delim[i])
           {
               *last = str_p + 1;
               *str_p = 0;
               return str;
           }
           i++;
       }
       str_p++;
   }
   return (char *)NULL;
}

void *_memcpy( void *dst, const void *src, INT n )
{
    test_param( NULL == dst );
    test_param( NULL == src );
    test_param( n < 1 );

    register U8 *pdst = (U8 *)dst;
    register U8 *psrc = (U8 *)src;

    for( ; 0 < n; n-- )
    {
        *(U8 *)pdst++ = *(U8 *)psrc++;
    }

    /*
    INT i, m;

    U32 *wdst = (U32 *)dst;  // текущая позиция в буфере назначения
    U32 *wsrc = (U32 *)src;  // текущая позиция в источнике
    U8 *cdst, *csrc;

    for( i = 0, m = n / sizeof(long); i < m; i++ )  // копируем основную часть блоками по 4 или 8 байт
    {
       *(wdst++) = *(wsrc++);                     // (в зависимости от платформы)
    }

    cdst = (U8 *)wdst;
    csrc = (U8 *)wsrc;

    for( i = 0, m = n % sizeof(long); i < m; i++ ) // остаток копируем побайтно
    {
       *(cdst++) = *(csrc++);
    }

    return dst;
    */

    return pdst;
}

void *_memset( void *dst, U8 v, INT n )
{
    test_param( NULL == dst );
    test_param( n < 1 );

    register U8 *pdst = (U8 *)dst;

    for( ; 0 < n; n-- )
    {
        *(U8 *)pdst++ = v;
    }

    return pdst;
}


//skip leading space or tab chars in string
char *skip_space( char *ptr )
{
    while(1)
    {
        char c = *ptr;
        if ( c == ' ' || c == 9 )
        {
            ptr++;
            continue;
        }
        else
            break;
    }
    return ptr;
}

//skip from string word which we expect to see
char *skip_expected_word (char *ptr, char *pword, U32 *presult)
{
    //assume error result
    *presult = 0;
    ptr = skip_space(ptr);
    while(1)
    {
        char c1,c2;
        c2 = *pword;
        if(c2==0)
        {
            //end of comparing word
            *presult=1;
            return ptr;
        }
        c1 = *ptr;
        if(c1 == c2)
        {
            //at this point strings are equal, but compare next chars
            ptr++;
            pword++;
            continue;
        }
        else
            break; //string has no expected word
    }
    return ptr;
}

//get word from string
char *get_and_skip_word (char *ptr, char *pword, U32 *presult)
{
    uint32_t i;
    char c;
    *presult=0;

    //assume error result
    *presult=0;
    ptr = skip_space(ptr);
    for(i=0; i<8; i++)
    {
        c = *ptr;
        if(c==' ' || c==9 || c==0 || c==0xd || c==0xa) {
            //end of getting word
            *pword=0;
            *presult=1;
            return ptr;
        }
        *pword++ = c;
        ptr++;
    }
    *pword=0;
    return ptr;
}

//read decimal integer from string
//returned 0xffffffff mean error
char *get_dec_integer( char *ptr, uint32_t *presult )
{
    uint32_t r=0;
    ptr=skip_space(ptr);
    while(1) {
        char c;
        c= *ptr;
        if (c>='0' && c<='9') {
            r = r * 10 + (c-'0');
            ptr++;
            continue;
        }
        else
        if (c == ' ' || c == 9) {
            *presult=r;
            break;
        }
        else
        {
            //unexpected char
            *presult=0xffffffff;
            break;
        }
    }
    return ptr;
}


/* Function for scale gray image
 * pixelsIn - 1-sized 2x array for vaules
 * pixelsOut - 1-sized 2x array for vaules
 * w,h - original image
 * w2,h2 - sized imagee
 */
int resize_bilinear( S32 *pixelsIn, S32 *pixelsOut,
                        int w, int h, int w2, int h2)
{
    S32 A, B, C, D, x, y, index, temp;
    float x_ratio = ((float)(w-1))/w2 ;
    float y_ratio = ((float)(h-1))/h2 ;
    float x_diff, y_diff;
    int offset = 0 ;

    for( int i = 0; i < h2; i++ )
    {
        for( int j = 0; j < w2; j++ )
        {
            x = (int)(x_ratio * j) ;
            y = (int)(y_ratio * i) ;
            x_diff = (x_ratio * j) - x ;
            y_diff = (y_ratio * i) - y ;
            index = y*w+x ;

            // get incoming data for matrix
            A = pixelsIn[index];
            B = pixelsIn[index+1];
            C = pixelsIn[index+w];
            D = pixelsIn[index+w+1];

            // Y = A(1-w)(1-h) + B(w)(1-h) + C(h)(1-w) + Dwh
            temp = (int)(
                A*(1-x_diff)*(1-y_diff) + B*(x_diff)*(1-y_diff) +
                C*(y_diff)*(1-x_diff) + D*(x_diff*y_diff) ) ;


            //ampl value
            //float tst = temp;// * 1.6;
            //temp = tst;

            //set value to color table
            pixelsOut[offset++] = temp;
        }
    }
    return 0;
}


// Given a byte to transmit, this returns the parity as a nibble
U8 DL_HammingCalculateParity128( U8 value )
{
	// Exclusive OR is associative and commutative, so order of operations and values does not matter.
	U8 parity;

	if ((value & 1) != 0)
	{
		parity = 0x3;
	}
	else
	{
		parity = 0x0;
	}

	if ((value & 2) != 0)
	{
		parity ^= 0x5;
	}

	if ((value & 4) != 0)
	{
		parity ^= 0x6;
	}

	if ((value & 8) != 0)
	{
		parity ^= 0x7;
	}

	if ((value & 16) != 0)
	{
		parity ^= 0x9;
	}

	if ((value & 32) != 0)
	{
		parity ^= 0xA;
	}

	if ((value & 64) != 0)
	{
		parity ^= 0xB;
	}

	if ((value & 128) != 0)
	{
		parity ^= 0xC;
	}

	return parity;
}

// Given two bytes to transmit, this returns the parity
// as a byte with the lower nibble being for the first byte,
// and the upper nibble being for the second byte.
U8 DL_HammingCalculateParity2416(U8 first, U8 second)
{
	return (DL_HammingCalculateParity128(second) << 4) | DL_HammingCalculateParity128(first);
}




#define UNCORRECTABLE	0xFF
#define ERROR_IN_PARITY	0xFE
#define NO_ERROR		0x00

// Private table. Faster and more compact than multiple if statements.
static const U8 _hammingCorrect128Syndrome[16] =
{
	NO_ERROR,			// 0
	ERROR_IN_PARITY,	// 1
	ERROR_IN_PARITY,	// 2
	0x01,				// 3
	ERROR_IN_PARITY,	// 4
	0x02,				// 5
	0x04,				// 6
	0x08,				// 7
	ERROR_IN_PARITY,	// 8
	0x10,				// 9
	0x20,				// 10
	0x40,				// 11
	0x80,				// 12
	UNCORRECTABLE,		// 13
	UNCORRECTABLE,		// 14
	UNCORRECTABLE,		// 15
};

// Private method
// Give a pointer to a received byte,
// and given a nibble difference in parity (parity ^ calculated parity)
// this will correct the received byte value if possible.
// It returns the number of bits corrected:
// 0 means no errors
// 1 means one corrected error
// 3 means corrections not possible
static U8 DL_HammingCorrect128Syndrome(U8 *value, U8 syndrome)
{
	// Using only the lower nibble (& 0x0F), look up the bit
	// to correct in a table
	U8 correction = _hammingCorrect128Syndrome[syndrome & 0x0F];

	if (correction != NO_ERROR)
	{
		if (correction == UNCORRECTABLE || value == NULL)
		{
			return 3; // Non-recoverable error
		}
		else
		{
			if (correction != ERROR_IN_PARITY)
			{
				*value ^= correction;
			}

			return 1; // 1-bit recoverable error;
		}
	}

	return 0; // No errors
}

// Given a pointer to a received byte and the received parity (as a lower nibble),
// this calculates what the parity should be and fixes the recevied value if needed.
// It returns the number of bits corrected:
// 0 means no errors
// 1 means one corrected error
// 3 means corrections not possible
U8 DL_HammingCorrect128( U8 *value, U8 parity )
{
	U8 syndrome;

	if (value == NULL)
	{
		return 3; // Non-recoverable error
	}

	syndrome = DL_HammingCalculateParity128(*value) ^ parity;

	if (syndrome != 0)
	{
		return DL_HammingCorrect128Syndrome(value, syndrome);
	}

	return 0; // No errors
}


// Given a pointer to a first value and a pointer to a second value and
// their combined given parity (lower nibble first parity, upper nibble second parity),
// this calculates what the parity should be and fixes the values if needed.
// It returns the number of bits corrected:
// 0 means no errors
// 1 means one corrected error
// 2 means two corrected errors
// 3 means corrections not possible
U8 DL_HammingCorrect2416(U8 *first, U8 *second, U8 parity)
{
	U8 syndrome;

	if (first == NULL || second == NULL)
	{
		return 3; // Non-recoverable error
	}

	syndrome = DL_HammingCalculateParity2416(*first, *second) ^ parity;

	if (syndrome != 0)
	{
		return DL_HammingCorrect128Syndrome(first, syndrome) + DL_HammingCorrect128Syndrome(second, syndrome >> 4);
	}

	return 0; // No errors
}

FLOAT pressure_to_altitude( FLOAT sea_level_Pa, S32 pressure_Pa )
{
	FLOAT temp;

    temp = (FLOAT)pressure_Pa / sea_level_Pa; //seaLevelhPa  -  The current hPa at sea level
    temp = 1.0f - powf( (FLOAT)temp, (FLOAT)0.190294957 );
    temp = 44330.0f * temp;

    return temp; //get altitude in dm
}

FLOAT mmh_to_Pa( FLOAT mmh )
{
	return mmh * 133.3223684f;
}

void _misc_test( void )
{
#ifndef RESET_CORE_COUNT
#define RESET_CORE_COUNT
#endif
#ifndef GET_CORE_COUNT
#define GET_CORE_COUNT 1
#endif
    INT i;
    char str[ 256 ];
    S32 tmpS32;
    U8 tmpU8;
    U32 tic;

    U8 crypt_data_64b[8+1] = { "12345678" }; //64bit
    U32 key_128b[ 4 ] = { 0x00112233, 0x44556677, 0x8899AABB, 0xCCDDEEFF }; //128bit
    U8 key_256b[32+1] = { "pass12345678#$@!....++++----$$$$" }; //256bit

    // TEA =====================================================================
    _memcpy( str, crypt_data_64b, sizeof(crypt_data_64b) );
    RESET_CORE_COUNT;
    cipher_TEA_encrypt( (U32 *)str, (U32 *)key_128b );
    tic = GET_CORE_COUNT;
    RESET_CORE_COUNT;
    cipher_TEA_decrypt( (U32 *)str, (U32 *)key_128b );
    tic = GET_CORE_COUNT;
    for( i = 0; i < 8; i++ )
    {
		if( str[i] != crypt_data_64b[i] )
		{
			test_param(1);
		}
    }

    //RTEA =====================================================================
    _memcpy( str, crypt_data_64b, sizeof(crypt_data_64b) );
    RESET_CORE_COUNT;
    cipher_RTEA_encrypt( (U32 *)str, (U32 *)key_256b );
    tic = GET_CORE_COUNT;
    RESET_CORE_COUNT;
    cipher_RTEA_decrypt( (U32 *)str, (U32 *)key_256b );
    tic = GET_CORE_COUNT;
    for( i = 0; i < 8; i++ )
    {
		if( str[i] != crypt_data_64b[i] )
		{
			test_param(1);
		}
    }
    test_param( tic > 30000 );

	// SQRT ====================================================================
	tmpS32 = sqrt_fixed( 4 );
	tmpS32 = sqrt_fixed( 400 );
    tmpS32 = sqrt_fixed( 437 );
    tmpS32 = sqrt_fixed( 1024 );
    tmpS32 = sqrt_fixed( 0 );
    tmpS32 = sqrt_fixed( -1 );
    tmpS32 = sqrt_fixed( 0xFFFFFFFF );
    tmpS32 = sqrt_fixed( 0x7FFFFFFF );
    tmpS32 = sqrt_fixed( 7 );



    // test str_UTF_to_ANSI ====================================================
    const char test_rus[] = "1w АБВвбвя";
    i = _strlen( test_rus );
    _memcpy( str, test_rus, i + 1 ); // + null terminal
    i = str_UTF_to_ANSI( str );
    const char test_eng[] = "1w333ddftgdf";
    i = _strlen( test_eng );
    i++; //null terminal
    _memcpy( str, test_eng, i + 1 ); // + null terminal
    i = str_UTF_to_ANSI( str );

    // BCD =====================================================================
    tmpU8 = bcd8_to_dec8( 0 );
    tmpU8 = bcd8_to_dec8( 1 );
    tmpU8 = bcd8_to_dec8( 8 );
    tmpU8 = bcd8_to_dec8( 15 );
    tmpU8 = bcd8_to_dec8( 25 );
    tmpU8 = bcd8_to_dec8( 169 );
    tmpU8 = bcd8_to_dec8( 255 );

    tmpU8 = dec8_to_bcd8( 0 );
    tmpU8 = dec8_to_bcd8( 1 );
    tmpU8 = dec8_to_bcd8( 8 );
    tmpU8 = dec8_to_bcd8( 15 );
    tmpU8 = dec8_to_bcd8( 25 );
    tmpU8 = dec8_to_bcd8( 169 );
    tmpU8 = dec8_to_bcd8( 255 );

    tmpU8 = char_to_hex( 0 );
    //_assert( tmp8 == 0x00 );
    tmpU8 = char_to_hex( tmpU8 );


    // hash Lu =================================================================
    RESET_CORE_COUNT;
    U32 tmpU32 = hash_Ly( (char *)"12345678" );

    test_param( 5 != tmpU32 );
    tic = GET_CORE_COUNT;



    //RAID test ================================================================
    U32 A, B, C;
    A = 0x01234567;
    B = 0x89ABCDEF;
    C = A ^ B; //

    // corrupt data on disk A
    A = 0x01284567;
    //recovery
    A = B ^ C;
    test_param( 0x01234567 != A );
    
    // corrupt data on disk B
    B = 0x89ABCD0F;
    //recovery
    B = A ^ C;
    test_param( 0x89ABCDEF != B );
    
    // corrupt data on disk C
    C = 0x55556666;
    //recovery
    C = A ^ B;
    test_param( 0x89ABCDEF != C );

    //==========================================================================
    tmpU8 = 0x55;
    tmpU8 = DL_HammingCalculateParity128( 0x00 );
    tmpU8 = DL_HammingCalculateParity128( 0x55 );
    tmpU8 = DL_HammingCalculateParity128( 0xAA );
    tmpU8 = DL_HammingCalculateParity128( 0xFF );

    U8 value = 0x55;
    tmpU8 = DL_HammingCorrect128( &value, tmpU8 );

    (void)tmpU32;(void)tmpU8;
    (void)tmpS32;(void)tic;
}
