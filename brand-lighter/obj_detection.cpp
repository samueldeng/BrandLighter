#include "stdafx.h"
#include "dbscan.h"
#include "common.h"

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/legacy/legacy.hpp"
#include "opencv2/legacy/compat.hpp"
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
# include <vector>


using namespace cv;
using namespace std;


void draw_cluster_on_mat(Mat base_mat, map<int, vector<Point>> category_map)
{
	cout << "---------------------------" << endl;
	for (auto iter = category_map.begin(); iter != category_map.end(); ++iter)
	{
		int cluster_id = iter->first;
		vector<Point> points_for_category = iter->second;
		cout << "[DEBUG] cluster_id = " << cluster_id << " size = " << points_for_category.size() << endl;

		/*
		* Draw Minimum Enclsoing Box or Circle.
		*/
		/*if (cluster_id >= 0)
		{

		Point2f center;
		Point2f vtx[4];
		float radius = 0;
		minEnclosingCircle(Mat(points_for_category), center, radius);
		RotatedRect box = minAreaRect(Mat(points_for_category));
		box.points(vtx);

		for (auto i = 0; i < 4; i++)
		line(img_visual_cluster, vtx[i], vtx[(i + 1) % 4], Scalar(0, 255, 0), 1, CV_AA);
		circle(img_visual_cluster, center, cvRound(radius), Scalar(0, 255, 255), 1, CV_AA);
		}
		*/

		RNG rng(cluster_id + 255);
		Scalar scla;
		if (cluster_id < 0)
		{
			scla = Scalar(255, 255, 255);
		}
		else
		{
			scla = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		}

		for (auto j = 0; j < points_for_category.size(); j++)
		{
			Point center = points_for_category[j];
			putText(base_mat, to_string(cluster_id), center, FONT_HERSHEY_COMPLEX_SMALL, 1, scla);
			circle(base_mat, center, 4, scla, 2);
		}
	}
}

struct increase_contrast_ret
{
	Mat img_object;
	Mat img_scene;
};

increase_contrast_ret increase_contrast(Mat model_img, Mat source_mat, model_param model)
{
	struct increase_contrast_ret ret;
	if (model.rgb_channel == GREYSCALE)
	{
		cvtColor(model_img, ret.img_object, CV_BGR2GRAY);
		cvtColor(source_mat, ret.img_scene, CV_BGR2GRAY);
	}
	else
	{
		Mat img_obj_color = model_img;
		Mat img_scene_color = source_mat;

		Mat img_obj_planes[3];
		split(img_obj_color, img_obj_planes);
		ret.img_object = img_obj_planes[model.rgb_channel]; //BGR Channel

		Mat img_scene_planes[3];
		split(img_scene_color, img_scene_planes);
		ret.img_scene = img_scene_planes[model.rgb_channel]; //BGR Channel	
	}
	return ret;
}

map<int, vector<Point>> cluster(vector<KeyPoint> best_point_in_scene, model_param model)
{
	map<int, vector<Point>> category_map;
	int size = best_point_in_scene.size();
	point_t points[10000];
	memset(points, 0, sizeof point_t);
	for (auto i = 0; i < best_point_in_scene.size(); i++)
	{
		points[i].x = best_point_in_scene[i].pt.x;
		points[i].y = best_point_in_scene[i].pt.y;
		points[i].z = 0;
		points[i].cluster_id = UNCLASSIFIED;
	}

	dbscan(points, size, model.epsilon, model.min_points, euclidean_dist);

	for (auto i = 0; i < size; i++)
	{
		point_t p = points[i];
		int cluster_id = p.cluster_id;
		category_map[cluster_id].push_back(Point(p.x, p.y));
	}
	return category_map;
}

double score_cluster(map<int, vector<Point>> category_map, model_param model)
{
	double object_sum = 0;
	cout << "---------------------------" << endl;
	for (auto iter = category_map.begin(); iter != category_map.end(); ++iter)
	{
		int cluster_id = iter->first;
		int cluster_size = iter->second.size();
		if (cluster_id < 0)
			continue;
		auto cluster_confidence = model.score_func(cluster_size, 0);
		object_sum += cluster_confidence;
		cout << "[DEBUG] cluster_id = " << cluster_id << "  cluster_confidence = " << cluster_confidence << endl;
	}
	cout << "---------------------------" << endl;
	cout << "detect " + model.model_name + " = " + to_string(object_sum) << endl;
	return object_sum;
}

objdet_ret detect_obj_with_score(Mat source_mat, model_param model)
{
	string model_name = model.model_name;
	Mat model_img = model.model_image;
	int rgb_channel = model.rgb_channel;
	float MAX_MATCH_DISTANCE = model.max_match_distance;
	double EPSILON = model.epsilon;
	unsigned int MINPOINTS = model.min_points;
	double (*score_func)(int, int) = model.score_func;

	increase_contrast_ret ic_ret = increase_contrast(model_img, source_mat, model);
	Mat img_object = ic_ret.img_object;
	Mat img_scene = ic_ret.img_scene;

	//Detect the keypoints using SURF Detector
	int hessian_threshold = 500;

	SurfFeatureDetector detector(hessian_threshold, 10, 5);
	//SiftFeatureDetector detector;

	vector<KeyPoint> keypoints_object;
	vector<KeyPoint> keypoints_scene;

	detector.detect(img_object, keypoints_object);
	detector.detect(img_scene, keypoints_scene);

	cout << "---------------------------" << endl;
	cout << "[DEBUG] keypoints_object.size = " + to_string(keypoints_object.size()) << endl;
	cout << "[DEBUG] keypoints_scene.size = " + to_string(keypoints_scene.size()) << endl;


	SurfDescriptorExtractor extractor;

	Mat descriptors_object, descriptors_scene;

	extractor.compute(img_object, keypoints_object, descriptors_object);
	extractor.compute(img_scene, keypoints_scene, descriptors_scene);

	Mat img_keypoint_object;
	drawKeypoints(img_object, keypoints_object, img_keypoint_object, Scalar::all(-1));

	BFMatcher matcher = BFMatcher(NORM_L2);

	vector<vector<DMatch>> matches_matrix;
	matcher.radiusMatch(descriptors_object, descriptors_scene, matches_matrix, MAX_MATCH_DISTANCE);

	Mat img_matches;
	drawMatches(img_object, keypoints_object, img_scene, keypoints_scene, matches_matrix, img_matches, Scalar::all(-1), Scalar::all(-1), vector<vector<char>>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	//	imshow("img_match", img_matches);

	vector<KeyPoint> best_point_in_scene;
	for (auto i = 0; i < matches_matrix.size(); i++)
	{
		vector<DMatch> matchForI = matches_matrix[i];
		for (auto j = 0; j < matchForI.size(); j++)
		{
			DMatch dmatch = matchForI[j];
			best_point_in_scene.push_back(keypoints_scene[dmatch.trainIdx]);
		}
	}

	map<int, vector<Point>> category_map = cluster(best_point_in_scene, model);

	objdet_ret ret;
	Mat img_visual_cluster = Mat(source_mat);
	draw_cluster_on_mat(img_visual_cluster, category_map);
	ret.visual_mat = img_visual_cluster;

	ret.score = score_cluster(category_map, model);
	return ret;
}
