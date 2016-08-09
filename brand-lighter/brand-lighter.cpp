#include "stdafx.h"
#include "obj_detection.h"
#include "score.h"

#include <vector>
#include <map>
#include <iostream>
#include <fstream>


using namespace std;

int main(int argc, char** argv)
{
	// Output Directory
	string output_dir = "c:/pic/result_new/";
	// Scene Picture Full Path
	string img_scene_full_path = "c:/pic/scene_mixed_logo/Front_10.jpg";

	// All the temaplate.
	map<string, detect_param> tempalte_map;

	//	string coal_rot = "c:/pic/template/Red_Can_Rot4.jpg";
	//	detect_param cola_rot_param = { "cola_rot", BLUE_CHANNEL, 0.18, 50, 2 };
	//	tempalte_map[coal_rot] = cola_rot_param;

	string coal = "c:/pic/template/Red_Can_2.jpg";
	detect_param cola_param = { "cola", BLUE_CHANNEL, 0.22, 50, 2 };
	tempalte_map[coal] = cola_param;

	string sprint = "c:/pic/template/Sprint_1.jpg";
	detect_param sprint_param = { "sprint", RED_CHANNEL, 0.25, 50, 2 };
	tempalte_map[sprint] = sprint_param;

	string suntory = "c:/pic/template/HiDPI_Suntory_2.jpg";
	detect_param suntory_param = { "suntory", GREYSCALE, 0.22, 50, 2 };
	tempalte_map[suntory] = suntory_param;

	string vita = "c:/pic/template/HiDPI_Vita_2.jpg";
	detect_param vita_param = { "vita", GREYSCALE, 0.22, 50, 2 };
	tempalte_map[vita] = vita_param;

	string up_xcd = "c:/pic/template/HiDPI_UP_Xcd_2.jpg";
	detect_param up_xcd_param = { "up_xcd", RED_CHANNEL, 0.22, 50, 2 };
	tempalte_map[up_xcd] = up_xcd_param;

	string fenda = "c:/pic/template/Fanta_1.jpg";
	detect_param fenda_param = { "fanta", RED_CHANNEL, 0.22, 50, 2 };
	tempalte_map[fenda] = fenda_param;

	string up_bhc = "c:/pic/template/HiDPI_UP_Bhc_2.jpg";
	detect_param up_bhc_param = { "up_bhc", GREYSCALE, 0.22, 50, 2 };
	tempalte_map[up_bhc] = up_bhc_param;

	// Object Detection Template by Tempalte.
	map<string, double> logo_count_map;
	double sum = 0;
	for (auto i = tempalte_map.begin(); i != tempalte_map.end(); ++i)
	{
		string img_object_full_path = i->first;

		double(*score_func)(int, int);
		detect_param param = i->second;
		if (param.obj_name == "up_xcd" || param.obj_name == "up_bhc" || param.obj_name == "vita")
			score_func = confidence_paperbox;
		else if (param.obj_name == "sprint")
			score_func = confidence_sprint;
		else
			score_func = confidence_common_can;
		double count = detect_obj_with_score(img_object_full_path, img_scene_full_path, output_dir, param, score_func);
		logo_count_map[param.obj_name] = count;
		sum += count;
	}

	// Output to console.
	cout << "---------------------------" << endl;
	cout << "sum: " << sum << endl;
	for (auto i = logo_count_map.begin(); i != logo_count_map.end(); ++i)
	{
		cout << i->first << ": " << "count:" << i->second << " percent:" << i->second / sum * 100 << "%" << endl;
	}

	//Output to file.
	cout << "---------------------------" << endl;
	auto img_scene_name = img_scene_full_path.substr(img_scene_full_path.find_last_of("/") + 1);
	ofstream fout("c:/pic/result_new/" + img_scene_name + ".txt");
	for (auto i = logo_count_map.begin(); i != logo_count_map.end(); ++i)
	{
		fout << i->first << ": " << "count:" << i->second << " percent:" << i->second / sum * 100 << "%" << endl;
	}

	cin.get();
	return 0;
}
