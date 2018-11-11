// pch.cpp: исходный файл, соответствующий предкомпилированному заголовку; нужен для компиляции

#include "pch.h"

// В целом этот файл можно пропустить, но не удаляйте его, если вы используете предкомпилированные заголовки.

// Файл bn_Kudriakov.c
//#include "bn.h"


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>



struct bn_s {
	int *body; // Тело большого числа
	int bodysize; // Размер массива body
	int sign; // Знак числа
};
static const int numeral_length = 16; // Длина цифры в битах

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const int DEBUG_FLAG = 1;
const int OK_FLAG = 0;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// "Удлиннить" до необходимого значения bodysize
static int bn_longer_with_saving_values(bn* t, const int new_size);

// "Удлиннить" до необходимого значения bodysize без сохранения значений
static int bn_longer_rubbish(bn* t, const int new_size);

//Очистить с нужной длиной
static int bn_set_nulls(bn* t, const int new_size);

//Умножить на int  //DO NOT USE WITH SHORT numeral_length - you may have wrong result (p != 0)
static int bn_int_mul_to(bn* t, int b);

// Прибавить int (числа складываются по модулю с сохранением знака t)  //DO NOT USE WITH SHORT numeral_length - you may have wrong result (p != 0)
static int bn_int_add_abs_to(bn* t, int b);

// Ограничить число нулём снизу
static double strange_abs(double x);

// Обычный int max(int, int);
static int max(int a, int b);

// Обычный int min(int, int);
static int min(int a, int b);

//Функция, возвращающая 1, если число больше 0
static int strange_function_1_0(int x);

// Сдвиг (работает)
static int bn_move(bn *t, const int n);

// Копирование с сохранением длины левого (размер памяти во время процеуры не уменьшается)
// Операция, аналогичная " = "
static int bn_copy(bn *t, const bn *right);

// Число значащих цифр
static int bn_real_size(const bn *t);

// Достаточная длина числа при переходе между единицами измерения (решение показательного неравенства)
static int strange_function_new_size_radix(const int length, const int radix_from, const int radix_to);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TO DEBAG
// Вывести структуру
int bn_print(bn* t)
{
	if (t == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (DEBUG_FLAG == 0)
	{
		return BN_OK;
	}
	printf("\n\nSign == %d ,  Length == %d\n", t->sign, t->bodysize);
	int i;
	for (i = t->bodysize - 1; i >= 0; i--)
	{
		printf("%7d ", i);
	}
	printf("\n");
	for (i = t->bodysize - 1; i >= 0; i--)
	{
		printf("%7d ", t->body[i]);
	}
	return BN_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// "Удлиннить" до необходимого значения bodysize
static int bn_longer_with_saving_values(bn* t, const int new_size)
{
	if (t == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (new_size <= t->bodysize)
		return BN_OK;
	int *a = (int *)malloc(new_size * sizeof(int));
	if (a == NULL)
		return BN_NO_MEMORY;
	if (t->body != NULL)
	{
		memcpy((void *)a, (void *)t->body, t->bodysize * sizeof(int));
		free((void *) t->body);
		memset((void *)(a + t->bodysize), 0, (new_size - t->bodysize) * sizeof(int)); // !!! CHECK to +-1 Errors  //CHECKED -- OK
	}
	else
	{
		memset((void *) a, 0, new_size * sizeof(int));
	}
	t->bodysize = new_size;
	t->body = a;
	return BN_OK;
}

// "Удлиннить" до необходимого значения bodysize без сохранения значений
static int bn_longer_rubbish(bn* t, const int new_size)
{
	if (t == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (new_size <= t->bodysize)
		return BN_OK;
	int *a = (int *)malloc(new_size * sizeof(int));
	if (a == NULL)
		return BN_NO_MEMORY;
	if (t->body != NULL)
	{
		free((void *)t->body);
	}
	t->bodysize = new_size;
	t->body = a;
	return BN_OK;
}

//Очистить с нужной длиной
static int bn_set_nulls(bn* t, const int new_size)
{
	if (t == NULL)
	{
		if (DEBUG_FLAG == 1)
			printf("\nERROR BN_NULL_OBJECT in \"bn_set_nulls\"\n");
		return BN_NULL_OBJECT;
	}
	if (new_size <= t->bodysize)
	{
		memset((void *) t->body, 0, t->bodysize * sizeof(int));
		t->sign = 0;
		if (DEBUG_FLAG == 1 && OK_FLAG == 1)
			printf("\nBN_OK 1 in \"bn_set_nulls\"\n");
		return BN_OK;
	}
	int *a = (int *)calloc(new_size, sizeof(int));
	if (a == NULL)
	{
		if (DEBUG_FLAG == 1)
			printf("\nERROR BN_NO_MEMORY in \"bn_set_nulls\" after using \"calloc\"\n");
		return BN_NO_MEMORY;
	}
	t->sign = 0;
	if (t->body != NULL)
		free((void *)(t->body));
	t->bodysize = new_size;
	t->body = a;

	if (DEBUG_FLAG == 1 && OK_FLAG == 1)
		printf("\nBN_OK 2 in \"bn_set_nulls\"\n");
	return BN_OK;
}

//Умножить на int  //DO NOT USE WITH SHORT numeral_length - you may have wrong result (p != 0)
static int bn_int_mul_to(bn* t, int b)
{
	if (t == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (b == 0)
	{
		return bn_set_nulls(t, 1);
	}
	if (b == 1)
	{
		return BN_OK;
	}
	if (b == -1)
	{
		return bn_neg(t);
	}
	if (b == (1 << numeral_length))
	{
		return bn_move(t, 1);
	}
	if (t->body[t->bodysize - 1] != 0)
	{
		int code = bn_longer_with_saving_values(t, t->bodysize + 2);
		if (code == BN_NO_MEMORY)
			return BN_NO_MEMORY;
	}
	if (b < 0)
	{
		b = -b;
		t->sign = -t->sign;
	}
	long long x = 0, p = 0;
	int i;
	for (i = 0; i < t->bodysize; i++)
	{
		x = t->body[i] * b + p;
		t->body[i] = (int)((1ll << numeral_length) - 1)&x;
		p = (x >> numeral_length);
	}
	if (p != 0 && DEBUG_FLAG == 1)
	{
		printf("\n You have a wrong result in function \"bn_int_add_abs_to\" p == %lld \n", p);
	}
	return BN_OK;
}

// Прибавить int (числа складываются по модулю с сохранением знака t) 
// DO NOT USE WITH SHORT numeral_length - you may have wrong result(p != 0)
// Если t = 0, оно принимает значение b, с учетом его знака
static int bn_int_add_abs_to(bn* t, int b)
{
	if (t == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (b == 0)
	{
		return BN_OK;
	}
	if (t->body[t->bodysize - 1] != 0)
	{
		int code = bn_longer_with_saving_values(t, t->bodysize + 2);
		if (code == BN_NO_MEMORY)
			return BN_NO_MEMORY;
	}
	if (t->sign == 0)
	{
		t->sign = 1;
		if (b < 0)
		{
			t->sign = -1;
		}
	}
	if (b < 0)
	{
		b = -b;
	}
	long long x = 0, p = b;
	int i;
	for (i = 0; p > 0 && i < t->bodysize; i++)
	{
		x = t->body[i] + p;
		t->body[i] = (int)((1ll << numeral_length) - 1)&x;
		p = (x >> numeral_length);
	}
	if (p != 0 && DEBUG_FLAG == 1)
	{
		printf("\nYou have a wrong result in function \"bn_int_mul_to\" p == %lld \n", p);
	}
	return BN_OK;
}

static char num_int_to_char(int x)
{
	if (x < 10)
		return '0' + x;
	return 'A' + x - 10;
}

static int num_char_to_int(char x)
{
	if (x >= '0' && x <= '9') return x - (int)'0';
	if (x >= 'A' && x <= 'Z') return x - (int)'A' + 10;
	if (x >= 'a' && x <= 'z') return x - (int)'a' + 10;
	return 0;
}


// Создать новое BN
bn *bn_new()
{
	bn * r = (bn *) malloc(sizeof bn);
	if (r == NULL) return NULL;
	r->bodysize = 0;
	r->body = NULL;
	r->sign = 0;
	int code = bn_set_nulls(r, 1);
	if (code == BN_NO_MEMORY)
	{
		free(r);
		return NULL;
	}
	return r;
}

// Создать копию существующего BN
bn *bn_init(bn const *orig)
{
	if (orig == NULL)
	{
		return NULL;
	}
	bn * r = (bn *)malloc(sizeof(bn));
	if (r == NULL)
		return NULL;
	r->bodysize = orig->bodysize;
	r->sign = orig->sign;
	r->body = (int *)malloc(sizeof(int) * r->bodysize);
	if (r->body == NULL)
	{
		free(r);
		return NULL;
	}
	memcpy((void *)r->body, (void *)orig->body, r->bodysize * sizeof(int));
	/*int i;
	for (i = 0; i < r->bodysize; i++)
	{
		r->body[i] = orig->body[i];
	}*/
	return r;
}

// Инициализировать значение BN десятичным представлением строки
int bn_init_string(bn *t, const char *init_string)
{
	return bn_init_string_radix(t, init_string, 10);
}

// Ограничить число нулём снизу
static double strange_abs(double x)
{
	if (x < 0) return 0;
	return x;
}

// Обычный int max(int, int);
static int max(int a, int b)
{
	if (a > b)
		return a;
	return b;
}

// Обычный int min(int, int);
static int min(int a, int b)
{
	if (a < b)
		return a;
	return b;
}

// Достаточная длина числа при переходе между единицами измерения (решение показательного неравенства)
static int strange_function_new_size_radix(const int length, const int radix_from, const int radix_to)
{
	if (radix_to == 1)
		return -1;														//CRAZY TO USE IT
	return 1 + (int)trunc(1.0 * (length + 1) * strange_abs(log(radix_from) / log(radix_to)));
}

// Инициализировать значение BN представлением строки
// в системе счисления radix
int bn_init_string_radix(bn *t, const char *init_string, int radix)
{
	if (t == NULL)
	{
		return BN_NULL_OBJECT;
	}

	int st_length = strlen(init_string);
	int p=0;
	int sign = 1;
	for (; p < st_length && init_string[p] == ' '; p++);
	if (init_string[p] == '-')
	{
		p += 1;
		sign = -1;
	}
	for (; p < st_length && init_string[p] == ' '; p++);
	for (; p < st_length && init_string[p] == '0'; p++);

	if (p == st_length)
	{
		return bn_set_nulls(t, 1);
	}
	int new_size = strange_function_new_size_radix(st_length - p, radix, 1<<numeral_length);
	int code = bn_set_nulls(t, new_size);
	if (code == BN_NO_MEMORY)
		return BN_NO_MEMORY;
	t->sign = sign;
	int i;
	for (i = p; i < st_length; i++)
	{
		code = max(code, bn_int_mul_to(t, radix));
		code = max(code, bn_int_add_abs_to(t, num_char_to_int(init_string[i])));
	}
	return code;
}

//Функция, возвращающая 1, если число больше 0
static int strange_function_1_0(int x)
{
	if (x > 0)
		return 1;
	return 0;
}

// Инициализировать значение BN заданным целым числом
int bn_init_int(bn *t, int init_int)
{
	if (t == NULL)
	{
		return BN_NULL_OBJECT;
	}
	// Если выпендриваться, то это, но не думаю, что стоит, ведь я вряд ли изменю длину цифры
	//int code = bn_set_nulls(t, strange_function_1_0(32 % numeral_length) + 32 / numeral_length);
	int code = bn_set_nulls(t, 2);
	if (code == BN_NO_MEMORY)
	{
		if (DEBUG_FLAG == 1)
			printf("\nERROR BN_NO_MEMORY in \"bn_init_int\" after using \"bn_set_nulls\"\n");
		return BN_NO_MEMORY;
	}
	// Drop it, now it is in function "bn_int_add_abs_to",
	// if you uncomment it, you'll have an error (wrong answer)
	/*
	if (init_int == 0)
	{
		return BN_OK;
	}
	t->sign = 1;
	if (init_int < 0)
	{
		init_int = -init_int;
		t->sign = -1;
	}
	*/
	//
	code = bn_int_add_abs_to(t, init_int);
	return code;
}

// Уничтожить BN (освободить память)
int bn_delete(bn *t)
{
	if (t == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (t->body != NULL)
		free(t->body);
	free(t);
	return BN_OK;
}


// Операция, аналогичная +=
int bn_add_to(bn *t, bn const *right)
{
	if (t == NULL || right == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (right->sign == 0)
	{
		return BN_OK;
	}
	if (t->sign == 0)
	{
		int code = bn_copy(t, right);
		return code;
	}
	if (t->sign == right->sign)
	{
		int code = 0;
		if (t->body[t->bodysize - 1] != 0 || t->bodysize < right->bodysize)
			code = bn_longer_with_saving_values(t, 1 + max(t->bodysize, right->bodysize));
		if (code != 0)
			return code;

		long long x = 0, p = 0;
		int i;
		for (i = 0; i < right->bodysize; i++)
		{
			p += right->body[i];
			x = t->body[i] + p;
			t->body[i] = (int)((1ll << numeral_length) - 1)&x;
			p = (x >> numeral_length);
		}
		for (i = right->bodysize; p > 0 && i < t->bodysize; i++)
		{
			x = t->body[i] + p;
			t->body[i] = (int)((1ll << numeral_length) - 1)&x;
			p = (x >> numeral_length);
		}
		if (p != 0 && DEBUG_FLAG == 1)
		{
			printf("\nYou have a wrong result in function \"bn_add_to\" p == %lld \n", p);
		}
		return BN_OK;
	}
	assert(t->sign != right->sign);
	bn *c = bn_init(right);
	if (c == NULL)
	{
		return BN_NO_MEMORY;
	}
	bn_neg(c);
	int code = bn_sub_to(t, c);
	bn_delete(c);
	return code;
}

// Операция, аналогичная -=
int bn_sub_to(bn *t, bn const *right)
{
	if (t == NULL || right == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (right->sign == 0)
	{
		return BN_OK;
	}
	if (t->sign == 0)
	{
		free((void *)t);
		t = bn_init(right);
		return BN_OK;
	}
	if (t->sign != right->sign)
	{
		bn *c = bn_init(right);
		if (c == NULL)
		{
			return BN_NO_MEMORY;
		}
		bn_neg(c);
		int code = bn_add_to(t, c);
		bn_delete(c);
		return code;
	}
	assert(t->sign == right->sign);
	int cmp_abs_value = bn_cmp(t, right);
	if (cmp_abs_value == 0)
	{
		int code = bn_set_nulls(t, 2);
		return code;
	}
	if (t->sign == -1)
	{
		cmp_abs_value = -cmp_abs_value;
	}
	// cmp_abs_value == 1, если знак результата совпадает с изначальным знаком, иначе -1;
	bn const *a = t, *b = right;
	if (cmp_abs_value == -1)
	{
		a = right;
		b = t;
		t->sign = -t->sign;
	}
	// "a" - большее по модулю число, "уменьшаемое"
	int code = 0;
	if (t->bodysize < right->bodysize)
		code = bn_longer_with_saving_values(t, right->bodysize);
	if (code != 0)
		return code;

	long long x = 0, p = 0;
	int i;
	for (i = 0; i < b->bodysize; i++)
	{
		
		x = a->body[i] - b->body[i] - p;
		p = 0;
		if (x < 0)
		{
			x += (1ll << numeral_length);
			p = 1;
		}
		t->body[i] = (int)((1ll << numeral_length) - 1)&x;
		if ((x >> numeral_length) != 0 && DEBUG_FLAG == 1)
		{
			printf("\nYou have a wrong result in function \"bn_add_to\" p == %lld, number = %d \n", p, i);
		}
	}
	for (i = b->bodysize; p > 0 && i < a->bodysize; i++)
	{
		x = a->body[i] - p;
		p = 0;
		if (x < 0)
		{
			x += (1ll << numeral_length);
			p = 1;
		}
		t->body[i] = (int)((1ll << numeral_length) - 1)&x;
		if ((x >> numeral_length) != 0 && DEBUG_FLAG == 1)
		{
			printf("\nYou have a wrong result in function \"bn_add_to\" p == %lld, number = %d \n", p, i);
		}
	}
	if (p != 0 && DEBUG_FLAG == 1)
	{
		printf("\nYou have a wrong result in function \"bn_add_to\" p == %lld \n", p);
	}
	return BN_OK;
}

// Сдвиг (работает)
static int bn_move(bn *t, const int n)
{
	if (t == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (n == 0 || t->sign == 0)
	{
		return BN_OK;
	}
	if (n > 0)
	{
		int p = 0;
		int i;
		for (i = t->bodysize - 1; i >= 0 && t->body[i] == 0; i--, p++);
		if (p < n)
		{
			int code = bn_longer_with_saving_values(t, t->bodysize + n - p + 5);
			if (code != 0)
			{
				return code;
			}
		}
		memmove((void *)(t->body + n), (void *)(t->body), (t->bodysize - n) * sizeof(int));			// CHEK IT LATER
		memset((void *)(t->body), 0, n * sizeof(int));
		return BN_OK;
	}
	if (n < 0)
	{
		const int p = -n;
		memmove((void *)(t->body), (void *)(t->body + p), (t->bodysize - p) * sizeof(int));
		memset((void *)(t->body + t->bodysize - 0 - p), 0, p * sizeof(int));						// CHEK IT LATER
		return BN_OK;
	}
	return BN_OK;
}

// Копирование с сохранением длины левого (размер памяти во время процеуры не уменьшается)
// Операция, аналогичная " = "
static int bn_copy(bn *t, const bn *right)
{
	if (t == NULL || right == NULL)
	{
		return BN_NO_MEMORY;
	}
	if (t->bodysize < right->bodysize)
	{
		int code = bn_longer_rubbish(t, right->bodysize);
		if (code != 0)
		{
			return code;
		}
	}
	t->sign = right->sign;
	assert(t->body != NULL);
	memcpy((void *)t->body, (void *)right->body, right->bodysize * sizeof(int));
	memset((void *)(t->body + right->bodysize), 0, (t->bodysize - right->bodysize) * sizeof(int));
	return BN_OK;
}

// Число значащих цифр
static int bn_real_size(const bn *t)
{
	if (t == NULL || t->body == NULL)
	{
		return -1;
	}
	int real_size_t = 0;
	int i;
	for (i = 0; i < t->bodysize; i++)
	{
		if (t->body[i] != 0)
			real_size_t = i + 1;
	}
	return real_size_t;
}

// Операция, аналогичная *=
int bn_mul_to(bn *t, bn const *right)
{
	if (t == NULL || right == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (right->sign == 0 || t->sign == 0)
	{
		bn_set_nulls(t, 2);
		return BN_OK;
	}
	bn *a = bn_new();
	if (a == NULL)
	{
		return BN_NO_MEMORY;
	}
	bn *b = bn_new();
	if (b == NULL)
	{
		bn_delete(a);
		return BN_NO_MEMORY;
	}
	const int t_real_size = bn_real_size(t), right_real_size = bn_real_size(right);
	const int probably_size = t_real_size + right_real_size;
	int code = 0;
	code = bn_set_nulls(b, probably_size);  // You could optimise it later
	if (code != 0)
	{
		bn_delete(a);
		bn_delete(b);
		return code;
	}
	t->sign *= right->sign;
	int i;
	//printf("\nPRINT     t\n");
	//bn_print(t);
	for (i = right_real_size - 1; i >= 0; i--)
	{
		//printf("\n\nN %d", i);
		code = max(code, bn_copy(a, t));
		//bn_print(a);
		code = max(code, bn_int_mul_to(a, right->body[i]));
		//bn_print(a);
		//printf("\n          PRINT     b\n");
		//bn_print(b);
		code = max(code, bn_move(b, 1));
		//bn_print(b);
		code = max(code, bn_add_to(b, a));
		//bn_print(b);
	}
	code = max(code, bn_copy(t, b));
	delete(a);
	delete(b);
	return code;
}
/*
// Операция, аналогичная /=
int bn_div_to(bn *t, bn const *right);
// Операция, аналогичная %=
int bn_mod_to(bn *t, bn const *right);
*/
/*
// Возвести число в степень degree
int bn_pow_to(bn *t, int degree);

// Извлечь корень степени reciprocal из BN (бонусная функция)
int bn_root_to(bn *t, int reciprocal);

// Аналоги операций x = l+r (l-r, l*r, l/r, l%r)
bn* bn_add(bn const *left, bn const *right);
bn* bn_sub(bn const *left, bn const *right);
bn* bn_mul(bn const *left, bn const *right);
bn* bn_div(bn const *left, bn const *right);
bn* bn_mod(bn const *left, bn const *right);

// Выдать представление BN в системе счисления radix в виде строки
// Строку после использования потребуется удалить.
const char *bn_to_string(bn const *t, int radix);
*/

// Если левое меньше, вернуть <0; если равны, вернуть 0; иначе >0
int bn_cmp(bn const *left, bn const *right)
{
	bn const *a, *b;
	if (left->sign < right->sign)
		return -1;
	if (left->sign > right->sign)
		return 1;
	if (left->sign == 0 && right->sign == 0)
		return 0;
	assert(left->sign == right->sign);
	int p = 0;
	int d = 0;
	if (left->bodysize >= right->bodysize)
	{
		a = left;
		b = right;
		d = 1;
	}
	else
	{
		a = right;
		b = left;
		d = -1;
	}
	int i;
	for (i = a->bodysize - 1; p == 0 && i >= b->bodysize; i--)
	{
		if (a->body[i] > 0)
			p = 1;
	}
	for (i = b->bodysize - 1; p == 0 && i >= 0; i--)
	{
		if (a->body[i] > b->body[i])
			p = 1;
		if (a->body[i] < b->body[i])
			p = -1;
	}
	return p * d * a->sign;
}

// Изменить знак на противоположный
int bn_neg(bn *t)
{
	if (t == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (t->sign == 0)
		return BN_OK;
	t->sign = - t->sign;
	return BN_OK;
}

// Взять модуль
int bn_abs(bn *t)
{
	if (t == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (t->sign == 0)
		return BN_OK;
	t->sign = 1;
	return BN_OK;
}

//-1 если t<0; 0 если t = 0, 1 если t>0
int bn_sign(bn const *t)
{
	return t->sign;
}

