#include "stdafx.h"
#include "opencv2/stitching.hpp"
#include "common.h"

#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <opencv2/highgui/highgui.hpp>

using namespace std;

int main(int argc, char** argv)
{
	// task meta parser.
	string task_meta_path = "c:/pic/working_home/tasks/5/meta";
	task_param task_meta = parse_task_meta_file(task_meta_path);

	// template parser.
	string cola_model_path = "c:/pic/working_home/model/cola.model";
	string fanta_model_path = "c:/pic/working_home/model/fanta.model";
	string upxcd_model_path = "c:/pic/working_home/model/up_xcd.model";

	vector<model_param> model_list;
	model_list.push_back(parse_model_file(cola_model_path));
	model_list.push_back(parse_model_file(fanta_model_path));
	model_list.push_back(parse_model_file(upxcd_model_path));

	// stitching images or not.
	if (task_meta.need_stitch)
	{
		Stitcher stitcher = Stitcher::createDefault();

		vector<Img> source_img_list = task_meta.source_image_list;
		vector<Mat> source_mat_list;
		for (Img img : source_img_list) source_mat_list.push_back(img.mat);
		
		Mat pano;
		Stitcher::Status status = stitcher.stitch(source_mat_list, pano);
		imshow("pano", pano);
		imwrite("c:/pic/stitching_test/result.jpg", pano);
		if (status != Stitcher::OK)
		{
			cout << "error to stitch" << endl;
			exit(1);
		}
		task_meta.need_stitch = false;
		task_meta.source_image_list.clear();
		task_meta.source_image_list.push_back({"x:/temp/pano.jpg", pano});
	}
	// obj det for every source img.
	int file_name_index = 1;
	for (auto source_img : task_meta.source_image_list)
	{
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
			logo_count_map[model_param.model_name] = ret.score;
			sum += ret.score;
		}

		// Output to console.
		cout << "---------------------------" << endl;
		cout << "sum: " << sum << endl;
		for (auto i = logo_count_map.begin(); i != logo_count_map.end(); ++i)
		{
			cout << i->first << ": " << "count:" << i->second << endl;
		}

		//Output to file.
		cout << "---------------------------" << endl;
		string source_img_path = source_img.file_path;
		auto img_scene_name = source_img_path.substr(source_img_path.find_last_of("/") + 1);
		ofstream fout(task_meta.output_dir + "/" + to_string(file_name_index++) + ".txt");
		fout << "FILEPATH: " << source_img.file_path << endl;
		fout << "---------------------------" << endl;
		for (auto i = logo_count_map.begin(); i != logo_count_map.end(); ++i)
		{
			fout << i->first << ": " << "count:" << i->second << endl;
		}
		fout.close();
		
	}

	waitKey();
	return 0;
}
