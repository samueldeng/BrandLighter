#include "stdafx.h"

double confidence_seg_p2(int cluster_size, int total_keypoints)
{
	//segment function;
	if (cluster_size < 3)
		return double(0);
	if (cluster_size >= 0.8 * total_keypoints)
		return double(1);

	// y = ax^2+bx;
	double a = double(25) / (16 * total_keypoints * total_keypoints - 60 * total_keypoints);
	double b = double(-75) / (16 * total_keypoints * total_keypoints - 60 * total_keypoints);
	return a * cluster_size * cluster_size + b * cluster_size;
}

double confidence_common_can(int cluster_size, int total)
{
	if (cluster_size < 3)
		return double(0);
	if (cluster_size == 3)
		return double(0.3);
	if (cluster_size == 4)
		return double(0.4);
	if (cluster_size == 5)
		return double(0.5);
	if (cluster_size == 6)
		return double(0.6);
	if (cluster_size == 7)
		return double(0.7);
	if (cluster_size == 8)
		return double(0.8);
	if (cluster_size >= 9)
		return double(1);
	return 0;
}

double confidence_paperbox(int cluster_size, int total)
{
	if (cluster_size < 10)
		return double(0);
	if (cluster_size >= 10)
		return double(1);
	return 0;
}

double confidence_sprint(int cluster_size, int total)
{
	if (cluster_size < 3)
		return double(0);
	if (cluster_size == 3)
		return double(0.5);
	if (cluster_size == 4)
		return double(0.8);
	if (cluster_size >= 5)
		return double(1);
	return 0;
}

