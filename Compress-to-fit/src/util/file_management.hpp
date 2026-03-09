#pragma once

#include <fstream>
#include <filesystem>
#include "utils.hpp"

namespace fs = std::filesystem;


namespace file
{

	/*
	Apply a certain function of type 'fun' to the bytes read in from file 'in_path' and written to 'out_path'.
	The type 'fun' shall take a pointer to the data and an integer for the size of the data, the return of 'fun' is discarded.
	This will not check if a path is accessible or existant, or any other form of checking.
	*/
	template <ptr_size_fun fun>
	void process_file(const fs::path& in_path, const fs::path& out_path, fun op);

	/*
	Given a file, split it into multiple files according to documentation.

	*/
	void split_file(const fs::path& path_, size_t portions = 1);

}//namespace file
