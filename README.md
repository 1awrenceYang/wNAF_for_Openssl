# <center>密码学编程选做2：给OpenSSL增加接口，用wNAF算法计算任意点乘</center>

### <center>作者：张浩旸  日期：2022/12/16  编程语言：C++  依赖库：OpenSSL</center>

### 1.运行截图

![image-20221216150612460](C:\Users\ASUS\AppData\Roaming\Typora\typora-user-images\image-20221216150612460.png)

这里以NIST曲线secp224r1为例，随机在曲线上取一个点进行wNAF计算，如图所示，结果正确

### 2.算法原理

**表示密度：**

每一个大于零的自然数，都可以用不同的表示方法进行表示，在一般的椭圆曲线点乘算法中，我们用普通的二进制来表示乘数。这里给出表示密度的定义：一个数的二进制表示的密度为表示中1比特的数量与全部比特数量的比值
$$
W=Number_{1bit}/Number_{All \ bits}
$$
二进制表示下，算法的伪代码是：

```python
Input:Positive number k,Point on curve P
Output:k*P
Q=0(无限远点)
Pi=BinaryForm(k)
for i in range(a):
    Q=2*Q
    Q=Q+Pi if Pi!=0
```

可以发现，在普通算法下，算法的主要开销是加法部分。也就是说，表示中1bit的占比越高，计算点乘的开销越大，速度越慢。而二进制的表示密度是`1/2`

**转化问题：**

经过上面的论证，我们将问题自然的转化为了降低表示的密度，因此wNAF算法引入了NAF（Non-Adjacent-Form）表示方法，将自然数k表示为
$$
k=\sum ki*2^i
$$
的形式，这里的ki为+1或者-1任意正整数都只有一个NAF表示形式，记为NAF(k)，这种表示形式要求任意两个连续的比特中最多只有一个比特非零，不可能出现连续两个比特都是非零的形式，所以NAF表示最多比二进制表示多1个比特的长度，非零比特数量减少，NAF的密度是`1/3`，这样就减少了加法操作的进行，比二进制的`1/2`更小

例如：39的二进制展开式：`1 0 0 1 1 1`，39的NAF表示NAF(39)：`1 0 1 0 0 -1`



**窗口方法**

在普通的点乘算法中，一个改进的思路是一次扫描更多的比特，上面提到的最普通的点乘就是窗口大小w=1的特殊情况。通过提升扫描数量，可以减少扫描次数，减少开销



**wNAF方法：**

既然提升扫描窗口的思想可以应用于二进制表示中，也应该可以应用于NAF表示中，因此引入了wNAF表示形式（Width-Non-Adjacent-Form）任何正整数k有唯一的wNAF表示记为wNAF(k)，同样表示为
$$
k=\sum ki*2^i
$$
但是这里的ki为奇数且`ki<2*w-1`，w就是窗口大小，wNAF表示形式要求任意的w个比特中最多有1个比特非零，这使得wNAF表示的密度降低为`1/(w+1)`、

**预计算：**

有了wNAF表示形式，我们可以预先计算表示形式的所有预计算点，然后在计算时应用即可



**计算wNAF展开形式的伪代码：**

```python
Input:Positive Integer k
Output:wNAF form of k
i=0
ki_list=[]
while k>=1::
    if(k%2==1):
        ki=k mod pow(2,w)
        k=k-ki
    else:
        ki=0
    ki_list.append(ki)
    k=k/2
    i=i+1
return (ki.inverse_list())
```

**多倍点的wNAF算法伪代码：**

```python
Input:wNAF form of a integer ki_list,EC point P
Output:k*P
ki_list=Precompute(ki_list)
Q=0(无限远点)
for item in ki_list:
	Q=2*Q
    if(item!=0):
        Q=Q+ki_list[i]*P
return Q
```

**预计算伪代码：**

```python
Input wNAF form of a integer ki_list,EC point P
Output:Precompute List of the integer
for item in ki_list:
    if(item!=0)
    	ki_list[i]=P*ki_list[i]
    else:
        ki_list[i]=0（无限远点）
return ki_list
```

### 3.主要接口

- `，#define RightJustify 1`：类型转换右对齐标志

- `#define LeftJustify 0`：类型转换左对齐标志

- `void Big2Bytes(BIGNUM* big, int length, int flag,char *Out)`：将BIGNUM类型转换为字节类型，第一个参数：待转换的BIGNUM类型，第二个参数：BIGNUM的长度，第三个参数：RightJustify右对齐，LeftJustify左对齐，第四个参数输出，**传入前应当在堆上分配空间**

- `void PrintBIGNUM(BIGNUM* big,char end='\n')`：打印BIGNUM到stdout，第一个参数是待打印的BIGNUM，第二个参数是打印后的字符，默认为换行符

- `typedef BIGNUM* OpensslBIG;`：类型别名

- `typedef EC_POINT* Point;`：类型别名

- `typedef struct MyNode`：二叉搜索树节点

- **`void (char* Data, unsigned int Value, Node* root,unsigned int ByteNumber)`**：将一个数据插入二叉搜索树中。**第一个参数是`Key`键**，用于**索引第二个参数`Value`**，**第三个参数是二叉搜索树的树根**，**最后一个参数用于决定从第一个参数中读取多长的数据作为索引**，**单位是字节**。我实现的树的索引规则是：**如果比特为0，向左；反之向右**，除了叶节点之外的节点中的`Value`成员都没有意义，**叶节点的`Value`成员是被索引的值**。

- `void HashMap(BIGNUM* index, EC_POINT* Value, Node* root, unsigned int ByteNum)`：`InsertDataIntoSearchTree`的更高级别抽象，将索引值换为了BIGNUM，被索引的值换为了EC_POINT椭圆曲线点。**第一个参数是BIGNUM类型的索引，第二个参数是被索引的值，类型是椭圆曲线点，第三个参数是树根，也是哈希表的标识符，第四个参数是读取索引值的长度，单位是字节**

- **`bool SearchData(char* Data, Node* root,unsigned int ByteNumber,unsigned int* outValue)`**：**第一个参数是Key键**，**第二个参数是搜索树的树根**，**第三个参数是从Key键中读取的字节长度**，**最后一个参数是读取到的Value**，返回值：如果索引的值存在，返回`true`，否则返回`false`，注意：**第三个参数返回的值只有返回值为`true`才有意义，否则不会对该参数做任何处理**

- `int HashMapSearch(BIGNUM* index, EC_POINT* Value, Node* root, unsigned int ByteNum)`：`SearchData`的高级别抽象，将索引值换为了BIGNUM，被索引的值换为了EC_POINT椭圆曲线点，**第一个参数是BIGNUM类型的索引，第二个参数是被索引的值，类型是椭圆曲线点，第三个参数是树根，也是哈希表的标识符，第四个参数是读取索引值的长度，单位是字节**

- `void InitRootNode(Node* root)`：初始化搜索树根

- ```c++
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
  ```

  wNAF表示形式类，两个成员变量分别是：当前的表示列表，当前表示长度。

  `void Append(BIGNUM* NewElement);`将新元素添加到列表末尾

  `void PrintThisList(char end='\n');`打印列表所有元素

  `void InverseList();`逆转列表

  `unsigned long long GetLength();`取得当前列表长度

  `List();`构造函数，初始化列表

- `void PrintBytes(char* text, int length)`：打印字节变量到stdout，第一个参数是字节首地址，第二个参数是打印读取长度

- `void MyBN_mod(BIGNUM* a, BIGNUM* b, BIGNUM* result)`：是`BN_mod`的高级别抽象，用于计算模，第一个参数是被模数，第二个参数是模数，最后一个参数是接受结果的。

- `void int2char(int input, char* output)`：取出第1个参数中的四个字节，放入第二个参数中，**注意第二个参数传入前应当在堆上分配空间**

- `void int2BIGNUM(int input, BIGNUM* output)`：整型转换为BIGNUM

- `List* wNAF_form(unsigned int w, BIGNUM* k)`：第一个参数是窗口大小，第二个参数是自然数k，返回k的wNAF表示形式

- `void GetXY2Big(Point InputPoint,BIGNUM *outX,BIGNUM *outY)`：将点坐标转换为BIGNUM类型，第一个参数是曲线点，第二个参数是X坐标，第二个参数是Y坐标

- `void GetXY2Char(Point InputPoint, char* outX, char* outY,unsigned int outXlength=32,unsigned int outYlength=32)：将点坐标转换为char类型，第一个参数是曲线点，第二个参数是X坐标，第二个参数是Y坐标`

- `void PrintPoint(Point InputPoint)`：打印点到stdout

- `Node* wNAF_Precompute(List* ki, EC_POINT* BasePoint)`：输入wNAF表示形式，椭圆曲线点BasePoint，返回这个曲线点的预计算表的哈希查找表

- `void wNAFmul(unsigned int w, BIGNUM* k, EC_POINT* P, EC_POINT* kP)`：wNAF点乘算法，输入窗口大小，自然数k，椭圆曲线点P，以及接受的结果kP

- `int CheckForm(List* ki, BIGNUM* k)`：检查wNAF形式计算是否正确，通过输入的wNAF表示检查是否能得出正确的原数，第一个参数是wNAF形式，第二个参数是正确的原数

