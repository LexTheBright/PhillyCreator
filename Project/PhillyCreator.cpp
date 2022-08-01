#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <assert.h>
#include <locale.h>
#include <mbstring.h>
#include <time.h>
typedef unsigned char UChar;
using namespace std;
int cou = 0; //флаг - первый приоритет 3 заменен на 6
int yes = 1; //флаг - пользователь не посчитал необходимым перепроверять словарь на наличие повторяющихся слов
int x = 0; // строк матрицы 
int y = 0; // столбцов матрицы
int z = 0; // число слов в исходном словаре
int gar_len = 0; // не будет слов, длиннее этого, точно
#define SHUFFLE_ITERATION_PER_WORD 1
#define NUMBER_OF_HEURISTIC_SEARCH int(trunc(sqrt(x * y / z))) + 3
#define NUMBER_OF_PASSAGEWAY_OF_HEURISTIC int(trunc(x * y / sqrt(z) / cbrt(z))) + 6
#define TOTAL_LETTERS x * y
#define NUMBER_OF_CDSW 100
#define NUMBER_OF_HIGHEST_PRIORITY 100
#define go_down 0
#define go_left 1
#define go_up 2
#define go_right 3

/* Функция GreatestCommonDivisor              */
/* Назначение:                                */
/*     нахождение наибольшего общего делителя */
/* Входные данные:                            */
/*     m - число                              */
/*     n - число                              */
/* Выходные данные:                           */
/*     отсутствуют                            */
/* Возвращаемое значение:                     */
/*     наибольший общий делитель двух чисел   */
int GreatestCommonDivisor(int m, int n)
{
	int answer = -1;
	while (answer < 0) {
		m = abs(m);
		n = abs(n);
		if (m > n) {
			swap(m, n);
		}
		if (m == 0) {
			answer = answer * (-1) * n;
			break;
		}
		if (m == n) {
			answer = answer * (-1) * n;
			break;
		}
		if (m == 1) {
			answer = answer * (-1);
			break;
		}
		if ((m % 2) == 0) {
			if ((n % 2) == 0) {
				answer = answer * 2;
				m >>= 1;
				n >>= 1;
			}
			else {
				m >>= 1;
			}
		}
		else {
			if ((n % 2) == 0) {
				n >>= 1;
			}
			else {
				n = (n - m) >> 1;
			}
		}
	}
	return answer;
}

void status_bar(int n) { //Завершено n %
	if (n) {
		cout << "\r";
	}
	cout << "Завершено: " << n << "%";
	if (n == 100) cout << endl;
}


struct characteristic_dictionary_state_word { // характеристическое слово состояния словаря (сколько сколькобукбенных слов в одном решении), ХССС, Икс-Трис 
	int * w;
	characteristic_dictionary_state_word (void) {
		w = new int[gar_len];
		assert(w);
		for (int i = 0; i < gar_len; i++) w[i] = 0;
	}
	characteristic_dictionary_state_word(characteristic_dictionary_state_word & existing) { // существующее слово на входе
		w = new int[gar_len];
		assert(w);
		for (int i = 0; i < gar_len; i++) w[i] = existing.w[i];
	}
	~characteristic_dictionary_state_word() {
		if (w) delete[] w;
	}
	void operator = (characteristic_dictionary_state_word & existing) {
		for (int i = 0; i < gar_len; i++) w[i] = existing.w[i];
	}
};

struct array_of_cdsw { // массив XCCC
	characteristic_dictionary_state_word * m;
	int used;
	array_of_cdsw(void) : used(0) {
		m = new characteristic_dictionary_state_word[NUMBER_OF_CDSW];
		assert(m);
	}
	void add(characteristic_dictionary_state_word & existing) {
		if (used < NUMBER_OF_CDSW) {
			m[used] = existing;
			used++;
		}
		else {
			int i = rand() % NUMBER_OF_CDSW;
			m[i] = existing;
		}
	}
	~array_of_cdsw() {
		if (m) delete[] m;
	}
};

struct elem_queue { // элемент двусвязной очереди
	int info;
	elem_queue * before;
	elem_queue * after;
	elem_queue(void) : info(-1), before(NULL), after(NULL) {};
	elem_queue(elem_queue * parent) {
		after = NULL;
		before = parent;
		info = -1;
	}
	void set(int i) {
		info = i;
	}
	~elem_queue() {
		if (before) delete before;
	}
};

struct queue { //двусвязная очередь
	elem_queue * front;
	elem_queue * rear;
	int size;
	queue(void) {
		rear = new elem_queue;
		front = rear;
		size = 0;
	}
	void enqueue(int new_value) { // добавить в очередь
		size++;
		rear->after = new elem_queue(rear);
		assert(rear->after);
		rear = rear->after;
		rear->set(new_value);
	}
	int dequeue(void) { // вытолкнуть из очереди
		if (size == 0) {
			return -1;
		}
		else {
			size--;
			int i = front->after->info;
			front = front->after;
			if (front->before) {
				delete front->before;
				front->before = NULL;
			}
			return i;
		}
	}
	~queue() {
		delete rear;
	}
};

struct dictionary { // изначальный словарь
	int number; // счётчик слов
	int total_lenght;  // суммарная длина слов в первоначальном словаре
	int max_len; // максимальная длина слова
	int min_len; // минимальная длина слова
	int Nfws; // число слов в итоговом наборе
	int * distribution; // распределение
	int * len; // массив длин слов
	int * to_rise; // номера слов, имеющих первый раз такое число букв, при упорядочении по возрастанию
	int * used_w; // использованы ли слова
	int * fws; // номера выбранных слов (итоговый набор)
	UChar ** d; // массив слов
	dictionary(void) : number(0), max_len(0), to_rise(NULL), total_lenght(0), min_len(-1), fws(NULL), Nfws(0) {
		d = new UChar *[z];
		len = new int[z];
		used_w = new int[z];
		distribution = new int[z];
		assert(d);
		assert(len);
		assert(used_w);
		assert(distribution);
		int i = 0;
		for (; i < z; i++) distribution[i] = used_w[i] = len[i] = 0;
	}
	~dictionary() {
		int i = 0;
		for (i = 0; i < number; i++) {
			delete[](d[i]);
		}
		delete[] d;
		delete[] len;
		delete[] used_w;
		delete[] distribution;
		if (to_rise) {
			delete[] to_rise;
			to_rise = NULL;
		}
		if (fws) {
			delete[] fws;
			fws = NULL;
		}
	}
	int add_word(UChar * s) { // добавить слово в словарь
		assert(number < z);
		used_w[number] = 0;
		int lenght = _mbslen(s);
		if (number == 0) {
			min_len = lenght;
		}
		else {
			if (lenght < min_len) min_len = lenght;
		}
		d[number] = new UChar[lenght + 1];
		assert(d[number]);
		_mbscpy_s(d[number], lenght + 1, s);
		len[number] = lenght;
		distribution[lenght]++;
		number++;
		if (lenght > max_len) {
			max_len = lenght;
		}
		if (number == z) gar_len = max_len + 1;
		total_lenght += lenght;
		return lenght;
	}
	void swap_words(int a, int b) {
		swap(len[a], len[b]);
		swap(d[a], d[b]);
		swap(used_w[a], used_w[b]);
	}
	int sort(int a) { // сортировать список слов, 0 -- возрастание, иначе убывание
		int i;
		if (to_rise) {
			delete[] to_rise;
			to_rise = NULL;
		}
		to_rise = new int[max_len + 1];
		for (i = 0; i <= max_len; i++) to_rise[i] = -1;
		assert(number == z);
		int j = 0;
		for (i = 0; i < number; i++) {
			for (j = i + 1; j < number; j++) {
				if (yes == 0) { // пользователь хотел скать неразличмые строки?
				if (a == 0) {
					if (_mbscmp(d[i], d[j]) == 0) {
						cout << "Найдены неразличимые строки " << d[i] << " и " << d[j] << endl;
						return 1;
					}
					// сортировка по ВОЗРАСТАНИЮ
					if (len[i] > len[j]) swap_words(i, j);
				}
				else {
					// сортировка по УБЫВАНИЮ
					if (len[i] < len[j]) swap_words(i, j);
				}
				}
			}
		}
		for (i = number - 1; i >= 0; i--) {
			to_rise[len[i]] = i;
		}
		yes = 0;
		return 0;
	}
	int shuffle(int s) { //перемешивание словаря
		int * n = new int[number];
		assert(n);
		int n_total = (-(SHUFFLE_ITERATION_PER_WORD - 1) * number);
		int i, j;
		for (i = 0; i < number; i++) { n[i] = (-SHUFFLE_ITERATION_PER_WORD); }
		cout << "Текущая операция — перетасовка словаря № " << s << endl;
		status_bar(0);
		while (n_total != number) {
			//Sleep(100);
			i = rand() % number;
			j = rand() % number;
			if ((n[i] == 0) && (n[j] == 0)) continue;
			swap_words(i, j);
			if (n[i] < 0) {
				n[i]++;
				n_total++;
			}
			if (n[j] < 0) {
				n[j]++;
				n_total++;
			}
			i = (number - n_total) * 100 / (number * SHUFFLE_ITERATION_PER_WORD); // процентов осталось
			switch (100 - i) {
			case 0:
				j = 1;
				break;
			case 100:
				j = 99;
				break;
			default:
				j = 100 - i;
				break;
			}
			status_bar(j);
		}
		status_bar(100);
		return (s + 1);
	}
	int check() { //проверка условий
		if (total_lenght < TOTAL_LETTERS) {
			cout << "Суммарная длина слов в словаре недостаточна для покрытия матрицы." << endl;
			return 1;
		}
		else {
			cout << "В матрице " << TOTAL_LETTERS << " ячеек, общая длина слов в словаре " << total_lenght << ", это допустимо." << endl;
		}
		if (min_len > TOTAL_LETTERS) {
			cout << "Все слова, содержащиеся в словаре, не помещаются в матрицу." << endl;
			return 2;
		}
		else {
			cout << "В матрице " << TOTAL_LETTERS << " ячеек, самое короткое слово в словаре имеет длину " << min_len << ", это допустимо." << endl;
		}
		cout << " >>:Желаете проверить словарь на наличие повторяющихся слов? (0-да, 1-нет):<<" << endl;
		cin >> yes;
			if (sort(0) == 1) {
				cout << "Недопустимо наличие повторяющихся слов в словаре." << endl;
				return 4;
			}
			else {
				cout << "Принято, что среди слов словаря нет повторяющихся." << endl;
			}
		int nod = len[0];
		int i, j;
		for (i = 1; i <= max_len; i++) {
			switch (nod) {
			case 1:
				return 0;
				break;
			case 2:
			case 3:
				j = to_rise[i];
				if (j != -1) {
					nod = ((len[j] % nod) ? 1 : nod);
				}
				break;
			default:
				j = to_rise[i];
				if (j != -1) {
					nod = GreatestCommonDivisor(nod, len[j]);
				}
				break;
			}
		}
		i = (TOTAL_LETTERS % nod);
		if (i) {
			cout << "Все слова словаря имеют длину, кратную " << nod << ", но число ячеек в матрице не кратно этому числу." << endl;
			return 8;
		}
		return 0;
	}
	int heuristic_scan() { // проход первого уровня
		int i, j;
		for (i = 0; i < number; i++) used_w[i] = 0;
		queue q;
		int last = -1; // последний вытолкнутый
		int sum = 0;
		for (i = 1; i <= NUMBER_OF_PASSAGEWAY_OF_HEURISTIC; i++) {
			cout << "Проход словаря № " << i << " из " << NUMBER_OF_PASSAGEWAY_OF_HEURISTIC << endl;
			for (j = 0; j < number; j++) {
				if (used_w[j] == 0) {
					if (((sum + len[j]) <= TOTAL_LETTERS) && (j != last)) {
						used_w[j] = 1;
						sum += len[j];
						q.enqueue(j);
					}
				}
			}
			if (sum == TOTAL_LETTERS) {
				break;
			}
			else {
				if (q.size != 0) { // не уверен, что надо.
					last = q.dequeue();
					if (last != -1) {
						sum -= len[last];
						used_w[last] = 0;
					}
				}
			}
		}
		return sum;
	}
	int lvl_one() { //первый уровень выбора слов
		clock_t start, finish;
		double duration;
		start = clock();
		int i = 1;
		do {
			//i++;
			i = shuffle(i);
			cout << "Попытка № " << (i - 1) << " из " << NUMBER_OF_HEURISTIC_SEARCH << endl;
			if (heuristic_scan() == TOTAL_LETTERS) {
				finish = clock();
				duration = (double)(finish - start) / CLOCKS_PER_SEC;
				cout << "На прохождение первого уровня затрачено " << duration << " секунд." << endl;
				cout << "Эвристические методы справились с задачей формирования набора слов." << endl;
				return 0;
			}
		} while (i <= NUMBER_OF_HEURISTIC_SEARCH);
		cout << "Сортировка." << endl;
		sort(1);
		if (heuristic_scan() == TOTAL_LETTERS) {
			finish = clock();
			duration = (double)(finish - start) / CLOCKS_PER_SEC;
			cout << "На прохождение первого уровня затрачено " << duration << " секунд." << endl;
			cout << "Эвристические методы справились с задачей формирования набора слов." << endl;
			return 0;
		}
		finish = clock();
		duration = (double)(finish - start) / CLOCKS_PER_SEC;
		cout << "На прохождение первого уровня затрачено " << duration << " секунд." << endl;
		cout << "Эвристические методы не справились с задачей формирования набора слов." << endl;
		return 1;
	}
	int lvl_two() { //второй уровень выбора слов
		sort(1);
		clock_t start, finish;
		double duration;
		start = clock();
		cout << "Запущен аналитический алгоритм второго уровня. Это может занять некоторое время." << endl;
		status_bar(0);
		int i, j, s = 0;
		for (i = 0; i < number; i++) used_w[i] = 0;
		array_of_cdsw ar;
		characteristic_dictionary_state_word cdsw;
		int direction = 0;
		if ((total_lenght >> 1) > TOTAL_LETTERS) { // если половина словаря больше числа клеток в матрице
			// to rise
			direction = 0;
			s = 0;
			for (i = 0; i <= max_len; i++) cdsw.w[i] = 0;
		}
		else {
			// to down
			direction = 1;
			s = total_lenght;
			for (i = 0; i <= max_len; i++) cdsw.w[i] = distribution[i];
		}
		for (i = max_len; i >= 0; i--) {
			analytical_search(ar, cdsw, i, direction, s);
			j = i * 100 / max_len; // процентов осталось
			switch (100 - j) {
			case 0:
				j = 1;
				break;
			case 100:
				j = 99;
				break;
			default:
				j = 100 - j;
				break;
			}
			status_bar(j);
		}
		status_bar(100);
		if (ar.used) {
			cout << "Начинается извлечение набора из характеристического слова..." << endl;
			s = rand() % ar.used;
			for (i = 0; i < max_len + 1; i++) cout << i << ' ' << ar.m[s].w[i] << endl; // вывод ХССС на экран
			for (i = 0; i < max_len + 1; i++) { // i -- длина слова
				while (ar.m[s].w[i] > 0) { // пока (число выбранных слов длины i) больше нуля
					for (j = to_rise[i]; len[j] == len[to_rise[i]]; j++) {}; // j -- номер первого слова длины i -- становится номером первого слова иной (следующей по sort(1)) длины
					j = (rand() % (j - to_rise[i])) + to_rise[i]; // j становится номером случайного слова длины i
					if (used_w[j] == 0) {
						used_w[j] = 1;
						ar.m[s].w[i]--;
					}
				}
			}
			cout << "Извлечение набора завершено." << endl;
			cout << "Аналитический алгоритм справился с задачей формирования набора слов." << endl;
			finish = clock();
			duration = (double)(finish - start) / CLOCKS_PER_SEC;
			cout << "На прохождение второго уровня затрачено " << duration << " секунд." << endl;
			return 0;
		}
		else {
			cout << "Аналитический алгоритм не нашёл допустимых наборов. Полагаю, их и нет для данного словаря при данной матрице." << endl;
			finish = clock();
			duration = (double)(finish - start) / CLOCKS_PER_SEC;
			cout << "На прохождение второго уровня затрачено " << duration << " секунд." << endl;
			return 1;
		}
	}
	int analytical_search(array_of_cdsw & ar, characteristic_dictionary_state_word & cdsw, int lenght_of_last_word, int direction, int sum) { // рекурсивная функция второго уровня выбора слов
		// direction == 0 -- to rise. else -- to down
		characteristic_dictionary_state_word new_w;
		int s, i;
		new_w = cdsw;
		if (direction) {
			if (!(0 < cdsw.w[lenght_of_last_word])) return 1;
			new_w.w[lenght_of_last_word]--;
			s = sum - lenght_of_last_word;
		}
		else {
			if (!(distribution[lenght_of_last_word] > cdsw.w[lenght_of_last_word])) return 1;
			new_w.w[lenght_of_last_word]++;
			s = sum + lenght_of_last_word;
		}
		if (s == TOTAL_LETTERS) {
			ar.add(new_w);
			return 0;
		}
		else {
			 if (direction) { // ищем по уменьшению
				 if (s > TOTAL_LETTERS) {
					 for (i = lenght_of_last_word; i <= max_len; i++) {
						 if ((0 < new_w.w[i]) && (s - i >= TOTAL_LETTERS)) {
							 analytical_search(ar, new_w, i, direction, s);
						 }
					 }
				 }
			 }
			 else { // ищем по увеличению
				 if (s < TOTAL_LETTERS) {
					 for (i = lenght_of_last_word; i <= max_len; i++) {
						 if ((new_w.w[i] < distribution[i]) && (s + i <= TOTAL_LETTERS)) {
							 analytical_search(ar, new_w, i, direction, s);
						 }
					 }
				 }
			 }
			 return 1;
		}
	}
	void FormFinalWordSet() { //формирования списка использованных слов (магазина)
		int i = 0, j;
		for (; i < number; i++) {
			if (used_w[i]) Nfws++;
		}
		i = -1;
		//shuffle(i);
		fws = new int[Nfws];
		fws[0] = -1;
		for (i = 0, j = 0; j < Nfws; i++) {
			if (used_w[i] == 1) {
				fws[j] = i;
				// вывод сформированного магазина
				cout << d[fws[j]] << endl;
				j++;
				if (j < Nfws) fws[j] = -1;
			}
		}
	}
};

struct cell { //ячейка филворда
	UChar sym;
	int v; // соседние символы, число
	int w; // соседние стены
	int p; // приоритет
	cell(void) {
		sym = 0;
		v = w = p = 0;
	}
	int set_p() {
		p = 3 * v + 2 * w;
		if ((cou == 0) && (p == 3)) {
			p = 6; cou++;
		}
		return p;
	}
};

struct coor { //ячейка, хранящая координаты
	int a;
	int b;
	coor(void) : a(0), b(0) {};
	void set(int i, int j) {
		a = i;
		b = j;
	}
	coor(coor & q) {
		a = q.a;
		b = q.b;
	}
	coor & operator = (coor & q) {
		a = q.a;
		b = q.b;
		return *this;
	}
};

struct matrix { //филворд
	cell ** m;
	matrix(void) {
		int i;
		m = new cell * [x];
		assert(m);
		for (i = 0; i < x; i++) {
			m[i] = new cell [y];
			assert(m[i]);
		}
		for (i = 0; i < x; i++) {
			m[i][0].w++;
			m[i][0].set_p();
			m[i][y - 1].w++;
			m[i][y - 1].set_p();
		}
		for (i = 0; i < y; i++) {
			m[0][i].w++;
			m[x - 1][i].w++;
			m[0][i].set_p();
			m[x - 1][i].set_p();
		}
	}
	~matrix() {
		int i;
		for (i = 0; i < x; i++) {
			if (m[i]) delete[] m[i];
		}
		if (m) delete[] m;
	}
	void new_p_cell(int i, int j) { // пересчет приоритета клетки
		m[i][j].v++;
		m[i][j].set_p();
	}
	void new_p_neighbor_cell(coor & q) { //пересчет приоритетов соседних клеток
		new_p_neighbor_cell(q.a, q.b);
	}
	void new_p_neighbor_cell(int i, int j) {  //пересчет приоритетов соседних клеток
		/*if (i > 0) new_p_cell(i - 1, j);
		if (i < x - 1) new_p_cell(i + 1, j);
		if (j > 0) new_p_cell(i, j - 1);
		if (j < y - 1) new_p_cell(i, j + 1);*/
		if (i > 0) {
			if (m[i - 1][j].sym == 0) {
				new_p_cell(i - 1, j);
			}
		} 
		if (i < x - 1) {
			if (m[i + 1][j].sym == 0) {
				new_p_cell(i + 1, j);
			}
		}
		if (j > 0) {
			if (m[i][j - 1].sym == 0) {
				new_p_cell(i, j - 1);
			}
		}
		if (j < y - 1) {
			if (m[i][j + 1].sym == 0) {
				new_p_cell(i, j + 1);
			}
		}
	}
	void choice_from_priority(int num_let, coor & last_let) { // выбор куда ставить букву  num_let - false => первая буква, иначе => не первая 
		int max = 0; //максимальный приоритет
		int used = 0; //клетка на рассмотрении?
		int found; //приоритет найден?
		int i;
		if (num_let) { //не первая
			coor pr_m[4];
			coor temp;
			for (found = 0; found <= 1; found++) {
				for (i = go_down; i <= go_right; i++) {
					temp = last_let;
					switch (i) {
					case go_down:
						if (last_let.a < x - 1) temp.a++;
						break;
					case go_left:
						if (last_let.b > 0) temp.b--;
						break;
					case go_up:
						if (last_let.a > 0) temp.a--;
						break;
					case go_right:
						if (last_let.b < y - 1) temp.b++;
					}
					if (found == 0) {
						if ((m[temp.a][temp.b].sym == 0) && (m[temp.a][temp.b].p > max)) max = m[temp.a][temp.b].p;
					}
					else {
						if ((m[temp.a][temp.b].sym == 0) && (m[temp.a][temp.b].p == max)) {
							pr_m[used] = temp;
							used++;
						}
					}
				}
			}
			used = rand() % used;
			last_let = pr_m[used];
		}
		else { // первая буква
			int j;
			coor pr_m[NUMBER_OF_HIGHEST_PRIORITY];
			for (found = 0; found <= 1; found++) {
				for (i = 0; i < x; i++) {
					for (j = 0; j < y; j++) {
						if (found == 0) {
							if ((m[i][j].sym == 0) && (m[i][j].p > max)) max = m[i][j].p;
						}
						else {
							if ((m[i][j].sym == 0) && (m[i][j].p == max)) {
								if (used < NUMBER_OF_HIGHEST_PRIORITY) {
									pr_m[used].set(i, j);
									used++;
								}
								else {
									pr_m[rand() % NUMBER_OF_HIGHEST_PRIORITY].set(i, j);
								}
							}
						}
					}
				}
			}
			used = rand() % used;
			last_let = pr_m[used];
		}
	}
	void set_sym(coor & q, UChar c) { // установить букву
		//set_sym(q.a, q.b, c);
		m[q.a][q.b].sym = c;
	}
	/*void set_sym(int i, int j, UChar c) {
		m[i][j].sym = c;
	}*/
};

void input_initial_information(FILE * stream) { //запрос входных данных
	cout << "Введите число строк и столбцов в матрице:" << endl;
	cin >> x >> y;
	assert(x > 0);
	assert(y > 0);
	fscanf_s(stream, "%d", &z); // выполнять ДО создания словаря
	fscanf_s(stream, "%d", &gar_len);
}

void load_dictionary(dictionary & dict, FILE * stream) { // загрузить словарь
	int i;
	UChar * s = new UChar[gar_len];
	for (i = 0; i < z; i++) {
		fscanf_s(stream, "%s", s, gar_len + 1);
		dict.add_word(s);
	}
	// не писать уничтожение строки!
}

void output_initial_information() { // дать информацию про входные данные
	cout << "x (строк в матрице) = " << x << endl;
	cout << "y (стоблцов в матрице) = " << y << endl;
	cout << "z (слов в словаре) = " << z << endl;
	cout << "gar_len (гарантированный максимум длин слов) = " << gar_len << endl;
}


void show_con(matrix &table) { //вывод филворда на экран
	int i, j;
	cout << endl << endl << endl << endl;
	cout << "          ";
	//for (j = 0; j < y + 2; j++) cout << (char)134 << " ";
	for (j = 0; j < y + 2; j++) cout <<  " ";
	cout << endl;
	for (i = 0; i < x; i++) {
		cout << "          ";
		//cout << (char)134;
		for (j = 0; j < y; j++) {
			cout << " " << table.m[i][j].sym;
			//if (j == (y - 1)) cout << " " << (char)134;
			if (j == (y - 1)) cout << " ";
		}
		cout << endl;
	}
	cout << "          ";
	for (j = 0; j < y + 2; j++) cout <<  " ";
	//for (j = 0; j < y + 2; j++) cout << (char)134 << " ";
	cout << endl << endl << endl << endl << endl;
}

int writeFile(matrix &table, dictionary &dict) { //формирования вывода данных в файл
	int i, j;
	ofstream dic;
	dic.open("in.txt", ios::trunc);
	dic << x << " " << y << " " << dict.Nfws << endl;
	for (i = 0; i < x; i++) {
		for (j = 0; j < y; j++) {
			dic << table.m[i][j].sym;
		}
		dic << endl;
	}
	for (i = 0, j = 0; j < dict.Nfws; i++) {
		if (dict.used_w[i] == 1) {
			dic << dict.d[dict.fws[j]] << endl;
			j++;
		}
	}
	dic.close();
	return 0;
}

int main(void) {
	setlocale(LC_ALL, ".1251");
	srand((unsigned)time(NULL));
	int i, j;
	FILE *stream = NULL;
	assert(!fopen_s(&stream, "dictionary.txt", "r"));
	assert(stream);
	input_initial_information(stream);
	dictionary dict;
	load_dictionary(dict, stream);
	fclose(stream);
	output_initial_information();

	i = dict.check(); // сортировка по росту длины, проверка на равенство, формирование массива начальных слов для длин, (sort(0))
	if (i == 0) {
		if (dict.lvl_one()) { // перетасовки и сортировка по убыванию длин при неудаче
			cout << "Покрывающий набор не сформирован на первом уровне." << endl;
			// запуск второго уровня
			if (dict.lvl_two()) {
				cout << "Покрывающий набор не сформирован на втором уровне." << endl;
				system("pause");
				return 0;
			}
			else {
				dict.FormFinalWordSet();
			}
		}
		else {
			//for (i = 0; i < z; i++) {
			//	cout << dict.used_w[i] << ' ' << dict.d[i] << endl;
			//	вывод словаря с отметками о взятии
			//}
			dict.FormFinalWordSet();
		}
	}
	else {
		cout << "Гарантированно не существует покрытия матрицы при данном словаре." << endl;
		system("pause");
		return 0;
	}

	//srand((unsigned)time(NULL));
	matrix table;
	coor temp_cell;
	for (i = 0; i < dict.Nfws; i++) {
		for (j = 0; j < dict.len[dict.fws[i]]; j++) {
			table.choice_from_priority(j, temp_cell); // найти клетку
			table.set_sym(temp_cell, dict.d[dict.fws[i]][j]); // записать символ
			table.new_p_neighbor_cell(temp_cell); // пересчитать соседние клетки (приоритеты)
		}
	}

	writeFile(table, dict);
	show_con(table);
	
	system("pause");
	return 0;
}
