7150 #include "types.h"
7151 #include "riscv.h"
7152 
7153 void*
7154 memset(void *dst, int c, uint n)
7155 {
7156   if ((int)dst%4 == 0 && n%4 == 0){
7157     c &= 0xFF;
7158     stosl(dst, (c<<24)|(c<<16)|(c<<8)|c, n/4);
7159   } else
7160     stosb(dst, c, n);
7161   return dst;
7162 }
7163 
7164 int
7165 memcmp(const void *v1, const void *v2, uint n)
7166 {
7167   const uchar *s1, *s2;
7168 
7169   s1 = v1;
7170   s2 = v2;
7171   while(n-- > 0){
7172     if(*s1 != *s2)
7173       return *s1 - *s2;
7174     s1++, s2++;
7175   }
7176 
7177   return 0;
7178 }
7179 
7180 void*
7181 memmove(void *dst, const void *src, uint n)
7182 {
7183   const char *s;
7184   char *d;
7185 
7186   s = src;
7187   d = dst;
7188   if(s < d && s + n > d){
7189     s += n;
7190     d += n;
7191     while(n-- > 0)
7192       *--d = *--s;
7193   } else
7194     while(n-- > 0)
7195       *d++ = *s++;
7196 
7197   return dst;
7198 }
7199 
7200 // memcpy exists to placate GCC.  Use memmove.
7201 void*
7202 memcpy(void *dst, const void *src, uint n)
7203 {
7204   return memmove(dst, src, n);
7205 }
7206 
7207 int
7208 strncmp(const char *p, const char *q, uint n)
7209 {
7210   while(n > 0 && *p && *p == *q)
7211     n--, p++, q++;
7212   if(n == 0)
7213     return 0;
7214   return (uchar)*p - (uchar)*q;
7215 }
7216 
7217 char*
7218 strncpy(char *s, const char *t, int n)
7219 {
7220   char *os;
7221 
7222   os = s;
7223   while(n-- > 0 && (*s++ = *t++) != 0)
7224     ;
7225   while(n-- > 0)
7226     *s++ = 0;
7227   return os;
7228 }
7229 
7230 // Like strncpy but guaranteed to NUL-terminate.
7231 char*
7232 safestrcpy(char *s, const char *t, int n)
7233 {
7234   char *os;
7235 
7236   os = s;
7237   if(n <= 0)
7238     return os;
7239   while(--n > 0 && (*s++ = *t++) != 0)
7240     ;
7241   *s = 0;
7242   return os;
7243 }
7244 
7245 
7246 
7247 
7248 
7249 
7250 int
7251 strlen(const char *s)
7252 {
7253   int n;
7254 
7255   for(n = 0; s[n]; n++)
7256     ;
7257   return n;
7258 }
7259 
7260 
7261 
7262 
7263 
7264 
7265 
7266 
7267 
7268 
7269 
7270 
7271 
7272 
7273 
7274 
7275 
7276 
7277 
7278 
7279 
7280 
7281 
7282 
7283 
7284 
7285 
7286 
7287 
7288 
7289 
7290 
7291 
7292 
7293 
7294 
7295 
7296 
7297 
7298 
7299 
