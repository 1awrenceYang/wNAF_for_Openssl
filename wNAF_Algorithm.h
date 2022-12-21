#ifndef  _WNAF_H
#define _WNAF_H
#include <openssl/bn.h>
#include<openssl/bio.h>
#include<openssl/ec.h>
#include<openssl/ecdsa.h>
#include<openssl/objects.h>
#include<openssl/err.h>
#include <openssl/bn.h>
#include<string>
#include<vector>
#include"applink.c"
#define RightJustify 1
#define LeftJustify 0
void Big2Bytes(BIGNUM* big, int length, int flag,char *Out)
{
	char* BUF = (char*)malloc(length * sizeof(char));
	int NumLength = BN_num_bytes(big);
	char* temp = (char*)malloc(NumLength * sizeof(char));
	int NumLength1 = BN_bn2bin(big, (unsigned char*)temp);
	if (flag == RightJustify)
	{
		for (int i = 0; i < length - NumLength; i++)
			BUF[i] = 0x00;
		for (int i = length - NumLength; i < length; i++)
			BUF[i] = temp[i - (length - NumLength)];
	}
	else if (flag == LeftJustify)
	{
		for (int i = 0; i < NumLength; i++)
			BUF[i] = temp[i];
		for (int i = NumLength; i < length; i++)
			BUF[i] = 0x00;
	}
	
	for (int i = 0; i < length; i++)
		Out[i] = BUF[i];
	free(temp);
	free(BUF);
}
void PrintBIGNUM(BIGNUM* big,char end='\n')
{
	BIO* out;
	out = BIO_new_fp(stdout, BIO_NOCLOSE);
	if (big != NULL)
		BN_print(out, big);
	BIO_free(out);
	printf("%c", end);
}
typedef BIGNUM* OpensslBIG;
typedef EC_POINT* Point;
typedef struct MyNode
{
	MyNode* LeftChild = NULL;
	MyNode* RightChild = NULL;
	EC_POINT* Value;
}Node;
void InsertDataIntoSearchTree(char* Data, EC_POINT* Value, Node* root, unsigned int ByteNumber,EC_GROUP*group)
{
	Node* Ptr = root;
	char Byte = 0x00;
	char Bit = 0x00;
	for (int i = 0; i < ByteNumber; i++)
	{
		Byte = Data[i];
		for (int j = 7; j >= 0; j--)
		{
			Bit = (Byte >> j) & 0x01;
			if (Bit == 0)
			{
				if (Ptr->LeftChild != NULL)
				{
					Ptr = Ptr->LeftChild;
					continue;
				}
				Node* LeftChild = (Node*)malloc(sizeof(Node));
				Ptr->LeftChild = LeftChild;
				Ptr = Ptr->LeftChild;
				Ptr->LeftChild = NULL;
				Ptr->RightChild = NULL;
				//Ptr->MyValueNode = NULL;
				Ptr->Value = EC_POINT_new(group);
				EC_POINT_copy(Ptr->Value, Value);//Ptr->Value = Value;
			}
			else if (Bit == 1)
			{
				if (Ptr->RightChild != NULL)
				{
					Ptr = Ptr->RightChild;
					continue;
				}
				Node* RightChild = (Node*)malloc(sizeof(Node));
				Ptr->RightChild = RightChild;
				Ptr = Ptr->RightChild;
				Ptr->LeftChild = NULL;
				Ptr->RightChild = NULL;
				//Ptr->MyValueNode = NULL;
				Ptr->Value = EC_POINT_new(group);
				EC_POINT_copy(Ptr->Value, Value); //Ptr->Value = Value;
			}
			if (i == 7 && j == 0)
			{
				Ptr->Value = EC_POINT_new(group);
				EC_POINT_copy(Ptr->Value, Value);//Ptr->Value = Value;
			}

		}
	}
}
void HashMap(BIGNUM* index, EC_POINT* Value, Node* root, unsigned int ByteNum)
{
	EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_secp224r1);
	char* indexChar = (char*)malloc(8 * sizeof(char));
	Big2Bytes(index, 8, RightJustify,indexChar);
	InsertDataIntoSearchTree(indexChar, Value, root, ByteNum, group);
	free(indexChar);
	EC_GROUP_free(group);
}
bool SearchData(char* Data, Node* root, unsigned int ByteNumber, EC_POINT* outValue)
{
	Node* Ptr = root;
	char Byte = 0x00;
	char Bit = 0x00;
	//*outValue = 01;
	for (int i = 0; i < ByteNumber; i++)
	{
		Byte = Data[i];
		for (int j = 7; j >= 0; j--)
		{
			Bit = (Byte >> j);
			Bit = Bit & 0x01;
			if (Bit == 0)
			{
				if (Ptr->LeftChild == NULL)
					return false;
				else
				{
					if (i == 7 && j == 0)
						EC_POINT_copy(outValue, Ptr->Value);//*outValue = Ptr->Value;
					Ptr = Ptr->LeftChild;
				}

			}
			if (Bit == 1)
			{
				if (Ptr->RightChild == NULL)
					return false;
				else
				{
					if (i == 7 && j == 0)
						EC_POINT_copy(outValue, Ptr->Value);
					Ptr = Ptr->RightChild;
				}

			}
		}
	}
	return true;
}
int HashMapSearch(BIGNUM* index, EC_POINT* Value, Node* root, unsigned int ByteNum)
{
	EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_secp224r1);
	char* indexChar = (char*)malloc(8 * sizeof(char));
	Big2Bytes(index, 8, RightJustify, indexChar);
	EC_POINT* point = EC_POINT_new(group);
	int r = SearchData(indexChar, root, ByteNum, point);
	EC_POINT_copy(Value, point);
	EC_POINT_free(point);
	EC_GROUP_free(group);
	free(indexChar);
	return r;
}
void InitRootNode(Node* root)
{
	root->LeftChild = NULL;
	root->RightChild = NULL;

}
class List
{
public:
	OpensslBIG* CurrentList;
	unsigned int CurrentLength;
	void Append(BIGNUM* NewElement);
	void PrintThisList(char end='\n');
	void InverseList();
	unsigned long long GetLength();
	List();
};
List::List(void)
{
	CurrentList = NULL;
	CurrentLength = 0;
}
unsigned long long List::GetLength()
{
	return this->CurrentLength;
}
void List::Append(BIGNUM* NewElement)
{
	if (this->CurrentList == NULL)
	{
		this->CurrentLength += 1;
		this->CurrentList = (OpensslBIG*)malloc(sizeof(OpensslBIG));
		this->CurrentList[0] = BN_new();
		BN_copy(this->CurrentList[0], NewElement);
		//this->PrintThisList();
	}
	else
	{
		OpensslBIG* TempList = (OpensslBIG*)malloc(this->CurrentLength * sizeof(OpensslBIG));//allocate memory
		for (int i = 0; i < this->CurrentLength; i++)//initilize the list
			TempList[i] = BN_new();
		for (int i = 0; i < this->CurrentLength; i++)//store value
			BN_copy(TempList[i], this->CurrentList[i]);
		for (int i = 0; i < this->CurrentLength; i++)//free each element
			BN_free(this->CurrentList[i]);
		free(this->CurrentList);//free the list
		this->CurrentList = NULL;
		this->CurrentLength += 1;
		this->CurrentList = (OpensslBIG*)malloc(this->CurrentLength * sizeof(OpensslBIG));
		for (int i = 0; i < this->CurrentLength; i++)
			this->CurrentList[i] = BN_new();
		for (int i = 0; i < this->CurrentLength - 1; i++)
			BN_copy(this->CurrentList[i], TempList[i]);
		BN_copy(this->CurrentList[this->CurrentLength - 1], NewElement);
		for (int i = 0; i < this->CurrentLength - 1; i++)
			BN_free(TempList[i]);
		free(TempList);
	}
}
void List::PrintThisList(char end)
{
	for (int i = 0; i < this->CurrentLength; i++)
		PrintBIGNUM(this->CurrentList[i], end);
}
void List::InverseList()
{
	OpensslBIG* TempList = (OpensslBIG*)malloc(this->CurrentLength * sizeof(OpensslBIG));
	for (int i = 0; i < this->CurrentLength; i++)
		TempList[i] = BN_new();
	for (int i = 0; i < this->CurrentLength; i++)
		BN_copy(TempList[this->CurrentLength - 1 - i], this->CurrentList[i]);
	for (int i = 0; i < this->CurrentLength; i++)
		BN_copy(this->CurrentList[i], TempList[i]);
	for (int i = 0; i < this->CurrentLength; i++)
		BN_free(TempList[i]);
	free(TempList);
	TempList = NULL;
}
void PrintBytes(char* text, int length)//输入字节长度
{
	for (int i = 0; i < length; i++)
	{
		printf("%02X", (uint8_t)text[i]);
		if ((i + 1) % 4 == 0)
			printf(" ");
		if ((i + 1) % 32 == 0)
			printf("\n");
	}
	printf("\n");
}

void MyBN_mod(BIGNUM* a, BIGNUM* b, BIGNUM* result)
{
	BN_CTX* TempCTX = BN_CTX_new();
	BIGNUM* Remainder = BN_new();
	BN_div(NULL, Remainder, a, b, TempCTX);
	//printf("remainder\n");
	//PrintBIGNUM(Remainder);
	BN_copy(result, Remainder);
	BN_CTX_free(TempCTX);
}
void int2char(int input, char* output)
{
	char* buf = (char*)malloc(4 * sizeof(char));
	char* ptr = (char*)&input;
	for (int i = 0; i < 4; i++)
		buf[i] = ptr[i];
	for (int i = 0; i < 4; i++)
		output[3 - i] = buf[i];
	free(buf);
}
void int2BIGNUM(int input, BIGNUM* output)
{
	char* buf = (char*)malloc(4 * sizeof(char));
	int2char(input, buf);
	BIGNUM* temp = BN_new();
	BN_bin2bn((const unsigned char*)buf, 4, temp);
	BN_copy(output, temp);
	BN_free(temp);
	free(buf);
}

List* wNAF_form(unsigned int w, BIGNUM* k)
{
	const unsigned char OneChar[1] = { 0x01 };
	const unsigned char TwoChar[1] = { 0x02 };
	const unsigned char ZeroChar[1] = { 0x00 };

	unsigned int counter = 1;
	OpensslBIG* wNAF = (OpensslBIG*)malloc(counter * sizeof(OpensslBIG));
	BIGNUM* One = BN_new();
	BIGNUM* Kmod2 = BN_new();
	BIGNUM* Two = BN_new();
	BIGNUM* Zero = BN_new();
	BIGNUM* kTemp = BN_new();
	BN_copy(kTemp, k);
	BN_bin2bn(OneChar, 1, One);
	BN_bin2bn(ZeroChar, 1, Zero);
	BN_bin2bn(TwoChar, 1, Two);

	BIGNUM* ki = BN_new();
	BIGNUM* bigW = BN_new();
	BIGNUM* TwoExpW = BN_new();
	BIGNUM* TempSub = BN_new();
	BN_CTX* ctx = BN_CTX_new();
	BN_CTX* DivCtx = BN_CTX_new();
	int2BIGNUM((int)w, bigW);
	//PrintBIGNUM(bigW);
	BN_exp(TwoExpW, Two, bigW, ctx);
	//PrintBIGNUM(TwoExpW);
	List* ki_List = new List;

	while (BN_cmp(k, One) == 1 || BN_cmp(k, One) == 0)//while (k>=1)
	{
		MyBN_mod(k, Two, Kmod2);
		if (BN_cmp(Kmod2, One) == 0)//if(k%2==1)
		{
			MyBN_mod(k, TwoExpW, ki);//ki=k%pow(2,2)
			//PrintBIGNUM(k);
			BN_sub(TempSub, k, ki);
			//PrintBIGNUM(ki);
			//PrintBIGNUM(TempSub);
			BN_copy(k, TempSub);//k=k-ki
		}
		else
		{
			BN_copy(ki, Zero);
		}
		//PrintBIGNUM(k, ':');
		BN_div(k, NULL, k, Two, DivCtx);
		//PrintBIGNUM(k, ':');
		//PrintBIGNUM(ki, ' ');
		ki_List->Append(ki);
	}
	ki_List->InverseList();
	BN_copy(k, kTemp);
	BN_free(One);
	BN_free(Kmod2);
	BN_free(Two);
	BN_free(Zero);
	BN_free(kTemp);
	BN_free(ki);
	BN_free(bigW);
	BN_free(TwoExpW);
	BN_free(TempSub);
	BN_CTX_free(ctx);
	BN_CTX_free(DivCtx);
	return ki_List;
}
void GetXY2Big(Point InputPoint,BIGNUM *outX,BIGNUM *outY)
{
	BIGNUM* X, * Y;
	BN_CTX* ctx = BN_CTX_new();
	X = BN_new();
	Y = BN_new();
	EC_GROUP* secp224r1 = EC_GROUP_new_by_curve_name(NID_secp224r1);
	EC_POINT_get_affine_coordinates(secp224r1, InputPoint, X, Y, ctx);
	BN_copy(outX, X);
	BN_copy(outY, Y);
	BN_free(X);
	BN_free(Y);
	BN_CTX_free(ctx);
	EC_GROUP_free(secp224r1);
	return;
}
void GetXY2Char(Point InputPoint, char* outX, char* outY,unsigned int outXlength=32,unsigned int outYlength=32)
{
	BIGNUM* X, * Y;
	BN_CTX* ctx = BN_CTX_new();
	X = BN_new();
	Y = BN_new();
	EC_GROUP* secp224r1 = EC_GROUP_new_by_curve_name(NID_secp224r1);
	EC_POINT_get_affine_coordinates(secp224r1, InputPoint, X, Y, ctx);
	unsigned int Xlength = 0, Ylength = 0;
	Xlength = BN_num_bytes(X);
	Ylength = BN_num_bytes(Y);
	char* Xchar = (char*)malloc(Xlength * sizeof(char));
	char* Ychar = (char*)malloc(Ylength * sizeof(char));
	Big2Bytes(X, outXlength, RightJustify, Xchar);
	Big2Bytes(Y, outYlength, RightJustify, Xchar);
	
	if ((outXlength - Xlength) < 0 || (outYlength - Ylength) < 0)
	{
		printf("Error in GetXY2Char");
		return;
	}
	unsigned int XZeroLength = outXlength - Xlength;
	unsigned int YZeroLength = outYlength - Ylength;
	for (int i = 0; i < (XZeroLength); i++)
		outX[i] = 0;
	//PrintBytes(Xchar, 32);
	for (int i = (XZeroLength); i < outXlength; i++)
	{

		outX[i] = Xchar[i];
		//printf("%02X", (uint8_t)Xchar[i]);
	}
	
	for (int i = 0; i < (YZeroLength); i++)
		outY[i] = 0;
	for (int i = (YZeroLength); i < outYlength; i++)
		outY[i] = Ychar[i];
	BN_free(X);
	BN_free(Y);
	BN_CTX_free(ctx);
	EC_GROUP_free(secp224r1);
	free(Xchar);
	free(Ychar);
	return;
}
void PrintPoint(Point InputPoint)
{
	BIGNUM* X, * Y;
	BN_CTX* ctx = BN_CTX_new();
	X = BN_new();
	Y = BN_new();
	EC_GROUP* secp224r1 = EC_GROUP_new_by_curve_name(NID_secp224r1);
	EC_POINT_get_affine_coordinates(secp224r1, InputPoint, X, Y, ctx);
	printf("X:");
	PrintBIGNUM(X);
	printf("Y:");
	PrintBIGNUM(Y);
	BN_free(X);
	BN_free(Y);
	BN_CTX_free(ctx);
	EC_GROUP_free(secp224r1);
}
Node* wNAF_Precompute(List* ki, EC_POINT* BasePoint)
{
	EC_GROUP* secp224r1 = EC_GROUP_new_by_curve_name(NID_secp224r1);
	EC_POINT* kiP = EC_POINT_new(secp224r1);
	BIGNUM* Zero = BN_new();
	BN_CTX* ctx = BN_CTX_new();
	char* Index = NULL;
	int2BIGNUM(0x00, Zero);
	Node* root = (Node*)malloc(sizeof(Node));
	InitRootNode(root);
	for (int i = 0; i < ki->CurrentLength; i++)
	{
		if (BN_cmp(ki->CurrentList[i], Zero) == 0)continue;
		else
		{
			EC_POINT_mul(secp224r1, kiP, Zero, BasePoint, ki->CurrentList[i], ctx);
			HashMap(ki->CurrentList[i], kiP, root, 8);
		}
	}
	EC_GROUP_free(secp224r1);
	EC_POINT_free(kiP);
	BN_free(Zero);
	BN_CTX_free(ctx);
	return root;
}
void wNAFmul(unsigned int w, BIGNUM* k, EC_POINT* P, EC_POINT* kP)
{
	BIGNUM* Two = BN_new();
	BIGNUM* Zero = BN_new();
	BN_CTX* ctx = BN_CTX_new();
	
	const unsigned char TwoChar[1] = { 0x02 };
	const unsigned char ZeroChar[1] = { 0x00 };
	BN_bin2bn(TwoChar, 1, Two);
	BN_bin2bn(ZeroChar, 1, Zero);
	List* ki = new List;
	Node* PrecomputeList = (Node*)malloc(sizeof(Node));
	InitRootNode(PrecomputeList);
	ki = wNAF_form(w, k);
	PrecomputeList = wNAF_Precompute(ki, P);
	EC_GROUP* secp224r1 = EC_GROUP_new_by_curve_name(NID_secp224r1);
	EC_POINT* TempStroe = EC_POINT_new(secp224r1);
	EC_POINT* Temp = EC_POINT_new(secp224r1);
	EC_POINT* Q = EC_POINT_new(secp224r1);
	EC_POINT_copy(TempStroe, P);
	unsigned int len = ki->CurrentLength;
	//ki->PrintThisList();
	for (int i = 0; i < len; i++)
	{
		EC_POINT_mul(secp224r1, Q, Zero, Q, Two, ctx);//Q=2Q
		//printf("Q=2Q\n");
		//PrintPoint(Q);
		if (BN_cmp(ki->CurrentList[i], Zero) == 0)continue;
		else
		{
			HashMapSearch(ki->CurrentList[i], Temp, PrecomputeList, 8);//Temp=ki*P
			//printf("ki*P:\n");
			//PrintPoint(Temp);
			//EC_POINT_mul(secp224r1, Temp, Zero, P, ki->CurrentList[i], ctx);
			EC_POINT_add(secp224r1, Q, Q, Temp, ctx);//Q=Q+ki*P
			//printf("Q=Q+ki*P\n");
			//PrintPoint(Q);
		}
	}
	EC_POINT_copy(kP, Q);
	EC_POINT_copy(P, TempStroe);
	BN_free(Two);
	BN_free(Zero);
	BN_CTX_free(ctx);
	EC_GROUP_free(secp224r1);
	EC_POINT_free(Temp);
	EC_POINT_free(TempStroe);
}
int CheckForm(List* ki, BIGNUM* k)
{
	BIGNUM* acc = BN_new();
	BIGNUM* T = BN_new();
	BIGNUM* Two = BN_new();
	BIGNUM* Current = BN_new();
	BIGNUM* Temp = BN_new();
	BN_CTX* ctx = BN_CTX_new();
	//BN_CTX* ctx2 = BN_CTX_new();
	const unsigned char Zero[1] = { 0x00 };
	const unsigned char TwoChar[2] = { 0x02 };
	BN_bin2bn(Zero, 1, acc);
	BN_bin2bn(TwoChar, 1, Two);
	BN_bin2bn(Zero, 1, T);
	unsigned int len = ki->CurrentLength;
	for (int i = 0; i < len; i++)
	{
		int2BIGNUM(i, Temp);
		BN_exp(Current, Two, Temp, ctx);
		//PrintBIGNUM(Current);
		//PrintBIGNUM(ki->CurrentList[len - 1 - i]);
		BN_mul(T, Current, ki->CurrentList[len - 1 - i], ctx);
		//PrintBIGNUM(T);
		BN_add(acc, acc, T);
		
		//PrintBIGNUM(acc);
	}
	//PrintBIGNUM(acc);
	if (BN_cmp(acc, k) == 0)
	{
		BN_free(acc);
		BN_free(T);
		BN_free(Two);
		BN_free(Current);
		BN_free(Temp);
		BN_CTX_free(ctx);
		return true;
	}
	else
	{
		BN_free(acc);
		BN_free(T);
		BN_free(Two);
		BN_free(Current);
		BN_free(Temp);
		BN_CTX_free(ctx);
		return false;
	}
}
#endif 

