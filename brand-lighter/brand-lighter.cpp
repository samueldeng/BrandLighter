#include "stdafx.h"
#include "opencv2/stitching.hpp"
#include "common.h"

#include <vector>
#include <map>
#include <iostream>
#include "json.h"

using namespace std;

int main(int argc, char* argv[])
{
	string task_meta_path = "c:/pic/working_home/tasks/6/meta";
	if (argc == 2)
	{
		task_meta_path = argv[1];
		cout << task_meta_path << endl;
	}
	// task meta parser.
	task_param task_meta = parse_task_meta_file(task_meta_path);

	// template parser.
	string cola_model_path = "c:/pic/working_home/model/cola.model";
//	string cola_back_model_path = "c:/pic/working_home/model/cola-back.model";
	string fanta_model_path = "c:/pic/working_home/model/fanta.model";
	string upxcd_model_path = "c:/pic/working_home/model/up_xcd.model";

	vector<model_param> model_list;
	model_list.push_back(parse_model_file(cola_model_path));
//	model_list.push_back(parse_model_file(cola_back_model_path));
	model_list.push_back(parse_model_file(fanta_model_path));
	model_list.push_back(parse_model_file(upxcd_model_path));

	// stitching images or not.
	if (task_meta.source_image_list.size() > 1)
	{
		Stitcher stitcher = Stitcher::createDefault();

		vector<Img> source_img_list = task_meta.source_image_list;
		vector<Mat> source_mat_list;
		for (Img img : source_img_list) source_mat_list.push_back(img.mat);
		
		Mat pano;
		Stitcher::Status status = stitcher.stitch(source_mat_list, pano);
//		imshow("pano", pano);
//		imwrite("c:/pic/stitching_test/result.jpg", pano);
		if (status != Stitcher::OK)
		{
			cout << "error to stitch" << endl;
			exit(1);
		}
		task_meta.source_image_list.clear();
		task_meta.source_image_list.push_back({"x:/temp/pano.jpg", pano});
	}

	// obj det for every source img.
	int file_name_index = 1;

	if (task_meta.source_image_list.size() != 1)
	{
		cout << "unknow error" << endl;
		exit(1);
	}

	auto source_img = task_meta.source_image_list[0];

	Json::Value root;
	root["filepath"] = source_img.file_path;

	// Object Detection Template by Tempalte.
	map<string, double> logo_count_map;
	double sum = 0;
	for (auto model_param : model_list)
	{
		//custom the score function.
		auto model_name = model_param.model_name;
		if (model_name == "up_xcd" || model_name == "up_bhc" || model_name == "vita")
			model_param.score_func = confidence_paperbox;
		else if (model_name == "sprint")
			model_param.score_func = confidence_sprint;
		else
			model_param.score_func = confidence_common_can;

		objdet_ret ret = detect_obj_with_score(source_img.mat, model_param);

		root["object-detection"][model_name]["score"] = ret.score;
		vector<string> plist;
		for (auto p : ret.cluster_center_list)
		{
			root["object-detection"][model_name]["point"].append(to_string(p.x) + "," + to_string(p.y));
		}

		//save intermediate image.
//		imwrite("c:/pic/working_home/temp/"+ to_string(sum) + ".jpg", ret.visual_mat);
		logo_count_map[model_param.model_name] = ret.score;
		sum += ret.score;
	}

	// Output to console.
	cout << root << endl;


	// output to db.



	getchar();
	return 0;
}
