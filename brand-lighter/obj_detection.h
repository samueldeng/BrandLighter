#include <string>

using namespace std;

#define BLUE_CHANNEL 0
#define GREEN_CHANNEL 1
#define RED_CHANNEL 2
#define GREYSCALE 3

struct detect_param
{
	string obj_name;
	int rgb_channel; // 0123 -> Blue, Green, Red, Grey
	float max_match_distance;
	double epsilon;
	unsigned int min_points;
};

double detect_obj_with_score(string img_object_full_path, string img_scene_full_path, string output_dir, detect_param param, double(*score)(int, int));

double detect_obj(string img_object_full_path, string img_scene_full_path, string output_dir, detect_param param);
