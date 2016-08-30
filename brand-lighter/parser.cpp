#include "stdafx.h"
#include "common.h"
#include <string>
#include <fstream>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace std;

model_param parse_model_file(string model_file_path)
{
	model_param model;
	ifstream fin(model_file_path);

	if (!fin.eof()) getline(fin, model.model_name);

	string model_path;
	if (!fin.eof()) getline(fin, model_path);

	Mat mat = imread(model_path);
	if (mat.empty())
	{
		cout << "fail to read model path" << model_path << endl;
		exit(1);
	}
	model.model_image = mat;

	string chan;
	if (!fin.eof()) getline(fin, chan);
	model.rgb_channel = stoi(chan);

	string mmd;
	if (!fin.eof()) getline(fin, mmd);
	model.max_match_distance = stod(mmd);

	string epsi;
	if (!fin.eof()) getline(fin, epsi);
	model.epsilon = stoi(epsi);

	string minp;
	if (!fin.eof()) getline(fin, minp);
	model.min_points = stoi(minp);

	fin.close();
	return model;
}

task_param parse_task_meta_file(string task_meta_file_path)
{
	task_param task_par;
	ifstream fin(task_meta_file_path);
	task_par.db_key = "";
	task_par.connection_string = "";

	if (!fin.eof()) getline(fin, task_par.db_key);
	if (!fin.eof()) getline(fin, task_par.connection_string);

	string source_image_path;
	while (!fin.eof() && getline(fin, source_image_path))
	{
		Mat mat = imread(source_image_path);
		if (!mat.empty())
		{
			Img img = {source_image_path, mat};
			task_par.source_image_list.push_back(img);
		}
		else
		{
			cout << "error to read " << source_image_path << endl;
		}
	}
	fin.close();
	return task_par;
}
