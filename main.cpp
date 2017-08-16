//This program produces LSD tables from csvs.
//Eigen is used for some of the algebra
//Created by George Ashley
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include "Eigen/Dense"
#include <numeric>
std::vector<std::vector<std::string> > load_db(std::istream &f)
{
	std::vector<std::vector<std::string> > database;
	std::string csvLine;
	while (std::getline(f, csvLine)) {
		bool is_empty = true;
		std::istringstream csvStream(csvLine);
		std::vector<std::string> csvRow;
		std::string csvCol;
		while (std::getline(csvStream, csvCol, ','))
			csvRow.push_back(csvCol);
		//Check if whole row is empty
		for (int i = 0; i < csvRow.size(); ++i)
		{
			if (!csvRow[i].empty())
				is_empty = false;
		}
		if(!is_empty)
			database.push_back(csvRow);
	}
	return database;
}
std::vector<std::string> load_all_sectors(std::vector<std::vector<std::string> > &db)
{
	std::vector<std::string> sectors;
	for (int i = 1; (i < db[0].size() - 2) && db[0][i].size(); ++i)
	{
		sectors.push_back(db[0][i]);
	}
	return sectors;
}
double build_Leontief_value(std::vector<std::vector<std::string> > &db, int index)
{
	//Size is reduced and data is converted from string to double
	std::vector<std::vector<double> > ddb(db.size() - 3, std::vector<double>(db.size() - 3,1));
	std::vector<double> mult_vec;
	for (int i = 0; i < db.size() - 3; ++i)
	{
		double Xj = stod(db[db.size() - 1][i+1]);
		//Divide columns by XJ
		for (int j = 0; j < db.size() - 3; ++j)
		{
			ddb[i][j] = stod(db[j+1][i+1]) / Xj;
		}
	}
	//Store the cut matrix for multiplication
	for (int i = 0; i < ddb.size(); ++i)
	{
		if (i != index)
		{
			mult_vec.push_back(ddb[index][i]);
		}
	}
	//Store cut matrix in Eigen vector for multiplication of inverse
	std::vector<double> z = mult_vec;
	double* point = &z[0];
	Eigen::Map<Eigen::VectorXd> mult_vector(point, mult_vec.size());
	//Erase the unused vectors
	for (int i = 0; i < ddb.size(); ++i)
	{
		ddb[i].erase(ddb[i].begin() + index);
	}
	ddb.erase(ddb.begin() + index);
	//Subtract Identity Matrix
	for (int i = 0; i < ddb.size(); ++i)
	{
		for (int j = 0; j < ddb.size(); ++j)
		{
			ddb[i][j] = 0 - ddb[i][j];
		}
		ddb[i][i] += 1;
	}
	//New Eigen matrix for faster inverting and multiplication
	Eigen::MatrixXd A(ddb.size(),ddb.size());
	for (int i = 0; i < ddb.size(); ++i)
	{
		std::vector<double> v = ddb[i];
		double* ptr = &v[0];
		Eigen::Map<Eigen::VectorXd> my_vect(ptr, ddb.size());
		A.col(i) = my_vect;
	}
	A = A.inverse();
	return (A*mult_vector).sum() + 1;
}
std::vector<double> build_Leontief_values(std::vector<std::vector<std::string> > &db)
{
	std::vector<double> values;
	for (int i = 1; i < db[0].size() - 2; ++i)
	{
		values.push_back(build_Leontief_value(db, i - 1));
	}
	return values;
}
double round(double in, double precision)
{
	return (int)(in / precision) * precision;
}
void write_LSD(std::string file_in, std::vector<std::string> &all_sectors, std::vector<double> &Leontief_values, std::vector<double> &LI_values)
{
	std::ofstream file;
	std::string temp = "LSD(";
	std::string temp2 = file_in;
	temp += temp2 + ").csv";
	file.open(temp.c_str());
	file << "LSD," << "Filename:  " << file_in << "\n\n";
	file << ",Leontief,XLI\n";
	for (int i = 0; i < all_sectors.size(); ++i)
	{
		file << all_sectors[i] << "," << round(Leontief_values[i],.0001) << ","<< round(LI_values[i],0.01) << "\n";
	}
}
std::vector<double> build_LI_values(std::vector<double> Leontief_values)
{
	int k = Leontief_values.size();
	double sum = std::accumulate(Leontief_values.begin(), Leontief_values.end(), 0.0);
	std::vector<double> ret(k);
	for (int i = 0; i < k; ++i)
	{
		ret[i] = Leontief_values[i]/(sum/k);
	}
	return ret;
}
int main(int argc, char* argv[])
{
	std::fstream file;
	std::string file_in;
	if (argc > 1) {
		file.open(argv[1]);
		file_in = argv[1];
		if (!file.is_open()) {
			std::cout << "File not found!\n";
			std::getchar();
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
			std::getchar();
			return 1;
		}
	}
	std::cout << "Loading database...\n";
	std::vector<std::vector<std::string> > db = load_db(file);
	std::cout << "Complete!\n";
	std::cout << "Loading Sector names...\n";
	std::vector<std::string> all_sectors = load_all_sectors(db);
	std::cout << "Complete!\n";
	std::cout << "Building Leontief values...\n";
	std::vector<double> Leontief_values = build_Leontief_values(db);
	std::vector<double> LI_values = build_LI_values(Leontief_values);
	std::cout << "Complete!\n";
	std::cout << "Writing to file...\n";
	write_LSD(file_in, all_sectors, Leontief_values, LI_values);
	std::cout << "Finished.";
}
