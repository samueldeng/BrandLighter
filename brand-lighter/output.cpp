#include "stdafx.h"
#include "output.h"
#include <iostream>
#include <atlstr.h> 
#include <vector>
#include <fstream>

using namespace std;

void write_output_to_db(string connectionstring, output_value value)
{
	CoInitialize(NULL);

	_ConnectionPtr sqlSp;

	HRESULT hr = sqlSp.CreateInstance(_uuidof(Connection));

	if (FAILED(hr))
	{
		cout << "initialize connection failure!" << endl;

		return;
	}

	try
	{
		_bstr_t strConnect = _bstr_t(connectionstring.c_str());

		sqlSp->Open(strConnect, "", "", adModeUnknown);
	}
	catch (_com_error &e)
	{
		cout << e.Description() << endl;
	}

	try
	{
		char* update = new char[1000];

		memset(update, 0, sizeof(update));

		strcat(update, "update coka_od_image set detected = 1, detecttime = '");

		strcat(update, value.time.c_str());

		strcat(update, "', detectedresult = ");

		char detectedresult[10];

		sprintf_s(detectedresult, sizeof(detectedresult), "%d", (value.success ? 1 : 0));

		strcat(update, detectedresult);

		strcat(update, ", detectederrormessage = '");

		strcat(update, value.errormessage.c_str());

		strcat(update, "' where odKey = '");

		strcat(update, value.key.c_str());

		strcat(update, "'");

		cout << update << endl;

		sqlSp->Execute(_bstr_t(update), NULL, adCmdText);

		delete[]update;
	}
	catch (_com_error &e)
	{
		cout << e.Description() << endl;
	}

	if (value.outputs.size() <= 0)
	{
		return;
	}

	try
	{
		char* insert = new char[4000];

		memset(insert, 0, sizeof(insert));

		strcat(insert, "insert into coka_od_output(odkey, odtype, odvalue) ");

		for (size_t i = 0; i < value.outputs.size(); i++)
		{
			output_item item = value.outputs[i];

			if (i != 0)
			{
				strcat(insert, " union ull ");
			}

			strcat(insert, "select '");

			strcat(insert, value.key.c_str());

			strcat(insert, "','");

			strcat(insert, item.brand.c_str());

			strcat(insert, "',");

			char count[10];

			sprintf_s(count, sizeof(count), "%.2f", item.count);

			strcat(insert, count);
		}

		cout << insert << endl;

		sqlSp->Execute(_bstr_t(insert), NULL, adCmdText);

		delete[] insert;
	}
	catch (_com_error &e)
	{
		cout << e.Description() << endl;
	}
}