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
#include <chrono>

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
	std::set<std::string> match_list{};
	std::chrono::duration<long double> time_span{};

	spelling_bee_information() = default;
	spelling_bee_information(const std::string& letters)
		: letters(letters)
	{}
};

spelling_bee_information solve_spelling_bee(const std::string& letters, const std::map<char, std::set<std::string>>& word_list) {
	spelling_bee_information result{ letters };

	/* get first time point */
	std::chrono::steady_clock::time_point start{ std::chrono::steady_clock::now() };

	/* only look up words containing the required letter: letter[0] */
	for (const auto& word : word_list.find(letters[0])->second) {
		if (word.length() >= 4 &&
			word.find_first_not_of(letters.c_str()) == std::string::npos &&
			word.find_first_of(letters[0]) != std::string::npos)
		{
			result.match_list.insert(word);
		}
	}

	/* get secound time point */
	std::chrono::steady_clock::time_point end{ std::chrono::steady_clock::now() };
	result.time_span = std::chrono::duration_cast<std::chrono::duration<long double>>(end - start);

	return result;
}

//spelling_bee_information solve_spelling_bee(const std::string& letters, const std::map<char, std::set<std::string>>& word_list) {
//	spelling_bee_information result{ letters };
//
//	/* get first time point */
//	std::chrono::steady_clock::time_point start{ std::chrono::steady_clock::now() };
//
//	for (const auto& ltr : letters) {
//		auto it = word_list.find(ltr);
//		if (it != word_list.end()) {
//			for (const auto& word : it->second) {
//				if (word.length() >= 4 &&
//					word.find_first_not_of(letters.c_str()) == std::string::npos &&
//					word.find_first_of(letters[0]) != std::string::npos)
//				{
//					result.match_list.insert(word);
//				}
//			}
//		}
//	}
//
//	/* get secound time point */
//	std::chrono::steady_clock::time_point end{ std::chrono::steady_clock::now() };
//	result.time_span = std::chrono::duration_cast<std::chrono::duration<long double>>(end - start);
//
//	return result;
//}

void process_batch(
	const std::vector<std::string>& batch,
	const std::map<char, std::set<std::string>>& word_list,
	std::vector<spelling_bee_information>& results,
	size_t start_index) {

	for (size_t ind{}; ind < batch.size(); ind++) {
		results[start_index + ind] = solve_spelling_bee(batch[ind], word_list);
	}
}

int main(int ac, char** av, char** ev) {
	std::map<std::string, std::string> env{};

	for (size_t ind{}; ev[ind] != nullptr; ind++) {
		std::vector<std::string> tokens{ tokenize(ev[ind], '=') };
		env[tokens[0]] = tokens[1];
	}

	std::set<std::string> combinations{};
	std::string bitmask(7, '1'); /* create seven leading one's         */
	bitmask.resize(26, '0');     /* create nineteen trailing zero's    */
	bitmask.shrink_to_fit();     /* result: 11111110000000000000000000 */

	/* gather all posible combinations of seven letter spelling bee games */
	do {
		std::string new_combination{};
		for (size_t ind{ bitmask.find_first_of('1') }; ind < bitmask.size(); ind = bitmask.find_first_of('1', ind + 1)) { new_combination += static_cast<char>('a' + ind); }
		combinations.insert(new_combination);
	} while (std::next_permutation(bitmask.begin(), bitmask.end(), std::greater<char>()));
	
	std::fstream file{ "./data/word_list.csv", std::ios_base::in };
	if (!file.is_open()) { std::cout << "Failed to open file \'./data/word_list.csv\'."; return 1; }

	/* load word list into memory */
	std::map<char, std::set<std::string>> word_list{};
	std::string line{};
	while (std::getline(file, line)) {
		std::stringstream ss{ line, std::ios_base::in };
		std::string field{};
		while (std::getline(ss, field, ',')) {
			for (const auto& ltr : field) word_list[ltr].insert(field);
		}
	}
	file.close();

	//std::vector<spelling_bee_information> spelling_bee_generations{};
	//for (const auto& combination : combinations) spelling_bee_generations.push_back(solve_spelling_bee(combination, word_list));

	/* convert set to vector for easier batch processing */
	std::vector<std::string> combinations_vec(combinations.begin(), combinations.end());
	std::vector<spelling_bee_information> spelling_bee_generations(combinations_vec.size());

	/* determine number of threads to use */
	unsigned int num_threads{ std::thread::hardware_concurrency() };
	if (num_threads == 0) num_threads = 4; /* fallback to 4 threads if detection fails */
	std::cout << "Using " << num_threads << " threads" << std::endl;

	{
		auto start_time{ std::chrono::steady_clock::now() };

		std::vector<std::thread> threads{};
		size_t batch_size{ combinations_vec.size() / num_threads };

		for (unsigned int ind{}; ind < num_threads; ++ind) {
			size_t start{ ind * batch_size };
			size_t end{ (ind == num_threads - 1) ? combinations_vec.size() : start + batch_size };

			std::vector<std::string> batch(
				combinations_vec.begin() + start,
				combinations_vec.begin() + end
			);

			threads.emplace_back(
				process_batch,
				std::move(batch),
				std::cref(word_list),
				std::ref(spelling_bee_generations),
				start
			);
		}

		/* wait for all threads to complete */
		for (auto& thread : threads) {
			thread.join();
		}

		auto end_time{ std::chrono::steady_clock::now() };
		auto duration{ std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time) };
		std::cout << "Thread method completed in " << duration.count() << " ms" << std::endl;
	}

	size_t total_matches{};
	long double total_time{};
	for (const auto& result : spelling_bee_generations) {
		total_matches += result.match_list.size();
		total_time += result.time_span.count();
	}
	std::cout << "Total matches found: " << total_matches << std::endl;
	std::cout << "Total processing time: " << total_time << " seconds" << std::endl;

	return 0;
}