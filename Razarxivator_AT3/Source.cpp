#include <iostream>
#include <string.h>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <thread>
#include <mutex>
#include <Windows.h>
#define THREAD_COUNT 4

#define SIZE_OF_ELEMENT 7

using namespace std;

struct item
{
	long double start;       // Начало диапазона символа
	long double finish;      // Конец диапазона символа
	long double probability; // Вероятность вхождения этог сивола
	char symbol;             // Сам символ
};

void range_modify(vector<item>* chars, int desired_element)
{
	long double start1 = (*chars)[desired_element].start;
	long double finish1 = (*chars)[desired_element].finish;
	(*chars)[0].start = start1;
	(*chars)[0].finish = start1 + (finish1 - start1) * (*chars)[0].probability;
	for (int i = 1; i < (*chars).size() - 1; i++)
	{
		(*chars)[i].start = (*chars)[i - 1].finish;
		(*chars)[i].finish = (*chars)[i].start + (finish1 - start1) * (*chars)[i].probability;
	}
	(*chars)[(*chars).size() - 1].start = (*chars)[(*chars).size() - 2].finish;
	(*chars)[(*chars).size() - 1].finish = finish1;

}

mutex MTX;
void ProcessPart(vector<vector<string>>& textPart, vector<item>& chars, vector<vector<string>>& parts, int number_of_part) {

	string text;
	vector<string> part;
	for(string key:textPart[number_of_part])
	{
		text = "";
		std::replace(key.begin(), key.end(), ',', '.');
		std::istringstream ss(key);
		long double number;

		ss >> std::setprecision(53) >>number;
		vector<item> chars_modified = chars;
		//cout << std::setprecision(53)<<number << endl;
		bool check = 0;
		for (int i = 0; i != SIZE_OF_ELEMENT; i++)
		{
			for (int i = 0; i <= chars.size() - 1; i++)
			{
				if (chars_modified[i].start<number && chars_modified[i].finish>number)
				{
					range_modify(&chars_modified, i);

					text += chars[i].symbol;
					break;
				}
			}
		}
		part.push_back(text);
	}
	MTX.lock();
	parts[number_of_part] = part;
	MTX.unlock();


}
int main()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	std::ifstream input("C:/Users/ivanp/Desktop/output.txt");
	std::ofstream output("C:/Users/ivanp/Desktop/final_text.txt");
	setlocale(LC_ALL, "Rus");
	int total_count;
	string dictionary = "10l3o2d1e1h1r1w1";
	string key = "0.7604403372";
	vector<item> chars;



	std::getline(input, dictionary); // считать первую строку в переменную dictionary

	string CC;
	bool one = 0;
	bool two = 0;
	for (int i = 0; i != dictionary.size() - 1; i++)
	{
		if (one == 0)
		{
			while (dictionary[i] >= 47 && dictionary[i] <= 58)
			{
				CC += dictionary[i];
				i++;
			}
			total_count = stoi(CC);
			one = 1;
		}

		item new_item;
		string c;
		new_item.symbol = dictionary[i];
		i++;
		while (dictionary[i] >= 47 && dictionary[i] <= 58)
		{
			c += dictionary[i];
			if (i == dictionary.size() - 1)
			{
				break;
				two = 1;
			}
			i++;
		}
		new_item.probability = stoi(c);
		chars.push_back(new_item);

		i--;

	}
	for (int i = 0; i <= chars.size() - 1; i++)
	{
		chars[i].probability /= total_count;
		if (i == 0)
		{
			chars[i].start = 0;
			chars[i].finish = chars[i].probability;
		}
		else
		{
			chars[i].start = chars[i - 1].finish;
			chars[i].finish = chars[i].start + chars[i].probability;
			if (i == chars.size() - 1)
			{
				chars[i].finish = 1;
			}
		}
	}
	SetConsoleTextAttribute(hConsole, BACKGROUND_GREEN | BACKGROUND_RED);
	cout << "Начальные диапазоны символов текста:";
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	cout << endl;
	for (item& c : chars)
	{
		cout << "Символ: " << c.symbol << "  Старт: " << c.start << "  Финиш: " << c.finish << endl;
	}


	vector<string> numbers;
	string temporary;
	while (std::getline(input, temporary))
	{
		numbers.push_back(temporary);     // Заносим все цифры из зашифровки в один вектор
	}
	
	vector<vector<string>> numbers_for_threads(THREAD_COUNT);

	int stringsPerThread = numbers.size() / THREAD_COUNT; // Распределяем числа между потоками

	for (int i = 0; i < THREAD_COUNT; i++) {

		int startIndex = i * stringsPerThread;
		int endIndex = startIndex + stringsPerThread;

		if (i == THREAD_COUNT - 1) {
			endIndex = numbers.size();
		}

		for (int j = startIndex; j < endIndex; j++) {
			numbers_for_threads[i].push_back(numbers[j]);
		}

	}

	vector<vector<string>> parts(THREAD_COUNT);

	// Запуск потоков
	vector<thread> threads(THREAD_COUNT);

	for (int i = 0; i < THREAD_COUNT; i++) {

		threads[i] = thread(ProcessPart, ref(numbers_for_threads), ref(chars), ref(parts), i);
	}
	for (int i = 0; i < THREAD_COUNT; i++) {

		threads[i].join();
	}

	for (int i = 0; i < parts.size(); i++) {
		for (int j = 0; j < parts[i].size(); j++) {
			output << parts[i][j];
		}
	}
	SetConsoleTextAttribute(hConsole, BACKGROUND_GREEN);
	cout << "Вывод текста в файл выполнен успешно!";
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	cout << endl;
}