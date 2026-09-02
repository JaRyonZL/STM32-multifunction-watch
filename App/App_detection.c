#include "App_detection.h"

/******************************************************************************
 * 文件名：App_detection.c（应用层）
 * 说   明：体感界面（迁移自旧工程 menu_Data.c 的 Detection_angle）
 *          MPU6050 DMP 暂缓未实现，本模块为预留占位
 ******************************************************************************/

/**
  * 函   数：App_detection_run
  * 功   能：体感界面入口（原Detection_angle：Pitch/Roll/Yaw 姿态与
  *          gyro/accel 数据显示，长按复位 MPU）
  * 说   明：MPU6050 暂缓，占位空实现；后续实现时接 Inf_mpu6050 取数，
  *          TLYPW 置位逻辑一并迁入
  */
void App_detection_run(void)
{
	/* 占位：MPU6050 DMP 暂缓实现 */
}
