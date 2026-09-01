#include "Inf_oled_gfx.h"
#include "Inf_oled.h"
#include <math.h>

/******************************************************************************
 * 文件名称：Inf_oled_gfx.c（接口层）
 * 说    明：OLED绘图库实现（由旧工程OLED.c图形函数部分保真迁移）
 ******************************************************************************/

/**
  * 函    数：判断点是否在多边形内（射线法）
  * 参    数：nvert 顶点个数
  * 参    数：vertx,verty 顶点坐标数组
  * 参    数：testx,testy 待测点坐标
  * 返 回 值：1=内部，0=外部
  * 说    明：算法参考 https://wrfranklin.org/Research/Short_Notes/pnpoly.html
  */
uint8_t Inf_oled_pnpoly(uint8_t nvert, int16_t *vertx, int16_t *verty, int16_t testx, int16_t testy)
{
	int16_t i, j, c = 0;

	for (i = 0, j = nvert - 1; i < nvert; j = i++)
	{
		if (((verty[i] > testy) != (verty[j] > testy)) &&
			(testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i]))
		{
			c = !c;
		}
	}

	return c;
}

/**
  * 函    数：判断点是否在指定扇形角度内
  * 参    数：X,Y 点坐标（相对圆心）
  * 参    数：StartAngle,EndAngle 起始/终止角度，范围-180~180
  * 返 回 值：1=内部，0=外部
  * 说    明：水平向右为0度，水平向左为180度或-180度，上方为正，顺时针旋转
  */
uint8_t Inf_oled_is_in_angle(int16_t X, int16_t Y, int16_t StartAngle, int16_t EndAngle)
{
	int16_t PointAngle;

	PointAngle = atan2(Y, X) / 3.14 * 180;

	if (StartAngle < EndAngle)
	{
		if (PointAngle >= StartAngle && PointAngle <= EndAngle)
		{
			return 1;
		}
	}
	else
	{
		if (PointAngle >= StartAngle || PointAngle <= EndAngle)
		{
			return 1;
		}
	}

	return 0;
}

/**
  * 函    数：画点
  * 参    数：X,Y 点坐标
  * 返 回 值：无
  */
void Inf_oled_draw_point(int8_t X, int8_t Y)
{
	if (X > 127) {return;}
	if (Y > 63) {return;}
	if (X < 0) {return;}
	if (Y < 0) {return;}

	Inf_oled_display_buf[Y / 8][X] |= 0x01 << (Y % 8);
}

/**
  * 函    数：取点
  * 参    数：X,Y 点坐标
  * 返 回 值：1=点亮，0=熄灭
  */
uint8_t Inf_oled_get_point(uint8_t X, uint8_t Y)
{
	if (X > 127) {return 0;}
	if (Y > 63) {return 0;}

	if (Inf_oled_display_buf[Y / 8][X] & 0x01 << (Y % 8))
	{
		return 1;
	}

	return 0;
}

/**
  * 函    数：画线
  * 参    数：X0,Y0 起点坐标
  * 参    数：X1,Y1 终点坐标
  * 返 回 值：无
  * 说    明：斜线使用Bresenham算法，避免耗时浮点运算
  */
void Inf_oled_draw_line(uint8_t X0, uint8_t Y0, uint8_t X1, uint8_t Y1)
{
	int16_t x, y, dx, dy, d, incrE, incrNE, temp;
	int16_t x0 = X0, y0 = Y0, x1 = X1, y1 = Y1;
	uint8_t yflag = 0, xyflag = 0;

	if (y0 == y1)		/* 横线 */
	{
		if (x0 > x1) {temp = x0; x0 = x1; x1 = temp;}

		for (x = x0; x <= x1; x++)
		{
			Inf_oled_draw_point(x, y0);
		}
	}
	else if (x0 == x1)	/* 竖线 */
	{
		if (y0 > y1) {temp = y0; y0 = y1; y1 = temp;}

		for (y = y0; y <= y1; y++)
		{
			Inf_oled_draw_point(x0, y);
		}
	}
	else				/* 斜线 */
	{
		if (x0 > x1)
		{
			temp = x0; x0 = x1; x1 = temp;
			temp = y0; y0 = y1; y1 = temp;
		}

		if (y0 > y1)
		{
			/* Y轴取反，画线方向由第四象限变为第一象限，取反不影响画线 */
			y0 = -y0;
			y1 = -y1;

			/* 置标志位，实际画点时再将坐标变换回去 */
			yflag = 1;
		}

		if (y1 - y0 > x1 - x0)	/* 斜率大于1 */
		{
			/* X轴Y轴互换，画线方向由0~90度范围变为第一象限0~45度范围 */
			temp = x0; x0 = y0; y0 = temp;
			temp = x1; x1 = y1; y1 = temp;

			/* 置标志位，实际画点时再将坐标变换回去 */
			xyflag = 1;
		}

		/* Bresenham算法画线，要求画线方向为第一象限0~45度范围 */
		dx = x1 - x0;
		dy = y1 - y0;
		incrE = 2 * dy;
		incrNE = 2 * (dy - dx);
		d = 2 * dy - dx;
		x = x0;
		y = y0;

		/* 画起点，同时根据标志位变换坐标 */
		if (yflag && xyflag){Inf_oled_draw_point(y, -x);}
		else if (yflag)		{Inf_oled_draw_point(x, -y);}
		else if (xyflag)	{Inf_oled_draw_point(y, x);}
		else				{Inf_oled_draw_point(x, y);}

		while (x < x1)
		{
			x++;
			if (d < 0)
			{
				d += incrE;
			}
			else
			{
				y++;
				d += incrNE;
			}

			/* 画每个点，同时根据标志位变换坐标 */
			if (yflag && xyflag){Inf_oled_draw_point(y, -x);}
			else if (yflag)		{Inf_oled_draw_point(x, -y);}
			else if (xyflag)	{Inf_oled_draw_point(y, x);}
			else				{Inf_oled_draw_point(x, y);}
		}
	}
}

/**
  * 函    数：画矩形
  * 参    数：X,Y 矩形左上角坐标
  * 参    数：Width,Height 矩形宽高
  * 参    数：IsFilled 是否填充
  * 返 回 值：无
  */
void Inf_oled_draw_rectangle(int8_t X, int8_t Y, uint8_t Width, uint8_t Height, uint8_t IsFilled)
{
	uint8_t i, j;

	if (!IsFilled)		/* 空心 */
	{
		for (i = X; i < X + Width; i++)
		{
			Inf_oled_draw_point(i, Y);
			Inf_oled_draw_point(i, Y + Height - 1);
		}
		for (i = Y; i < Y + Height; i++)
		{
			Inf_oled_draw_point(X, i);
			Inf_oled_draw_point(X + Width - 1, i);
		}
	}
	else				/* 填充 */
	{
		for (i = X; i < X + Width; i++)
		{
			for (j = Y; j < Y + Height; j++)
			{
				Inf_oled_draw_point(i, j);
			}
		}
	}
}

/**
  * 函    数：画三角形
  * 参    数：X0,Y0 X1,Y1 X2,Y2 三个顶点坐标
  * 参    数：IsFilled 是否填充
  * 返 回 值：无
  * 说    明：填充时用射线法判断包围盒内每点是否在三角形内
  */
void Inf_oled_draw_triangle(uint8_t X0, uint8_t Y0, uint8_t X1, uint8_t Y1,
                            uint8_t X2, uint8_t Y2, uint8_t IsFilled)
{
	uint8_t minx = X0, miny = Y0, maxx = X0, maxy = Y0;
	uint8_t i, j;
	int16_t vx[] = {X0, X1, X2};
	int16_t vy[] = {Y0, Y1, Y2};

	if (!IsFilled)		/* 空心：三条直线 */
	{
		Inf_oled_draw_line(X0, Y0, X1, Y1);
		Inf_oled_draw_line(X0, Y0, X2, Y2);
		Inf_oled_draw_line(X1, Y1, X2, Y2);
	}
	else				/* 填充 */
	{
		if (X1 < minx) {minx = X1;}
		if (X2 < minx) {minx = X2;}
		if (Y1 < miny) {miny = Y1;}
		if (Y2 < miny) {miny = Y2;}

		if (X1 > maxx) {maxx = X1;}
		if (X2 > maxx) {maxx = X2;}
		if (Y1 > maxy) {maxy = Y1;}
		if (Y2 > maxy) {maxy = Y2;}

		/* 遍历包围盒，点在三角形内则画点 */
		for (i = minx; i <= maxx; i++)
		{
			for (j = miny; j <= maxy; j++)
			{
				if (Inf_oled_pnpoly(3, vx, vy, i, j)) {Inf_oled_draw_point(i, j);}
			}
		}
	}
}

/**
  * 函    数：画圆
  * 参    数：X,Y 圆心坐标
  * 参    数：Radius 半径
  * 参    数：IsFilled 是否填充
  * 返 回 值：无
  * 说    明：Bresenham算法画圆
  */
void Inf_oled_draw_circle(uint8_t X, uint8_t Y, uint8_t Radius, uint8_t IsFilled)
{
	int16_t x, y, d, j;

	d = 1 - Radius;
	x = 0;
	y = Radius;

	/* 画每个八分之一圆弧的起始点 */
	Inf_oled_draw_point(X + x, Y + y);
	Inf_oled_draw_point(X - x, Y - y);
	Inf_oled_draw_point(X + y, Y + x);
	Inf_oled_draw_point(X - y, Y - x);

	if (IsFilled)		/* 填充 */
	{
		for (j = -y; j < y; j++)
		{
			Inf_oled_draw_point(X, Y + j);
		}
	}

	while (x < y)
	{
		x++;
		if (d < 0)
		{
			d += 2 * x + 1;
		}
		else
		{
			y--;
			d += 2 * (x - y) + 1;
		}

		/* 画每个八分之一圆弧的点 */
		Inf_oled_draw_point(X + x, Y + y);
		Inf_oled_draw_point(X + y, Y + x);
		Inf_oled_draw_point(X - x, Y - y);
		Inf_oled_draw_point(X - y, Y - x);
		Inf_oled_draw_point(X + x, Y - y);
		Inf_oled_draw_point(X + y, Y - x);
		Inf_oled_draw_point(X - x, Y + y);
		Inf_oled_draw_point(X - y, Y + x);

		if (IsFilled)	/* 填充 */
		{
			for (j = -y; j < y; j++)
			{
				Inf_oled_draw_point(X + x, Y + j);
				Inf_oled_draw_point(X - x, Y + j);
			}

			for (j = -x; j < x; j++)
			{
				Inf_oled_draw_point(X - y, Y + j);
				Inf_oled_draw_point(X + y, Y + j);
			}
		}
	}
}

/**
  * 函    数：画椭圆
  * 参    数：X,Y 椭圆中心坐标
  * 参    数：A,B 横轴/纵轴半长
  * 参    数：IsFilled 是否填充
  * 返 回 值：无
  */
void Inf_oled_draw_ellipse(uint8_t X, uint8_t Y, uint8_t A, uint8_t B, uint8_t IsFilled)
{
	int16_t x, y, j;
	int16_t a = A, b = B;
	float d1, d2;

	x = 0;
	y = b;
	d1 = b * b + a * a * (-b + 0.5);

	if (IsFilled)	/* 填充 */
	{
		for (j = -y; j < y; j++)
		{
			Inf_oled_draw_point(X, Y + j);
			Inf_oled_draw_point(X, Y + j);
		}
	}

	/* 画椭圆左侧起始点 */
	Inf_oled_draw_point(X + x, Y + y);
	Inf_oled_draw_point(X - x, Y - y);
	Inf_oled_draw_point(X - x, Y + y);
	Inf_oled_draw_point(X + x, Y - y);

	/* 画椭圆中间部分 */
	while (b * b * (x + 1) < a * a * (y - 0.5))
	{
		if (d1 <= 0)
		{
			d1 += b * b * (2 * x + 3);
		}
		else
		{
			d1 += b * b * (2 * x + 3) + a * a * (-2 * y + 2);
			y--;
		}
		x++;

		if (IsFilled)	/* 填充 */
		{
			for (j = -y; j < y; j++)
			{
				Inf_oled_draw_point(X + x, Y + j);
				Inf_oled_draw_point(X - x, Y + j);
			}
		}

		/* 画椭圆中间部分轮廓点 */
		Inf_oled_draw_point(X + x, Y + y);
		Inf_oled_draw_point(X - x, Y - y);
		Inf_oled_draw_point(X - x, Y + y);
		Inf_oled_draw_point(X + x, Y - y);
	}

	/* 画椭圆右侧部分 */
	d2 = b * b * (x + 0.5) * (x + 0.5) + a * a * (y - 1) * (y - 1) - a * a * b * b;

	while (y > 0)
	{
		if (d2 <= 0)
		{
			d2 += b * b * (2 * x + 2) + a * a * (-2 * y + 3);
			x++;
		}
		else
		{
			d2 += a * a * (-2 * y + 3);
		}
		y--;

		if (IsFilled)	/* 填充 */
		{
			for (j = -y; j < y; j++)
			{
				Inf_oled_draw_point(X + x, Y + j);
				Inf_oled_draw_point(X - x, Y + j);
			}
		}

		/* 画椭圆右侧部分轮廓点 */
		Inf_oled_draw_point(X + x, Y + y);
		Inf_oled_draw_point(X - x, Y - y);
		Inf_oled_draw_point(X - x, Y + y);
		Inf_oled_draw_point(X + x, Y - y);
	}
}

/**
  * 函    数：画圆弧
  * 参    数：X,Y 圆心坐标
  * 参    数：Radius 半径
  * 参    数：StartAngle,EndAngle 起始/终止角度，范围-180~180
  * 参    数：IsFilled 是否填充
  * 返 回 值：无
  * 说    明：Bresenham算法画圆，画每点时判断是否在指定角度内
  */
void Inf_oled_draw_arc(uint8_t X, uint8_t Y, uint8_t Radius, int16_t StartAngle, int16_t EndAngle, uint8_t IsFilled)
{
	int16_t x, y, d, j;

	d = 1 - Radius;
	x = 0;
	y = Radius;

	/* 画每个八分之一圆弧的起始点 */
	if (Inf_oled_is_in_angle(x, y, StartAngle, EndAngle)) {Inf_oled_draw_point(X + x, Y + y);}
	if (Inf_oled_is_in_angle(-x, -y, StartAngle, EndAngle)) {Inf_oled_draw_point(X - x, Y - y);}
	if (Inf_oled_is_in_angle(y, x, StartAngle, EndAngle)) {Inf_oled_draw_point(X + y, Y + x);}
	if (Inf_oled_is_in_angle(-y, -x, StartAngle, EndAngle)) {Inf_oled_draw_point(X - y, Y - x);}

	if (IsFilled)	/* 填充 */
	{
		for (j = -y; j < y; j++)
		{
			if (Inf_oled_is_in_angle(0, j, StartAngle, EndAngle)) {Inf_oled_draw_point(X, Y + j);}
		}
	}

	while (x < y)
	{
		x++;
		if (d < 0)
		{
			d += 2 * x + 1;
		}
		else
		{
			y--;
			d += 2 * (x - y) + 1;
		}

		if (Inf_oled_is_in_angle(x, y, StartAngle, EndAngle)) {Inf_oled_draw_point(X + x, Y + y);}
		if (Inf_oled_is_in_angle(y, x, StartAngle, EndAngle)) {Inf_oled_draw_point(X + y, Y + x);}
		if (Inf_oled_is_in_angle(-x, -y, StartAngle, EndAngle)) {Inf_oled_draw_point(X - x, Y - y);}
		if (Inf_oled_is_in_angle(-y, -x, StartAngle, EndAngle)) {Inf_oled_draw_point(X - y, Y - x);}
		if (Inf_oled_is_in_angle(x, -y, StartAngle, EndAngle)) {Inf_oled_draw_point(X + x, Y - y);}
		if (Inf_oled_is_in_angle(y, -x, StartAngle, EndAngle)) {Inf_oled_draw_point(X + y, Y - x);}
		if (Inf_oled_is_in_angle(-x, y, StartAngle, EndAngle)) {Inf_oled_draw_point(X - x, Y + y);}
		if (Inf_oled_is_in_angle(-y, x, StartAngle, EndAngle)) {Inf_oled_draw_point(X - y, Y + x);}

		if (IsFilled)	/* 填充 */
		{
			for (j = -y; j < y; j++)
			{
				if (Inf_oled_is_in_angle(x, j, StartAngle, EndAngle)) {Inf_oled_draw_point(X + x, Y + j);}
				if (Inf_oled_is_in_angle(-x, j, StartAngle, EndAngle)) {Inf_oled_draw_point(X - x, Y + j);}
			}

			for (j = -x; j < x; j++)
			{
				if (Inf_oled_is_in_angle(-y, j, StartAngle, EndAngle)) {Inf_oled_draw_point(X - y, Y + j);}
				if (Inf_oled_is_in_angle(y, j, StartAngle, EndAngle)) {Inf_oled_draw_point(X + y, Y + j);}
			}
		}
	}
}
