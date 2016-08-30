#include <vector>
#include <opencv2/core/core.hpp>
using namespace std;
using namespace cv;

#define BLUE_CHANNEL 0
#define GREEN_CHANNEL 1
#define RED_CHANNEL 2
#define GREYSCALE 3

struct Img
{
	string file_path;
	Mat mat;
};

struct model_param
{
	string model_name;
	Mat model_image;
	int rgb_channel;
	float max_match_distance;
	double epsilon;
	unsigned int min_points;
	double (*score_func)(int, int);
};

struct task_param
{
	string output_dir;
	bool need_stitch;
	vector<Img> source_image_list;
};

struct objdet_ret
{
	double score;
	Mat visual_mat;
	vector<Point> cluster_center_list;
};

model_param parse_model_file(string model_file_path);
task_param parse_task_meta_file(string task_meta_file_path);

double confidence_seg_p2(int cluster_size, int total_keypoints);
double confidence_common_can(int cluster_size, int total);
double confidence_paperbox(int cluster_size, int total);
double confidence_sprint(int cluster_size, int total);

objdet_ret detect_obj_with_score(Mat source_mat, model_param model);
