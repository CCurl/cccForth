#include "shared.h"
#undef TOS
#undef NOS
#define TOS st.i[s]
#define NOS st.i[s-1]
FIB_T st;
char* y;
int base=10, h, s, r, p, u, t, v, isBye=0;

void pc(int c) { putc(c, stdout); }
void ps(const char *x) { while (*x) pc(*(x++)); }
void pn(CELL num, CELL b) {
    UCELL n = (UCELL)num;
    char x[32], * cp = &x[31];
    if ((b == 10) && (num < 0)) { pc('-'); n = -num; }
    *(cp) = 0;
    do {
        int x = (n % b) + '0';
        n = n / b;
        *(--cp) = ('9' < x) ? (x + 7) : x;
    } while (n);
    ps(cp);
}

/*   */ void X() { if (u) { pc(u); pc('?'); } p=0; }
/*   */ void N() { }
/* ~ */ void f33() { ps("[!]"); }
/* " */ void f34() { while (st.b[p] != '"') pc(st.b[p++]); ++p; }
/* # */ void f35() { t=TOS; st.i[++s]=t; }
/* $ */ void f36() { t=TOS; TOS=NOS; NOS=t; }
/* % */ void f37() { t=NOS; st.i[++s]=t; }
/* & */ void f38() { u=NOS; t=TOS; NOS=u/t; TOS=u/t; }
/* ' */ void f39() { st.i[++s]=st.b[p++]; }
/* ( */ void f40() { if (TOS==0) { while (st.b[p]!=')') ++p; } }
/* ) */ void f41() { ; }
/* * */ void f42() { NOS; NOS *= TOS; --s; }
/* + */ void f43() { NOS; NOS += TOS; --s; }
/* , */ void f44() { t=st.i[s--]; pc(t); }
/* - */ void f45() { NOS; NOS -= TOS; --s; }
/* . */ void f46() { t=st.i[s--]; pn(t,base); }
/* / */ void f47() { NOS; NOS /= TOS; --s; }
/* 0 */ void fN() { ++s; TOS=u-'0'; while (BTW(st.b[p],'0','9')) { TOS=(TOS*10)+(st.b[p++]-'0'); } }
/* : */ void f58() { ; }
/* ; */ void f59() { if (RB<r) { p=0; r=RB+1; } else { p=st.i[r++]; } }
/* < */ void f60() { NOS=(NOS <TOS)?-1:0; --s; }
/* = */ void f61() { NOS=(NOS==TOS)?-1:0; --s; }
/* > */ void f62() { NOS=(NOS >TOS)?-1:0; --s; }
/* ? */ void f63() { ; }
/* @ */ void f64() { ps("[@]"); }
/* I */ void fI() { st.i[++s]=st.i[r]; }
/* [ */ void f91() { st.i[--r]=p; st.i[--r]=st.i[s--]; st.i[--r]=st.i[s--]; }
/* \ */ void f92() { --s; }
/* ] */ void f93() { ++st.i[r]; if (st.i[r]<=st.i[r+1]) { p=st.i[r+2]; } else { r+=3; } }
/* ^ */ void f94() { ; }
/* _ */ void f95() { TOS=-TOS; }
/* ` */ void f96() { ; }
/* d */ void fd() { --TOS; }
/* b */ void fb() { u=st.b[p++];
        if (u=='&') { NOS=NOS&TOS; --s; }
        if (u=='|') { NOS=NOS|TOS; --s; }
        if (u=='^') { NOS=NOS^TOS; --s; }
        if (u=='~') { TOS=~TOS; }
        else { --p; pc(32); } }
/* c */ void fc() { u=st.b[p++];
        if (u=='@') { ps("[c@]"); }
        if (u=='!') { ps("[c!]"); } }
/* i */ void fi() { ++TOS; }
/* x */ void fx() { u=st.b[p++];
        if (u=='Q') { isBye=1; }
        else if (u=='T') { st.i[++s]=clock(); } }
/* { */ void f123() { ; }
/* | */ void f124() { ; }
/* } */ void f125() { ; }
/* ~ */ void f126() { TOS=(TOS)?0:-1; }
void (*q[127])() = { X,X,X,X,X,X,X,X,X,X,N,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,N,
    f33,f34,f35,f36,f37,f38,f39,f40,f41,f42,f43,f44,f45,f46,f47,fN,fN,fN,fN,fN,fN,fN,fN,fN,fN,
    f58,f59,f60,f61,f62,f63,f64,X,X,X,X,X,X,X,X,fI,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,
    f91,f92,f93,f94,f95,f96,X,fb,fc,fd,X,X,X,X,fi,X,X,X,X,X,X,X,X,X,X,X,X,X,X,fx,X,X,f123,f124,f125,f126 };
void I() {
    base = 10;
    s = SB; r = RB; h = CB; v = VB;
    isBye = 0;
    for (t = 0; t < (VMSZ/4); t++) { st.i[t] = 0; }
}
void R(int x) {
    s = (s<SB) ? (SB-1) : s;
    r = (RB<r) ? (RB+1) : r;
    p = x;
    while (p) { u=st.b[p++]; q[u](); }
}
