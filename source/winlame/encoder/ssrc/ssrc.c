#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <time.h>

#define NDEBUG

#define VERSION "1.29"

#ifndef HIGHPREC
typedef float REAL;
double AA=120;
double DF=100;
int FFTFIRLEN=16384;
#define M 15
#else
typedef double REAL;
double AA=170;
double DF=100;
int FFTFIRLEN=65536;
#define M 15
#endif


#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795028842
#endif

#define RANDBUFLEN 65536

#define RINT(x) ((x) >= 0 ? ((int)((x) + 0.5)) : ((int)((x) - 0.5)))

#if 0
double fact[M+1];
#endif

extern void rdft(int, int, REAL *, int *, REAL *);
extern double dbesi0(double);

const int scoeffreq[] = {0,48000,44100,37800,32000,22050,48000,44100};


const int scoeflen[] =  {1,16,20,16,16,15,16,15};
const int samp[] =  {8,18,27,8,8,8,10,9};

const double shapercoefs[8][21] = {
  {-1}, /* triangular dither */

  {-2.8720729351043701172,   5.0413231849670410156,  -6.2442994117736816406,   5.8483986854553222656,
   -3.7067542076110839844,   1.0495119094848632812,   1.1830236911773681641,  -2.1126792430877685547,
    1.9094531536102294922,  -0.99913084506988525391,  0.17090806365013122559,  0.32615602016448974609,
   -0.39127644896507263184,  0.26876461505889892578, -0.097676105797290802002, 0.023473845794796943665,
  }, /* 48k, N=16, amp=18 */

  {-2.6773197650909423828,   4.8308925628662109375,  -6.570110321044921875,    7.4572014808654785156,
   -6.7263274192810058594,   4.8481650352478027344,  -2.0412089824676513672,  -0.7006359100341796875,
    2.9537565708160400391,  -4.0800385475158691406,   4.1845216751098632812,  -3.3311812877655029297,
    2.1179926395416259766,  -0.879302978515625,       0.031759146600961685181, 0.42382788658142089844,
   -0.47882103919982910156,  0.35490813851356506348, -0.17496839165687561035,  0.060908168554306030273,
  }, /* 44.1k, N=20, amp=27 */

  {-1.6335992813110351562,   2.2615492343902587891,  -2.4077029228210449219,   2.6341717243194580078,
   -2.1440362930297851562,   1.8153258562088012695,  -1.0816224813461303711,   0.70302653312683105469,
   -0.15991993248462677002, -0.041549518704414367676, 0.29416576027870178223, -0.2518316805362701416,
    0.27766478061676025391, -0.15785403549671173096,  0.10165894031524658203, -0.016833892092108726501,
  }, /* 37.8k, N=16 */

  {-0.82901298999786376953,  0.98922657966613769531, -0.59825712442398071289,  1.0028809309005737305,
   -0.59938216209411621094,  0.79502451419830322266, -0.42723315954208374023,  0.54492527246475219727,
   -0.30792605876922607422,  0.36871799826622009277, -0.18792048096656799316,  0.2261127084493637085,
   -0.10573341697454452515,  0.11435490846633911133, -0.038800679147243499756, 0.040842197835445404053,
  }, /* 32k, N=16 */

  {-0.065229974687099456787, 0.54981261491775512695,  0.40278548002243041992,  0.31783768534660339355,
    0.28201797604560852051,  0.16985194385051727295,  0.15433363616466522217,  0.12507140636444091797,
    0.08903945237398147583,  0.064410120248794555664, 0.047146003693342208862, 0.032805237919092178345,
    0.028495194390416145325, 0.011695005930960178375, 0.011831838637590408325,
  }, /* 22.05k, N=15 */

  {-2.3925774097442626953,   3.4350297451019287109,  -3.1853709220886230469,   1.8117271661758422852,
    0.20124770700931549072, -1.4759907722473144531,   1.7210904359817504883,  -0.97746700048446655273,
    0.13790138065814971924,  0.38185903429985046387, -0.27421241998672485352, -0.066584214568138122559,
    0.35223302245140075684, -0.37672343850135803223,  0.23964276909828186035, -0.068674825131893157959,
  }, /* 48k, N=16, amp=10 */

  {-2.0833916664123535156,   3.0418450832366943359,  -3.2047898769378662109,   2.7571926116943359375,
   -1.4978630542755126953,   0.3427594602108001709,   0.71733748912811279297, -1.0737057924270629883,
    1.0225815773010253906,  -0.56649994850158691406,  0.20968692004680633545,  0.065378531813621520996,
   -0.10322438180446624756,  0.067442022264003753662, 0.00495197344571352005,
  }, /* 44.1k, N=15, amp=9 */

#if 0
  { -3.0259189605712890625,  6.0268716812133789062,  -9.195003509521484375,   11.824929237365722656,
   -12.767142295837402344,  11.917946815490722656,   -9.1739168167114257812,   5.3712320327758789062,
    -1.1393624544143676758, -2.4484779834747314453,   4.9719839096069335938,  -6.0392003059387207031,
     5.9359521865844726562, -4.903278350830078125,    3.5527443885803222656,  -2.1909697055816650391,
     1.1672389507293701172, -0.4903914332389831543,   0.16519790887832641602, -0.023217858746647834778,
  }, /* 44.1k, N=20 */
#endif
};

double **shapebuf;
int shaper_type,shaper_len,shaper_clipmin,shaper_clipmax;
REAL *randbuf;
int randptr;
int quiet = 0;
int lastshowed2;
time_t starttime,lastshowed;

#define POOLSIZE 97

int init_shaper(int freq,int nch,int min,int max,int dtype,int pdf,double noiseamp)
{
  int i;
  int pool[POOLSIZE];

  for(i=1;i<6;i++) if (freq == scoeffreq[i]) break;
  if ((dtype == 3 || dtype == 4) && i == 6) {
    fprintf(stderr,"Warning: ATH based noise shaping for destination frequency %dHz is not available, using triangular dither\n",freq);
  }
  if (dtype == 2 || i == 6) i = 0;
  if (dtype == 4 && (i == 1 || i == 2)) i += 5;

  shaper_type = i;

  shapebuf = malloc(sizeof(double *)*nch);
  shaper_len = scoeflen[shaper_type];

  for(i=0;i<nch;i++)
    shapebuf[i] = calloc(shaper_len,sizeof(double));

  shaper_clipmin = min;
  shaper_clipmax = max;

  randbuf = malloc(sizeof(REAL) * RANDBUFLEN);

  for(i=0;i<POOLSIZE;i++) pool[i] = rand();

  switch(pdf)
    {
    case 0: // rectangular
      for(i=0;i<RANDBUFLEN;i++)
	{
	  int r,p;

	  p = rand() % POOLSIZE;
	  r = pool[p]; pool[p] = rand();
	  randbuf[i] = noiseamp * (((double)r)/RAND_MAX-0.5);
	}
      break;

    case 1: // triangular
      for(i=0;i<RANDBUFLEN;i++)
	{
	  int r1,r2,p;

	  p = rand() % POOLSIZE;
	  r1 = pool[p]; pool[p] = rand();
	  p = rand() % POOLSIZE;
	  r2 = pool[p]; pool[p] = rand();
	  randbuf[i] = noiseamp * ((((double)r1)/RAND_MAX)-(((double)r2)/RAND_MAX));
	}
      break;

    case 2: // gaussian
      for(i=0;i<RANDBUFLEN;i++)
	{
	  int sw=0;
	  double t,u;
	  double r;
	  int p;

	  if (sw == 0) {
	    sw = 1;

	    p = rand() % POOLSIZE;
	    r = ((double)pool[p])/RAND_MAX; pool[p] = rand();

	    t = sqrt(-2 * log(1-r));

	    p = rand() % POOLSIZE;
	    r = ((double)pool[p])/RAND_MAX; pool[p] = rand();
	  
	    u = 2 * M_PI * r;
	    
	    randbuf[i] = noiseamp * t * cos(u);
	  } else {
	    sw = 0;

	    randbuf[i] = noiseamp * t * sin(u);
	  }
	}
      break;
    }

  randptr = 0;

  if (dtype == 0 || dtype == 1) return 1;
  return samp[shaper_type];
}

int do_shaping(double s,double *peak,int dtype,int ch)
{
  double u,h;
  int i;

  if (dtype == 1) {
    s += randbuf[randptr++ & (RANDBUFLEN-1)];

    if (s < shaper_clipmin) {
      double d = (double)s/shaper_clipmin;
      *peak = *peak < d ? d : *peak;
      s = shaper_clipmin;
    }
    if (s > shaper_clipmax) {
      double d = (double)s/shaper_clipmax;
      *peak = *peak < d ? d : *peak;
      s = shaper_clipmax;
    }

    return RINT(s);
  }

  h = 0;
  for(i=0;i<shaper_len;i++) h += shapercoefs[shaper_type][i]*shapebuf[ch][i];
  s += h;
  u = s;
  s += randbuf[randptr++ & (RANDBUFLEN-1)];
  if (s < shaper_clipmin) {
    double d = (double)s/shaper_clipmin;
    *peak = *peak < d ? d : *peak;
    s = shaper_clipmin;
  }
  if (s > shaper_clipmax) {
    double d = (double)s/shaper_clipmax;
    *peak = *peak < d ? d : *peak;
    s = shaper_clipmax;
  }
  s = RINT(s);
  for(i=shaper_len-2;i>=0;i--) shapebuf[ch][i+1] = shapebuf[ch][i];
  shapebuf[ch][0] = s-u;

  return (int)s;
}

void quit_shaper(int nch)
{
  int i;

  for(i=0;i<nch;i++) free(shapebuf[i]);
  free(shapebuf);
  free(randbuf);
}

double alpha(double a)
{
  if (a <= 21) return 0;
  if (a <= 50) return 0.5842*pow(a-21,0.4)+0.07886*(a-21);
  return 0.1102*(a-8.7);
}

#if 0
double izero(double x)
{
  double ret = 1;
  int m;

  for(m=1;m<=M;m++)
    {
      double s,t;
      int i;
      t = pow(x/2,m)/fact[m];
      ret += t*t;
    }

  return ret;
}
#endif

double win(double n,int len,double alp,double iza)
{
  return dbesi0(alp*sqrt(1-4*n*n/(((double)len-1)*((double)len-1))))/iza;
}

double sinc(double x)
{
  return x == 0 ? 1 : sin(x)/x;
}

double hn_lpf(int n,double lpf,double fs)
{
  double t = 1/fs;
  double omega = 2*M_PI*lpf;
  return 2*lpf*t*sinc(n*omega*t);
}

void usage(void)
{
  printf("http://shibatch.sourceforge.net/\n\n");
  printf("usage: ssrc [<options>] <source wav file> <destination wav file>\n");
  printf("options : --rate <sampling rate>     output sample rate\n");
  printf("          --att <attenuation(dB)>    attenuate signal\n");
  printf("          --bits <number of bits>    output quantization bit length\n");
  printf("          --tmpfile <file name>      specify temporal file\n");
  printf("          --twopass                  two pass processing to avoid clipping\n");
  printf("          --normalize                normalize the wave file\n");
  printf("          --quiet                    nothing displayed except error\n");
  printf("          --dither [<type>]          dithering\n");
  printf("                                       0 : no dither\n");
  printf("                                       1 : no noise shaping\n");
  printf("                                       2 : triangular spectral shape\n");
  printf("                                       3 : ATH based noise shaping\n");
  printf("                                       4 : less dither amplitude than type 3\n");
  printf("          --pdf <type> [<amp>]       select p.d.f. of noise\n");
  printf("                                       0 : rectangular\n");
  printf("                                       1 : triangular\n");
  printf("                                       2 : Gaussian\n");
#ifndef HIGHPREC
  printf("          --profile <type>           specify profile\n");
  printf("                                       standard : the default quality\n");
  printf("                                       fast     : fast, not so bad quality\n");
#endif
}

void fmterr(int x)
{
  fprintf(stderr,"unknown error %d\n",x);
  exit(-1);
}

void setstarttime(void)
{
  starttime = time(NULL);
  lastshowed = 0;
  lastshowed2 = -1;
}

void showprogress(double p)
{
  int eta,pc;
  time_t t;
  if (quiet) return;

  t = time(NULL)-starttime;
  if (p == 0) eta = 0; else eta = t*(1-p)/p;

  pc = (int)(p*100);

  if (pc != lastshowed2 || t != lastshowed) {
    printf(" %3d%% processed",pc);
    lastshowed2 = pc;
  }
  if(t != lastshowed) {
    printf(", ETA =%4dsec",eta);
    lastshowed = t;
  }
  printf("\r");
  fflush(stdout);
}

int gcd(int x, int y)
{
    int t;

    while (y != 0) {
        t = x % y;  x = y;  y = t;
    }
    return x;
}

double upsample(FILE *fpi,FILE *fpo,int nch,int bps,int dbps,int sfrq,int dfrq,double gain,unsigned int chanklen,int twopass,int dither)
{
  int frqgcd,osf,fs1,fs2;
  REAL **stage1,*stage2;
  int n1,n1x,n1y,n2,n2b;
  int filter2len;
  int *f1order,*f1inc;
  int *fft_ip = NULL;
  REAL *fft_w = NULL;
  unsigned char *rawinbuf,*rawoutbuf;
  REAL *inbuf,*outbuf;
  REAL **buf1,**buf2;
  double peak=0;
  int spcount = 0;
  int i,j;

  filter2len = FFTFIRLEN; /* stage 2 filter length */

  /* Make stage 1 filter */

  {
    double aa = AA; /* stop band attenuation(dB) */
    double lpf,delta,d,df,alp,iza;
    double guard = 2;

    frqgcd = gcd(sfrq,dfrq);

    fs1 = sfrq / frqgcd * dfrq;

    if (fs1/dfrq == 1) osf = 1;
    else if (fs1/dfrq % 2 == 0) osf = 2;
    else if (fs1/dfrq % 3 == 0) osf = 3;
    else {
      fprintf(stderr,"Resampling from %dHz to %dHz is not supported.\n",sfrq,dfrq);
      fprintf(stderr,"%d/gcd(%d,%d)=%d must be divided by 2 or 3.\n",sfrq,sfrq,dfrq,fs1/dfrq);
      exit(-1);
    }

    df = (dfrq*osf/2 - sfrq/2) * 2 / guard;
    lpf = sfrq/2 + (dfrq*osf/2 - sfrq/2)/guard;

    delta = pow(10,-aa/20);
    if (aa <= 21) d = 0.9222; else d = (aa-7.95)/14.36;

    n1 = fs1/df*d+1;
    if (n1 % 2 == 0) n1++;

    alp = alpha(aa);
    iza = dbesi0(alp);
    //printf("iza = %g\n",iza);

    n1y = fs1/sfrq;
    n1x = n1/n1y+1;

    f1order = malloc(sizeof(int)*n1y*osf);
    for(i=0;i<n1y*osf;i++) {
      f1order[i] = fs1/sfrq-(i*(fs1/(dfrq*osf)))%(fs1/sfrq);
      if (f1order[i] == fs1/sfrq) f1order[i] = 0;
    }

    f1inc = malloc(sizeof(int)*n1y*osf);
    for(i=0;i<n1y*osf;i++) {
      f1inc[i] = f1order[i] < fs1/(dfrq*osf) ? nch : 0;
      if (f1order[i] == fs1/sfrq) f1order[i] = 0;
    }

    stage1 = malloc(sizeof(REAL *)*n1y);
    stage1[0] = malloc(sizeof(REAL)*n1x*n1y);

    for(i=1;i<n1y;i++) {
      stage1[i] = &(stage1[0][n1x*i]);
      for(j=0;j<n1x;j++) stage1[i][j] = 0;
    }

    for(i=-(n1/2);i<=n1/2;i++)
      {
	stage1[(i+n1/2)%n1y][(i+n1/2)/n1y] = win(i,n1,alp,iza)*hn_lpf(i,lpf,fs1)*fs1/sfrq;
      }
  }

  /* Make stage 2 filter */

  {
    double aa = AA; /* stop band attenuation(dB) */
    double lpf,delta,d,df,alp,iza;
    int ipsize,wsize;

    delta = pow(10,-aa/20);
    if (aa <= 21) d = 0.9222; else d = (aa-7.95)/14.36;

    fs2 = dfrq*osf;

    for(i=1;;i = i * 2)
      {
	n2 = filter2len * i;
	if (n2 % 2 == 0) n2--;
	df = (fs2*d)/(n2-1);
	lpf = sfrq/2;
	if (df < DF) break;
      }

    alp = alpha(aa);

    iza = dbesi0(alp);

    for(n2b=1;n2b<n2;n2b*=2);
    n2b *= 2;

    stage2 = malloc(sizeof(REAL)*n2b);

    for(i=0;i<n2b;i++) stage2[i] = 0;

    for(i=-(n2/2);i<=n2/2;i++) {
      stage2[i+n2/2] = win(i,n2,alp,iza)*hn_lpf(i,lpf,fs2)/n2b*2;
    }

    ipsize    = 2+sqrt(n2b);
    fft_ip    = malloc(sizeof(int)*ipsize);
    fft_ip[0] = 0;
    wsize     = n2b/2;
    fft_w     = malloc(sizeof(REAL)*wsize);

    rdft(n2b,1,stage2,fft_ip,fft_w);
  }

  /* Apply filters */

  setstarttime();

  {
    int n2b2 = n2b/2;
    int rp;        // inbufのfs1での次に読むサンプルの場所を保持
    int ds;        // 次にdisposeするsfrqでのサンプル数
    int nsmplwrt1; // 実際にファイルからinbufに読み込まれた値から計算した
                   // stage2 filterに渡されるサンプル数
    int nsmplwrt2; // 実際にファイルからinbufに読み込まれた値から計算した
                   // stage2 filterに渡されるサンプル数
    int s1p;       // stage1 filterから出力されたサンプルの数をn1y*osfで割った余り
    int init,ending;
    unsigned int sumread,sumwrite;
    int osc;
    REAL *ip,*ip_backup;
    int s1p_backup,osc_backup;
    int k,ch,p;
    int inbuflen;
    int delay = 0;

    buf1 = malloc(sizeof(REAL *)*nch);
    for(i=0;i<nch;i++)
      {
	buf1[i] = malloc(sizeof(REAL)*(n2b2/osf+1));
	for(j=0;j<(n2b2/osf+1);j++) buf1[i][j] = 0;
      }

    buf2 = malloc(sizeof(REAL *)*nch);
    for(i=0;i<nch;i++) buf2[i] = malloc(sizeof(REAL)*n2b);

    rawinbuf  = calloc(nch*(n2b2+n1x),bps);
    rawoutbuf = malloc(dbps*nch*(n2b2/osf+1));

    inbuf  = calloc(nch*(n2b2+n1x),sizeof(REAL));
    outbuf = malloc(sizeof(REAL)*nch*(n2b2/osf+1));

    s1p = 0;
    rp  = 0;
    ds  = 0;
    osc = 0;

    init = 1;
    ending = 0;
    inbuflen = n1/2/(fs1/sfrq)+1;
    delay = (double)n2/2/(fs2/dfrq);

    sumread = sumwrite = 0;

    for(;;)
      {
	int nsmplread,toberead,toberead2;

	toberead2 = toberead = floor((double)n2b2*sfrq/(dfrq*osf))+1+n1x-inbuflen;
	if (toberead+sumread > chanklen) {
	  toberead = chanklen-sumread;
	}

	nsmplread = fread(rawinbuf,1,bps*nch*toberead,fpi);
	nsmplread /= bps*nch;

	switch(bps)
	  {
	  case 1:
	    for(i = 0; i < nsmplread * nch; i++)
	      inbuf[nch * inbuflen + i] =
		(1 / (REAL)0x7f) * ((REAL)((unsigned char *)rawinbuf)[i]-128);
	    break;

	  case 2:
#ifndef BIGENDIAN
	    for(i=0;i<nsmplread*nch;i++)
	      inbuf[nch*inbuflen+i] = (1/(REAL)0x7fff)*(REAL)((short *)rawinbuf)[i];
#else
	    for(i=0;i<nsmplread*nch;i++) {
	      inbuf[nch*inbuflen+i] = (1/(REAL)0x7fff)*
		(((int)rawinbuf[i*2]) |
		 (((int)((char *)rawinbuf)[i*2+1]) << 8));
	    }
#endif
	    break;

	  case 3:
	    for(i=0;i<nsmplread*nch;i++) {
	      inbuf[nch*inbuflen+i] = (1/(REAL)0x7fffff)*
		((((int)rawinbuf[i*3  ]) << 0 ) |
		 (((int)rawinbuf[i*3+1]) << 8 ) |
		 (((int)((char *)rawinbuf)[i*3+2]) << 16));
	    }
	    break;

	  case 4:
	    for(i=0;i<nsmplread*nch;i++) {
	      inbuf[nch*inbuflen+i] = (1/(REAL)0x7fffffff)*
		((((int)rawinbuf[i*4  ]) << 0 ) |
		 (((int)rawinbuf[i*4+1]) << 8 ) |
		 (((int)rawinbuf[i*4+2]) << 16) |
		 (((int)((char *)rawinbuf)[i*4+3]) << 24));
	    }
	    break;
	  }

	for(;i<nch*toberead2;i++) inbuf[nch*inbuflen+i] = 0;

	inbuflen += toberead2;

	sumread += nsmplread;

	ending = feof(fpi) || sumread >= chanklen;

	//nsmplwrt1 = ((rp-1)*sfrq/fs1+inbuflen-n1x)*dfrq*osf/sfrq;
	//if (nsmplwrt1 > n2b2) nsmplwrt1 = n2b2;
	nsmplwrt1 = n2b2;


	// apply stage 1 filter

	ip = &inbuf[((sfrq*(rp-1)+fs1)/fs1)*nch];

	s1p_backup = s1p;
	ip_backup  = ip;
	osc_backup = osc;

	for(ch=0;ch<nch;ch++)
	  {
	    REAL *op = &outbuf[ch];
	    int fdo = fs1/(dfrq*osf),no = n1y*osf;

	    s1p = s1p_backup; ip = ip_backup+ch;

	    switch(n1x)
	      {
	      case 7:
		for(p=0;p<nsmplwrt1;p++)
		  {
		    int s1o = f1order[s1p];

		    buf2[ch][p] =
		      stage1[s1o][0] * *(ip+0*nch)+
		      stage1[s1o][1] * *(ip+1*nch)+
		      stage1[s1o][2] * *(ip+2*nch)+
		      stage1[s1o][3] * *(ip+3*nch)+
		      stage1[s1o][4] * *(ip+4*nch)+
		      stage1[s1o][5] * *(ip+5*nch)+
		      stage1[s1o][6] * *(ip+6*nch);
		    
		    ip += f1inc[s1p];

		    s1p++;
		    if (s1p == no) s1p = 0;
		  }
		break;

	      case 9:
		for(p=0;p<nsmplwrt1;p++)
		  {
		    int s1o = f1order[s1p];

		    buf2[ch][p] =
		      stage1[s1o][0] * *(ip+0*nch)+
		      stage1[s1o][1] * *(ip+1*nch)+
		      stage1[s1o][2] * *(ip+2*nch)+
		      stage1[s1o][3] * *(ip+3*nch)+
		      stage1[s1o][4] * *(ip+4*nch)+
		      stage1[s1o][5] * *(ip+5*nch)+
		      stage1[s1o][6] * *(ip+6*nch)+
		      stage1[s1o][7] * *(ip+7*nch)+
		      stage1[s1o][8] * *(ip+8*nch);
		    
		    ip += f1inc[s1p];

		    s1p++;
		    if (s1p == no) s1p = 0;
		  }
		break;

	      default:
		for(p=0;p<nsmplwrt1;p++)
		  {
		    REAL tmp = 0;
		    REAL *ip2=ip;

		    int s1o = f1order[s1p];

		    for(i=0;i<n1x;i++)
		      {
			tmp += stage1[s1o][i] * *ip2;
			ip2 += nch;
		      }
		    buf2[ch][p] = tmp;

		    ip += f1inc[s1p];

		    s1p++;
		    if (s1p == no) s1p = 0;
		  }
		break;
	      }

	    osc = osc_backup;

	    // apply stage 2 filter

	    for(p=nsmplwrt1;p<n2b;p++) buf2[ch][p] = 0;

	    //for(i=0;i<n2b2;i++) printf("%d:%g ",i,buf2[ch][i]);

	    rdft(n2b,1,buf2[ch],fft_ip,fft_w);

	    buf2[ch][0] = stage2[0]*buf2[ch][0];
	    buf2[ch][1] = stage2[1]*buf2[ch][1]; 

	    for(i=1;i<n2b/2;i++)
	      {
		REAL re,im;

		re = stage2[i*2  ]*buf2[ch][i*2] - stage2[i*2+1]*buf2[ch][i*2+1];
		im = stage2[i*2+1]*buf2[ch][i*2] + stage2[i*2  ]*buf2[ch][i*2+1];

		//printf("%d : %g %g %g %g %g %g\n",i,stage2[i*2],stage2[i*2+1],buf2[ch][i*2],buf2[ch][i*2+1],re,im);

		buf2[ch][i*2  ] = re;
		buf2[ch][i*2+1] = im;
	      }

	    rdft(n2b,-1,buf2[ch],fft_ip,fft_w);

	    for(i=osc,j=0;i<n2b2;i+=osf,j++)
	      {
		REAL f = (buf1[ch][j] + buf2[ch][i]);
		op[j*nch] = f;
	      }

	    nsmplwrt2 = j;

	    osc = i - n2b2;

	    for(j=0;i<n2b;i+=osf,j++)
	      buf1[ch][j] = buf2[ch][i];
	  }

	rp += nsmplwrt1 * (sfrq / frqgcd) / osf;

	if (twopass) {
	  for(i=0;i<nsmplwrt2*nch;i++)
	    {
	      REAL f = outbuf[i] > 0 ? outbuf[i] : -outbuf[i];
	      peak = peak < f ? f : peak;
	      ((REAL *)rawoutbuf)[i] = outbuf[i];
	    }
	} else {
	  switch(dbps)
	    {
	    case 1:
	      {
		REAL gain2 = gain * (REAL)0x7f;
		ch = 0;

		for(i=0;i<nsmplwrt2*nch;i++)
		  {
		    int s;

		    if (dither) {
		      s = do_shaping(outbuf[i]*gain2,&peak,dither,ch);
		    } else {
		      s = RINT(outbuf[i]*gain2);

		      if (s < -0x80) {
			double d = (double)s/-0x80;
			peak = peak < d ? d : peak;
			s = -0x80;
		      }
		      if (0x7f <  s) {
			double d = (double)s/ 0x7f;
			peak = peak < d ? d : peak;
			s =  0x7f;
		      }
		    }

		    ((unsigned char *)rawoutbuf)[i] = s + 0x80;

		    ch++;
		    if (ch == nch) ch = 0;
		  }
	      }
	    break;

	    case 2:
	      {
		REAL gain2 = gain * (REAL)0x7fff;
		ch = 0;

		for(i=0;i<nsmplwrt2*nch;i++)
		  {
		    int s;

		    if (dither) {
		      s = do_shaping(outbuf[i]*gain2,&peak,dither,ch);
		    } else {
		      s = RINT(outbuf[i]*gain2);

		      if (s < -0x8000) {
			double d = (double)s/-0x8000;
			peak = peak < d ? d : peak;
			s = -0x8000;
		      }
		      if (0x7fff <  s) {
			double d = (double)s/ 0x7fff;
			peak = peak < d ? d : peak;
			s =  0x7fff;
		      }
		    }

#ifndef BIGENDIAN
		    ((short *)rawoutbuf)[i] = s;
#else
		    ((char *)rawoutbuf)[i*2  ] = s & 255; s >>= 8;
		    ((char *)rawoutbuf)[i*2+1] = s & 255;
#endif
		    ch++;
		    if (ch == nch) ch = 0;
		  }
	      }
	    break;

	    case 3:
	      {
		REAL gain2 = gain * (REAL)0x7fffff;
		ch = 0;

		for(i=0;i<nsmplwrt2*nch;i++)
		  {
		    int s;

		    if (dither) {
		      s = do_shaping(outbuf[i]*gain2,&peak,dither,ch);
		    } else {
		      s = RINT(outbuf[i]*gain2);

		      if (s < -0x800000) {
			double d = (double)s/-0x800000;
			peak = peak < d ? d : peak;
			s = -0x800000;
		      }
		      if (0x7fffff <  s) {
			double d = (double)s/ 0x7fffff;
			peak = peak < d ? d : peak;
			s =  0x7fffff;
		      }
		    }

		    ((char *)rawoutbuf)[i*3  ] = s & 255; s >>= 8;
		    ((char *)rawoutbuf)[i*3+1] = s & 255; s >>= 8;
		    ((char *)rawoutbuf)[i*3+2] = s & 255;

		    ch++;
		    if (ch == nch) ch = 0;
		  }
	      }
	    break;

#if 0
	    case 4:
	      {
		REAL gain2 = gain * (REAL)0x7fffffff;
		ch = 0;

		for(i=0;i<nsmplwrt2*nch;i++)
		  {
		    int s;

		    if (dither) {
		      s = do_shaping(outbuf[i]*gain2,&peak,dither,ch);
		    } else {
		      s = RINT(outbuf[i]*gain2);

		      if (s < -0x80000000) {
			double d = (double)s/-0x80000000;
			peak = peak < d ? d : peak;
			s = -0x80000000;
		      }
		      if (0x7fffffff <  s) {
			double d = (double)s/ 0x7fffffff;
			peak = peak < d ? d : peak;
			s =  0x7fffffff;
		      }
		    }

		    ((char *)rawoutbuf)[i*4  ] = s & 255; s >>= 8;
		    ((char *)rawoutbuf)[i*4+1] = s & 255; s >>= 8;
		    ((char *)rawoutbuf)[i*4+2] = s & 255; s >>= 8;
		    ((char *)rawoutbuf)[i*4+3] = s & 255;

		    ch++;
		    if (ch == nch) ch = 0;
		  }
	      }
	    break;
#endif
	    }
	}

	if (!init) {
	  if (ending) {
	    if ((double)sumread*dfrq/sfrq+2 > sumwrite+nsmplwrt2) {
	      if (dbps*nch*nsmplwrt2 != fwrite(rawoutbuf,1,dbps*nch*nsmplwrt2,fpo)) {
		fprintf(stderr,"fwrite error(1).\n");
		abort();
	      }
	      sumwrite += nsmplwrt2;
	    } else {
	      if (dbps*nch*(floor((double)sumread*dfrq/sfrq)+2-sumwrite) != 
		  fwrite(rawoutbuf,1,dbps*nch*(floor((double)sumread*dfrq/sfrq)+2-sumwrite),fpo)) {
		fprintf(stderr,"fwrite error(2).\n");
		abort();
	      }
	      break;
	    }
	  } else {
	    if (dbps*nch*nsmplwrt2 != fwrite(rawoutbuf,1,dbps*nch*nsmplwrt2,fpo)) {
	      fprintf(stderr,"fwrite error(3).\n");
	      abort();
	    }
	    sumwrite += nsmplwrt2;
	  }
	} else {
	  int pos,len;
	  if (nsmplwrt2 < delay) {
	    delay -= nsmplwrt2;
	  } else {
	    if (ending) {
	      if ((double)sumread*dfrq/sfrq+2 > sumwrite+nsmplwrt2-delay) {
		if (dbps*nch*(nsmplwrt2-delay) != fwrite(rawoutbuf+dbps*nch*delay,1,dbps*nch*(nsmplwrt2-delay),fpo)) {
		  fprintf(stderr,"fwrite error(4).\n");
		  abort();
		}
		sumwrite += nsmplwrt2-delay;
	      } else {
		if (dbps*nch*(floor((double)sumread*dfrq/sfrq)+2-sumwrite-delay) !=
		    fwrite(rawoutbuf+dbps*nch*delay,1,dbps*nch*(floor((double)sumread*dfrq/sfrq)+2-sumwrite-delay),fpo)) {
		  fprintf(stderr,"fwrite error(5).\n");
		  abort();
		}
		break;
	      }
	    } else {
	      if (dbps*nch*(nsmplwrt2-delay) != fwrite(rawoutbuf+dbps*nch*delay,1,dbps*nch*(nsmplwrt2-delay),fpo)) {
		fprintf(stderr,"fwrite error(6).\n");
		abort();
	      }
	      sumwrite += nsmplwrt2-delay;
	      init = 0;
	    }
	  }
	}

	{
	  int ds = (rp-1)/(fs1/sfrq);

	  assert(inbuflen >= ds);

	  memmove(inbuf,inbuf+nch*ds,sizeof(REAL)*nch*(inbuflen-ds));
	  inbuflen -= ds;
	  rp -= ds*(fs1/sfrq);
	}

	if ((spcount++ & 7) == 7) showprogress((double)sumread / chanklen);
      }
  }

  showprogress(1);

  free(f1order);
  free(f1inc);
  free(stage1[0]);
  free(stage1);
  free(stage2);
  free(fft_ip);
  free(fft_w);
  for(i=0;i<nch;i++) free(buf1[i]);
  free(buf1);
  for(i=0;i<nch;i++) free(buf2[i]);
  free(buf2);
  free(inbuf);
  free(outbuf);
  free(rawinbuf);
  free(rawoutbuf);

  return peak;
}

double downsample(FILE *fpi,FILE *fpo,int nch,int bps,int dbps,int sfrq,int dfrq,double gain,unsigned int chanklen,int twopass,int dither)
{
  int frqgcd,osf,fs1,fs2;
  REAL *stage1,**stage2;
  int n2,n2x,n2y,n1,n1b;
  int filter1len;
  int *f2order,*f2inc;
  int *fft_ip = NULL;
  REAL *fft_w = NULL;
  unsigned char *rawinbuf,*rawoutbuf;
  REAL *inbuf,*outbuf;
  REAL **buf1,**buf2;
  int i,j;
  int spcount = 0;
  double peak=0;

  filter1len = FFTFIRLEN; /* stage 1 filter length */

  /* Make stage 1 filter */

  {
    double aa = AA; /* stop band attenuation(dB) */
    double lpf,delta,d,df,alp,iza;
    int ipsize,wsize;

    frqgcd = gcd(sfrq,dfrq);

    if (dfrq/frqgcd == 1) osf = 1;
    else if (dfrq/frqgcd % 2 == 0) osf = 2;
    else if (dfrq/frqgcd % 3 == 0) osf = 3;
    else {
      fprintf(stderr,"Resampling from %dHz to %dHz is not supported.\n",sfrq,dfrq);
      fprintf(stderr,"%d/gcd(%d,%d)=%d must be divided by 2 or 3.\n",dfrq,sfrq,dfrq,dfrq/frqgcd);
      exit(-1);
    }

    fs1 = sfrq*osf;

    delta = pow(10,-aa/20);
    if (aa <= 21) d = 0.9222; else d = (aa-7.95)/14.36;

    n1 = filter1len;
    for(i=1;;i = i * 2)
      {
	n1 = filter1len * i;
	if (n1 % 2 == 0) n1--;
	df = (fs1*d)/(n1-1);
	lpf = (dfrq-df)/2;
	if (df < DF) break;
      }

    alp = alpha(aa);

    iza = dbesi0(alp);

    for(n1b=1;n1b<n1;n1b*=2);
    n1b *= 2;

    stage1 = malloc(sizeof(REAL)*n1b);

    for(i=0;i<n1b;i++) stage1[i] = 0;

    for(i=-(n1/2);i<=n1/2;i++) {
      stage1[i+n1/2] = win(i,n1,alp,iza)*hn_lpf(i,lpf,fs1)*fs1/sfrq/n1b*2;
    }

    ipsize    = 2+sqrt(n1b);
    fft_ip    = malloc(sizeof(int)*ipsize);
    fft_ip[0] = 0;
    wsize     = n1b/2;
    fft_w     = malloc(sizeof(REAL)*wsize);

    rdft(n1b,1,stage1,fft_ip,fft_w);
  }

  /* Make stage 2 filter */

  if (osf == 1) {
    fs2 = sfrq/frqgcd*dfrq;
    n2 = 1;
    n2y = n2x = 1;
    f2order = malloc(sizeof(int)*n2y);
    f2order[0] = 0;
    f2inc = malloc(sizeof(int)*n2y);
    f2inc[0] = sfrq/dfrq;
    stage2 = malloc(sizeof(REAL *)*n2y);
    stage2[0] = malloc(sizeof(REAL)*n2x*n2y);
    stage2[0][0] = 1;
  } else {
    double aa = AA; /* stop band attenuation(dB) */
    double lpf,delta,d,df,alp,iza;
    double guard = 2;

    fs2 = sfrq / frqgcd * dfrq ;

    df = (fs1/2 - sfrq/2) * 2 / guard;
    lpf = sfrq/2 + (fs1/2 - sfrq/2)/guard;

    delta = pow(10,-aa/20);
    if (aa <= 21) d = 0.9222; else d = (aa-7.95)/14.36;

    n2 = fs2/df*d+1;
    if (n2 % 2 == 0) n2++;

    alp = alpha(aa);
    iza = dbesi0(alp);

    n2y = fs2/fs1; // 0でないサンプルがfs2で何サンプルおきにあるか？
    n2x = n2/n2y+1;

    f2order = malloc(sizeof(int)*n2y);
    for(i=0;i<n2y;i++) {
      f2order[i] = fs2/fs1-(i*(fs2/dfrq))%(fs2/fs1);
      if (f2order[i] == fs2/fs1) f2order[i] = 0;
    }

    f2inc = malloc(sizeof(int)*n2y);
    for(i=0;i<n2y;i++) {
      f2inc[i] = (fs2/dfrq-f2order[i])/(fs2/fs1)+1;
      if (f2order[i+1==n2y ? 0 : i+1] == 0) f2inc[i]--;
    }

    stage2 = malloc(sizeof(REAL *)*n2y);
    stage2[0] = malloc(sizeof(REAL)*n2x*n2y);

    for(i=1;i<n2y;i++) {
      stage2[i] = &(stage2[0][n2x*i]);
      for(j=0;j<n2x;j++) stage2[i][j] = 0;
    }

    for(i=-(n2/2);i<=n2/2;i++)
      {
	stage2[(i+n2/2)%n2y][(i+n2/2)/n2y] = win(i,n2,alp,iza)*hn_lpf(i,lpf,fs2)*fs2/fs1;
      }
  }

  /* Apply filters */

  setstarttime();

  {
    int n1b2 = n1b/2;
    int rp;        // inbufのfs1での次に読むサンプルの場所を保持
    int rps;       // rpを(fs1/sfrq=osf)で割った余り
    int rp2;       // buf2のfs2での次に読むサンプルの場所を保持
    int ds;        // 次にdisposeするsfrqでのサンプル数
    int nsmplwrt1; // 実際にファイルからinbufに読み込まれた値から計算した
                   // stage2 filterに渡されるサンプル数
    int nsmplwrt2; // 実際にファイルからinbufに読み込まれた値から計算した
                   // stage2 filterに渡されるサンプル数
    int s2p;       // stage1 filterから出力されたサンプルの数をn1y*osfで割った余り
    int init,ending;
    int osc;
    REAL *bp,*bp_backup; // rp2から計算される．buf2の次に読むサンプルの位置
    int rps_backup,s2p_backup,osc_backup;
    int k,ch,p;
    int inbuflen=0;
    unsigned int sumread,sumwrite;
    int delay = 0;
    REAL *op;

    //    |....B....|....C....|   buf1      n1b2+n1b2
    //|.A.|....D....|             buf2  n2x+n1b2
    //
    // まずinbufからBにosf倍サンプリングしながらコピー
    // Cはクリア
    // BCにstage 1 filterをかける
    // DにBを足す
    // ADにstage 2 filterをかける
    // Dの後ろをAに移動
    // CをDにコピー

    buf1 = malloc(sizeof(REAL *)*nch);
    for(i=0;i<nch;i++)
      buf1[i] = malloc(n1b*sizeof(REAL));

    buf2 = malloc(sizeof(REAL *)*nch);
    for(i=0;i<nch;i++) {
      buf2[i] = malloc(sizeof(REAL)*(n2x+1+n1b2));
      for(j=0;j<n2x+n1b2;j++) buf2[i][j] = 0;
    }

    rawinbuf  = calloc(nch*(n1b2/osf+osf+1),bps);
    rawoutbuf = malloc(dbps*nch*((double)n1b2*sfrq/dfrq+1));
    inbuf = calloc(nch*(n1b2/osf+osf+1),sizeof(REAL));
    outbuf = malloc(sizeof(REAL)*nch*((double)n1b2*sfrq/dfrq+1));

    op = outbuf;

    s2p = 0;
    rp  = 0;
    rps = 0;
    ds  = 0;
    osc = 0;
    rp2 = 0;

    init = 1;
    ending = 0;
    delay = (double)n1/2/((double)fs1/dfrq)+(double)n2/2/((double)fs2/dfrq);

    sumread = sumwrite = 0;

    for(;;)
      {
	int nsmplread;
	int toberead;

	toberead = (n1b2-rps-1)/osf+1;
	if (toberead+sumread > chanklen) {
	  toberead = chanklen-sumread;
	}

	nsmplread = fread(rawinbuf,1,bps*nch*toberead,fpi);
	nsmplread /= bps*nch;

	switch(bps)
	  {
	  case 1:
	    for(i = 0; i < nsmplread * nch; i++)
	      inbuf[nch * inbuflen + i] =
		(1 / (REAL)0x7f) * ((REAL)((unsigned char *)rawinbuf)[i]-128);
	    break;

	  case 2:
#ifndef BIGENDIAN
	    for(i=0;i<nsmplread*nch;i++)
	      inbuf[nch*inbuflen+i] = (1/(REAL)0x7fff)*(REAL)((short *)rawinbuf)[i];
#else
	    for(i=0;i<nsmplread*nch;i++) {
	      inbuf[nch*inbuflen+i] = (1/(REAL)0x7fff)*
		(((int)rawinbuf[i*2]) |
		 (((int)((char *)rawinbuf)[i*2+1]) << 8));
	    }
#endif
	    break;

	  case 3:
	    for(i=0;i<nsmplread*nch;i++) {
	      inbuf[nch*inbuflen+i] = (1/(REAL)0x7fffff)*
		((((int)rawinbuf[i*3  ]) << 0 ) |
		 (((int)rawinbuf[i*3+1]) << 8 ) |
		 (((int)((char *)rawinbuf)[i*3+2]) << 16));
	    }
	    break;

	  case 4:
	    for(i=0;i<nsmplread*nch;i++) {
	      inbuf[nch*inbuflen+i] = (1/(REAL)0x7fffffff)*
		((((int)rawinbuf[i*4  ]) << 0 ) |
		 (((int)rawinbuf[i*4+1]) << 8 ) |
		 (((int)rawinbuf[i*4+2]) << 16) |
		 (((int)((char *)rawinbuf)[i*4+3]) << 24));
	    }
	    break;
	  }

	for(;i<nch*toberead;i++) inbuf[i] = 0;

	sumread += nsmplread;

	ending = feof(fpi) || sumread >= chanklen;

	rps_backup = rps;
	s2p_backup = s2p;

	for(ch=0;ch<nch;ch++)
	  {
	    rps = rps_backup;

	    for(k=0;k<rps;k++) buf1[ch][k] = 0;

	    for(i=rps,j=0;i<n1b2;i+=osf,j++)
	      {
		assert(j < ((n1b2-rps-1)/osf+1));

		buf1[ch][i] = inbuf[j*nch+ch];

		for(k=i+1;k<i+osf;k++) buf1[ch][k] = 0;
	      }

	    assert(j == ((n1b2-rps-1)/osf+1));

	    for(k=n1b2;k<n1b;k++) buf1[ch][k] = 0;

	    rps = i - n1b2;
	    rp += j;

	    rdft(n1b,1,buf1[ch],fft_ip,fft_w);

	    buf1[ch][0] = stage1[0]*buf1[ch][0];
	    buf1[ch][1] = stage1[1]*buf1[ch][1]; 

	    for(i=1;i<n1b2;i++)
	      {
		REAL re,im;

		re = stage1[i*2  ]*buf1[ch][i*2] - stage1[i*2+1]*buf1[ch][i*2+1];
		im = stage1[i*2+1]*buf1[ch][i*2] + stage1[i*2  ]*buf1[ch][i*2+1];

		buf1[ch][i*2  ] = re;
		buf1[ch][i*2+1] = im;
	      }

	    rdft(n1b,-1,buf1[ch],fft_ip,fft_w);

	    for(i=0;i<n1b2;i++) {
	      buf2[ch][n2x+1+i] += buf1[ch][i];
	    }

	    {
	      int t1 = rp2/(fs2/fs1);
	      if (rp2%(fs2/fs1) != 0) t1++;

	      bp = &(buf2[ch][t1]);
	    }

	    s2p = s2p_backup;

	    for(p=0;bp-buf2[ch]<n1b2+1;p++)
	      {
		REAL tmp = 0;
		REAL *bp2;
		int s;
		int s2o;

		bp2 = bp;
		s2o = f2order[s2p];
		bp += f2inc[s2p];
		s2p++;

		if (s2p == n2y) s2p = 0;

		assert((bp2-&(buf2[ch][0]))*(fs2/fs1)-(rp2+p*(fs2/dfrq)) == s2o);

		for(i=0;i<n2x;i++)
		  tmp += stage2[s2o][i] * *bp2++;

		op[p*nch+ch] = tmp;
	      }

	    nsmplwrt2 = p;
	  }

	rp2 += nsmplwrt2 * (fs2 / dfrq);

	if (twopass) {
	  for(i=0;i<nsmplwrt2*nch;i++)
	    {
	      REAL f = outbuf[i] > 0 ? outbuf[i] : -outbuf[i];
	      peak = peak < f ? f : peak;
	      ((REAL *)rawoutbuf)[i] = outbuf[i];
	    }
	} else {
	  switch(dbps)
	    {
	    case 1:
	      {
		REAL gain2 = gain * (REAL)0x7f;
		ch = 0;

		for(i=0;i<nsmplwrt2*nch;i++)
		  {
		    int s;

		    if (dither) {
		      s = do_shaping(outbuf[i]*gain2,&peak,dither,ch);
		    } else {
		      s = RINT(outbuf[i]*gain2);

		      if (s < -0x80) {
			double d = (double)s/-0x80;
			peak = peak < d ? d : peak;
			s = -0x80;
		      }
		      if (0x7f <  s) {
			double d = (double)s/ 0x7f;
			peak = peak < d ? d : peak;
			s =  0x7f;
		      }
		    }

		    ((unsigned char *)rawoutbuf)[i] = s + 0x80;

		    ch++;
		    if (ch == nch) ch = 0;
		  }
	      }
	    break;

	    case 2:
	      {
		REAL gain2 = gain*(REAL)0x7fff;
		ch = 0;

		for(i=0;i<nsmplwrt2*nch;i++)
		  {
		    int s;

		    if (dither) {
		      s = do_shaping(outbuf[i]*gain2,&peak,dither,ch);
		    } else {
		      s = RINT(outbuf[i]*gain2);

		      if (s < -0x8000) {
			double d = (double)s/-0x8000;
			peak = peak < d ? d : peak;
			s = -0x8000;
		      }
		      if (0x7fff <  s) {
			double d = (double)s/ 0x7fff;
			peak = peak < d ? d : peak;
			s =  0x7fff;
		      }
		    }

#ifndef BIGENDIAN
		    ((short *)rawoutbuf)[i] = s;
#else
		    ((char *)rawoutbuf)[i*2  ] = s & 255; s >>= 8;
		    ((char *)rawoutbuf)[i*2+1] = s & 255;
#endif

		    ch++;
		    if (ch == nch) ch = 0;
		  }
	      }
	    break;

	    case 3:
	      {
		REAL gain2 = gain * (REAL)0x7fffff;
		ch = 0;

		for(i=0;i<nsmplwrt2*nch;i++)
		  {
		    int s;

		    if (dither) {
		      s = do_shaping(outbuf[i]*gain2,&peak,dither,ch);
		    } else {
		      s = RINT(outbuf[i]*gain2);

		      if (s < -0x800000) {
			double d = (double)s/-0x800000;
			peak = peak < d ? d : peak;
			s = -0x800000;
		      }
		      if (0x7fffff <  s) {
			double d = (double)s/ 0x7fffff;
			peak = peak < d ? d : peak;
			s =  0x7fffff;
		      }
		    }

		    ((char *)rawoutbuf)[i*3  ] = s & 255; s >>= 8;
		    ((char *)rawoutbuf)[i*3+1] = s & 255; s >>= 8;
		    ((char *)rawoutbuf)[i*3+2] = s & 255;

		    ch++;
		    if (ch == nch) ch = 0;
		  }
	      }
	    break;

#if 0
	    case 4:
	      {
		REAL gain2 = gain * (REAL)0x7fffffff;
		ch = 0;

		for(i=0;i<nsmplwrt2*nch;i++)
		  {
		    double s;

		    if (dither) {
		      s = do_shaping(outbuf[i]*gain2,&peak,dither,ch);
		    } else {
		      s = RINT(outbuf[i]*gain2);

		      if (s < -0x80000000) {
			double d = (double)s/-0x80000000;
			peak = peak < d ? d : peak;
			s = -0x80000000;
		      }
		      if (0x7fffffff <  s) {
			double d = (double)s/ 0x7fffffff;
			peak = peak < d ? d : peak;
			s =  0x7fffffff;
		      }
		    }

		    ((char *)rawoutbuf)[i*4  ] = s & 255; s >>= 8;
		    ((char *)rawoutbuf)[i*4+1] = s & 255; s >>= 8;
		    ((char *)rawoutbuf)[i*4+2] = s & 255; s >>= 8;
		    ((char *)rawoutbuf)[i*4+3] = s & 255;

		    ch++;
		    if (ch == nch) ch = 0;
		  }
	      }
	    break;
#endif
	    }
	}

	if (!init) {
	  if (ending) {
	    if ((double)sumread*dfrq/sfrq+2 > sumwrite+nsmplwrt2) {
	      if (dbps*nch*nsmplwrt2 != fwrite(rawoutbuf,1,dbps*nch*nsmplwrt2,fpo)) {
	      }
	      sumwrite += nsmplwrt2;
	    } else {
	      fwrite(rawoutbuf,1,dbps*nch*(floor((double)sumread*dfrq/sfrq)+2-sumwrite),fpo);
	      break;
	    }
	  } else {
	    fwrite(rawoutbuf,1,dbps*nch*nsmplwrt2,fpo);
	    sumwrite += nsmplwrt2;
	  }
	} else {
	  int pos,len;
	  if (nsmplwrt2 < delay) {
	    delay -= nsmplwrt2;
	  } else {
	    if (ending) {
	      if ((double)sumread*dfrq/sfrq+2 > sumwrite+nsmplwrt2-delay) {
		fwrite(rawoutbuf+dbps*nch*delay,1,dbps*nch*(nsmplwrt2-delay),fpo);
		sumwrite += nsmplwrt2-delay;
	      } else {
		fwrite(rawoutbuf+dbps*nch*delay,1,dbps*nch*(floor((double)sumread*dfrq/sfrq)+2-sumwrite-delay),fpo);
		break;
	      }
	    } else {
	      fwrite(rawoutbuf+dbps*nch*delay,1,dbps*nch*(nsmplwrt2-delay),fpo);
	      sumwrite += nsmplwrt2-delay;
	      init = 0;
	    }
	  }
	}

	{
	  int ds = (rp2-1)/(fs2/fs1);

	  if (ds > n1b2) ds = n1b2;

	  for(ch=0;ch<nch;ch++)
	    memmove(buf2[ch],buf2[ch]+ds,sizeof(REAL)*(n2x+1+n1b2-ds));

	  rp2 -= ds*(fs2/fs1);
	}

	for(ch=0;ch<nch;ch++)
	  memcpy(buf2[ch]+n2x+1,buf1[ch]+n1b2,sizeof(REAL)*n1b2);

	if ((spcount++ & 7) == 7) showprogress((double)sumread / chanklen);
      }
  }

  showprogress(1);

  free(stage1);
  free(fft_ip);
  free(fft_w);
  free(f2order);
  free(f2inc);
  free(stage2[0]);
  free(stage2);
  for(i=0;i<nch;i++) free(buf1[i]);
  free(buf1);
  for(i=0;i<nch;i++) free(buf2[i]);
  free(buf2);
  free(inbuf);
  free(outbuf);
  free(rawinbuf);
  free(rawoutbuf);

  return peak;
}

double no_src(FILE *fpi,FILE *fpo,int nch,int bps,int dbps,double gain,int chanklen,int twopass,int dither)
{
  double peak=0;
  int ch=0,sumread=0;

  setstarttime();

  while(sumread < chanklen*nch)
    {
      REAL f;
      int s;
      unsigned char buf[3];

      switch(bps) {
      case 1:
	fread(buf,1,1,fpi);
	f = (1 / (REAL)0x7f) * ((REAL)((unsigned char *)buf)[0]-128);
	break;
      case 2:
	fread(buf,2,1,fpi);
#ifndef BIGENDIAN
	f = (1 / (REAL)0x7fff) * ((REAL)((short *)buf)[0]);
#else
	f = (1 / (REAL)0x7fff) * (((int)buf[0]) | (((int)(((char *)buf)[1])) << 8));
#endif
	break;
      case 3:
	fread(buf,3,1,fpi);
	f = (1 / (REAL)0x7fffff) * 
	  ((((int)buf[0]) << 0 ) |
	   (((int)buf[1]) << 8 ) |
	   (((int)((char *)buf)[2]) << 16));
	break;
      case 4:
	fread(buf,4,1,fpi);
	f = (1 / (REAL)0x7fffffff) * 
	  ((((int)buf[0]) << 0 ) |
	   (((int)buf[1]) << 8 ) |
	   (((int)buf[2]) << 16) |
	   (((int)((char *)buf)[3]) << 24));
	break;
      };

      if (feof(fpi)) break;
      f *= gain;

      if (!twopass) {
	switch(dbps) {
	case 1:
	  f *= 0x7f;
	  s = dither ? do_shaping(f,&peak,dither,ch) : RINT(f);
	  buf[0] = s + 128;
	  fwrite(buf,sizeof(char),1,fpo);
	  break;
	case 2:
	  f *= 0x7fff;
	  s = dither ? do_shaping(f,&peak,dither,ch) : RINT(f);
	  buf[0] = s & 255; s >>= 8;
	  buf[1] = s & 255;
	  fwrite(buf,sizeof(char),2,fpo);
	  break;
	case 3:
	  f *= 0x7fffff;
	  s = dither ? do_shaping(f,&peak,dither,ch) : RINT(f);
	  buf[0] = s & 255; s >>= 8;
	  buf[1] = s & 255; s >>= 8;
	  buf[2] = s & 255;
	  fwrite(buf,sizeof(char),3,fpo);
	  break;
#if 0
	case 4:
	  f *= 0x7fffffff;
	  s = dither ? do_shaping(f,&peak,dither,ch) : RINT(f);
	  buf[0] = s & 255; s >>= 8;
	  buf[1] = s & 255; s >>= 8;
	  buf[2] = s & 255; s >>= 8;
	  buf[3] = s & 255;
	  fwrite(buf,sizeof(char),4,fpo);
	  break;
#endif
	};
      } else {
	REAL p = f > 0 ? f : -f;
	peak = peak < p ? p : peak;
	fwrite(&f,sizeof(REAL),1,fpo);
      }

      ch++;
      if (ch == nch) ch = 0;
      sumread++;

      if ((sumread & 0x3ffff) == 0) showprogress((double)sumread / (chanklen*nch));
    }

  showprogress(1);

  return peak;
}

int extract_int(unsigned char *buf)
{
#ifndef BIGENDIAN
  return *(int *)buf;
#else
  return ((int)buf[0]) | (((int)buf[1]) << 8) | 
    (((int)buf[2]) << 16) | (((int)((char *)buf)[3]) << 24);
#endif
}

unsigned int extract_uint(unsigned char *buf)
{
#ifndef BIGENDIAN
  return *(int *)buf;
#else
  return ((unsigned int)buf[0]) | (((unsigned int)buf[1]) << 8) | 
    (((unsigned int)buf[2]) << 16) | (((unsigned int)((char *)buf)[3]) << 24);
#endif
}

short extract_short(unsigned char *buf)
{
#ifndef BIGENDIAN
  return *(short *)buf;
#else
  return ((short)buf[0]) | (((short)((char *)buf)[1]) << 8);
#endif
}

void bury_int(unsigned char *buf,int i)
{
#ifndef BIGENDIAN
  *(int *)buf = i;
#else
  buf[0] = i & 0xff; i >>= 8;
  buf[1] = i & 0xff; i >>= 8;
  buf[2] = i & 0xff; i >>= 8;
  buf[3] = i & 0xff;
#endif
}

void bury_uint(unsigned char *buf,unsigned int i)
{
#ifndef BIGENDIAN
  *(int *)buf = i;
#else
  buf[0] = i & 0xff; i >>= 8;
  buf[1] = i & 0xff; i >>= 8;
  buf[2] = i & 0xff; i >>= 8;
  buf[3] = i & 0xff;
#endif
}

void bury_short(unsigned char *buf,short s)
{
#ifndef BIGENDIAN
  *(short *)buf = s;
#else
  buf[0] = s & 0xff; s >>= 8;
  buf[1] = s & 0xff;
#endif
}

int fread_int(FILE *fp)
{
#ifndef BIGENDIAN
  int ret;
  fread(&ret,4,1,fp);

  return ret;
#else
  unsigned char buf[4];
  fread(&buf,1,4,fp);
  return extract_int(buf);
#endif
}

unsigned int fread_uint(FILE *fp)
{
#ifndef BIGENDIAN
  unsigned int ret;
  fread(&ret,4,1,fp);

  return ret;
#else
  unsigned char buf[4];
  fread(&buf,1,4,fp);
  return extract_uint(buf);
#endif
}

int fread_short(FILE *fp)
{
#ifndef BIGENDIAN
  short ret;
  fread(&ret,2,1,fp);
  return ret;
#else
  unsigned char buf[2];
  fread(&buf,1,2,fp);
  return extract_short(buf);
#endif
}

void fwrite_int(FILE *fp,int i)
{
#ifndef BIGENDIAN
  fwrite(&i,4,1,fp);
#else
  unsigned char buf[4];
  bury_int(buf,i);
  fwrite(&buf,1,4,fp);
#endif
}

void fwrite_uint(FILE *fp,unsigned int i)
{
#ifndef BIGENDIAN
  fwrite(&i,4,1,fp);
#else
  unsigned char buf[4];
  bury_uint(buf,i);
  fwrite(&buf,1,4,fp);
#endif
}

void fwrite_short(FILE *fp,short s)
{
#ifndef BIGENDIAN
  fwrite(&s,2,1,fp);
#else
  unsigned char buf[4];
  bury_int(buf,s);
  fwrite(&buf,1,2,fp);
#endif
}

int main(int argc,char **argv)
{
  char *sfn,*dfn,*tmpfn=NULL;
  FILE *fpi,*fpo,*fpt;
  int twopass,normalize,dither,pdf,samp;
  int nch,bps;
  unsigned int length;
  int sfrq,dfrq,dbps;
  double att,peak,noiseamp;
  int i,j;

  // parse command line options

  dfrq = -1;
  att = 0;
  dbps = -1;
  twopass = 0;
  normalize = 0;
  dither = 0;
  pdf = 0;
  noiseamp = 0.18;

  for(i=1;i<argc;i++)
    {
      if (argv[i][0] != '-') break;

      if (strcmp(argv[i],"--rate") == 0) {
	dfrq = atoi(argv[++i]);
	continue;
      }

      if (strcmp(argv[i],"--att") == 0) {
	att = atof(argv[++i]);
	continue;
      }

      if (strcmp(argv[i],"--bits") == 0) {
	dbps = atoi(argv[++i]);
	if (dbps != 8 && dbps != 16 && dbps != 24) {
	  fprintf(stderr,"Error: Only 8bit, 16bit and 24bit PCM are supported.\n");
	  exit(-1);
	}
	dbps /= 8;
	continue;
      }

      if (strcmp(argv[i],"--twopass") == 0) {
	twopass = 1;
	continue;
      }

      if (strcmp(argv[i],"--normalize") == 0) {
	twopass = 1;
	normalize = 1;
	continue;
      }

      if (strcmp(argv[i],"--dither") == 0) {
	char *endptr;
	dither = strtol(argv[i+1],&endptr,10);
	if (*endptr == '\0') {
	  if (dither < 0 || dither > 4) {
	    fprintf(stderr,"unrecognized dither type : %s\n",argv[i+1]);
	    exit(-1);
	  }
	  i++;
	} else dither = -1;
	continue;
      }

      if (strcmp(argv[i],"--pdf") == 0) {
	char *endptr;
	pdf = strtol(argv[i+1],&endptr,10);
	if (*endptr == '\0') {
	  if (pdf < 0 || pdf > 2) {
	    fprintf(stderr,"unrecognized p.d.f. type : %s\n",argv[i+1]);
	    exit(-1);
	  }
	  i++;
	} else {
	  fprintf(stderr,"unrecognized p.d.f. type : %s\n",argv[i+1]);
	  exit(-1);
	}

	noiseamp = strtod(argv[i+1],&endptr);
	if (*endptr == '\0') {
	  i++;
	} else {
	  static double presets[] = {0.7,0.5,0.18};
	  noiseamp = presets[pdf];
	}

	continue;
      }

      if (strcmp(argv[i],"--quiet") == 0) {
	quiet = 1;
	continue;
      }

      if (strcmp(argv[i],"--tmpfile") == 0) {
	tmpfn = argv[++i];
	continue;
      }

#ifndef HIGHPREC
      if (strcmp(argv[i],"--profile") == 0) {
	if (strcmp(argv[i+1],"fast") == 0) {
	  AA = 96;
	  DF = 8000;
	  FFTFIRLEN = 1024;
	} else if (strcmp(argv[i+1],"standard") == 0) {
	  /* nothing to do */
	} else {
	  fprintf(stderr,"unrecognized profile : %s\n",argv[i+1]);
	  exit(-1);
	}
	i++;
	continue;
      }
#else
      if (strcmp(argv[i],"--profile") == 0) {
	fprintf(stderr,"Warning : --profile option is ignored.\n");
	i++;
	continue;
      }
#endif

      fprintf(stderr,"unrecognized option : %s\n",argv[i]);
      exit(-1);
    }

#ifndef HIGHPREC
  if (!quiet) printf("Shibatch sampling rate converter version " VERSION "\n\n");
#else
  if (!quiet) printf("Shibatch sampling rate converter version " VERSION "(high precision)\n\n");
#endif

  if (argc-i != 2) {usage(); exit(-1);}

  sfn = argv[i];
  dfn = argv[i+1];

  fpi = fopen(sfn,"rb");

  if (!fpi) {fprintf(stderr,"cannot open input file.\n"); exit(-1);}

  /* read wav header */

  {
    unsigned char ibuf[576*2*2];
    short word;
    int dword;

    if (getc(fpi) != 'R') fmterr(1);
    if (getc(fpi) != 'I') fmterr(1);
    if (getc(fpi) != 'F') fmterr(1);
    if (getc(fpi) != 'F') fmterr(1);

    dword = fread_int(fpi);

    if (getc(fpi) != 'W') fmterr(2);
    if (getc(fpi) != 'A') fmterr(2);
    if (getc(fpi) != 'V') fmterr(2);
    if (getc(fpi) != 'E') fmterr(2);
    if (getc(fpi) != 'f') fmterr(2);
    if (getc(fpi) != 'm') fmterr(2);
    if (getc(fpi) != 't') fmterr(2);
    if (getc(fpi) != ' ') fmterr(2);

    dword = fread_int(fpi);
    fread(ibuf,dword,1,fpi);

    if (extract_short(&ibuf[0]) != 1) {
      fprintf(stderr,"Error: Only PCM is supported.\n");
      exit(-1);
    }
    nch = extract_short(&ibuf[2]);
    sfrq = extract_int(&ibuf[4]);
    bps = extract_int(&ibuf[8]);
    if ((int)bps % sfrq*nch != 0) fmterr(4);

    bps /= sfrq*nch;

    for(;;)
      {
	char buf[4];
	buf[0] = getc(fpi);
	buf[1] = getc(fpi);
	buf[2] = getc(fpi);
	buf[3] = getc(fpi);
	length = fread_uint(fpi);
	if (buf[0] == 'd' && buf[1] == 'a' && buf[2] == 't' && buf[3] == 'a') break;
	if (feof(fpi)) break;
	fseek(fpi,length,SEEK_CUR);
      }
    if (feof(fpi)) {
      fprintf(stderr,"Couldn't find data chank\n");
      exit(-1);
    }
  }

  if (bps != 1 && bps != 2 && bps != 3 && bps != 4) {
    fprintf(stderr,"Error : Only 8bit, 16bit, 24bit and 32bit PCM are supported.\n");
    exit(-1);
  }

  if (dbps == -1) {
    if (bps != 1) dbps = bps;
    else dbps = 2;
    if (dbps == 4) dbps = 3;
  }

  if (dfrq == -1) dfrq = sfrq;

  if (dither == -1) {
    if (dbps < bps) {
      if (dbps == 1) dither = 4; else dither = 3;
    } else {
      dither = 1;
    }
  }

  if (!quiet) {
    const char *dtype[] = {
      "none","no noise shaping","triangular spectral shape","ATH based noise shaping","ATH based noise shaping(less amplitude)"
    };
    const char *ptype[] = {
      "rectangular","triangular","gaussian"
    };
    printf("frequency : %d -> %d\n",sfrq,dfrq);
    printf("attenuation : %gdB\n",att);
    printf("bits per sample : %d -> %d\n",bps*8,dbps*8);
    printf("nchannels : %d\n",nch);
    printf("length : %d bytes, %g secs\n",length,(double)length/bps/nch/sfrq);
    if (dither == 0) {
      printf("dither type : none\n");
    } else {
      printf("dither type : %s, %s p.d.f, amp = %g\n",dtype[dither],ptype[pdf],noiseamp);
    }
    printf("\n");
  }

  if (twopass) {
    if (tmpfn) {
      fpt = fopen(tmpfn,"w+b");
    } else {
      fpt = tmpfile();
    }
    if (!fpt) {fprintf(stderr,"cannot open temporary file.\n"); exit(-1);}
  }

  fpo = fopen(dfn,"wb");

  if (!fpo) {fprintf(stderr,"cannot open output file.\n"); exit(-1);}

  /* generate wav header */

  {
    short word;
    int dword;

    fwrite("RIFF",4,1,fpo);
    dword = 0;
    fwrite_int(fpo,dword);

    fwrite("WAVEfmt ",8,1,fpo);
    dword = 16;
    fwrite_int(fpo,dword);
    word = 1;
    fwrite_short(fpo,word); /* format category, PCM */
    word = nch;
    fwrite_short(fpo,word); /* channels */
    dword = dfrq;
    fwrite_int(fpo,dword); /* sampling rate */
    dword = dfrq*nch*dbps;
    fwrite_int(fpo,dword); /* bytes per sec */
    word = dbps*nch;
    fwrite_short(fpo,word); /* block alignment */
    word = dbps*8;
    fwrite_short(fpo,word); /* bits per sample */

    fwrite("data",4,1,fpo);
    dword = 0;
    fwrite_int(fpo,dword);
  }

#if 0
  {
    int i,j;

    for(i=0;i<=M;i++)
      {
	fact[i] = 1;
	for(j=1;j<=i;j++) fact[i] *= j;
      }
  }
#endif

  if (dither) {
    int min,max;
    if (dbps == 1) {min = -0x80; max = 0x7f;}
    if (dbps == 2) {min = -0x8000; max = 0x7fff;}
    if (dbps == 3) {min = -0x800000; max = 0x7fffff;}
    if (dbps == 4) {min = -0x80000000; max = 0x7fffffff;}

    samp = init_shaper(dfrq,nch,min,max,dither,pdf,noiseamp);
  }

  if (twopass) {
    REAL gain;
    int ch=0;
    unsigned int fptlen,sumread;

    if (!quiet) printf("Pass 1\n");

    if (normalize) {
      if (sfrq < dfrq) peak = upsample(fpi,fpt,nch,bps,sizeof(REAL),sfrq,dfrq,1,length/bps/nch,twopass,dither);
      else if (sfrq > dfrq) peak = downsample(fpi,fpt,nch,bps,sizeof(REAL),sfrq,dfrq,1,length/bps/nch,twopass,dither);
      else peak = no_src(fpi,fpt,nch,bps,sizeof(REAL),1,length/bps/nch,twopass,dither);
    } else {
      if (sfrq < dfrq) peak = upsample(fpi,fpt,nch,bps,sizeof(REAL),sfrq,dfrq,pow(10,-att/20),length/bps/nch,twopass,dither);
      else if (sfrq > dfrq) peak = downsample(fpi,fpt,nch,bps,sizeof(REAL),sfrq,dfrq,pow(10,-att/20),length/bps/nch,twopass,dither);
      else peak = no_src(fpi,fpt,nch,bps,sizeof(REAL),pow(10,-att/20),length/bps/nch,twopass,dither);
    }

    if (!quiet) printf("\npeak : %gdB\n",20*log10(peak));

    if (!normalize) {
      if (peak < pow(10,-att/20)) peak = 1;
      else peak *= pow(10,att/20);
    } else peak *= pow(10,att/20);

    if (!quiet) printf("\nPass 2\n");
    
    if (dither) {
      switch(dbps)
	{
	case 1:
	  gain = (normalize || peak >= (0x7f-samp)/(double)0x7f) ? 1/peak*(0x7f-samp) : 1/peak*0x7f;
	  break;
	case 2:
	  gain = (normalize || peak >= (0x7fff-samp)/(double)0x7fff) ? 1/peak*(0x7fff-samp) : 1/peak*0x7fff;
	  break;
	case 3:
	  gain = (normalize || peak >= (0x7fffff-samp)/(double)0x7fffff) ? 1/peak*(0x7fffff-samp) : 1/peak*0x7fffff;
	  break;
#if 0
	case 4:
	  gain = (normalize || peak >= (0x7fffffff-samp)/(double)0x7fffffff) ? 1/peak*(0x7fffffff-samp) : 1/peak*0x7fffffff;
	  break;
#endif
	}
    } else {
      switch(dbps)
	{
	case 1:
	  gain = 1/peak * 0x7f;
	  break;
	case 2:
	  gain = 1/peak * 0x7fff;
	  break;
	case 3:
	  gain = 1/peak * 0x7fffff;
	  break;
#if 0
	case 4:
	  gain = 1/peak * 0x7fffffff;
	  break;
#endif
	}
    }
    randptr = 0;

    setstarttime();

    fptlen = ftell(fpt) / sizeof(REAL);
    sumread = 0;

    fseek(fpt,0,SEEK_SET);
    for(;;)
      {
	REAL f;
	int s;

	if (fread(&f,sizeof(REAL),1,fpt) == 0) break;
	f *= gain;
	sumread++;

	switch(dbps) {
	case 1:
	  {
	    unsigned char buf[1];
	    s = dither ? do_shaping(f,&peak,dither,ch) : RINT(f);

	    buf[0] = s + 128;

	    fwrite(buf,sizeof(char),1,fpo);
	  }
	  break;
	case 2:
	  {
	    char buf[2];
	    s = dither ? do_shaping(f,&peak,dither,ch) : RINT(f);

	    buf[0] = s & 255; s >>= 8;
	    buf[1] = s & 255;

	    fwrite(buf,sizeof(char),2,fpo);
	  }
	  break;
	case 3:
	  {
	    char buf[3];
	    s = dither ? do_shaping(f,&peak,dither,ch) : RINT(f);

	    buf[0] = s & 255; s >>= 8;
	    buf[1] = s & 255; s >>= 8;
	    buf[2] = s & 255;

	    fwrite(buf,sizeof(char),3,fpo);
	  }
	  break;
#if 0
	case 4:
	  {
	    char buf[4];
	    s = dither ? do_shaping(f,&peak,dither,ch) : RINT(f);

	    buf[0] = s & 255; s >>= 8;
	    buf[1] = s & 255; s >>= 8;
	    buf[2] = s & 255; s >>= 8;
	    buf[3] = s & 255;

	    fwrite(buf,sizeof(char),4,fpo);
	  }
	  break;
#endif
	}

	ch++;
	if (ch == nch) ch = 0;

	if ((sumread & 0x3ffff) == 0) showprogress((double)sumread / fptlen);
      }
    showprogress(1);
    if (!quiet) printf("\n");
    fclose(fpt);
    if (tmpfn != NULL) {
      if (remove(tmpfn))
	fprintf(stderr,"Failed to remove %s\n",tmpfn);
    }
  } else {
    if (sfrq < dfrq) peak = upsample(fpi,fpo,nch,bps,dbps,sfrq,dfrq,pow(10,-att/20),length/bps/nch,twopass,dither);
    else if (sfrq > dfrq) peak = downsample(fpi,fpo,nch,bps,dbps,sfrq,dfrq,pow(10,-att/20),length/bps/nch,twopass,dither);
    else peak = no_src(fpi,fpo,nch,bps,dbps,pow(10,-att/20),length/bps/nch,twopass,dither);
    if (!quiet) printf("\n");
  }

  if (dither) {
    quit_shaper(nch);
  }

  if (!twopass && peak > 1) {
    if (!quiet) printf("clipping detected : %gdB\n",20*log10(peak));
  }

  {
    short word;
    int dword;
    int len;

    fseek(fpo,0,SEEK_END);
    len = ftell(fpo);

    fseek(fpo,4,SEEK_SET);
    dword = len-8;
    fwrite_int(fpo,dword);

    fseek(fpo,40,SEEK_SET);
    dword = len-44;
    fwrite_int(fpo,dword);
  }

  exit(0);
}
