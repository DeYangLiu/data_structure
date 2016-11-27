Ternary Search Tree
====
一个节点在32位平台上大小为5*4=20个字节。文章[TST]提到的是针对字符串的情况，
value就是key,所以不必有value字段，并且利用C字符串以NUL结尾，不必有isEnd字段;
并且还可利用指针的低2bit总是零。如果限制key为正常ascii可见字符，
把节点压缩到3*4个字节是可以做到的。

节点数量：
一棵満的树高为h三叉树，总共有
1+3+3^2 + ... + 3^(h-1) = (3^h-1)/2
个节点，叶子节点有3^(h-1)。
内部节点只是叶子节点的一半。

test sessions:
$ gcc ternary_search_tree.c -D test_ternary_search_tree=main -g -Wall && ./a.out====dump all
bug : 4
cat : 1
cats : 2
up : 3
====search key:val==
cats:2
bu:0
cat:1
====after remove cat
bug : 4
cats : 2
up : 3
====after remove cats
bug : 4
up : 3
====after remove all (nil)

[TST] http://www.cs.princeton.edu/~rs/strings/paper.pdf

