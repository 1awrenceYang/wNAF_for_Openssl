#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib,"libssl.lib")
#pragma comment(lib,"libcrypto.lib")
#include"wNAF_Algorithm.h"
int    main()

{
    BN_CTX* ctx = BN_CTX_new();
    const unsigned char RealChar[20] = { 0x74 ,0xa3 ,0x9e ,0x57 ,0xea ,0xfe ,0x2e ,0x94 ,0x32 ,0xfe ,0xe0 ,0xf9 ,0xb2 ,0xa4 ,0x02 ,0x4b ,0xa4 ,0x14 ,0xba ,0x13 };
    const unsigned char Schar[1] = { 0x27 };
    BIGNUM* Zero = BN_new();
    BIGNUM* Small = BN_new();
    const unsigned char ZeroChar[1] = { 0x00 };
    BN_bin2bn(ZeroChar, 1, Zero);
    BN_bin2bn(Schar, 1, Small);
    EC_GROUP* secp224r1 = EC_GROUP_new_by_curve_name(NID_secp224r1);
    EC_POINT* G = EC_POINT_new(secp224r1);
    G = (EC_POINT*)EC_GROUP_get0_generator(secp224r1);
    EC_POINT* kP = EC_POINT_new(secp224r1);
    EC_POINT* Out = EC_POINT_new(secp224r1);
    BIGNUM* a, * b, * c, * One;
    One = BN_new();
    BIGNUM* RealOne = BN_new();
    BIGNUM* RealOne2 = BN_new();
    BN_bin2bn(RealChar, 20, RealOne);
    List* Mylist = new List;
    Mylist = wNAF_form(3, RealOne);
    Node* root = wNAF_Precompute(Mylist, G);
    wNAFmul(3, RealOne, G, kP);
    PrintPoint(kP);
   // PrintBIGNUM()
    EC_POINT_mul(secp224r1, Out, Zero, G, RealOne, ctx);
    PrintPoint(Out);






    

}