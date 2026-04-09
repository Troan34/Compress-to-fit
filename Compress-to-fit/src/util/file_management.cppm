export module file_util;

import util;
import std;
namespace fs = std::filesystem;

namespace file
{

	/*
	Apply a certain function of type 'fun' to the bytes read in from file 'in_path' and written to 'out_path'.
	The type 'fun' shall take a pointer to the data and an integer for the size of the data, the return of 'fun' is discarded.
	This will not check if a path is accessible or existant, or any other form of checking.
	*/
	export template <ptr_size_pred fun>
	void process_file(const fs::path& in_path, const fs::path& out_path, fun op);

	/*
	Given a path, the file will be created there.
	If the file already exists and the file signature is correct (if not: throw), append new data to the end of the file.
	*/
	export template <typename T>
	void write_file(std::span<T> buffer, const fs::path& out_path);

	/*
	Given a file, split it into multiple files according to documentation.
	*/
	export void split_file(const fs::path& path_, size_t portions);

	//Check the signature of the file at 'path', throws if check fails (and if it doesn't exist)
	void check_signature(const fs::path& path);

}//namespace file
