//This program produces LSD tables from csvs.
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

//lhs and rhs cant be dim 0
std::vector<std::vector<double> > multi(std::vector<std::vector<double> > &lhs, std::vector<std::vector<double> > &rhs)
{
	std::vector<std::vector<double> > res(rhs.size());
	//make res with correct dim
	for (int i = 0; i < res.size(); ++i)
	{
		res[i] = std::vector<double>(lhs.size());
	}
	//do multiplication
	for (int i = 0; i < lhs.size(); ++i)
	{
		for (int j = 0; j < rhs[0].size(); ++j)
		{
			for (int k = 0; k < lhs[0].size(); ++k)
			{
				res[i][j] += lhs[i][k] * rhs[k][j];
			}
		}
	}
	return res;
}

std::vector<std::vector<double> > add(std::vector<std::vector<double> > &lhs, std::vector<std::vector<double> > &rhs)
{
	std::vector<std::vector<double> > res = lhs;
	//do multiplication
	for (int i = 0; i < lhs.size(); ++i)
	{
		for (int j = 0; j < lhs[0].size(); ++j)
		{
			res[i][j] += rhs[i][j];
		}
	}
	return res;
}
std::vector<std::vector<double> > scale(std::vector<std::vector<double> > &mat, double factor)
{
	std::vector<std::vector<double> > res = mat;
	for (int i = 0; i < res.size(); ++i)
	{
		for (int j = 0; j < res[0].size(); ++j)
		{
			res[i][j] *= factor;
		}
	}
	return res;
}

//takes inverse of matrix that is only diagonal elements
std::vector<std::vector<double> > simple_inverse(std::vector<std::vector<double> > &mat)
{
	std::vector<std::vector<double> > res = mat;
	for (int i = 0; i < res.size(); ++i)
	{
		res[i][i] = 1 / res[i][i];
	}
	return res;
}
double sqr_error(std::vector<std::vector<double> > mat)
{
	double res = 0;
	for (int i = 0; i < mat.size(); ++i)
	{
		for (int j = 0; j < mat[0].size(); ++j)
		{
			res += mat[i][j] * mat[i][j];
		}
	}
	return res;
}
std::vector<std::vector<double> > inverse(std::vector<std::vector<double> > &mat, double epsilon)
{
	std::vector<std::vector<double> > B = mat;
	//B is identity
	for (int i = 0; i < B.size(); ++i)
	{
		for (int j = 0; j < B[0].size(); ++j)
		{
			B[i][j] = 0;
		}
		B[i][i] = 1;
	}
	std::vector<std::vector<double> > res = B;
	std::vector<std::vector<double> > D = mat;
	//D is only the diagonal
	for (int i = 0; i < D.size(); ++i)
	{
		for (int j = 0; j < D[0].size(); ++j)
		{
			if (i == j)
			{
				continue;
			}
			D[i][j] = 0;
		}
	}
	//M + D must equal mat
	std::vector<std::vector<double> > M = add(mat, scale(D, -1));
	//we only want invers of D from now on
	D = simple_inverse(D);
	//negative of D to avoid calling scale ervy loop
	std::vector<std::vector<double> > nD = scale(D, -1);
	std::vector<std::vector<double> > n_identity = scale(B, -1);
	std::vector<std::vector<double> > DB = multi(D,B);
	int counter = 0;
	while (1)
	{
		++counter;
		std::vector<std::vector<double> > prev = res;
		res = add(multi(nD, multi(M, prev)), DB);
		//end condition
		if (counter >= 10)
		{
			if (sqr_error(add(multi(mat, res), n_identity)) < epsilon)
			{
				break;
			}
			counter = 0;
		}
	}
	return res;
}

std::vector<std::vector<std::string> > load_db(std::istream &f)
{
	std::vector<std::vector<std::string> > database;
	std::string csvLine;
	while (std::getline(f, csvLine)) {
		std::istringstream csvStream(csvLine);
		std::vector<std::string> csvRow;
		std::string csvCol;
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


double build_Leontief_value(std::vector<std::vector<std::string> > db, int index)
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
	//Inverse - Thanks to https://github.com/lavagater for the sweet inverse function
	ddb = inverse(ddb, 0.000000001);
	//multiply and add
	double counter = 0;
	for (int i = 0; i < mult_vec.size(); ++i)
	{
		for (int j = 0; j < mult_vec.size(); ++j)
		{
			counter += mult_vec[j] * ddb[j][i];
		}
	}
	return counter + 1;
}

std::vector<double> build_Leontief_values(std::vector<std::vector<std::string> > &db)
{
	std::vector<double> values;
	for (int i = 1; i < db.size() - 2; ++i)
	{
		values.push_back(build_Leontief_value(db, i - 1));
	}
	return values;
}

void write_LSD(std::string file_in, std::vector<std::string> &all_sectors, std::vector<double> &Leontief_values)
{
	std::ofstream file;
	std::string temp = "LSD(";
	std::string temp2 = file_in;
	temp += temp2 + ").csv";
	file.open(temp.c_str());
	file << "LSD," << "Filename:  " << file_in << "\n";
	for (int i = 0; i < all_sectors.size(); ++i)
	{
		file << all_sectors[i] << "," << Leontief_values[i] << "\n";
	}
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
	write_LSD(file_in, all_sectors, Leontief_values);
}
