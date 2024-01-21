#include <iostream>
#include <string.h>
#include <string>
#include <map>
#include <vector>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <thread>
#include <mutex>
#include <Windows.h> 
#include <sstream>
#define THREAD_COUNT 4

#define SIZE_OF_ELEMENT 7


using namespace std;
char final_symbol = '!';
struct simvol
{
    char symbol;
    int count;
};
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
string answer_to_string(long double answer)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(53) << answer;

    std::string result = oss.str();

    return result;
}
mutex PRT;
void ProcessPart(const string& textPart, vector<item>& chars, vector<vector<string>>& parts, int number_of_part) {
    vector<string> part;
    for (int i = 0; i < textPart.length(); i += SIZE_OF_ELEMENT)
    {
        string word = textPart.substr(i, SIZE_OF_ELEMENT);
        vector<item> chars_modified = chars;


        for (auto letter : word)
        {
            for (int i = 0; i != chars_modified.size(); i++)
            {
                if (chars_modified[i].symbol == letter)
                {

                    range_modify(&chars_modified, i);

                    break;
                }


            }
        }
        long double answer = chars_modified[0].finish;

        //cout << "Число, которое должно быть занесено: " << chars_modified[0].finish << endl;
        part.push_back(answer_to_string(answer));


    }
    PRT.lock();
    parts[number_of_part] = part;
    PRT.unlock();
}



int main()
{
    std::ifstream input("C:/Users/ivanp/Desktop/text.txt");
    std::ofstream output("C:/Users/ivanp/Desktop/output.txt");
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);


    setlocale(LC_ALL, "Rus");



    string text;

    input.seekg(0, std::ios::end);
    text.reserve(input.tellg());
    input.seekg(0, std::ios::beg);

    text.assign((std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>());
    std::replace(text.begin(), text.end(), '\n', ' ');


    // Разбиение текста на N частей
    vector<string> textParts(THREAD_COUNT);
    int partSize = text.size() / THREAD_COUNT;
    for (int i = 0; i < THREAD_COUNT; i++) {
        textParts[i] = text.substr(i * partSize, partSize);
    }
    //



    string dictionary;
    map<char, int> char_count;

    for (char c : text) {
        char_count[c]++;
    }

    int total_count = text.length(); // Хранит значения количества символов в тексте
    dictionary += to_string(total_count);
    vector<item> chars;

    int count;
    char symbol;
    map<char, int>::iterator item_ref;

    while (char_count.size())
    {
        count = 0;
        for (auto it = char_count.begin(); it != char_count.end(); ++it)
        {
            if (it->second > count)
            {
                count = it->second;
                symbol = it->first;
                item_ref = it;
            }
        }
        dictionary += symbol;
        dictionary += to_string(count);
        item new_item;
        new_item.probability = count;
        new_item.symbol = symbol;
        chars.push_back(new_item);
        char_count.erase(item_ref);

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
        cout << setprecision(10) << "символ: " << c.symbol << "  старт: " << c.start << "  финиш: " << c.finish << endl;
    }
    cout << endl;
    output << dictionary;
    output << endl;
    cout << "Dict: " << dictionary << endl;

    vector<vector<string>> parts(THREAD_COUNT); // Вектор который содержит векторы(которые принадлежат кадой части текста) с числами

    // Запуск потоков
    vector<thread> threads(THREAD_COUNT);

    for (int i = 0; i < THREAD_COUNT; i++) {

        threads[i] = thread(ProcessPart, textParts[i], ref(chars), ref(parts), i);
    }
    for (int i = 0; i < THREAD_COUNT; i++) {

        threads[i].join();
    }

    // Вывод результата в файл

    for (int i = 0; i < parts.size(); i++) {
        for (int j = 0; j < parts[i].size(); j++) {
            output << parts[i][j] << endl;
        }
    }

    SetConsoleTextAttribute(hConsole, BACKGROUND_GREEN);
    cout << "Вывод в файл выполнен успешно!";
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    cout << endl;

}