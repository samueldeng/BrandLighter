#pragma once
#include <vector>
#import <c:\Program Files\Common Files\System\ado\msado15.dll> no_namespace rename("EOF", "adoEOF") rename("BOF", "adoBOF")

using namespace std;

struct output_item
{
	string brand;

	double count;
};

struct output_value
{
	string time;

	string key;

	bool success;

	string errormessage;

	vector<output_item> outputs;
};

void write_output_to_db(string connectionstring, output_value value);
