// Try again make project.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
//#include "pch.cpp"
#include <stdio.h>

int test_number_for_test_bp_cmp = 0, flag_for_test_number_for_test_bp_cmp = 0;

void for_test_bp_cmp(int val, const char *st1, const char *st2)
{
	++test_number_for_test_bp_cmp;
	bn *a = bn_new();
	bn *b = bn_new();
	bn_init_string(a, st1);
	bn_init_string(b, st2);
	if (val != bn_cmp(a, b))
	{
		flag_for_test_number_for_test_bp_cmp = 1;
		printf("\nERROR in \"bn_cmp\"  num %d\n", test_number_for_test_bp_cmp);
		printf("val = %d, \nst1 = %s,  \nst2 = %s\n", val, st1, st2);
		bn_print(a);
		bn_print(b);
	}
	bn_delete(a);
	bn_delete(b);
}

void test_bn_cmp()
{
	for_test_bp_cmp(-1, "-1", "111111111112222222223333333333333444444445555555555");
	for_test_bp_cmp(0, "0", "0");
	for_test_bp_cmp(1, "123456789", "123456788");
	for_test_bp_cmp(1, "25000005", "70000");
	for_test_bp_cmp(1, "-70000", "-25000005");
	for_test_bp_cmp(0, "0", "0");
	for_test_bp_cmp(0, "0", "0");
	for_test_bp_cmp(0, "0", "0");
	for_test_bp_cmp(0, "0", "0");
	for_test_bp_cmp(0, "0", "0");
	for_test_bp_cmp(0, "0", "0");
	for_test_bp_cmp(0, "0", "0");
	for_test_bp_cmp(0, "0", "0");
	for_test_bp_cmp(0, "0", "0");
	for_test_bp_cmp(0, "0", "0");
	for_test_bp_cmp(0, "0", "0");
	if (flag_for_test_number_for_test_bp_cmp == 0)
	{
		printf("\n\"bn_cmp\" is OK\n");
	}
	else
	{
		printf("\nERRORS in \"bn_cmp\"\n");
	}
	return;
}


int main() {
	test_bn_cmp();
	bn *a = bn_new(); // a = 0
	bn *b = bn_init(a); // b тоже = 0
	int code;
	code = bn_init_string(a, "123456789012345678");
	code = bn_init_string(b, "999");
	bn_print(a);
	bn_print(b);
	
	// a = 123456789012345678
	// Здесь и далее code - код ошибки.
	// 0 - всё хорошо
	// Не 0 - что-то пошло не так.
	
	code = bn_add_to(a, b); // a = 123456789012346677
	bn_print(a);
	/*
	code = bn_pow_to(b, 5); // b = 999^5
	code = bn_root_to(b, 5); // b = 999
	code = bn_init_int(a, 0); // a = 0
	code = bn_div_to(b, a); // OOPS, 999/0
	if (code != 0) {
		4
			printf("bn_div_to failed, code=%d\n", code);
	}
	*/
	bn *c = bn_new();
	code = bn_init_int(c, 222);
	bn *d = bn_new();
	code = bn_init_int(d, 333);
	bn_print(c);
	bn_print(d);
	printf("\nbn_cmp(c, d) = %d", bn_cmp(c, d));
	code = bn_sub_to(c, d);
	bn_print(c);
	code = bn_add_to(c, d);
	bn_print(c);
	code = bn_sub_to(d, c);
	bn_print(d);
	code = bn_sub_to(d, c);
	bn_print(d);
	code = bn_add_to(d, c);
	bn_print(d);
	code = bn_add_to(d, c);
	bn_print(d);
	/*
	bn *e = bn_add(c, d); // e = 555
	bn_print(e);
	bn *f = bn_mod(e, c); // f = 111
	const char *r1 = bn_to_string(f, 10); // r1 -> "111"
	printf("f=%s\n", r1);
	free(r1);
	code = bn_cmp(c, d);
	if (code < 0) {
		printf("c < d\n");
	}
	else if (code == 0) {
		printf("c == d\n");
	}
	else {
		printf("c > d\n");
	}
	*/
	code = bn_neg(b); // b = -999;
	bn_print(b);
	int bsign = bn_sign(b); // bsign = -1
	code = bn_abs(b); // b = 999;
	/*bn_delete(f); // Забудете удалить - провалите тесты
	bn_delete(e);*/
	bn_delete(d);
	bn_delete(c);
	bn_delete(b);
	bn_delete(a);
	return 0;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
