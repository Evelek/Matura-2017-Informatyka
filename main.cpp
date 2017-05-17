#include <iostream>
#include <future>
#include <thread>
#include <mutex>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <iterator>
#include <vector>
#include <utility>
using namespace std::chrono_literals; // used for this_thread::sleep_for() - C++14
using std::vector;
using std::pair;
using std::mutex;
using std::ifstream;
using std::future;
using std::cout;
using std::endl;

using mapFile = vector<vector<int>>;
using coordinate = vector<pair<int, int>>;

void contrastFirst(mutex &readyMutex, const mapFile &map, coordinate &cords, size_t row_min, size_t row_max, size_t col_min, size_t col_max, size_t add_row, size_t add_col);
void contrastSecond(mutex &readyMutex, const mapFile &map, coordinate &cords, size_t row_min, size_t row_max, size_t col_min, size_t col_max, size_t add_row, size_t add_col);
size_t longVertical(mapFile &map, size_t start, size_t end);

int main() {

	// Read data from file
	ifstream readFile;
	readFile.open("dane.txt");

	if (!readFile.is_open()) {
		cout << "There was an error while opening the file.\n";
		std::this_thread::sleep_for(2000ms);
		return -1;
	}

	// 2D vector
	mapFile map;
	for (size_t i = 0; i < 200; i++) {
		vector<int> row;
		map.push_back(row);
	}

	int temp;
	for (size_t row = 0; row < 200; ++row) {
		for (size_t col = 0; col < 320; ++col) {
			readFile >> temp;
			map[row].push_back(temp);
		}
	}

	if (readFile.fail()) {
		cout << "There was an error while reading data.\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		return -1;
	}

	readFile.close();

	//---------------------------------------------------------------------

	// Zad 6 a)
	int max = map[0][0];
	int min = map[0][0];
	for (auto &vec : map) {
		for (auto &element : vec) {
			if (element < min)
				min = element;
			if (element > max)
				max = element;
		}
	}
	cout << "max: " << max << endl;
	cout << "min: " << min << endl;

	//---------------------------------------------------------------------

	// Zad 6 b)
	int counts = 0;
	for (size_t row = 0; row < 200; ++row) {
		for (size_t col = 0; col < 160; ++col) {
			if (map[row][col] != map[row][319 - col]) {
				counts++;
				break;
			}
		}
	}
	cout << "The lowest number or rows: " << counts << endl;

	//----------------------------------------------------------------------

	// Zad 6 c)
	vector<pair<int, int>> cords;
	mutex readyMutex;
	future<void> c1 = async(std::launch::async, contrastFirst, ref(readyMutex), cref(map), ref(cords), 0, 200, 0, 319, 0, 1);
	future<void> c2 = async(std::launch::async, contrastFirst, ref(readyMutex), cref(map), ref(cords), 0, 199, 0, 320, 1, 0);
	future<void> c3 = async(std::launch::async, contrastSecond, ref(readyMutex), cref(map), ref(cords), 199, 0, 319, 1, 0, 1);
	future<void> c4 = async(std::launch::async, contrastSecond, ref(readyMutex), cref(map), ref(cords), 199, 1, 319, 0, 1, 0);

	c1.get();
	c2.get();
	c3.get();
	c4.get();

	sort(cords.begin(), cords.end());
	auto unique_iterator = unique(cords.begin(), cords.end());
	cords.erase(unique_iterator, cords.end());
	cout << "Contrastings: " << cords.size() << endl;

	//-----------------------------------------------------------------------

	// Zad 6 d)
	future<size_t> f1 = async(std::launch::async, longVertical, map, 0, 160);
	future<size_t> f2 = async(std::launch::async, longVertical, map, 160, 320);

	size_t first_result = f1.get();
	size_t second_result = f2.get();

	if (first_result < second_result)
		cout << "The longest vertical line: " << second_result << endl;
	else
		cout << "The longest vertical line: " << first_result << endl;

	std::cin.get();
}

void contrastFirst(mutex &readyMutex, const mapFile &map, coordinate &cords, size_t row_min, size_t row_max, size_t col_min, size_t col_max, size_t add_row, size_t add_col) {
	for (size_t row = row_min; row < row_max; ++row) {
		for (size_t col = col_min; col < col_max; ++col) {
			if (abs(map[row][col] - map[row + add_row][col + add_col]) > 128) {
				{
					std::unique_lock<mutex> uniqueLock(readyMutex);
					cords.push_back(std::make_pair(row, col));
					cords.push_back(std::make_pair(row + add_row, col + add_col));
					std::this_thread::yield();
				}
			}
		}
	}
}
void contrastSecond(mutex &readyMutex, const mapFile &map, coordinate &cords, size_t row_min, size_t row_max, size_t col_min, size_t col_max, size_t add_row, size_t add_col) {
	for (size_t row = row_min; row > row_max; --row) {
		for (size_t col = col_min; col > col_max; --col) {
			if (abs(map[row][col] - map[row - add_row][col - add_col]) > 128) {
				{
					std::unique_lock<mutex> uniqueLock(readyMutex);
					cords.push_back(std::make_pair(row, col));
					cords.push_back(std::make_pair(row - add_row, col - add_col));
					std::this_thread::yield();
				}
			}
		}
	}
}

size_t longVertical(mapFile &map, size_t start, size_t end) {
	vector<int> vec_col;
	size_t max_val = 0;
	size_t temp_val;
	for (size_t col = start; col < end; ++col) {
		for (size_t row = 0; row < 199; ++row) {
			if (map[row][col] == map[row + 1][col]) {
				vec_col.push_back(row);
				vec_col.push_back(row + 1);
			}
			else {
				sort(vec_col.begin(), vec_col.end());
				auto unique_iter = unique(vec_col.begin(), vec_col.end());
				vec_col.erase(unique_iter, vec_col.end());
				temp_val = vec_col.size();
				if (temp_val > max_val)
					max_val = temp_val;
				vec_col.clear();
			}
		}
	}
	return max_val;
}
