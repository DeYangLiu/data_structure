/*
LL(1):
First Set:
 推导后完全由终结符构成的串，第一个终结符。
First(A) := {a \in N | A经过0或多次推导为a...}

Follow Set:
 所有可能的推导中紧随其后的终结符。
Follow(A) := {a \in N | S经过0或多次推导出现...Aa...}

计算规则：
First(u v) = First(First(u) First(v))
First(N) = {First(w) | 所有产生式 N --> w}
First(x) = {x | x 是任一终结符或\eps}
Follow(A) = {First(First(v) Follow(B)) | 所有产生式B --> u A v}
Select(A --> w) = First(First(w) Follow(A))
非终结符A是LL(k)的，如果任意两个Select(A --> w) 和Select(A --> u)没有交集。
文法是LL(k)的，如果任一非终结符都是LL(k)的。
从确定性看，任何LL(k)文法都是LL(k+1)文法。

例子1是LL(1)的：
S --> aSb
S --> \eps
Select(S --> aSb) = First(First(aSb) Follow(S)) = First({a} {b, #}) 
 = First({a#,ab}) = {a}
Select(S --> \eps) = First(First(\eps) Follow(S)) = First({\eps} {b, #})
 = First({\eps b, \eps #}) = {b}

例子2不是LL(1)的：
            First/Follow
E --> E + T {n,(}/{#,+,)}
E --> T     {n,(}/{#,+,)}
T --> T * F {n,(}/{#,+,*,)}
T --> F     {n,(}/{#,+,*,)}
F --> (E)   {(}/{#,+,*,)}
F --> n     {n}/{#,+,*,)}
原因是左递归。
A --> Ax
A --> y
实际上为 A --> yx*
转换为右线性文法：
A --> yB
B --> xB
B --> \eps
公共左因子的转换：
A --> xy
A --> xz
==>
A --> x B
B --> y
B --> z
把正则表达式引入LL(k)文法：
w = x*
 Select(loop) = Select(x) = First(First(x) Follow(w))
 Select(exit) = Follow(w)
*/
int nextToken;
void getToken();

//F :== '(' E ')' | num
void F()
{
	if(nextToken == '('){
		getToken();
		E();
		if(nextToken == ')'){
			getToken();
		}else{
			err();
		}
	}
	else if(nextToken == num){
		getToken();
	}
	else{
		err();
	}
}

//T :== F {'*' F}*
void T()
{
	F();
	while(nextToken == '*'){
		getToken();
		F();
	}
	
}

//E :== T {'+' T}*
void E()
{
	T();
	while(nextToken == '+'){
		getToken();
		T();
	}
}

int main()
{
	getToken();
	E();
	if(nextToken() == 0){
		//ok
	}else{
		err();
	}
}
