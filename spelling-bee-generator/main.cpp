#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <functional>
#include <sstream>
#include <mutex>
#include <thread>
#include <atomic>
#include <future>
#include <unordered_map>
#include <unordered_set>
#include <chrono>

#include "dump.h"

static std::vector<std::string> tokenize(const std::string& str, const char& dlm) {
	std::vector<std::string> tokens{};
	for (size_t ind{}, offset{}; offset < str.size(); ind = offset + 1) {
		offset = str.find_first_of(dlm, ind);
		tokens.push_back(str.substr(ind, offset));
	}
	return tokens;
}

struct spelling_bee_information {
	std::string letters{};
	std::unordered_set<std::string> match_list{};
	std::chrono::nanoseconds time_span{};

	spelling_bee_information() = default;
	spelling_bee_information(const std::string& letters)
		: letters(letters)
	{}
};

spelling_bee_information solve_spelling_bee(const std::string& letters, const std::unordered_map<std::string, std::unordered_set<std::string>>& word_list, const std::unordered_map<char, std::unordered_set<std::string>>& letter_list) {
	spelling_bee_information result{ letters };

	/* get first time point */
	std::chrono::steady_clock::time_point start{ std::chrono::steady_clock::now() };

	/* only look up words containing the required letter: letter[0] */
	for (const auto& unique_letters : letter_list.find(letters[0])->second) {
		if (unique_letters.find_first_not_of(letters.c_str()) == std::string::npos) {
			for (const auto& word : word_list.find(unique_letters)->second) result.match_list.insert(word);
		}
	}

	/* get secound time point */
	std::chrono::steady_clock::time_point end{ std::chrono::steady_clock::now() };
	result.time_span = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

	return result;
}

void process_batch(const std::vector<std::string>& batch, const std::unordered_map<std::string, std::unordered_set<std::string>>& word_list, const std::unordered_map<char, std::unordered_set<std::string>>& letter_list, std::vector<spelling_bee_information>& results, size_t start_index) {

	for (size_t ind{}; ind < batch.size(); ind++) {
		results[start_index + ind] = solve_spelling_bee(batch[ind], word_list, letter_list);
	}
}

static std::string get_unique_letters(const std::string& word) {
	std::set<char> unique_chars(word.begin(), word.end());
	return std::string(unique_chars.begin(), unique_chars.end());
}

/*
	loads a csv file into memory; will return ',' on failure
*/
static std::vector<std::string> load_csv(const std::filesystem::path& path) {
	std::vector<std::string> csv{};

	/* open file */
	std::fstream file{ path, std::ios_base::in };
	if (!file.is_open()) return { ",", "failed to open file" };

	/* parse file */
	for (std::string token{}; std::getline(file, token, ',');) { csv.push_back(token); }
	file.close();

	return csv;
}

/*
*	p is the permutation
*	n is the size of the permutation
*/
static inline bool next_permutation(uint64_t& p, const uint64_t& n) {
	uint64_t next_bit{ p & (~p + 1) };
	uint64_t result{ p + next_bit };

	// overflow or exceeded n bits -> no next permutation
	if (result == 0 || result >= (1ULL << n))
		return false;

	uint64_t ones{ p ^ result };      // bits that changed
	ones = (ones >> 2) / next_bit;    // reposition trailing ones

	p = result | ones;
	return true;
}

static inline bool next_permutation(uint32_t& p, const uint32_t& n) {
	uint32_t lowest_bit{ p & (~p + 1) };
	uint32_t result{ p + lowest_bit };

	// overflow or exceeded n bits -> no next permutation
	if (result == 0 || result >= (1ULL << n))
		return false;

	uint32_t ones{ p ^ result };     // bits that changed
	ones = (ones >> 2) / lowest_bit; // reposition trailing ones

	p = result | ones;
	return true;
}

int main(/*int ac, char** av, char** ev*/) {
	//std::unordered_map<std::string, std::string> env{};

	//for (size_t ind{}; ev[ind] != nullptr; ind++) {
	//	std::vector<std::string> tokens{ tokenize(ev[ind], '=') };
	//	env[tokens[0]] = tokens[1];
	//}

	std::unordered_set<uint32_t> combinations{};
	uint32_t bitmask{ 0x00000007F }; /* 0000 0000 0000 0000 0000 0000 0111 1111 */
	std::cout << "bitmask : " << utl::dump_bin(bitmask) << "\n";

	do {
		//std::cout << "Bin : " << utl::dump_bin(bitmask) << "\n";
		combinations.insert(bitmask);
	} while (next_permutation(bitmask, 26));
	std::cout << "bitmask : " << utl::dump_bin(bitmask) << "\n";

	auto csv{ load_csv("./data/word_list.csv") };
	if (csv[0] == ",") { return 1; }

	return 0;
}