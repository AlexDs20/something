#include <string.h>
#include <math.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

#define ASSERT_TRUE(x)  do { if (!(x)) return __LINE__; } while(0)
#define ASSERT_FALSE(x) do { if ((x))  return __LINE__; } while(0)

#define ASSERT_EQ(a,b)  do { if ((a)!=(b)) return __LINE__; } while(0)
#define ASSERT_NEQ(a,b) do { if ((a)==(b)) return __LINE__; } while(0)

#define ASSERT_EQ_INT(a,b)   do { if ((int)(a)      != (int)(b))        return __LINE__; } while(0)
#define ASSERT_EQ_UINT(a,b)  do { if ((unsigned)(a) != (unsigned)(b))   return __LINE__; } while(0)
#define ASSERT_EQ_SIZE(a,b)  do { if ((size_t)(a)   != (size_t)(b))     return __LINE__; } while(0)
#define ASSERT_NEQ_INT(a,b)  do { if ((int)(a)      != (int)(b))        return __LINE__; } while(0)
#define ASSERT_NEQ_UINT(a,b) do { if ((unsigned)(a) != (unsigned)(b))   return __LINE__; } while(0)
#define ASSERT_NEQ_SIZE(a,b) do { if ((size_t)(a)   != (size_t)(b))     return __LINE__; } while(0)

#define ASSERT_LT(a,b) do { if (!((a) < (b))) return __LINE__; } while(0)
#define ASSERT_LE(a,b) do { if (!((a) <= (b))) return __LINE__; } while(0)
#define ASSERT_GT(a,b) do { if (!((a) > (b))) return __LINE__; } while(0)
#define ASSERT_GE(a,b) do { if (!((a) >= (b))) return __LINE__; } while(0)

#define ASSERT_NOT_NULL(p) do { if ((p) == NULL) return __LINE__; } while(0)
#define ASSERT_NULL(p)     do { if ((p) != NULL) return __LINE__; } while(0)

#define ASSERT_MEMEQ(a,b,n)  do { if (memcmp((a),(b),(n)) != 0) return __LINE__; } while(0)
#define ASSERT_MEMNEQ(a,b,n) do { if (memcmp((a),(b),(n)) == 0) return __LINE__; } while(0)

#define ASSERT_STREQ(a,b)  do { if (strcmp((a),(b)) != 0) return __LINE__; } while(0)
#define ASSERT_STRNEQ(a,b) do { if (strcmp((a),(b)) == 0) return __LINE__; } while(0)

#define ASSERT_FLOAT_EQ(a,b,eps)  do { if (fabs((a)-(b)) > (eps)) return __LINE__; } while(0)
#define ASSERT_FLOAT_NEQ(a,b,eps) do { if (fabs((a)-(b)) < (eps)) return __LINE__; } while(0)

#define ASSERT_DOUBLE_EQ(a,b,eps)  do { if (fabs((double)(a)-(double)(b)) > (eps)) return __LINE__; } while(0)
#define ASSERT_DOUBLE_NEQ(a,b,eps) do { if (fabs((double)(a)-(double)(b)) < (eps)) return __LINE__; } while(0)
