#include "ss_crypto.h"
#include <openssl/des.h>
#include <openssl/md5.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

char *base64_encode(const unsigned char *data, size_t ilen, size_t *olen) {
    BIO *b64=BIO_new(BIO_f_base64()), *bio=BIO_new(BIO_s_mem());
    bio=BIO_push(b64,bio); BIO_set_flags(bio,BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio,data,(int)ilen); BIO_flush(bio);
    BUF_MEM *bp; BIO_get_mem_ptr(bio,&bp);
    char *o=(char*)malloc(bp->length+1);
    if(o){memcpy(o,bp->data,bp->length);o[bp->length]=0;if(olen)*olen=bp->length;}
    BIO_free_all(bio); return o;
}

unsigned char *base64_decode(const char *data, size_t ilen, size_t *olen) {
    unsigned char *buf=(unsigned char*)malloc(ilen);
    if(!buf) return NULL; memset(buf,0,ilen);
    BIO *b64=BIO_new(BIO_f_base64()), *bio=BIO_new_mem_buf(data,(int)ilen);
    bio=BIO_push(b64,bio); BIO_set_flags(bio,BIO_FLAGS_BASE64_NO_NL);
    int dl=BIO_read(bio,buf,(int)ilen); if(dl<0)dl=0;
    if(olen)*olen=(size_t)dl; BIO_free_all(bio); return buf;
}

char *md5_hex(const char *input) {
    if(!input) return NULL;
    return md5_bytes_hex((const unsigned char*)input, strlen(input));
}

char *md5_bytes_hex(const unsigned char *input, size_t len) {
    unsigned char d[MD5_DIGEST_LENGTH]; MD5(input,len,d);
    char *h=(char*)malloc(33); if(!h) return NULL;
    for(int i=0;i<MD5_DIGEST_LENGTH;i++) sprintf(h+i*2,"%02x",d[i]);
    h[32]=0; return h;
}

char *desede_cbc_encrypt(const char *pt, const char *ks, const char *ivs) {
    if(!pt||!ks||!ivs) return NULL;
    unsigned char kb[24]; memset(kb,0,24);
    size_t kl=strlen(ks); if(kl>24)kl=24; memcpy(kb,ks,kl);
    unsigned char iv[8]; memset(iv,0,8);
    size_t il=strlen(ivs); if(il>8)il=8; memcpy(iv,ivs,il);
    size_t pl=strlen(pt); int pad=8-(pl%8); size_t pdl=pl+pad;
    unsigned char *padded=(unsigned char*)malloc(pdl);
    if(!padded) return NULL;
    memcpy(padded,pt,pl); memset(padded+pl,pad,pad);
    EVP_CIPHER_CTX *ctx=EVP_CIPHER_CTX_new();
    if(!ctx){free(padded);return NULL;}
    unsigned char *ct=(unsigned char*)malloc(pdl+16);
    if(!ct){EVP_CIPHER_CTX_free(ctx);free(padded);return NULL;}
    int ol=0,fl=0;
    EVP_EncryptInit_ex(ctx,EVP_des_ede3_cbc(),NULL,kb,iv);
    EVP_CIPHER_CTX_set_padding(ctx,0);
    EVP_EncryptUpdate(ctx,ct,&ol,padded,(int)pdl);
    EVP_EncryptFinal_ex(ctx,ct+ol,&fl); ol+=fl;
    EVP_CIPHER_CTX_free(ctx); free(padded);
    size_t b64l=0; char *b64=base64_encode(ct,(size_t)ol,&b64l);
    free(ct); return b64;
}

static const char *SS_KEY="123d6cedf626dy54233aa1w6";
static const char *SS_IV="wEiphTn!";
static const char *SS_APP_KEY="moviebox";
static char ss_token[33]={0};

void ss_generate_token(char *out, size_t len) {
    static const char hx[]="0123456789abcdef";
    srand((unsigned)time(NULL));
    size_t m=len-1; if(m>32)m=32;
    for(size_t i=0;i<m;i++) out[i]=hx[rand()%16];
    out[m]=0;
}

long long ss_get_expiry(void) { return (long long)time(NULL)+60*60*12; }

char *ss_build_query(const char *jp) {
    if(!jp) return NULL;
    if(ss_token[0]==0) ss_generate_token(ss_token,sizeof(ss_token));
    char *enc=desede_cbc_encrypt(jp,SS_KEY,SS_IV); if(!enc) return NULL;
    char *akh=md5_hex(SS_APP_KEY); if(!akh){free(enc);return NULL;}
    char *mak=md5_hex(SS_APP_KEY); if(!mak){free(enc);free(akh);return NULL;}
    size_t cl=strlen(mak)+strlen(SS_KEY)+strlen(enc)+1;
    char *cat=(char*)malloc(cl); if(!cat){free(enc);free(akh);free(mak);return NULL;}
    sprintf(cat,"%s%s%s",mak,SS_KEY,enc);
    char *ver=md5_hex(cat); free(cat); free(mak);
    if(!ver){free(enc);free(akh);return NULL;}
    size_t bs=strlen(akh)+strlen(ver)+strlen(enc)+128;
    char *bj=(char*)malloc(bs);
    if(!bj){free(enc);free(akh);free(ver);return NULL;}
    sprintf(bj,"{\"app_key\":\"%s\",\"verify\":\"%s\",\"encrypt_data\":\"%s\"}",akh,ver,enc);
    free(akh);free(ver);free(enc);
    size_t b64l=0; char *b64=base64_encode((const unsigned char*)bj,strlen(bj),&b64l);
    free(bj); if(!b64) return NULL;
    size_t fs=b64l*3+256; char *pd=(char*)malloc(fs);
    if(!pd){free(b64);return NULL;}
    char *p=pd; p+=sprintf(p,"data=");
    for(size_t i=0;i<b64l;i++){
        char c=b64[i];
        if(c=='+')p+=sprintf(p,"%%2B");
        else if(c=='/')p+=sprintf(p,"%%2F");
        else if(c=='=')p+=sprintf(p,"%%3D");
        else *p++=c;
    }
    free(b64);
    sprintf(p,"&appid=27&platform=android&version=160&medium=Website%%26token%s",ss_token);
    return pd;
}
