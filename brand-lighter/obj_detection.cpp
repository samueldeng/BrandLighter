#include "stdafx.h"
#include "dbscan.h"
#include "score.h"
#include "obj_detection.h"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
# include "opencv2/objdetect/objdetect.hpp"
# include "opencv2/features2d/features2d.hpp"
# include "opencv2/calib3d/calib3d.hpp"
# include "opencv2/nonfree/nonfree.hpp"
# include "opencv2/legacy/legacy.hpp"
# include "opencv2/legacy/compat.hpp"
#include "opencv2/opencv_modules.hpp"
#include "opencv2/nonfree/nonfree.hpp"

#include <iostream>
# include <vector>

using namespace cv;
using namespace std;


double detect_obj_with_score(string img_object_full_path, string img_scene_full_path, string output_dir, detect_param param, double(*score_func)(int, int))
{
	int rgb_channel = param.rgb_channel;
	float MAX_MATCH_DISTANCE = param.max_match_distance;
	double EPSILON = param.epsilon;
	unsigned int MINPOINTS = param.min_points;

	Mat img_object;
	Mat img_scene;
	if (rgb_channel == 3)
	{
		img_object = imread(img_object_full_path, CV_LOAD_IMAGE_GRAYSCALE);
		img_scene = imread(img_scene_full_path, CV_LOAD_IMAGE_GRAYSCALE);
		if (!img_object.data || !img_scene.data)
		{
			cout << " --(!) Error reading images " << endl;
			return -1;
		}
	}
	else
	{
		Mat img_obj_color = imread(img_object_full_path);
		Mat img_scene_color = imread(img_scene_full_path);
		if (!img_obj_color.data || !img_scene_color.data)
		{
			cout << " --(!) Error reading images " << endl;
			return -1;
		}

		Mat img_obj_planes[3];
		split(img_obj_color, img_obj_planes);
		img_object = img_obj_planes[rgb_channel]; //BGR Channel

		Mat img_scene_planes[3];
		split(img_scene_color, img_scene_planes);
		img_scene = img_scene_planes[rgb_channel]; //BGR Channel	
	}
	//	imshow("img_scene", img_scene);


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
	//	imshow("img_keypoint_object", img_keypoint_object);

	BFMatcher matcher = BFMatcher(NORM_L2);

	vector<vector<DMatch>> matches_matrix;
	matcher.radiusMatch(descriptors_object, descriptors_scene, matches_matrix, MAX_MATCH_DISTANCE);

	Mat img_matches;
	drawMatches(img_object, keypoints_object, img_scene, keypoints_scene, matches_matrix, img_matches, Scalar::all(-1), Scalar::all(-1), vector<vector<char>>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	//	imshow("img_match", img_matches);
	//	imwrite("c:/pic/feature_match_cluster/" + img_obj_name + "-" + img_scene_name + "-" + to_string(max_match_distance) + "-match.jpg", img_matches);

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

	dbscan(points, size, EPSILON, MINPOINTS, euclidean_dist);

	map<int, vector<Point>> category_map;
	for (auto i = 0; i < size; i++)
	{
		point_t p = points[i];
		int cluster_id = p.cluster_id;
		category_map[cluster_id].push_back(Point(p.x, p.y));
	}

	cout << "---------------------------" << endl;
	Mat img_visual_cluster = Mat(imread(img_scene_full_path));
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
			putText(img_visual_cluster, to_string(cluster_id), center, FONT_HERSHEY_COMPLEX_SMALL, 1, scla);
			circle(img_visual_cluster, center, 4, scla, 2);
		}
	}
//	imshow("img_visual_cluster", img_visual_cluster);

	auto img_obj_name = img_object_full_path.substr(img_object_full_path.find_last_of("/") + 1);
	auto img_scene_name = img_scene_full_path.substr(img_scene_full_path.find_last_of("/") + 1);
//	imwrite(output_dir + img_obj_name + "-" + img_scene_name + "-" + to_string(MAX_MATCH_DISTANCE) + "-" + to_string(EPSILON) + "-" + to_string(MINPOINTS) + "-cluster.jpg", img_visual_cluster);

	double cola_sum = 0;
	cout << "---------------------------" << endl;
	for (auto iter = category_map.begin(); iter != category_map.end(); ++iter)
	{
		int cluster_id = iter->first;
		int cluster_size = iter->second.size();
		if (cluster_id < 0)
			continue;
		auto cluster_confidence = score_func(cluster_size, keypoints_object.size());
		cola_sum += cluster_confidence;
		cout << "[DEBUG] cluster_id = " << cluster_id << "  cluster_confidence = " << cluster_confidence << endl;
	}
	cout << "---------------------------" << endl;
	cout << "detect " + param.obj_name + " = " + to_string(cola_sum) << endl;

	waitKey(0);
	return cola_sum;
}
