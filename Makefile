
CFLAGS := -g -O0 -Wall

AA: aatree.c
	 gcc -o $@ $^ -Dtest_aatree=main $(CFLAGS)



tst: ternary_search_tree.c
	 gcc -o $@  $^ -D test_ternary_search_tree=main $(CFLAGS)

clean:
	rm -f *.exe dump coding tst a.out *.txt
