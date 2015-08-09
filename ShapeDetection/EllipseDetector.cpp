#include "EllipseDetector.h"
#include "IShapeDetector.h"
std::vector<cv::RotatedRect> EllipseDetector::excute(cv::Mat src) 
{
	/*
	 * ͼ���Ԥ����
	 */
	cv::Mat prepocessed_src = this->prepocess(src);
	/*
	 * ��ȡͼ��ı�Ե
	 */
	cv::Mat edge = this->getEdge(prepocessed_src);
	/*
	 * ����ȡ��Բ
	 */
	std::vector<cv::RotatedRect> raw_ellipses = this->getRawEllipses(edge);
	/*
	 * �Դ���ȡ����Բ����У��
	 */
	std::vector<cv::RotatedRect> final_ellipses = this->tuneRawEliipses(raw_ellipses, src);
	/*
	 * ���ؽ��
	 */
	return final_ellipses;
}

cv::Mat EllipseDetector::prepocess(cv::Mat src) 
{
	cv::Mat img_smooth;
	//double kernel_length = 25.0;
	//cv::bilateralFilter(src, img_smooth, 5,kernel_length*2,kernel_length/2);
	cv::GaussianBlur(src, img_smooth, cv::Size(5,5), 0);
	return img_smooth;
}

cv::Mat EllipseDetector::getEdge(cv::Mat prepocessed_src) 
{
	/*-----------------�Ƚ���Ҫ�Ĳ���---------------*/
	double grad_thresh = 35;
	/*-------------------------------------------*/
	// ����ͼ�����������ݶ�
	cv::Mat grad_x, grad_y, grad;
	cv::Sobel(prepocessed_src, grad_x, prepocessed_src.depth(), 1, 0);
	cv::Sobel(prepocessed_src, grad_y, prepocessed_src.depth(), 0, 1);
	// �޳�һЩ�����ݶȵ㣬��ѧ��̬ѧ��������
	grad_x = (grad_x > grad_thresh) * 255;
	grad_y = (grad_y > grad_thresh) * 255;
	cv::Mat se(5,5,CV_8U,cv::Scalar(1));
	grad   = cv::abs(grad_x) + cv::abs(grad_y);
	morphologyEx(grad,grad,cv::MORPH_CLOSE,se);
	// ���ô�򷨼�����ֵ������canny��ȡ�߽�
	cv::Mat edge;
	double high_threshold = cv::threshold(prepocessed_src,cv::Mat(), 0, 255, cv::THRESH_OTSU);
	cv::Canny(prepocessed_src, edge, 0.5 * high_threshold, high_threshold, 7);
	edge = edge.mul(grad);
	//cv::imshow("grad", grad);
	//cv::imshow("edge", edge);
	return edge;
}

std::vector<cv::RotatedRect> EllipseDetector::getRawEllipses(cv::Mat edge)
{
	/*-----------------�Ƚ���Ҫ�Ĳ���---------------*/
	unsigned int min_contour_size = 200;
	double center_threshold = 5.0;
	double distance_threshold = 10.0;
	/*-------------------------------------------*/
	std::vector<std::vector<cv::Point> > contours;                          
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(edge, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
	std::vector<cv::RotatedRect> boxes;
	cv::Point2f mean_center(0.0,0.0);
	for (unsigned int i=0; i < contours.size(); ++i)
	{
		if (contours[i].size() > min_contour_size) {
			cv::Mat pointsf;
			cv::Mat(contours[i]).convertTo(pointsf, CV_32F);
			cv::RotatedRect box = fitEllipse(pointsf);
			if( MAX(box.size.width, box.size.height) > MIN(box.size.width, box.size.height)*30 )
				continue;
			mean_center += box.center;
			boxes.push_back(box);
		} 
	}
	mean_center.x /= (float) boxes.size();
	mean_center.y /= (float) boxes.size();
	std::vector<cv::RotatedRect> raw_ellipses;
	/*
	 ** ��������һ������,ʵ����Ҫ��һ��k-means���������͵͵��
	 */
	for (unsigned int i = 0; i < boxes.size(); ++i) 
	{
		float x_diff = this->diff(
			boxes[i].center.x,
			mean_center.x
		);
		float y_diff = this->diff(
			boxes[i].center.y,
			mean_center.y
		);
		if ( x_diff < center_threshold && y_diff < center_threshold) 
			raw_ellipses.push_back(boxes[i]);
	}
	/*
	 * ����������ֵ�Ƚ���һ������
	 */
	raw_ellipses = this->sort(raw_ellipses);
	/*
	 * �޳�̫�����Բ��ȡ�����Ǹ�
	 */
	std::vector<cv::RotatedRect> final_raw_ellipse;
	final_raw_ellipse.push_back(raw_ellipses[0]);
	for (unsigned int i = 0; i < raw_ellipses.size(); ++i)
	{
		float height_diff = this->diff(
			final_raw_ellipse[final_raw_ellipse.size()-1].size.height,
			raw_ellipses[i].size.height
		);
		float width_diff = this->diff(
			final_raw_ellipse[final_raw_ellipse.size()-1].size.width,
			raw_ellipses[i].size.width
		);
		if ( height_diff > distance_threshold && width_diff  > distance_threshold)
			final_raw_ellipse.push_back(raw_ellipses[i]);
	}
	return final_raw_ellipse;
}


std::vector<cv::RotatedRect> EllipseDetector::tuneRawEliipses(std::vector<cv::RotatedRect> raw_ellipses, cv::Mat src)
{
	return raw_ellipses;
}


std::vector<cv::RotatedRect> EllipseDetector::sort(std::vector<cv::RotatedRect> src)
{
	for (unsigned int i = 0; i < src.size(); ++i)
	{
		for (unsigned int j = i+1; j < src.size(); ++j) 
		{
			if( MAX(src[i].size.width, src[j].size.height) >
				MAX(src[i].size.width, src[j].size.height) )
			{
				cv::RotatedRect tmp = src[i];
				src[i] = src[j];
				src[j] = tmp;
			}
		}
	}
	return src;
}
