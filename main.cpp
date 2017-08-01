//This program produces LSD tables from csvs.
//Created by George Ashley
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

std::vector<std::vector<std::string> > load_db(std::istream &f)
{
	std::vector<std::vector<std::string> > database;
	std::string csvLine;
	// read every line from the stream
	while (std::getline(f, csvLine)) {
		std::istringstream csvStream(csvLine);
		std::vector<std::string> csvRow;
		std::string csvCol;
		// read every element from the line that is seperated by commas
		// and put it into the vector or strings
		while (std::getline(csvStream, csvCol, ','))
			csvRow.push_back(csvCol);
		database.push_back(csvRow);
	}
	return database;
}
std::vector<std::string> load_all_sectors(std::vector<std::vector<std::string> > &db)
{
	std::vector<std::string> sectors;
	for (int i = 1; i < db.size() - 2; ++i)
	{
		sectors.push_back(db[0][i]);
	}
	return sectors;
}
int main(int argc, char* argv[])
{
	std::fstream file;
	std::string file_in;
	if (argc > 1) {
		file.open(argv[1]);
		if (!file.is_open()) {
			std::cout << "File not found!\n";
			return 1;
		}

	}
	else
	{
		std::cout << "Enter input filename: " << std::endl;
		std::cin >> file_in;
		file.open(file_in);
		if (!file.is_open()) {
			std::cout << "File not found!\n";
			return 1;
		}
	}
	std::vector<std::vector<std::string> > db = load_db(file);
	std::vector<std::string> all_sectors = load_all_sectors(db);
}

