// pch.cpp: исходный файл, соответствующий предкомпилированному заголовку; нужен для компиляции
#define DEBUG

#include "pch.h"



// В целом этот файл можно пропустить, но не удаляйте его, если вы используете предкомпилированные заголовки.

// Файл bn_Kudriakov.c
//#include "bn.h"


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#ifndef DEBUG
enum bn_codes {
	BN_OK, BN_NULL_OBJECT, BN_NO_MEMORY, BN_DIVIDE_BY_ZERO
};
#endif // !DEBUG


struct bn_s {
	int *body; // Тело большого числа
	int bodysize; // Размер массива body
	int sign; // Знак числа
};
static const int numeral_length = 11; // Длина цифры в битах

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int DEBUG_FLAG = 0;
const int OK_FLAG = 0;

void Change_debug_flag(int v)
{
	DEBUG_FLAG = v;
	return;
}
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
//static int min(int a, int b);

//Функция, возвращающая 1, если число больше 0
static int strange_function_1_0(int x);

//Функция, возвращающая 1, если число больше 0 для ull
//static int strange_function_ull_1_0(unsigned long long x);

// Сдвиг (работает)
static int bn_move(bn *t, const int n);

// Копирование с сохранением длины левого (размер памяти во время процеуры не уменьшается)
// Операция, аналогичная " = "
static int bn_copy(bn *t, const bn *right);

// Число значащих цифр
static int bn_real_size(const bn *t);

// Достаточная длина числа при переходе между единицами измерения (решение показательного неравенства)
static int strange_function_new_size_radix(const int length, const int radix_from, const int radix_to);

// Инициализировать с нужной длинной
//static bn * bn_new_length(int new_length);

//Деление на короткое число  -  абсолютные знаения (-5 / 3 = -1)
static int bn_int_abs_div_to(bn *t, int b, int * mod);

// Если левое меньше по абсолютному значению, вернуть <0; если равны, вернуть 0; иначе >0
static int bn_abs_cmp(bn const *left, bn const *right);

//Создать нулевое число с нужной длиной
static bn* bn_init_nulls_length(int val_length);

// Возвести число в степень degree с внешним буфером
static int bn_pow_to_root(bn *t, int degree, int real_size, bn *copy);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// TO DEBAG
// Вывести структуру
int bn_print(bn const * t)
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
	printf("\n\n");
	for (i = t->bodysize - 1; i >= 0; i--)
	{
		printf("%7d ", t->body[i]);
	}
	return BN_OK;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Деление на короткое число  -  абсолютные знаения (-5 / 3 = -1)
static int bn_int_abs_div_to(bn *t, int b, int * mod)
{
	if (t == NULL || t->body == NULL)
	{
		return 1;
	}
	if (b == 0)
	{
		return 3;
	}
	if (t->sign == 0)
	{
		return 0;
	}
	if (b < 0)
	{
		b = -b;
		t->sign = -t->sign;
	}
	int i;
	long long p = 0;
	int k = 0;
	//bn_print(t);
	for (i = t->bodysize - 1; i >= 0; i--)
	{
		//bn_print(t);
		t->body[i] = t->body[i] + (int)(p << numeral_length);
		p = t->body[i] % b;
		t->body[i] = t->body[i] / b;
		if (t->body[i] != 0)
			k = 1;
	}
	*mod = (int)p;
	if (k == 0)
	{
		t->sign = 0;
	}
	return 0;
}

//Деление на длинное короткое число  -  абсолютные знаения (-5 / 3 = -1)
static int bn111_int_abs_div_to(bn *t, unsigned long long b, unsigned long long * mod)
{
	if (t == NULL || t->body == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (b == 0)
	{
		return BN_DIVIDE_BY_ZERO;
	}
	if (t->sign == 0)
	{
		int code = bn_set_nulls(t, t->bodysize);
		return code;
	}
	int i;
	unsigned long long x = 0, p = 0;
	int k = 0;
	//bn_print(t);
	for (i = t->bodysize - 1; i >= 0; i--)
	{
		//bn_print(t);
		x = (unsigned long long) t->body[i] + (int)(p << numeral_length);
		//printf("\nx = %-9llu  p = %-9llu  x%%b = %-9llu  x/b = %-9llu  b = %-9llu\n", x, p, x%b, x / b, b);
		p = x % b;
		t->body[i] = x / b;
		if (t->body[i] != 0)
			k = 1;
	}
	*mod = (int)p;
	if (k == 0)
	{
		t->sign = 0;
	}
	return BN_OK;
}

//Деление на длинное короткое число с обрезкой -  абсолютные знаения (-5 / 3 = -1)
static int bn222_int_abs_div(bn *t, bn *r, unsigned long long b, int cut_length, unsigned long long * mod, int * position)
{
	if (t == NULL || t->body == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (b == 0)
	{
		return BN_DIVIDE_BY_ZERO;
	}
	if (t->sign == 0)
	{
		int code = bn_set_nulls(r, r->bodysize);
		return code;
	}
	if (r->bodysize < t->bodysize)
	{
		int code = bn_set_nulls(r, t->bodysize);
		if (code != 0)
			return code;
	}
	//memset((void *)(r), 0, r->bodysize);										//maybe drop it later
	memset((void *)(r + r->bodysize - cut_length), 0, cut_length);			//CHECK IT
	r->sign = t->sign;
	int i;
	unsigned long long x = 0, p = 0;
	int k = 0;
	*position = -1;
	//bn_print(t);
	for (i = t->bodysize - 1; i >= cut_length; i--)
	{
		//bn_print(t);
		x = (unsigned long long) t->body[i] + (int)(p << numeral_length);
		if (DEBUG_FLAG == 1)
			printf("\nx = %-9llu  p = %-9llu  x%%b = %-9llu  x/b = %-9llu  b = %-9llu\n", x, p, x%b, x / b, b);
		p = x % b;
		r->body[i - cut_length] = x / b;
		if (r->body[i - cut_length] != 0)
		{
			if (k == 0)
			{
				*position = i - cut_length;
			}
			k = 1;
		}
	}
	*mod = (int)p;
	if (k == 0)
	{
		//printf("k==0 in bn111_int_abs_div_to");
		r->sign = 0;
	}
	return BN_OK;
}

// Инициализировать с нужной длинной
/*static bn * bn_new_length(int new_length)
{
	bn * r = (bn *)malloc(sizeof(bn));
	if (r == NULL) return NULL;
	r->bodysize = 0;
	r->body = NULL;
	r->sign = 0;
	int code = bn_longer_rubbish(r, new_length);
	if (code == 2)
	{
		free(r);
		return NULL;
	}
	return r;
}*/

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
		free((void *)t->body);
		memset((void *)(a + t->bodysize), 0, ((int)(new_size - t->bodysize)) * sizeof(int)); // !!! CHECK to +-1 Errors  //CHECKED -- OK
	}
	else
	{
		memset((void *)a, 0, new_size * sizeof(int));
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
			printf("\nERROR 1 in \"bn_set_nulls\"\n");
		return BN_NULL_OBJECT;
	}
	if (new_size <= t->bodysize)
	{
		memset((void *)t->body, 0, t->bodysize * sizeof(int));
		t->sign = 0;
		if (DEBUG_FLAG == 1 && OK_FLAG == 1)
			printf("\n0 1 in \"bn_set_nulls\"\n");
		return BN_OK;
	}
	int *a = (int *)calloc(new_size, sizeof(int));
	if (a == NULL)
	{
		if (DEBUG_FLAG == 1)
			printf("\nERROR 2 in \"bn_set_nulls\" after using \"calloc\"\n");
		return BN_NO_MEMORY;
	}
	t->sign = 0;
	if (t->body != NULL)
		free((void *)(t->body));
	t->bodysize = new_size;
	t->body = a;

	if (DEBUG_FLAG == 1 && OK_FLAG == 1)
		printf("\n0 2 in \"bn_set_nulls\"\n");
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
	if (b < 0)
	{
		b = -b;
		t->sign = -t->sign;
	}
	if (b == (1 << numeral_length))
	{
		return bn_move(t, 1);
	}
	if (b == (1ll << (2 * numeral_length)))
	{
		return bn_move(t, 2);
	}
	if (t->body[t->bodysize - 1] != 0)
	{
		int code = bn_longer_with_saving_values(t, t->bodysize + 2);
		if (code == 2)
			return BN_NO_MEMORY;
	}
	long long x = 0, p = 0;
	int i;
	for (i = 0; i < t->bodysize; i++)
	{
		x = (unsigned long long) t->body[i] * b + p;
		t->body[i] = (int)((1ll << numeral_length) - 1)&x;
		p = (x >> numeral_length);
	}
	if (p != 0 && DEBUG_FLAG == 1)
	{
		printf("\n You have a wrong result in function \"bn_int_mul_to\" p == %lld \n", p);
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
	if (t->bodysize < 4 || t->body[t->bodysize - 1] != 0 || t->body[t->bodysize - 2] != 0 || t->body[t->bodysize - 3] != 0 || t->body[t->bodysize - 4] != 0)
	{
		int code = bn_longer_with_saving_values(t, t->bodysize + 4);
		if (code == 2)
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
		printf("\nYou have a wrong result in function \"bn_int_abs_add_to\" p == %lld \n", p);
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
	return BN_OK;
}


// Создать новое BN
bn *bn_new()
{
	bn * r = (bn *)malloc(sizeof(bn));
	if (r == NULL) return NULL;
	r->bodysize = 0;
	r->body = NULL;
	r->sign = 0;
	int code = bn_set_nulls(r, 1);
	if (code == 2)
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
	return r;
}

// Ограничить число нулём снизу
static double strange_abs(double x)
{
	if (x < 0) return BN_OK;
	return x;
}

// Обычный int max(int, int);
static int max(int a, int b)
{
	if (a > b)
		return a;
	return b;
}
/*
// Обычный int min(int, int);
static int min(int a, int b)
{
	if (a < b)
		return a;
	return b;
}
*/
// Достаточная длина числа при переходе между единицами измерения (решение показательного неравенства)
static int strange_function_new_size_radix(const int length, const int radix_from, const int radix_to)
{
	if (radix_to == 1)
		return -1;														//CRAZY TO USE IT
	return 5 + (int)trunc(1.0 * (length)* strange_abs(log(radix_from) / log(radix_to)));
}

// Инициализировать значение BN представлением строки
// в системе счисления radix
int bn_init_string_radix(bn *t, const char *init_string, int radix)
{
	if (t == NULL)
	{
		return BN_NULL_OBJECT;
	}

	int st_length = (int)strlen(init_string);
	int p = 0;
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
	int new_size = strange_function_new_size_radix(st_length - p, radix, 1 << numeral_length);
	int code = bn_set_nulls(t, new_size);
	if (code == 2)
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
/*
//Функция, возвращающая 1, если число больше 0 для ull
static int strange_function_ull_1_0(unsigned long long x)
{
	if (x > 0)
		return 1;
	return 0;
}
*/

// Инициализировать значение BN десятичным представлением строки
int bn_init_string(bn *t, const char *init_string)
{
	return bn_init_string_radix(t, init_string, 10);
}

// Инициализировать значение BN заданным целым числом
int bn_init_int(bn *t, int init_int)
{
	if (t == NULL)
	{
		return BN_NULL_OBJECT;
	}
	// Если выпендриваться, то это, но не думаю, что стоит, ведь я вряд ли изменю длину цифры
	int code = bn_set_nulls(t, strange_function_1_0(32 % numeral_length) + 32 / numeral_length);
	//int code = bn_set_nulls(t, 2);
	if (code == BN_NO_MEMORY)
	{
		if (DEBUG_FLAG == 1)
			printf("\nERROR 2 in \"bn_init_int\" after using \"bn_set_nulls\"\n");
		return BN_NO_MEMORY;
	}
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
		free((void *)t->body);
	free((void *)t);
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
		int code = bn_copy(t, right);
		if (code != 0)
			return code;
		t->sign = -t->sign;
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

		x = (unsigned long long) a->body[i] - b->body[i] - p;
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
		memmove((void *)(t->body + n), (void *)(t->body), ((int)(t->bodysize - n)) * sizeof(int));			// CHECK IT LATER //CHECKED
		memset((void *)(t->body), 0, n * sizeof(int));
		return BN_OK;
	}
	if (n < 0)
	{
		const int p = -n;
		memmove((void *)(t->body), (void *)(t->body + p), ((int)(t->bodysize - p)) * sizeof(int));
		memset((void *)(t->body + t->bodysize - 0 - p), 0, p * sizeof(int));						// CHECK IT LATER //CHECKED
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
	memset((void *)(t->body + right->bodysize), 0, ((int)(t->bodysize - right->bodysize)) * sizeof(int));
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

//Умножение столбиком
int bn_mul_to_STOLBIK(bn *t, bn const *right, const int t_real_size, const int right_real_size)
{
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
	bn_delete(a);
	bn_delete(b);
	return code;
}
/*
int Mul_memory_header()
{
	//Really, you need a new structure for this function, and class-functions better.
	// I don'really want do it in this project, because it should be new project with fast memory or mayby
	// "smart STEK" without alone pointers - only one big space of memory.
	// not now, please
}

//Умножение методом Карацубы с постоянным перезаказом памяти
//(нужна обложка для избежания перезаказа, голова рекурсии, где будет учтена вся память, а затем удалена)
int bn_mul_to_Karatsuba_oh_no_memory_not_optimized(bn *t, bn const *right, const int t_real_size, const int right_real_size)
{
	const int N = max(t_real_size, right_real_size);
	const int M = N / 2;
	bn *A0 = bn_init_nulls_length(M + 2), *A1 = bn_init_nulls_length(M + 2),
		*B0 = bn_init_nulls_length(M + 2), *B1 = bn_init_nulls_length(M + 2);
	memcpy(A0, t, M * sizeof(int));
	memcpy(B0, right, M * sizeof(int));
	memcpy(A1, t + M, (N - M) * sizeof(int));
	memcpy(B1, right + M, (N - M) * sizeof(int));


}
*/
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
	const int t_real_size = bn_real_size(t), right_real_size = bn_real_size(right);
	int code;
	code = bn_mul_to_STOLBIK(t, right, t_real_size, right_real_size);
	return code;
}

//Создать нулевое число с нужной длиной
static bn* bn_init_nulls_length(int val_length)
{
	bn * r = (bn *)malloc(sizeof(bn));
	if (r == NULL) return NULL;
	r->bodysize = 0;
	r->body = NULL;
	r->sign = 0;
	int code = bn_set_nulls(r, val_length);
	if (code == 2)
	{
		free(r);
		return NULL;
	}
	return r;
}

static int Function_to_div_to(bn const *t, bn const * right, bn* result, bn* copy, bn* mul_res, int i, int l, int r, int sr, int *newl, int *newr, int *val_cmp_in_circle, int* true_val, int *flag)
{
	if (*flag == 0)
		return BN_OK;
	result->body[i] = sr;
	//int code = bn_copy(copy, result);
	//code = max(code, bn_mul_to(copy, right));
	int code = bn_copy(copy, right);
	code = max(code, bn_int_mul_to(copy, sr));
	code = max(code, bn_move(copy, i));
	code = max(code, bn_add_to(copy, mul_res));
	*val_cmp_in_circle = bn_abs_cmp(copy, t);
	if (DEBUG_FLAG == 1)
	{
		printf("\n\nresult = ");
		bn_print(result);
		//printf("\n\ncopy = ");
		//bn_print(copy);
		//printf("\n\nt = ");
		//bn_print(t);
		printf("\nval_cmp_in_circle = %d, true_val = %d, l = %d, sr = %d, r = %d\n", *val_cmp_in_circle, *true_val, l, sr, r);
	}
	if (*val_cmp_in_circle == 0)
	{
		*true_val = result->body[i];
		code = max(code, bn_copy(mul_res, copy));
		*flag = 0;
	}
	else if (*val_cmp_in_circle == 1)
	{
		*newr = sr;
	}
	else if (*val_cmp_in_circle == -1)
	{
		*newl = sr;
		*true_val = result->body[i];
	}
	return code;
}


// Операция, аналогичная /=
int bn_div_to(bn *t, bn const *right)
{
	if (t == NULL || right == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (right->sign == 0)
	{
		return BN_DIVIDE_BY_ZERO;
	}
	if (t->sign == 0)
	{
		return BN_OK;
	}
	int RESULT_SIGN = t->sign * right->sign;
	/*
	17 / 10 = 1
	- 17 / 10 = -2
	17 / -10 = -2
	- 17 / -10 = 1
	*/
	int val_cmp = bn_abs_cmp(t, right);
	if (val_cmp < 0)
	{
		int code = bn_set_nulls(t, t->bodysize);
		if (RESULT_SIGN == -1)
		{
			code = max(code, bn_int_add_abs_to(t, -1));
		}
		return code;
	}
	if (val_cmp == 0)
	{
		int code = bn_set_nulls(t, t->bodysize);
		code = max(code, bn_int_add_abs_to(t, RESULT_SIGN));
		return code;
	}
	unsigned long long b = 0;
	int right_real_size = bn_real_size(right);
	//int num_to_ll = sizeof(unsigned long long) * 2 / numeral_length;
	int num_to_ll = 2;
	int i;
	for (i = right_real_size - 1; i >= 0 && i >= right_real_size - num_to_ll; i--)
	{
		b = (b << numeral_length);
		b += right->body[i];
	}

	if (num_to_ll >= right_real_size)
	{
		unsigned long long mod;
		int code = bn111_int_abs_div_to(t, b, &mod);
		if (code != 0)
		{
			return code;
		}
		t->sign = RESULT_SIGN;
		if (RESULT_SIGN == -1 && mod > 0)
		{
			code = bn_int_add_abs_to(t, 1);
		}
		return BN_OK;
	}

	int t_real_size = bn_real_size(t);
	int result_size = t_real_size - right_real_size + 2;
	bn *result = bn_init_nulls_length(result_size);
	if (result == NULL)
	{
		return BN_NO_MEMORY;
	}
	result->sign = RESULT_SIGN;
	bn *copy = bn_init_nulls_length(result_size + right_real_size);
	//bn *copy = bn_init_nulls_length(t->bodysize);
	if (copy == NULL)
	{
		bn_delete(result);
		return BN_NO_MEMORY;
	}
	bn *mul_res = bn_init_nulls_length(result_size + right_real_size);
	//bn *copy = bn_init_nulls_length(t->bodysize);
	if (mul_res == NULL)
	{
		bn_delete(result);
		bn_delete(mul_res);
		return BN_NO_MEMORY;
	}
	if (DEBUG_FLAG == 1)
	{
		printf("\n\nt = ");
		bn_print(t);
		printf("\nright = \n");
		bn_print(right);
	}
	int code = 0;
	int flag = 1;
	int val_cmp_in_circle = 0;
	for (i = result_size - 1; i >= 0 && flag == 1; i--)
	{
		if (DEBUG_FLAG == 1)
			printf("\nITERATION i == %d\n", i);
		int l = 0;
		int r = (1 << numeral_length) - 1;
		//int flag1 = 0;
		int true_val = 0;
		code = max(code, Function_to_div_to(t, right, result, copy, mul_res, i, 0, (1 << numeral_length), 1, &l, &r, &val_cmp_in_circle, &true_val, &flag));
		if (true_val == 0)
		{
			result->body[i] = true_val;
		}
		else
		{
			while (flag != 0 && l <= r)
			{
				const int ll = l, rr = r;
				if (l == r - 2)
				{
					code = max(code, Function_to_div_to(t, right, result, copy, mul_res, i, ll, rr, ll, &l, &r, &val_cmp_in_circle, &true_val, &flag));
					code = max(code, Function_to_div_to(t, right, result, copy, mul_res, i, ll, rr, ll + 1, &l, &r, &val_cmp_in_circle, &true_val, &flag));
					code = max(code, Function_to_div_to(t, right, result, copy, mul_res, i, ll, rr, rr, &l, &r, &val_cmp_in_circle, &true_val, &flag));
					result->body[i] = true_val;
					break;
				}
				else if (l == r - 1)
				{
					code = max(code, Function_to_div_to(t, right, result, copy, mul_res, i, ll, rr, ll, &l, &r, &val_cmp_in_circle, &true_val, &flag));
					code = max(code, Function_to_div_to(t, right, result, copy, mul_res, i, ll, rr, rr, &l, &r, &val_cmp_in_circle, &true_val, &flag));
					result->body[i] = true_val;
					break;
				}
				else if (l == r)
				{
					code = max(code, Function_to_div_to(t, right, result, copy, mul_res, i, ll, rr, ll, &l, &r, &val_cmp_in_circle, &true_val, &flag));
					result->body[i] = true_val;
					break;
				}
				else
				{
					code = max(code, Function_to_div_to(t, right, result, copy, mul_res, i, ll, rr, (ll + rr) / 2, &l, &r, &val_cmp_in_circle, &true_val, &flag));
				}
			}
		}
		int code = bn_copy(copy, right);
		code = max(code, bn_int_mul_to(copy, result->body[i]));
		code = max(code, bn_move(copy, i));
		code = max(code, bn_add_to(mul_res, copy));
	}
	if (RESULT_SIGN == -1 && val_cmp_in_circle != 0)
	{
		code = bn_int_add_abs_to(result, -1);
	}
	code = max(code, bn_copy(t, result));
	bn_delete(result);
	bn_delete(mul_res);
	bn_delete(copy);
	return code;
}



// Операция, аналогичная /=
int bn_div_to1(bn *t, bn const *right)
{
	if (t == NULL || right == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (right->sign == 0)
	{
		return BN_DIVIDE_BY_ZERO;
	}
	if (t->sign == 0)
	{
		return BN_OK;
	}
	int RESULT_SIGN = t->sign * right->sign;
	/*
	17 / 10 = 1
	- 17 / 10 = -2
	17 / -10 = -2
	- 17 / -10 = 1
	*/
	int val_cmp = bn_abs_cmp(t, right);
	if (val_cmp < 0)
	{
		int code = bn_set_nulls(t, t->bodysize);
		if (RESULT_SIGN == -1)
		{
			code = max(code, bn_int_add_abs_to(t, -1));
		}
		return code;
	}
	if (val_cmp == 0)
	{
		int code = bn_set_nulls(t, t->bodysize);
		code = max(code, bn_int_add_abs_to(t, RESULT_SIGN));
		return code;
	}
	unsigned long long b = 0;
	int right_real_size = bn_real_size(right);
	//int num_to_ll = sizeof(unsigned long long) * 2 / numeral_length;
	int num_to_ll = 2;
	int i;
	for (i = right_real_size - 1; i >= 0 && i >= right_real_size - num_to_ll; i--)
	{
		b = (b << numeral_length);
		b += right->body[i];
	}

	if (num_to_ll >= right_real_size)
	{
		unsigned long long mod;
		int code = bn111_int_abs_div_to(t, b, &mod);
		if (code != 0)
		{
			return code;
		}
		t->sign = RESULT_SIGN;
		if (RESULT_SIGN == -1 && mod > 0)
		{
			code = bn_int_add_abs_to(t, 1);
		}
		return BN_OK;
	}
	return BN_OK;

	//bn * t_big = bn_new_length(t->bodysize);
	bn * t_big = bn_init(t);
	if (t_big == NULL)
	{
		return BN_NO_MEMORY;
	}
	if (DEBUG_FLAG == 1)
	{
		int val = bn_cmp(t_big, t);
		if (val != 0)
			printf("\n\"bn_init\" in \"bn_div_to\" is uncorrect\n");
	}
	/*int code = bn_copy(t_big, t);
	if (code != 0)
	{
		return code;
	}
	int p_sign_to_add = t->sign;*/
	bn_abs(t_big);
	bn * r = bn_new();
	if (r == NULL)
	{
		return BN_NO_MEMORY;
	}
	bn * c = bn_new();
	if (c == NULL)
	{
		return BN_NO_MEMORY;
	}
	int code = 0;
	unsigned long long mod = 0;
	int p = 1;
	int k = 0;
	for (; t_big->sign != 0; k++, p = -p)
	{
		if (DEBUG_FLAG == 1)
		{
			printf("\nNEW_ITERATION  ==  %d, b == %llu\n", k, b);
			printf("\n__T_BIG");
			bn_print(t_big);
			printf("\n__R");
			bn_print(r);
			printf("\n__C");
			bn_print(c);
		}
		int position = -1;
		code = max(code, bn222_int_abs_div(t_big, r, b, num_to_ll + 1, &mod, &position));
		if (DEBUG_FLAG == 1)
		{
			printf("\nAFTER bn222_int_abs_div_to(t_big, r, b, 0, &mod)\n");
			printf("\nb == %llu, mod == %llu\n", b, mod);
			printf("\n__T_BIG");
			bn_print(t_big);
			printf("\n__R");
			bn_print(r);
		}
		bn_abs(r);
		if ((r->sign == 0) || ((position <= num_to_ll) && (b > (((unsigned long long)r->body[1]) << numeral_length) + (unsigned long long)r->body[0])))
		{
			if (DEBUG_FLAG == 1)
			{
				printf("\nLAST_ITERATION  ==  %d\n", k);
				printf("\n__T_BIG");
				bn_print(t_big);
				printf("\n__R");
				bn_print(r);
				printf("\np == %d\n", p);
			}
			/*if (p == 1)
			{
				code = max(code, bn_int_add_abs_to(r, 1));
			}*/
			if (p == 1)
				code = max(code, bn_add_to(c, r));
			else
				code = max(code, bn_sub_to(c, r));
			if (DEBUG_FLAG == 1)
			{
				printf("\nAFTER bn_add/sub_to(c, r)\n");
				printf("\n__C");
				bn_print(c);
			}
			bn_abs(c);
			code = max(code, bn_mul_to(r, right));
			if (DEBUG_FLAG == 1)
			{
				printf("\nAFTER bn_mul_to(r, right)\n");
				printf("\n__R");
				bn_print(r);
			}
			bn_abs(r);
			code = max(code, bn_sub_to(t_big, r));
			if (DEBUG_FLAG == 1)
			{
				printf("\nAFTER bn_sub_to(t_big, r)\n");
				printf("\n__T_BIG");
				bn_print(t_big);
				bn_abs(t_big);
			}
			break;
		}
		if (p == 1)
			code = max(code, bn_add_to(c, r));
		else
			code = max(code, bn_sub_to(c, r));

		if (DEBUG_FLAG == 1)
		{
			printf("\nAFTER bn_add/sub_to(c, r)\n");
			printf("\n__C");
			bn_print(c);
		}
		bn_abs(c);
		code = max(code, bn_mul_to(r, right));
		if (DEBUG_FLAG == 1)
		{
			printf("\nAFTER bn_mul_to(r, right)\n");
			printf("\n__R");
			bn_print(r);
		}
		bn_abs(r);
		code = max(code, bn_sub_to(t_big, r));
		if (DEBUG_FLAG == 1)
		{
			printf("\nAFTER bn_sub_to(t_big, r)\n");
			printf("\n__T_BIG");
			bn_print(t_big);
			bn_abs(t_big);
		}
	}
	code = max(code, bn_copy(r, c));
	code = max(code, bn_mul_to(r, right));
	r->sign = t->sign;
	int val1 = bn_cmp(r, t);
	if (val1 < 0)
	{
		if (RESULT_SIGN < 0)
			code = max(code, bn_int_add_abs_to(c, 1));
	}
	code = max(code, bn_copy(t, c));
	t->sign = RESULT_SIGN;
	if (RESULT_SIGN == -1 && mod == 0)
	{
		code = bn_int_add_abs_to(t, 1);
	}
	code = max(code, bn_delete(t_big));
	code = max(code, bn_delete(r));
	code = max(code, bn_delete(c));
	return code;
}

// Операция, аналогичная %=
int bn_mod_to(bn *t, bn const *right)
{
	if (t == NULL || right == NULL)
	{
		return BN_NULL_OBJECT;
	}
	bn *copy = bn_init(t);
	if (copy == NULL)
	{
		return BN_NO_MEMORY;
	}
	int code = bn_div_to(copy, right);
	if (code != 0)
	{
		bn_delete(copy);
		return code;
	}
	code = bn_mul_to(copy, right);
	if (code != 0)
	{
		bn_delete(copy);
		return code;
	}
	code = bn_sub_to(t, copy);
	bn_delete(copy);
	return code;
}


// Возвести число в степень degree
int bn_pow_to(bn *t, int degree)
{
	if (t == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (t->sign == 0 || degree == 1)
	{
		return BN_OK;
	}
	if (degree == 0)
	{
		int code = bn_init_int(t, 1);
		return code;
	}
	const int real_size = bn_real_size(t);
	if (real_size == 1)
	{
		if (t->body[0] == 1)		// Быстрое возведение в степень числа +-1
		{
			if (degree % 2 == 0)
				t->sign = 1;
			return BN_OK;
		}
		if (t->body[0] == 2)		// Быстрое возведение в степень числа +-2
		{
			//printf
			int copy_sign = t->sign;
			if (degree % 2 == 0)
				copy_sign = 1;
			int cel = degree / numeral_length;
			int drob = degree % numeral_length;
			int code = bn_set_nulls(t, cel + 2);
			if (code != 0)
				return code;
			t->sign = copy_sign;
			t->body[cel] = (1 << drob);
			return BN_OK;
		}
	}
	bn *copy = bn_init(t);
	if (copy == NULL)
	{
		return BN_NO_MEMORY;
	}
	int code = bn_init_int(t, 1);
	if (code != 0)
	{
		bn_delete(copy);
		return code;
	}
	code = bn_longer_with_saving_values(t, real_size * degree);
	if (code != 0)
	{
		bn_delete(copy);
		return code;
	}
	int i;
	for (i = 31; i >= 0 && (((1 << i)&degree) == 0); i--);
	for (; i >= 0; i--)
	{
		code = max(code, bn_mul_to(t, t));
		if (((1 << i)&degree) != 0)
			code = max(code, bn_mul_to(t, copy));
	}
	bn_delete(copy);
	return code;
}

//Умножить на int  //DO NOT USE WITH SHORT numeral_length - you may have wrong result (p != 0)
static int bn_int_mul_to_to_square(bn* t, int b, int position)
{
	if (t == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (b == 0)
	{
		return bn_set_nulls(t, 1);
	}
	
	if (t->bodysize < 4 || t->body[t->bodysize - 1] != 0 || t->body[t->bodysize - 2] != 0 || t->body[t->bodysize - 3] != 0 || t->body[t->bodysize - 4] != 0)
	{
		int code = bn_longer_with_saving_values(t, t->bodysize + 4);
		if (code == 2)
			return BN_NO_MEMORY;
	}
	bn_move(t, position);
	long long x = 0, p = 0;
	int i;
	for (i = position; i < t->bodysize; i++)
	{
		x = (unsigned long long) t->body[i] * b + p;
		t->body[i] = (int)((1ll << numeral_length) - 1)&x;
		p = (x >> numeral_length);
	}
	if (p != 0 && DEBUG_FLAG == 1)
	{
		printf("\n You have a wrong result in function \"bn_int_mul_to\" p == %lld \n", p);
	}
	return BN_OK;
}

static int bn_int_add_abs_to_square(bn* t, int b, int position)
{
	if (t == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (b == 0)
	{
		return BN_OK;
	}
	if (t->bodysize < 4 || t->body[t->bodysize - 1] != 0 || t->body[t->bodysize - 2] != 0 || t->body[t->bodysize - 3] != 0 || t->body[t->bodysize - 4] != 0)
	{
		int code = bn_longer_with_saving_values(t, t->bodysize + 4);
		if (code == 2)
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
	for (i = position; p > 0 && i < t->bodysize; i++)
	{
		x = t->body[i] + p;
		t->body[i] = (int)((1ll << numeral_length) - 1)&x;
		p = (x >> numeral_length);
	}
	if (p != 0 && DEBUG_FLAG == 1)
	{
		printf("\nYou have a wrong result in function \"bn_int_abs_add_to\" p == %lld \n", p);
	}
	return BN_OK;
}

//Возвести сумму в квадрат (a + b)^2 = a^2 + 2ab + b^2
// a^2 и a изестны - упрощение операций
int bn_square(bn *res, bn *mul_res, bn* buffer, int b, int p)
{
	int code = 0;
	code = max(code, bn_copy(buffer, res));
	code = max(code, bn_int_mul_to_to_square(buffer, 2 * b, p));
	code = max(code, bn_int_add_abs_to_square(buffer, b*b, 2 * p));
	code = max(code, bn_add_to(buffer, mul_res));
	return code;
}

// Квадратный корень
static int bn_sqrt_to(bn* t)
{
	int reciprocal = 2;
	int real_size = bn_real_size(t);
	int sign = t->sign;
	if (t->sign == 0)
		return BN_OK;

	bn * copy = bn_init(t);
	if (copy == NULL)
	{
		return BN_NO_MEMORY;
	}
	copy->sign = sign;
	bn * result_bin = bn_init_nulls_length(t->bodysize);
	if (result_bin == NULL)
	{
		bn_delete(copy);
		return BN_NO_MEMORY;
	}
	bn* mul_res = bn_init_nulls_length(t->bodysize);
	if (mul_res == NULL)
	{
		bn_delete(copy);
		bn_delete(result_bin);
		return BN_NO_MEMORY;
	}
	/*bn* copy1 = bn_init_nulls_length(t->bodysize);
	if (mul_res == NULL)
	{
		bn_delete(copy);
		bn_delete(result_bin);
		bn_delete(mul_res);
		return BN_NO_MEMORY;
	}*/
	int code = 0;
	code = max(code, bn_set_nulls(t, t->bodysize));
	t->sign = 1;
	mul_res->sign = 1;
	result_bin->sign = 1;
	int val1 = -1;
	int i;
	int sqr_real_size = real_size / reciprocal + 2;
	if (DEBUG_FLAG == 1)
		printf("\nreal_size = %d", real_size);
	for (i = sqr_real_size - 1; i >= 0; i--)
	{
		if (DEBUG_FLAG == 1)
		{
			printf("\nmul_res, i %d", i);
			bn_print(mul_res);
		}
		val1 = -1;
		int l = 0, r = (1 << numeral_length);
		int sr = 1, true_val = 0;
		for (; ; )
		{
			code = max(code, bn_square(t, mul_res, result_bin, sr, i));			
			val1 = bn_abs_cmp(result_bin, copy);
			if (DEBUG_FLAG == 1)
			{
				printf("\nt, i = %d, sr = %d, true_val = %d", i, sr, true_val);
				bn_print(t);
				printf("\nresult bin, %d", i);
				bn_print(result_bin);
			}
			if (val1 == 0)
			{
				bn_delete(copy);
				//bn_delete(copy1);
				bn_delete(result_bin);
				bn_delete(mul_res);
				t->sign = sign;
				t->body[i] = sr;
				if (DEBUG_FLAG == 1)
					printf("\nexit %d\n", i);
				return code;
			}
			else if (val1 < 0)
			{
				true_val = sr;
				l = sr;
				sr = (l + r) / 2;
			}
			else
			{
				r = sr;
				sr = (l + r) / 2;
			}
			if (r - l < 2)
			{
				if (true_val == 0)
					sqr_real_size--;
				break;
			}
		}
		code = max(code, bn_square(t, mul_res, result_bin, true_val, i));
		t->body[i] = true_val;
		code = max(code, bn_copy(mul_res, result_bin));
	}
	bn_delete(copy);
	//bn_delete(copy1);
	bn_delete(result_bin);
	bn_delete(mul_res);
	t->sign = sign;
	return code;
}



// Возвести число в степень degree с внешним буфером
static int bn_pow_to_root(bn *t, int degree, int real_size, bn *copy)
{
	if (t == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (t->sign == 0 || degree == 1)
	{
		return BN_OK;
	}
	if (degree == 0)
	{
		int code = bn_init_int(t, 1);
		return code;
	}
	//const int real_size = bn_real_size(t);
	if (real_size == 1)
	{
		if (t->body[0] == 1)		// Быстрое возведение в степень числа +-1
		{
			if (degree % 2 == 0)
				t->sign = 1;
			return BN_OK;
		}
		if (t->body[0] == 2)		// Быстрое возведение в степень числа +-2
		{
			//printf
			int copy_sign = t->sign;
			if (degree % 2 == 0)
				copy_sign = 1;
			int cel = degree / numeral_length;
			int drob = degree % numeral_length;
			int code = bn_set_nulls(t, cel + 2);
			if (code != 0)
				return code;
			t->sign = copy_sign;
			t->body[cel] = (1 << drob);
			return BN_OK;
		}
	}
	int code = bn_copy(copy, t);
	code = max(code, bn_init_int(t, 1));
	code = bn_longer_with_saving_values(t, real_size * degree);
	int i;
	for (i = 31; i >= 0 && (((1 << i)&degree) == 0); i--);
	for (; i >= 0; i--)
	{
		code = max(code, bn_mul_to(t, t));
		if (((1 << i)&degree) != 0)
			code = max(code, bn_mul_to(t, copy));
	}
	return code;
}

// Извлечь корень степени reciprocal из BN (бонусная функция)
int bn_root_to(bn *t, int reciprocal)
{
	if (t == NULL)
	{
		return BN_NULL_OBJECT;
	}

	////////
	if (reciprocal == 2)
		return bn_sqrt_to(t);
	////////

	int real_size = bn_real_size(t);
	int sign = t->sign;

	bn * copy = bn_init(t);
	if (copy == NULL)
	{
		return BN_NO_MEMORY;
	}
	copy->sign = sign;
	bn * pr1 = bn_init_nulls_length(t->bodysize);
	if (pr1 == NULL)
	{
		bn_delete(copy);
		return BN_NO_MEMORY;
	}
	bn* pr2 = bn_init_nulls_length(t->bodysize);
	if (pr2 == NULL)
	{
		bn_delete(copy);
		bn_delete(pr1);
		return BN_NO_MEMORY;
	}
	bn* copy1 = bn_init_nulls_length(t->bodysize);
	if (pr2 == NULL)
	{
		bn_delete(copy);
		bn_delete(pr1);
		bn_delete(pr2);
		return BN_NO_MEMORY;
	}
	int code = 0;
	code = max(code, bn_set_nulls(t, t->bodysize));
	t->sign = 1;
	int val1 = -1;
	int i;
	int sqr_real_size = real_size / reciprocal + 2;
	for (i = sqr_real_size - 1; i >= 0; i--)
	{
		val1 = -1;
		int l = 0, r = (1 << numeral_length);
		int sr = 1, true_val = 0;
		for (; ; )
		{
			t->body[i] = sr;
			code = max(code, bn_copy(pr1, t));
			code = max(code, bn_pow_to_root(pr1, reciprocal, sqr_real_size, copy1));
			//code = max(code, bn_pow_to(pr1, reciprocal));
			val1 = bn_abs_cmp(pr1, copy);
			if (val1 == 0)
			{
				bn_delete(copy);
				bn_delete(copy1);
				bn_delete(pr1);
				bn_delete(pr2);
				t->sign = sign;
				//printf("\nexit %d\n", i);
				return code;
			}
			else if (val1 < 0)
			{
				true_val = sr;
				l = sr;
				sr = (l + r) / 2;
			}
			else
			{
				r = sr;
				sr = (l + r) / 2;
			}
			if (r - l < 2)
			{
				if (true_val == 0)
					sqr_real_size--;
				t->body[i] = true_val;
				break;
			}
		}
	}
	bn_delete(copy);
	bn_delete(copy1);
	bn_delete(pr1);
	bn_delete(pr2);
	t->sign = sign;
	return code;
}

// Аналоги операций x = l+r (l-r, l*r, l/r, l%r)
bn* bn_add(bn const *left, bn const *right)
{
	if (left == NULL || right == NULL)
	{
		return NULL;
	}
	bn* r = bn_init(left);
	bn_add_to(r, right);
	return r;
}
bn* bn_sub(bn const *left, bn const *right)
{
	if (left == NULL || right == NULL)
	{
		return NULL;
	}
	bn* r = bn_init(left);
	bn_sub_to(r, right);
	return r;
}
bn* bn_mul(bn const *left, bn const *right)
{
	if (left == NULL || right == NULL)
	{
		return NULL;
	}
	bn* r = bn_init(left);
	bn_mul_to(r, right);
	return r;
}

bn* bn_div(bn const *left, bn const *right)
{
	if (left == NULL || right == NULL)
	{
		return NULL;
	}
	bn* r = bn_init(left);
	bn_div_to(r, right);
	return r;
}
bn* bn_mod(bn const *left, bn const *right)
{
	if (left == NULL || right == NULL)
	{
		return NULL;
	}
	bn* r = bn_init(left);
	bn_mod_to(r, right);
	return r;
}

// Выдать представление BN в системе счисления radix в виде строки
// Строку после использования потребуется удалить.
const char *bn_to_string(bn const *t, int radix)
{
	if (t == NULL)
	{
		return NULL;
	}
	if (t->sign == 0)
	{
		char *st = (char *)malloc(2 * sizeof(char));
		st[0] = '0';
		st[1] = 0;
		return st;
	}
	/*
	int n=0;
	while (x>0)
	{
		a[n]=pr1(x%r);
		x/=r;
		n++;
		a[n]=0;
	}
	*/
	char *st = (char *)malloc((int)sizeof(char) * (2 + strange_function_new_size_radix(bn_real_size(t), (1 << numeral_length), radix)));
	int n = 0;
	/*if (t->sign == -1)
	{
		st[0] = '-';
		n = 1;
	}*/
	bn* a = bn_init(t);
	//bn_print(a);
	/*if (t = NULL)
	{
		strcpy(st, "2");
		return st;
	}*/
	int code = 0;
	int mod = 0;
	while (a->sign != 0)
	{
		code = max(code, bn_int_abs_div_to(a, radix, &mod));
		//bn_print(a);
		st[n] = num_int_to_char(mod);
		n++;
		st[n] = 0;
	}
	bn_delete(a);
	//printf("\nbefore move\n%s\n", st);
	int p = 0;
	if (t->sign == -1)
	{
		memmove((void *)(st + 1), (void *)(st), (n + 1) * sizeof(char));
		st[0] = '-';
		p = 1;
		n++;
	}
	//printf("\nafter move\n%s\n", st);
	int i;
	for (i = p; i < p + (n - p) / 2; i++)
	{
		char c = st[i];
		st[i] = st[n - 1 - (i - p)];
		st[n - 1 - (i - p)] = c;
	}
	//printf("\nafter reverse\n%s\n", st);
	return st;
}

// Если левое меньше по модулю, вернуть <0; если равны, вернуть 0; иначе >0
static int bn_abs_cmp(bn const *left, bn const *right)
{
	if (left == NULL || right == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (left->sign == 0 && right->sign == 0)
		return 0;
	if (left->sign == 0)
		return -1;
	if (right->sign == 0)
		return 1;
	bn const *a, *b;
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
	return p * d;
}


// Если левое меньше, вернуть <0; если равны, вернуть 0; иначе >0
int bn_cmp(bn const *left, bn const *right)
{
	if (left == NULL || right == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (left->sign < right->sign)
		return -1;
	if (left->sign > right->sign)
		return 1;
	if (left->sign == 0 && right->sign == 0)
		return 0;
	assert(left->sign == right->sign);
	return left->sign * bn_abs_cmp(left, right);
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
	t->sign = -t->sign;
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

