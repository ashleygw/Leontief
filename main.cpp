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

double determinant(int n, std::vector<std::vector<double> > mat, double d = 0)
{
	int c, subi, i, j, subj;
	std::vector<std::vector<double> > submat(mat.size(), std::vector<double>(mat.size(), 1));
	if (n == 2)
	{
		return((mat[0][0] * mat[1][1]) - (mat[1][0] * mat[0][1]));
	}
	else
	{
		for (c = 0; c < n; c++)
		{
			subi = 0;
			for (i = 1; i < n; i++)
			{
				subj = 0;
				for (j = 0; j < n; j++)
				{
					if (j == c)
					{
						continue;
					}
					submat[subi][subj] = mat[i][j];
					subj++;
				}
				subi++;
			}
			d = d + (pow(-1, c) * mat[0][c] * determinant(n - 1, submat, d));
		}
	}
	return d;
}

double build_Leontief_value(std::vector<std::vector<std::string> > db, int index)
{
	std::vector<std::vector<double> > ddb(db.size() - 3, std::vector<double>(db.size() - 3,1));
	for (int i = 0; i < db.size() - 3; ++i)
	{
		double Xj = stod(db[db.size() - 1][i+1]);
		//Divide columns by XJ
		for (int j = 0; j < db.size() - 3; ++j)
		{
			ddb[i][j] = stod(db[i+1][j+1]) / Xj;
		}
	}
	//Subtract Identity Matrix
	for (int i = 0; i < db.size(); ++i)
	{
		for (int j = 0; j < db.size(); ++j)
		{
			ddb[i][j] = (i == j) ? 1 - ddb[i][j] : 0 - ddb[i][j];
		}
	}
	//Inverse
	double dt = determinant(ddb.size(), ddb);

	return 5.5;
}

std::vector<double> build_Leontief_values(std::vector<std::vector<std::string> > &db)
{
	std::vector<double> values;
	for (int i = 1; i < db.size() - 2; ++i)
	{
		values.push_back(build_Leontief_value(db, i));
	}
	return values;
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
	std::vector<double> Leontief_values = build_Leontief_values(db);
}

