#ifndef IMAGEPREPROC_H
#define IMAGEPREPROC_H

class ImagePreproc
{
public:
	static void preprocess(const cv::Mat& src,
		                         cv::Mat& dst);

/************************************************************************/
/* ��������ƣ����캯����Ϊ˽�У���������                                     */
/************************************************************************/
private:
	ImagePreproc(){}
};

#endif