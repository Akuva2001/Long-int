// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.

/*
#ifndef PCH_H
#define PCH_H

// TODO: add headers that you want to pre-compile here

#endif //PCH_H
*/

#pragma once
// Файл bn.h


struct bn_s;
typedef struct bn_s bn;

enum bn_codes {
	BN_OK, BN_NULL_OBJECT, BN_NO_MEMORY, BN_DIVIDE_BY_ZERO
};

void Change_debug_flag(int v);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Вывести структуру
int bn_print(bn const * t);
// Сдвиг массива (здесь для отладки)
//int bn_move(bn* t, int n);

// Копирование с сохранением длины левого (размер памяти во время процеуры не уменьшается)
// Операция, аналогичная " = "
//int bn_copy(bn *t, const bn *right);
//Деление на короткое число  -  абсолютные знаения (-5 / 3 = -1)
//int bn_int_abs_div_to(bn *t, int b, int * mod);
unsigned long long sqrt_long_long(unsigned long long a);
int bn_square(bn *res, bn *mul_res, bn* buffer, int b);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bn *bn_new(); // Создать новое BN
bn *bn_init(bn const *orig); // Создать копию существующего BN
///*
// Инициализировать значение BN десятичным представлением строки
int bn_init_string(bn *t, const char *init_string);

// Инициализировать значение BN представлением строки
// в системе счисления radix
int bn_init_string_radix(bn *t, const char *init_string, int radix);

// Инициализировать значение BN заданным целым числом
int bn_init_int(bn *t, int init_int);

// Уничтожить BN (освободить память)
int bn_delete(bn *t);

// Операции, аналогичные +=, -=, *=, /=, %=
int bn_add_to(bn *t, bn const *right);
int bn_sub_to(bn *t, bn const *right);
int bn_mul_to(bn *t, bn const *right);
int bn_div_to(bn *t, bn const *right);
int bn_mod_to(bn *t, bn const *right);

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

// Если левое меньше, вернуть <0; если равны, вернуть 0; иначе >0
int bn_cmp(bn const *left, bn const *right);
int bn_neg(bn *t); // Изменить знак на противоположный
int bn_abs(bn *t); // Взять модуль
int bn_sign(bn const *t); //-1 если t<0; 0 если t = 0, 1 если t>0
//*/