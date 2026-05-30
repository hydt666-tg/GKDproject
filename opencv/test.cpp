
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main()
{
    // 读取背景图片和视频
    Mat bgImg = imread("/home/wmx/桌面/project/GKDproject/opencv/背景图.jpg"); // 替换为你的背景图片路径
    VideoCapture cap("/home/wmx/桌面/project/GKDproject/opencv/绿幕素材.mp4"); // 替换为你的绿幕视频路径

    if (bgImg.empty() || !cap.isOpened())
    {
        cout << "无法打开背景图片或视频！" << endl;
        return -1;
    }

    // 获取视频帧的尺寸，并调整背景图尺寸与之匹配
    int frameWidth = cap.get(CAP_PROP_FRAME_WIDTH);
    int frameHeight = cap.get(CAP_PROP_FRAME_HEIGHT);
    resize(bgImg, bgImg, Size(frameWidth, frameHeight));

    // 定义绿色的HSV范围（可根据实际视频光照情况微调）
    Scalar lowerGreen = Scalar(45, 43, 110);  // HSV下限
    Scalar upperGreen = Scalar(77, 255, 255); // HSV上限

    // 保存视频
    // 获取视频的帧率和帧大小
    double fps = cap.get(CAP_PROP_FPS);
    Size frameSize(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT));

    // 创建 VideoWriter 对象
    VideoWriter writer("/home/wmx/桌面/project/GKDproject/opencv/output.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, frameSize);

    Mat frame, hsv, mask;
    while (cap.read(frame))
    {
        if (frame.empty())
            break;

        // 预处理，轻微模糊以减少噪声
        Mat blurredFrame;
        GaussianBlur(frame, blurredFrame, Size(3, 3), 0);

        // 转换色彩空间: BGR -> HSV
        cvtColor(blurredFrame, hsv, COLOR_BGR2HSV);

        // 创建掩码：识别绿色区域
        inRange(hsv, lowerGreen, upperGreen, mask); // 在HSV图像中根据阈值范围创建掩码
        Mat mask1 = mask.clone();

        // 形态学操作示例：先开运算去除小噪点，再闭运算填充小孔洞
        Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));
        morphologyEx(mask, mask, MORPH_OPEN, kernel);  // 开运算
        morphologyEx(mask, mask, MORPH_CLOSE, kernel); // 闭运算

        Mat result = Mat::zeros(frame.size(), frame.type());
        for (int i = 0; i < frame.rows; i++)
        {
            for (int j = 0; j < frame.cols; j++)
            {
                if (mask.at<uchar>(i, j) == 0)
                { // 掩码为0（黑色）是前景
                    result.at<Vec3b>(i, j) = frame.at<Vec3b>(i, j);
                }
                else
                { // 掩码为255（白色）是背景
                    result.at<Vec3b>(i, j) = bgImg.at<Vec3b>(i, j);
                }
            }
        }

        // 显示结果
        imshow("Original Video", frame);
        // imshow("blurredFrame",blurredFrame);
        // imshow("mask1", mask1);
        // imshow("Mask", mask);
        imshow("Result", result);

        // 将帧写入输出视频文件
        writer.write(result);

        // 按ESC退出
        if (waitKey(30) == 27)
            break;
    }

    // 释放 VideoCapture 和 VideoWriter 对象
    cap.release();
    writer.release();
    destroyAllWindows();
    return 0;
}
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stddef.h>
#include <unistd.h>

#define BUFFER_SIZE 4

// 定义全局变量
int buffer[BUFFER_SIZE] = {-1,-1,-1,-1};
int product_number = 0;

sem_t empty;  // 同步信号量：记录空缓冲区数量
sem_t full;   // 同步信号量：记录满缓冲区数量
sem_t mutex;  // 互斥信号量：保证缓冲区互斥访问

// 生产者线程
void * producer() {
    int product = rand() % 50;  // 生产随机数产品
    int i = 0;

    sleep(1);

    // ========== 填空 1：empty P 操作 ==========
    sem_wait(&empty);
    // ========== 填空 2：mutex P 操作 ==========
    sem_wait(&mutex);

    // 临界区：生产并放入缓冲区
    printf("produce NO.%d product,value is:%d\n", product_number + 1, product);
    buffer[product_number] = product;
    printf("now, buff have %d products, they are: ", product_number + 1);
    for (i = 0; i < product_number + 1; i++) {
        printf("%d   ", buffer[i]);
    }
    printf("\n\n");
    product_number++;

    sleep(1);

    // ========== 填空 3：mutex V 操作 ==========
    sem_post(&mutex);
    // ========== 填空 4：full V 操作 ==========
    sem_post(&full);

    return NULL;
}

// 消费者线程
void * consumer() {
    int i = 0;

    // ========== 填空 5：full P 操作 ==========
    sem_wait(&full);
    // ========== 填空 6：mutex P 操作 ==========
    sem_wait(&mutex);

    // 临界区：从缓冲区取出产品
    printf("consume NO.1 product,value is:%d\n", buffer[0]);
    printf("now, buff have %d products, they are: ", product_number - 1);
    for (i = 0; i < product_number - 1; i++) {
        buffer[i] = buffer[i + 1];
        printf("%d   ", buffer[i]);
    }
    printf("\n\n");
    buffer[product_number - 1] = -1;
    product_number--;

    sleep(2);

    // ========== 填空 7：mutex V 操作 ==========
    sem_post(&mutex);
    // ========== 填空 8：empty V 操作 ==========
    sem_post(&empty);

    return NULL;
}

int main() {
    pthread_t id_producer[10];
    pthread_t id_consumer[10];
    int ret = 0, i = 0;

    // 初始化信号量
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    sem_init(&mutex, 0, 1);

    pthread_t tid = pthread_self();
    printf("the main thread id is %lu \n", tid);

    // 创建 10 个生产者 + 10 个消费者
    for (i = 0; i < 10; i++) {
        ret = pthread_create(&id_producer[i], NULL, producer, NULL);
        printf("the thread id is %lu \n", id_producer[i]);
        ret = pthread_create(&id_consumer[i], NULL, consumer, NULL);
    }

    // 等待所有线程结束
    for (i = 0; i < 10; i++) {
        pthread_join(id_producer[i], NULL);
        printf("the thread id join is %lu \n", id_producer[i]);
        pthread_join(id_consumer[i], NULL);
    }

    // 销毁信号量
    sem_destroy(&empty);
    sem_destroy(&full);
    sem_destroy(&mutex);

    printf("The End...\n");
    return 0;
}