
/*


http://habrahabr.ru/post/247385/

https://www.google.com.ua/search?q=vector+quantizer&oq=vector+quantizer&aqs=chrome..69i57j0l5.2033j0j4&sourceid=chrome&es_sm=93&ie=UTF-8
https://github.com/GyverLibs/FFT_C/blob/main/src/FFT_C.h
http://habrahabr.ru/post/268797/
    
    */
#include "_dsp.h"
#include "board.h"
#include "_misc.h"
#include "_crc.h"
#include "_debug.h"
#include <math.h>


// https://chipenable.ru/index.php/programming-avr/144-sqrt-root.html
U32 _dsp_sqrtS32_( U32 value )
{
    unsigned int x;
    x = (value/0x3f + 0x3f)>>1;
    x = (value/x + x)>>1;
    x = (value/x + x)>>1;
    return (x);
}

U32 _dsp_sqrtU32( U32 value )
{
	test_param( value > 0x40000000 );

    U32 m, y, b;
    m = 0x40000000;
    y = 0;
    while( m != 0 )
    {
       b = y | m;
       y = y >> 1;
       if( value >= b )
       {
          value = value - b;
          y = y | m;
       }
       m = m >> 2;
    }
    return y;
}

U64 _dsp_sqrtU64( U64 value ) //todo
{
    U64 m, y, b;
    m = 0x4000000000000000;
    y = 0;
    while( m != 0 )
    {
       b = y | m;
       y = y >> 1;
       if( value >= b )
       {
          value = value - b;
          y = y | m;
       }
       m = m >> 2;
    }
    return y;
}



U32 _dsp_sqrt_( U32 value )
{
    //lint -save -e40 -e10 -e845 -e522
    register U32 d = 0, a = 0;

    // Находим количество битов дополняющих до 32, по сути a + b = 32, где и номер старшего бита
    // Затем из 32 вычитаем значение, получаем номер старшего бита b = 32 - a и кладем в 'а'
#if (__ARMCC_VERSION >= 6010050)
    __asm volatile (
    "CLZ    %[A], %[VALUE]"           "\n\t"
    "RSB    %[A], %[A], 0x1F"
            : [A]"=&r" (ax)
            : [VALUE]"r" (value), [A]"r" (ax)
    );
#else
//    __asm {
//        CLZ    ax, value
//        RSB    ax, ax, 0x1F
//    };
#endif


    /* Нахождение квадратного корня методом Ньютона. Основная формула для вычисления:
   *     sqrt = (sqrt' + value / sqrt') / 2
   *  где:
   *     value - значение корень которого находим
   *     sqrt - текущее значение приближения
   *     sqrt' - значение приближения из предыдущего шага
   *  Для ускорения первым приближением выбирается значение равное value сдвинутому на половину
   */
	d = 1 << ((a >> 1)+1);    // первое приближение
	a = (d + ( value >> a )) >> 1;
	d = (a + value / a)>>1;
	a = (d + value / d)>>1;
	d = (a + value / a)>>1;
	a = (d + value / d)>>1;

	return a;
}

void _dsp_dft( INT n, DSP_FFT_DIRECTION_t inverse, FLOAT *gRe, FLOAT *gIm, FLOAT *GRe, FLOAT *GIm )
{
    INT w,x;
    
    for( w = 0; w < n; w++ )
    {
        GRe[w] = GIm[w] = 0;
        for( x = 0; x < n; x++ )
        {
            FLOAT a = ( -2 * M_PI * w * x ) / (FLOAT)n;
            if( FT_DIRECT == inverse ) a = -a;
            FLOAT ca = cosf(a);
            FLOAT sa = sinf(a);
            GRe[w] += gRe[x] * ca - gIm[x] * sa;
            GIm[w] += gRe[x] * sa + gIm[x] * ca;
        }
        if( FT_DIRECT != inverse )
        {
            GRe[w] /= n;
            GIm[w] /= n;
        }
    }
}


/*
int32_t mul144(int32_t x) { return (x << 7) + (x << 4); }
int32_t mul96(int32_t x)  { return (x << 6) + (x << 5); }
int32_t mul48(int32_t x)  { return (x << 5) + (x << 4); }
typedef int32_t (*fmul32)(int32_t);
const fmul32 fmulVec[4] = { mul144, mul96, mul48};
// calculate FFT[10] for 32 samples
U8 fft10()
{
    U8 i;
    int32_t a = 0;
    for ( i = 0; i < 4; ++i)
    {
        a += fmulVec[i](i);
    }
}
*/

// https://lodev.org/cgtutor/fourier.html#signals
void _dsp_fft( INT n, DSP_FFT_DIRECTION_t inverse, FLOAT *gRe, FLOAT *gIm, FLOAT *GRe, FLOAT *GIm )
{
    //Calculate m=log_2(n)
    int m = 0;
    int p = 1;
    while(p < n)
    {
        p *= 2;
        m++;
    }
    //Bit reversal
    GRe[n - 1] = gRe[n - 1];
    GIm[n - 1] = gIm[n - 1];
    int j = 0;
    for(int i = 0; i < n - 1; i++)
    {
        GRe[i] = gRe[j];
        GIm[i] = gIm[j];
        int k = n / 2;
        while( k <= j )
        {
            j -= k;
            k /= 2;
        }
        j += k;
    }
    //Calculate the FFT
    FLOAT ca = -1.0;
    FLOAT sa = 0.0;
    INT l1 = 1, l2 = 1;
    for( INT l = 0; l < m; l++)
    {
        l1 = l2;
        l2 *= 2;
        FLOAT u1 = 1.0;
        FLOAT u2 = 0.0;
        for( INT j = 0; j < l1; j++)
        {
            for( INT i = j; i < n; i += l2)
            {
                INT i1 = i + l1;
                FLOAT t1 = u1 * GRe[i1] - u2 * GIm[i1];
                FLOAT t2 = u1 * GIm[i1] + u2 * GRe[i1];
                GRe[i1] = GRe[i] - t1;
                GIm[i1] = GIm[i] - t2;
                GRe[i] += t1;
                GIm[i] += t2;
            }
            FLOAT z =  u1 * ca - u2 * sa;
            u2 = u1 * sa + u2 * ca;
            u1 = z;
        }
        sa = sqrtf( (1.0 - ca) / 2.0 );
        if(!inverse) sa =-sa;
        ca = sqrtf( (1.0 + ca) / 2.0 );
    }
    //Divide through n if it isn't the IDFT
    if( FT_DIRECT == inverse )
        for( INT i = 0; i < n; i++)
        {
            GRe[i] /= n;
            GIm[i] /= n;
        }
}





// https://ru.wikibooks.org/wiki/%D0%A0%D0%B5%D0%B0%D0%BB%D0%B8%D0%B7%D0%B0%D1%86%D0%B8%D0%B8_%D0%B0%D0%BB%D0%B3%D0%BE%D1%80%D0%B8%D1%82%D0%BC%D0%BE%D0%B2/%D0%91%D1%8B%D1%81%D1%82%D1%80%D0%BE%D0%B5_%D0%BF%D1%80%D0%B5%D0%BE%D0%B1%D1%80%D0%B0%D0%B7%D0%BE%D0%B2%D0%B0%D0%BD%D0%B8%D0%B5_%D0%A4%D1%83%D1%80%D1%8C%D0%B5

//_________________________________________________________________________________________
//_________________________________________________________________________________________
//
// NAME:          FFT.
// PURPOSE:       Быстрое преобразование Фурье: Комплексный сигнал в комплексный спектр и обратно.
//                В случае действительного сигнала в мнимую часть (Idat) записываются нули.
//                Количество отсчетов - кратно 2**К - т.е. 2, 4, 8, 16, ... (см. комментарии ниже).
//
//
// PARAMETERS:
//
//    float *Rdat    [in, out] - Real part of Input and Output Data (Signal or Spectrum)
//    float *Idat    [in, out] - Imaginary part of Input and Output Data (Signal or Spectrum)
//    int    N       [in]      - Input and Output Data length (Number of samples in arrays)
//    int    LogN    [in]      - Logarithm2(N)
//    int    Ft_Flag [in]      - Ft_Flag = FT_ERR_DIRECT  (i.e. -1) - Direct  FFT  (Signal to Spectrum)
//		                 Ft_Flag = FT_ERR_INVERSE (i.e.  1) - Inverse FFT  (Spectrum to Signal)
//
// RETURN VALUE:  false on parameter error, true on success.
//_________________________________________________________________________________________
//
// NOTE: In this algorithm N and LogN can be only:
//       N    = 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384;
//       LogN = 2, 3,  4,  5,  6,   7,   8,   9,   10,   11,   12,   13,    14;
//_________________________________________________________________________________________
//_________________________________________________________________________________________

BOOL  _dsp_FFT_FLOAT( float *Rdat, float *Idat, int N, int LogN, DSP_FFT_DIRECTION_t Ft_Flag )
{
    // parameters error check:
	test_param( (Rdat == NULL) || (Idat == NULL) );
	test_param( (N > 16384) || (N < 1));
	test_param( !_number_is_2_POW_K(N) );
	test_param( (LogN < 2) || (LogN > 14) );
    test_param( (Ft_Flag != FT_DIRECT) && (Ft_Flag != FT_INVERSE) );
    
    register int  i, j, n, k, io, ie, in, nn;
    float         ru, iu, rtp, itp, rtq, itq, rw, iw, sr;
    
    static const float Rcoef[14] =
    {  
        -1.0000000000000000F,  0.0000000000000000F,  0.7071067811865475F,
        0.9238795325112867F,  0.9807852804032304F,  0.9951847266721969F,
        0.9987954562051724F,  0.9996988186962042F,  0.9999247018391445F,
        0.9999811752826011F,  0.9999952938095761F,  0.9999988234517018F,
        0.9999997058628822F,  0.9999999264657178F
    };
    static const float Icoef[14] =
    {   
         0.0000000000000000F, -1.0000000000000000F, -0.7071067811865474F,
        -0.3826834323650897F, -0.1950903220161282F, -0.0980171403295606F,
        -0.0490676743274180F, -0.0245412285229122F, -0.0122715382857199F,
        -0.0061358846491544F, -0.0030679567629659F, -0.0015339801862847F,
        -0.0007669903187427F, -0.0003834951875714F
    };
    
    nn = N >> 1;
    ie = N;
    for (n = 1; n<=LogN; n++)
    {
        rw = Rcoef[LogN - n];
        iw = Icoef[LogN - n];
        if (Ft_Flag == FT_INVERSE) iw = -iw;
        in = ie >> 1;
        ru = 1.0F;
        iu = 0.0F;
        for (j=0; j<in; j++)
        {
            for (i=j; i<N; i+=ie)
            {
                io       = i + in;
                rtp      = Rdat[i]  + Rdat[io];
                itp      = Idat[i]  + Idat[io];
                rtq      = Rdat[i]  - Rdat[io];
                itq      = Idat[i]  - Idat[io];
                Rdat[io] = rtq * ru - itq * iu;
                Idat[io] = itq * ru + rtq * iu;
                Rdat[i]  = rtp;
                Idat[i]  = itp;
            }
            
            sr = ru;
            ru = ru * rw - iu * iw;
            iu = iu * rw + sr * iw;
        }
        
        ie >>= 1;
    }
    
    for (j= i = 1; i<N; i++)
    {
        if(i < j)
        {
            io       = i - 1;
            in       = j - 1;
            rtp      = Rdat[in];
            itp      = Idat[in];
            Rdat[in] = Rdat[io];
            Idat[in] = Idat[io];
            Rdat[io] = rtp;
            Idat[io] = itp;
        }
        k = nn;
        while (k < j)
        {
            j   = j - k;
            k >>= 1;
        }
        j = j + k;
    }
    
    if(Ft_Flag == FT_DIRECT) return TRUE;
    
    rw = 1.0F / N;
    
    for(i=0; i<N; i++)
    {
        Rdat[i] *= rw;
        Idat[i] *= rw;
    }

  return TRUE;
}





// sin/cos =====================================================================
#define ANGLE_64                        0
#define ANGLE_360                       1

#if ANGLE_64
#define COSPI_TABLE_SIZE                16
static const S32 cospi[COSPI_TABLE_SIZE+1]=
{
    (S32)(RANGE * 1.00000000F),  //0
    (S32)(RANGE * 0.99518473F),
    (S32)(RANGE * 0.98078528F),
    (S32)(RANGE * 0.95694034F),
    (S32)(RANGE * 0.92387953F),
    (S32)(RANGE * 0.88192126F),
    (S32)(RANGE * 0.83146961F),
    (S32)(RANGE * 0.77301045F),//7
    (S32)(RANGE * 0.70710678F),
    (S32)(RANGE * 0.63439328F),
    (S32)(RANGE * 0.55557023F),
    (S32)(RANGE * 0.47139674F),
    (S32)(RANGE * 0.38268343F),
    (S32)(RANGE * 0.29028468F),
    (S32)(RANGE * 0.19509032F),
    (S32)(RANGE * 0.09801714F),//15
    (S32)(RANGE * 0.0F)  //16
};
#endif

#if ANGLE_360
#define COSPI_TABLE_SIZE                90
static const S32 cospi[COSPI_TABLE_SIZE+1]=
{
    (S32)((FLOAT)RANGE * 1.00000000000000000000000000000000F), //0
    (S32)((FLOAT)RANGE * 0.99984769515639123915701155881391F), //1
    (S32)((FLOAT)RANGE * 0.99939082701909573000624344004393F), //2
    (S32)((FLOAT)RANGE * 0.99862953475457387378449205843944F), //3
    (S32)((FLOAT)RANGE * 0.99756405025982424761316268064426F), //4
    (S32)((FLOAT)RANGE * 0.99619469809174553229501040247389F), //5
    (S32)((FLOAT)RANGE * 0.99452189536827333692269194498057F), //6
    (S32)((FLOAT)RANGE * 0.99254615164132203498006158933058F), //7
    (S32)((FLOAT)RANGE * 0.99026806874157031508377486734485F), //8
    (S32)((FLOAT)RANGE * 0.98768834059513772619004024769344F), //9
    (S32)((FLOAT)RANGE * 0.98480775301220805936674302458952F), //10
    (S32)((FLOAT)RANGE * 0.98162718344766395349650489981814F), //11
    (S32)((FLOAT)RANGE * 0.97814760073380563792856674786960F), //12
    (S32)((FLOAT)RANGE * 0.97437006478523522853969448008827F), //13
    (S32)((FLOAT)RANGE * 0.97029572627599647230637787403399F), //14
    (S32)((FLOAT)RANGE * 0.96592582628906828674974319972890F), //15
    (S32)((FLOAT)RANGE * 0.96126169593831886191649704855706F), //16
    (S32)((FLOAT)RANGE * 0.95630475596303548133865081661842F), //17
    (S32)((FLOAT)RANGE * 0.95105651629515357211643933337938F), //18
    (S32)((FLOAT)RANGE * 0.94551857559931681034812470751940F), //19
    (S32)((FLOAT)RANGE * 0.93969262078590838405410927732473F), //20
    (S32)((FLOAT)RANGE * 0.93358042649720174899004306313957F), //21
    (S32)((FLOAT)RANGE * 0.92718385456678740080647445113696F), //22
    (S32)((FLOAT)RANGE * 0.92050485345244032739689472330046F), //23
    (S32)((FLOAT)RANGE * 0.91354545764260089550212757198532F), //24
    (S32)((FLOAT)RANGE * 0.90630778703664996324255265675432F), //25
    (S32)((FLOAT)RANGE * 0.89879404629916699278229567669579F), //26
    (S32)((FLOAT)RANGE * 0.89100652418836786235970957141363F), //27
    (S32)((FLOAT)RANGE * 0.88294759285892694203217136031572F), //28
    (S32)((FLOAT)RANGE * 0.87461970713939580028463695866108F), //29
    (S32)((FLOAT)RANGE * 0.86602540378443864676372317075294F), //30
    (S32)((FLOAT)RANGE * 0.85716730070211228746521798014476F), //31
    (S32)((FLOAT)RANGE * 0.84804809615642597038617617869039F), //32
    (S32)((FLOAT)RANGE * 0.83867056794542402963759094180455F), //33
    (S32)((FLOAT)RANGE * 0.82903757255504169200633684150164F), //34
    (S32)((FLOAT)RANGE * 0.81915204428899178968448838591684F), //35
    (S32)((FLOAT)RANGE * 0.80901699437494742410229341718282F), //36
    (S32)((FLOAT)RANGE * 0.79863551004729284628400080406894F), //37
    (S32)((FLOAT)RANGE * 0.78801075360672195669397778783585F), //38
    (S32)((FLOAT)RANGE * 0.77714596145697087997993774367240F), //39
    (S32)((FLOAT)RANGE * 0.76604444311897803520239265055542F), //40
    (S32)((FLOAT)RANGE * 0.75470958022277199794298421956102F), //41
    (S32)((FLOAT)RANGE * 0.74314482547739423501469704897426F), //42
    (S32)((FLOAT)RANGE * 0.73135370161917048328754360827562F), //43
    (S32)((FLOAT)RANGE * 0.71933980033865113935605467445671F), //44
    (S32)((FLOAT)RANGE * 0.70710678118654752440084436210485F), //45
    (S32)((FLOAT)RANGE * 0.69465837045899728665640629942269F), //46
    (S32)((FLOAT)RANGE * 0.68199836006249850044222578471113F), //47
    (S32)((FLOAT)RANGE * 0.66913060635885821382627333068678F), //48
    (S32)((FLOAT)RANGE * 0.65605902899050728478249596402342F), //49
    (S32)((FLOAT)RANGE * 0.64278760968653932632264340990726F), //50
    (S32)((FLOAT)RANGE * 0.62932039104983745270590245827997F), //51
    (S32)((FLOAT)RANGE * 0.61566147532565827966881109284366F), //52
    (S32)((FLOAT)RANGE * 0.60181502315204827991797700044149F), //53
    (S32)((FLOAT)RANGE * 0.58778525229247312916870595463907F), //54
    (S32)((FLOAT)RANGE * 0.57357643635104609610803191282616F), //55
    (S32)((FLOAT)RANGE * 0.55919290347074683016042813998599F), //56
    (S32)((FLOAT)RANGE * 0.54463903501502708222408369208157F), //57
    (S32)((FLOAT)RANGE * 0.52991926423320495404678115181609F), //58
    (S32)((FLOAT)RANGE * 0.51503807491005421008163193639814F), //59
    (S32)((FLOAT)RANGE * 0.50000000000000000000000000000000F), //60
    (S32)((FLOAT)RANGE * 0.48480962024633702907537962241578F), //61
    (S32)((FLOAT)RANGE * 0.46947156278589077595946228822784F), //62
    (S32)((FLOAT)RANGE * 0.45399049973954679156040836635787F), //63
    (S32)((FLOAT)RANGE * 0.43837114678907741745273454065827F), //64
    (S32)((FLOAT)RANGE * 0.42261826174069943618697848964773F), //65
    (S32)((FLOAT)RANGE * 0.40673664307580020775398599034150F), //66
    (S32)((FLOAT)RANGE * 0.39073112848927375506208458888909F), //67
    (S32)((FLOAT)RANGE * 0.37460659341591203541496377450120F), //68
    (S32)((FLOAT)RANGE * 0.35836794954530027348413778941347F), //69
    (S32)((FLOAT)RANGE * 0.34202014332566873304409961468226F), //70
    (S32)((FLOAT)RANGE * 0.32556815445715666871400893579472F), //71
    (S32)((FLOAT)RANGE * 0.30901699437494742410229341718282F), //72
    (S32)((FLOAT)RANGE * 0.29237170472273672809746869537714F), //73
    (S32)((FLOAT)RANGE * 0.27563735581699918564997157461130F), //74
    (S32)((FLOAT)RANGE * 0.25881904510252076234889883762405F), //75
    (S32)((FLOAT)RANGE * 0.24192189559966772256044237410035F), //76
    (S32)((FLOAT)RANGE * 0.20791169081775933710174228440513F), //78
    (S32)((FLOAT)RANGE * 0.19080899537654481240514048795839F), //79
    (S32)((FLOAT)RANGE * 0.17364817766693034885171662676931F), //80
    (S32)((FLOAT)RANGE * 0.15643446504023086901010531946717F), //81
    (S32)((FLOAT)RANGE * 0.13917310096006544411249666330111F), //82
    (S32)((FLOAT)RANGE * 0.12186934340514748111289391923153F), //83
    (S32)((FLOAT)RANGE * 0.10452846326765347139983415480250F), //84
    (S32)((FLOAT)RANGE * 0.08715574274765817355806427083747F), //85
    (S32)((FLOAT)RANGE * 0.06975647374412530077595883519414F), //86
    (S32)((FLOAT)RANGE * 0.05233595624294383272211862960908F), //87
    (S32)((FLOAT)RANGE * 0.03489949670250097164599518162533F), //88
    (S32)((FLOAT)RANGE * 0.01745240643728351281941897851632F), //89
    (S32)((FLOAT)RANGE * 0.00000000000000000000000000000000F)  //90
};
#endif


S32 CODE sin_fixed( S32 angle )
{
#if ANGLE_64
    S32 a1, a2;
    a1 = angle & 0x000F;
    a2 = angle >> 4 ;
    switch ( a2 )
    {
    case 0:
        a2 = (cospi[16 - a1]);
        break;
    case 1:
        a2 = (cospi[a1]);
        break;
    case 2:
        a2 = (cospi[16 - a1] * (-1));
        break;
    case 3:
        a2 = (cospi[a1] * (-1));
        break;
    default:
        a2 = 0;
        break;
    }

    return a2;
#endif
#if ANGLE_360
    S32 a1, a2;
    a1 = angle % 90;
    a2 = ( angle / 90 ) % 4;
    switch ( a2 ) //get quadrant
    {
    case 0:
        a2 = (cospi[90 - a1]);
        break;
    case 1:
        a2 = (cospi[a1]);
        break;
    case 2:
        a2 = (cospi[90 - a1] * (-1));
        break;
    case 3:
        a2 = (cospi[a1] * (-1));
        break;
    default:
        a2 = 0;
        break;
    }

    return a2;
#endif
}

S32 CODE cos_fixed( S32 angle )
{
#if ANGLE_64
    S32 a1, a2;
    a1 = angle & 0x000F;
    a2 = angle >> 4;
    switch ( a2 )
    {
    case 0:
        a2 = (cospi[a1]);
        break;
    case 1:
        a2 = (cospi[COSPI_TABLE_SIZE - a1] * (-1));
        break;
    case 2:
        a2 = (cospi[a1] * (-1));
        break;
    case 3:
        a2 = (cospi[COSPI_TABLE_SIZE - a1]);
        break;
    default:
        a2 = 0;
        break;
    }

    return a2;
#endif
#if ANGLE_360
    S32 a1, a2;
    a1 = angle % 90;
    a2 = angle / 90;
    switch ( a2 ) //get quadrant
    {
    case 0:
        a2 = (cospi[a1]);
        break;
    case 1:
        a2 = (cospi[COSPI_TABLE_SIZE - a1] * (-1));
        break;
    case 2:
        a2 = (cospi[a1] * (-1));
        break;
    case 3:
        a2 = (cospi[COSPI_TABLE_SIZE - a1]);
        break;
    default:
        a2 = 0;
        break;
    }

    return a2;
#endif
}


FLOAT _grad_to_rad( FLOAT grad )
{
    return (grad * M_PI) / 180.0;
}

FLOAT _rad_to_grad( FLOAT rad )
{
    return ( rad * 180.0 ) / M_PI;
}


//__align(4) #pragma pack(push, 1)
static const S16 SineTable[360] = 
{
         0,    174,    348,    523,    697,    871,   1045,   1218,   1391, 
      1564,   1736,   1908,   2079,   2249,   2419,   2588,   2756,   2923, 
      3090,   3255,   3420,   3583,   3746,   3907,   4067,   4226,   4383, 
      4539,   4694,   4848,   4999,   5150,   5299,   5446,   5591,   5735, 
      5877,   6018,   6156,   6293,   6427,   6560,   6691,   6819,   6946, 
      7071,   7193,   7313,   7431,   7547,   7660,   7771,   7880,   7986, 
      8090,   8191,   8290,   8386,   8480,   8571,   8660,   8746,   8829, 
      8910,   8987,   9063,   9135,   9205,   9271,   9335,   9396,   9455, 
      9510,   9563,   9612,   9659,   9702,   9743,   9781,   9816,   9848, 
      9876,   9902,   9925,   9945,   9961,   9975,   9986,   9993,   9998, 
     10000,   9998,   9993,   9986,   9975,   9961,   9945,   9925,   9902, 
      9876,   9848,   9816,   9781,   9743,   9702,   9659,   9612,   9563, 
      9510,   9455,   9396,   9335,   9271,   9205,   9135,   9063,   8987, 
      8910,   8829,   8746,   8660,   8571,   8480,   8386,   8290,   8191, 
      8090,   7986,   7880,   7771,   7660,   7547,   7431,   7313,   7193, 
      7071,   6946,   6819,   6691,   6560,   6427,   6293,   6156,   6018, 
      5877,   5735,   5591,   5446,   5299,   5150,   5000,   4848,   4694, 
      4539,   4383,   4226,   4067,   3907,   3746,   3583,   3420,   3255, 
      3090,   2923,   2756,   2588,   2419,   2249,   2079,   1908,   1736, 
      1564,   1391,   1218,   1045,    871,    697,    523,    348,    174, 
         0,   -174,   -348,   -523,   -697,   -871,  -1045,  -1218,  -1391, 
     -1564,  -1736,  -1908,  -2079,  -2249,  -2419,  -2588,  -2756,  -2923, 
     -3090,  -3255,  -3420,  -3583,  -3746,  -3907,  -4067,  -4226,  -4383, 
     -4539,  -4694,  -4848,  -4999,  -5150,  -5299,  -5446,  -5591,  -5735, 
     -5877,  -6018,  -6156,  -6293,  -6427,  -6560,  -6691,  -6819,  -6946, 
     -7071,  -7193,  -7313,  -7431,  -7547,  -7660,  -7771,  -7880,  -7986, 
     -8090,  -8191,  -8290,  -8386,  -8480,  -8571,  -8660,  -8746,  -8829, 
     -8910,  -8987,  -9063,  -9135,  -9205,  -9271,  -9335,  -9396,  -9455, 
     -9510,  -9563,  -9612,  -9659,  -9702,  -9743,  -9781,  -9816,  -9848, 
     -9876,  -9902,  -9925,  -9945,  -9961,  -9975,  -9986,  -9993,  -9998, 
    -10000,  -9998,  -9993,  -9986,  -9975,  -9961,  -9945,  -9925,  -9902, 
     -9876,  -9848,  -9816,  -9781,  -9743,  -9702,  -9659,  -9612,  -9563, 
     -9510,  -9455,  -9396,  -9335,  -9271,  -9205,  -9135,  -9063,  -8987, 
     -8910,  -8829,  -8746,  -8660,  -8571,  -8480,  -8386,  -8290,  -8191, 
     -8090,  -7986,  -7880,  -7771,  -7660,  -7547,  -7431,  -7313,  -7193, 
     -7071,  -6946,  -6819,  -6691,  -6560,  -6427,  -6293,  -6156,  -6018, 
     -5877,  -5735,  -5591,  -5446,  -5299,  -5150,  -5000,  -4848,  -4694, 
     -4539,  -4383,  -4226,  -4067,  -3907,  -3746,  -3583,  -3420,  -3255, 
     -3090,  -2923,  -2756,  -2588,  -2419,  -2249,  -2079,  -1908,  -1736, 
     -1564,  -1391,  -1218,  -1045,   -871,   -697,   -523,   -348,   -174
}; 


S16	_sin_table (S16 angle)
{
	S16 a = (angle % 359);
	return SineTable[a];
}


S16	_cos_table (S16 angle)
{
	S16 a = ((angle + 90) % 359);
	return SineTable[a];
}


int next_amp(int dph)
{
    static int phase = 0;
    int amp;
    phase += dph;
    amp = 511.5 * sin(2 * M_PI * phase / 0x100000000L);
    return amp;
}


U32 _next_amp(U32 dph)
{
    static U32 phase = 0;
    U32 amp;
    phase += dph;
    amp = 511.5 * sin(2 * M_PI * phase / 0x100000000L);
    return amp;
}


S32 _dsp_avg( S32 *pbuf, UINT len )
{
    S64 result = 0;
    while( len-- )
    {
        result += *pbuf++;
    }

#if DEBUG_DSP_OVERFLOW
    if( result > 0xFFFFFFFF ) //test
    {
        return -1;
    }
#endif

    return result / len;
}


RET _dsp_sint_sin( S32 *pbuf, U32 period_size, U32 ampl, S32 adder )
{
	U32 i;

    if( period_size > 10000 ) return RET_ERROR;
    //if( ampl > 10000 ) return RET_ERROR;

    DOUBLE k = (DOUBLE)360.0 / (DOUBLE)period_size;
    for( i = 0; i < period_size; i++ )
    {
        DOUBLE fsin = sin( ( _PI * (DOUBLE)i * k ) / 180.0 ) * (DOUBLE)ampl;
        pbuf[ i ] = fsin + adder;
        //raw_buf[0][ i ] = fsin / 300;
        //xsprintf( str, "    rom_sin[%u] = 16'h%u %d;\r\n", sin_cnt, usin, (U32(fsin*1000000)) );
        //log_print( str );
    }

    return RET_OK;
}


RET _dsp_sint_cos( S32 *pbuf, U32 period_size, U32 ampl, S32 adder )
{
	U32 i;

    if( period_size > 10000 ) return RET_ERROR;
    if( ampl > 10000 ) return RET_ERROR;

    DOUBLE k = (DOUBLE)360.0 / (DOUBLE)period_size;
    for( i = 0; i < period_size; i++ )
    {
        DOUBLE fcos = cos( ( _PI * (DOUBLE)i * k ) / 180.0 ) * (DOUBLE)ampl;
        pbuf[ i ] = fcos + adder;
        //raw_buf[0][ i ] = fsin / 300;
        //xsprintf( str, "    rom_cos[%u] = 16'h%u %d;\r\n", sin_cnt, usin, (U32(fsin*1000000)) );
        //log_print( str );
    }

    return RET_OK;
}


U32 _dsp_dds_sint_adder( U32 sampling_rate, U32 freq )
{
    if( freq >= sampling_rate )
    {
        return 1;
    }
    // DDSd = 2n × f / fDDSCLK
    // DDSd = 2^32 × 1 kHz / 100 kHz = 42949673
    U32 DDSd = ( (U64)4294967296 * (U64)freq ) / (U64)sampling_rate;
    //The theoretical frequency resolution is given by
    //fDDSCLK / 2n = 2 MHz / 232 = 0.00046... Hz.
    return DDSd;
}


RET _dsp_goertzel_init( U32 sampling_rate, U32 freq, S8 *psin_buf, S8 *pcos_buf, U32 size, U32 ampl )
{
    UINT i;

    DOUBLE omega = ( 2.0 * _PI * (DOUBLE)freq ) / (DOUBLE)sampling_rate;
    for( i = 0; i < size; i++ )
    {
        psin_buf[ i ] = sin( (DOUBLE)i * omega ) * (DOUBLE)ampl;
        pcos_buf[ i ] = cos( (DOUBLE)i * omega ) * (DOUBLE)ampl;
    }

    return RET_OK;
}



S64 _dsp_goertzel_run( S8 *psin_buf, S8 *pcos_buf, S32 *pbuf, U32 size, U32 scaling )
{
    S64 re = 0;
    S64 im = 0;
    S64 mag;

    while( size-- )
    {
        S32 v = *pbuf++;
        re += *psin_buf++ * v;
        im += *pcos_buf++ * v;
    }

    re = re * re;
    im = im * im;

    mag = re + im;

    mag = mag >> scaling;

#if DEBUG_DSP_OVERFLOW
    if( mag > 0x7FFFFFFF ) //test
    {
        return 1;
    }
#endif

    return ( mag );//_dsp_sqrtU32
}

U64 _dsp_goertzel_S32_init( U32 sampling_rate, U32 freq, S32 *pbuf, U32 size, U32 ampl, U32 scaling )
{
	return 0;
}

//S32 _dsp_goertzel_S32_( S16 *pbuf, S32 Fs, S32 Ft, INT N, U32 b )


U64 _dsp_goertzel( U32 sampling_rate, U32 freq, S32 *pbuf, U32 size, U32 ampl, U32 scaling )
{
    S64 re = 0;
    S64 im = 0;
    S64 mag;

    //DOUBLE omega = ( 2.0 * _PI * (DOUBLE)freq ) / size; //(DOUBLE)sampling_rate;
    FLOAT omega = 2.0*M_PI*((FLOAT)size*(FLOAT)freq/(FLOAT)sampling_rate)/(FLOAT)size;

    FLOAT cw = cos( omega );
    FLOAT sw = sin( omega );
    FLOAT ca = 2. * cw;

    FLOAT v0, v1, v2;

    v0 = v1 = v2 = 0;
    while( size-- )
    {
        v0 = v1;
        v1 = v2;
        v2 = (FLOAT)(*pbuf) + ca * v1 - v0;
        pbuf++;
    }
    re = cw * v2 - v1;
    im = sw * v2;

    re = _abs( re );//* re;
    im = _abs( im );//* im;

    mag = re + im;

    mag = mag >> scaling;
#if DEBUG_DSP_OVERFLOW
    if( mag > 100000 ) //test
    {
        return 100000;
    }
#endif

    //return _dsp_sqrt( mag );
    return ( mag );
}

////convert from Re/Im to Amplitude/Phase
//AnalyseResultType do_result_conversion(float real, float imag)
//{
//  AnalyseResultType result;

//  float amplitude = imag * imag + real * real;
//  amplitude = sqrtf(amplitude);
//  result.Amplitude = (uint16_t)(amplitude / INT_COEF);

//  float phase = atan2f(imag, real);
//  phase = phase * (float)(PHASE_MULT * HMAX_ANGLE) / M_PI;
//  result.Phase = (int16_t)(phase);

//  return result;
//}


/*
 * Fs - Hz sampling frequency
 * S = (float)(1<<b); // scaling factor scaling is 2^b
 */

FLOAT _dsp_goertzel_FLOAT( S32 *pbuf, U32 Fs, U32 Ft, U32 N, U32 ampl, U32 S )
{
	// w = 2*pi*k/N;
    FLOAT omega = ( 2.0 * _PI * (FLOAT)Ft ) / (FLOAT)Fs;
    FLOAT cw = cos( omega ) * (FLOAT)ampl;
    FLOAT sw = sin( omega ) * (FLOAT)ampl;
    FLOAT c = 2.0 * cw;
    FLOAT z0, z1, z2;
    z1 = z2 = 0;
    FLOAT tmp;
    while( N-- )
    {
        tmp = *pbuf++;
        z0 = tmp + c * z1 - z2;
        z2 = z1;
        z1 = z0;
    }
    FLOAT It = cw * z1 - z2;
    FLOAT Qt = sw * z2;

    FLOAT I = It * It;
    FLOAT Q = Qt * Qt;

    FLOAT mag = I + Q;

    return sqrt( mag );
}

#define MAXN 1000
S32 win[MAXN]; // Window

S32 _dsp_goertzel_S32_( S16 *pbuf, S32 Fs, S32 Ft, INT N, U32 b )
{
	FLOAT w, S;

	S = 1 << b;

    //FLOAT omega;// = ( 2.0 * _PI * (DOUBLE)freq ) / (DOUBLE)ADC_SAMPLING_RATE;
    for( INT  i = 0; i < N; i++ )
	{
		 // init window (Hamming)
         win[i] = (S32)round(S*(0.54 -0.46*cosf(2.0*M_PI*(FLOAT)i/(FLOAT)(N-1))));
	}
	// init Goertzel constants
	// CORDIC may be used here to compute sin() and cos()
	w = 2.0*M_PI*round((FLOAT)N*(FLOAT)Ft/(FLOAT)Fs)/(FLOAT)N;
	S32 cw = (S32)round(S*cosf(w));
    S32 c = cw * 2; //<<1
	S32 sw = (S32)round(S*sinf(w));

	// Goertzel reset
	S32 z0;
	S32 z1 = 0;
	S32 z2 = 0;
	for( INT  n = 0; n < N; n++ )
	{
        S32 x = *pbuf++;
        //x = ( ( x * win[n] ) >> b ); // windowing

		// **** GOERTZEL ITERATION ****
		z0 = x + ((c*z1)>>b) - z2; // Goertzel iteration
		z2 = z1;
		z1 = z0; // Goertzel status update
	}

	// CORDIC may be used here to compute atan2() and sqrt()
    S64 I = ((cw*z1)>>b) - z2; // Goertzel final I
    S64 Q = ((sw*z1)>>b); // Goertzel final Q
    S64 M = I*I + Q*Q; // magnitude squared

    return sqrt( M );
}


void _dsp_test( void )
{
	INT i;
    //U32 tic;
    S32 tmpU32;

	// SQRT ====================================================================
    U32 sqrt_values[8] = { 4, 6555, 9999999, 0x7FFFFFFF };
    U32 sqrt_results[8] = { 2, 80, 3162, 0x7FFFFFFF };
    for( i = 0; i < 4; i++ )
    {
        //RESET_CORE_COUNT;
		tmpU32 = _dsp_sqrtU32( sqrt_values[i] );
        //tic = GET_CORE_COUNT;
		if( sqrt_results[i] != tmpU32 )
		{
			test_param(1);
		}
    }


    //(void)tic;
}


