#include "shared.h"
#undef TOS
#undef NOS
#define TOS st.i[s]
#define NOS st.i[s-1]
#define FN ((u-'A')*26)+st.b[p++]-'A'

FIB_T st;
char *y, yy[80];
int base=10, state=0, cb, here, sb, s, rb, r, vb, v, p, u, t, isBye=0;

void push(CELL x) { st.i[++s] = x; }
CELL pop() { return st.i[s--]; }
void ps(const char *x) { while (*x) pc(*(x++)); }

void pn(CELL num, CELL b) {
    UCELL n = (UCELL)num;
    y = &yy[40];
    if ((b == 10) && (num < 0)) { pc('-'); n = -num; }
    *(y) = 0;
    do {
        t = (n % b) + '0';
        n = n / b;
        *(--y) = ('9' < t) ? (t + 7) : t;
    } while (n);
    ps(y);
}

void doDotS() {
    pc('(');
    for (int d = sb; d <= s; d++) {
        if (sb<d) { pc(' '); }
        pn(st.i[d], base);
    }
    pc(')');
}

int T(char *x, char d) {
    int l = 0;
    while (*x!=d) {
        u=*x; ++l; ++x;
        if (u=='%') { 
            u=*x; ++l; ++x;
            if (u=='d') { pn(st.i[s--], 10); }
            else if (u=='x') { pn(st.i[s--], 16); }
            else if (u=='b') { pn(st.i[s--], 2); }
            else if (u=='c') { pc(st.i[s--]); }
            else if (u=='q') { pc('"'); }
            else if (u=='n') { pc('\n'); }
            else { pc(u); }
        } else { pc(u); }
    }
    return l+1;
}

/*     */ void X() { if (u) { pc(u); pc('?'); } p=0; }
/*     */ void N() { }
/*  !  */ void f33() { st.i[TOS]=NOS; s-=2; }
/*  "  */ void f34() { y=&st.b[p]; p+=T(y,'"'); }
/*  #  */ void f35() { t=TOS; st.i[++s]=t; }
/*  $  */ void f36() { t=TOS; TOS=NOS; NOS=t; }
/*  %  */ void f37() { t=NOS; st.i[++s]=t; }
/*  &  */ void f38() { u=NOS; t=TOS; NOS=u/t; TOS=u/t; }
/*  '  */ void f39() { st.i[++s]=st.b[p++]; }
/*  (  */ void f40() { if (st.i[s--]==0) { while (st.b[p]!=')') ++p; } }
/*  *  */ void f42() { NOS; NOS *= TOS; --s; }
/*  +  */ void f43() { NOS; NOS += TOS; --s; }
/*  ,  */ void f44() { t=st.i[s--]; pc(t); }
/*  -  */ void f45() { NOS; NOS -= TOS; --s; }
/*  .  */ void f46() { t=st.i[s--]; pn(t,base); }
/*  /  */ void f47() { NOS; NOS /= TOS; --s; }
/*  0  */ void fN() { ++s; TOS=u-'0'; while (BTW(st.b[p],'0','9')) { TOS=(TOS*10)+(st.b[p++]-'0'); } }
/*  :  */ void f58() { u=st.b[p++]; t=FN; if (sb<=t) {  ps("-fn(1)-"); return; } here=st.i[t]=p; state=1; }
/*  ;  */ void f59() { if (rb<r) { p=0; r=rb+1; } else { p=st.i[r++]; } }
/*  <  */ void f60() { if (st.b[p]=='=') { ++p; NOS=(NOS<=TOS)?-1:0; } else { NOS=(NOS<TOS)?-1:0; } --s; }
/*  =  */ void f61() { NOS=(NOS==TOS)?-1:0; --s; }
/*  >  */ void f62() { if (st.b[p]=='=') { ++p; NOS=(NOS>=TOS)?-1:0; } else { NOS=(NOS>TOS)?-1:0; } --s; }
/*  @  */ void f64() { TOS=st.i[TOS]; }
/* A-Z */ void fAZ() { t=FN; if (sb<=t) { ps("-fn(2)-"); return; }
        if (st.i[t]) { if (st.b[p]!=';') { st.i[--r]=p; } p=st.i[t]; } }
/*  [  */ void f91() { st.i[--r]=p; st.i[--r]=st.i[s--]; st.i[--r]=st.i[s--]; }
/*  \  */ void f92() { --s; }
/*  ]  */ void f93() { ++st.i[r]; if (st.i[r]<=st.i[r+1]) { p=st.i[r+2]; } else { r+=3; } }
/*  _  */ void f95() { TOS=-TOS; }
/*  `  */ void f96() { y=yy; while (st.b[p]!='`') { *(y++)=st.b[p++]; } *y=0; ++p; system(yy); }
/*  b  */ void fb() { u=st.b[p++];
        if (u=='&') { NOS=NOS&TOS; --s; }
        else if (u=='|') { NOS=NOS|TOS; --s; }
        else if (u=='^') { NOS=NOS^TOS; --s; }
        else if (u=='~') { TOS=~TOS; }
        else { --p; pc(32); } }
/*  c  */ void fc() { u=st.b[p++];
        if (u=='@') { TOS=st.b[TOS]; }
        else if (u=='!') { st.b[TOS]=(char)NOS; } }
/*  e  */ void fe() { p=st.i[s--]; }
/*  f  */ void ff() { u=st.b[p++]; if (u=='.') { ps(stringF(yy, "%f", st.f[s--])); }
        else if (u=='f') { st.f[s]=(float)st.i[s]; }
        else if (u=='i') { st.i[s]=(int)st.f[s]; }
        else if (u=='+') { st.f[s-1]+=st.f[s]; s--; }
        else if (u=='-') { st.f[s-1]-=st.f[s]; s--; }
        else if (u=='*') { st.f[s-1]*=st.f[s]; s--; }
        else if (u=='/') { st.f[s-1]/=st.f[s]; s--; } }
/*  i  */ void fi() { st.i[++s]=st.i[r]; }
/*  k  */ void fk() { u=st.b[p++];
        if (u=='?') { st.i[++s]=charAvailable(); }
        if (u=='@') { st.i[++s]=getChar(); } }
/*  x  */ void fx() { u=st.b[p++]; if (u=='U') { ++r; }
        else if (u=='I') { st.i[r]+=st.i[s--]; }
        else if (u=='T') { st.i[++s]=clock(); }
        else if (u=='S') { doDotS(); } 
        else if (u=='H') { st.i[++s]=here; }
        else if (u=='B') { st.i[++s]=base; }
        else if (u=='b') { base=st.i[s--]; }
        else if (u=='Y') { y=&st.b[st.i[s--]]; system(y); }
        else if (u=='X') { I(sb,rb-sb,vb-cb); }
        else if (u=='Q') { isBye=1; }
        else p=doExt(u,p); }
/*  z  */ void fz() { y=&st.b[st.i[s--]]; T(y,0); }
/*  {  */ void f123() { st.i[--r]=p; if (TOS==0) while (st.b[p]!='}') { ++p; } }
/*  |  */ void f124() { t=TOS; while (st.b[p]!='|') { st.b[t++]=st.b[p++]; } st.b[t++]=0; TOS=t; ++p; }
/*  }  */ void f125() { if (TOS) { p=st.i[r]; } else { ++r; --s; } }
/*  ~  */ void f126() { TOS=(TOS)?0:-1; }
void (*q[127])() = { X,X,X,X,X,X,X,X,X,X,N,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,N,
    f33,f34,f35,f36,f37,f38,f39,f40,N,f42,f43,f44,f45,f46,f47,fN,fN,fN,fN,fN,fN,fN,fN,fN,fN,f58,f59,f60,f61,f62,X,f64,
    fAZ,fAZ,fAZ,fAZ,fAZ,fAZ,fAZ,fAZ,fAZ,fAZ,fAZ,fAZ,fAZ,fAZ,fAZ,fAZ,fAZ,fAZ,fAZ,fAZ,fAZ,fAZ,fAZ,fAZ,fAZ,fAZ,
    f91,f92,f93,X,f95,f96,X,fb,fc,X,fe,ff,X,X,fi,X,fk,X,X,X,X,X,X,X,X,X,X,X,X,fx,X,fz,f123,f124,f125,f126 };
void I(int nf, int ss, int cs) {
    // [funcs][stacks][code][vars]
    base = 10;
    s=sb=nf; r=rb=(sb+ss); here=cb=(rb*4); v=vb=cb+cs;
    --s;
    isBye = last = xt = 0;
    for (t=0; t<(VMSZ/4); t++) { st.i[t]=0; }
    systemWords();
}
void R(int x) {
    s = (s<sb) ? (sb-1) : s;
    r = (rb<r) ? (rb+1) : r;
    p = x;
    while (p) {
        u = st.b[p++];
        if (state == 1) { 
            if (u < 32) { return; }
            here=p; 
        } else { q[u](); }
    }
}
void E(char* b) {
    if (*b) {
        // ps("\nexec:["); ps(b); pc(']');
        t=here; while (*b) { st.b[t++]=*(b++); }
        st.b[t]=0; R(here);
    }
}
