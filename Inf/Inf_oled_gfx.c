#include "Inf_oled_gfx.h"
#include "Inf_oled.h"
#include "Com_oled_res.h"
#include "Inf_font.h"
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


/******************************************************************************
 * 显示函数（字符/数字/字符串/中文/图片）
 ******************************************************************************/

/**
  * 函    数：x的y次方
  * 参    数：X 底数
  * 参    数：Y 指数
  * 返 回 值：计算结果
  */
uint32_t Inf_oled_pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;

	while (Y--)
	{
		Result *= X;
	}

	return Result;
}

/**
  * 函    数：显示一个字符
  * 参    数：X,Y 字符左上角坐标
  * 参    数：Char 要显示的字符（ASCII可见字符）
  * 参    数：FontSize 字体大小（OLED_8X16/OLED_6X8）
  * 返 回 值：无
  */
void Inf_oled_show_char(int8_t X, int8_t Y, char Char, uint8_t FontSize)
{
	if (FontSize == OLED_8X16)
	{
		Inf_oled_show_image(X, Y, 8, 16, OLED_F8x16[Char - ' ']);
	}
	else if (FontSize == OLED_6X8)
	{
		Inf_oled_show_image(X, Y, 6, 8, OLED_F6x8[Char - ' ']);
	}
}

/**
  * 函    数：显示ASCII字符串
  * 参    数：X,Y 字符串左上角坐标
  * 参    数：String 字符串（ASCII字符组成）
  * 参    数：FontSize 字体大小
  * 返 回 值：无
  */
void Inf_oled_show_ascii(int8_t X, int8_t Y, char* String, uint8_t FontSize)
{
	uint8_t i;

	for (i = 0; String[i] != '\0'; i++)
	{
		Inf_oled_show_char(X + i * FontSize, Y, String[i], FontSize);
	}
}

/**
  * 函    数：显示十进制无符号数
  * 参    数：X,Y 左上角坐标
  * 参    数：Number 要显示的数（0~4294967295）
  * 参    数：Length 长度（0~10）
  * 参    数：FontSize 字体大小
  * 返 回 值：无
  */
void Inf_oled_show_num(int8_t X, int8_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
	uint8_t i;

	if (X > 127) {return;}
	if (Y > 64) {return;}

	for (i = 0; i < Length; i++)
	{
		Inf_oled_show_char(X + i * FontSize, Y, Number / Inf_oled_pow(10, Length - i - 1) % 10 + '0', FontSize);
	}
}

/**
  * 函    数：显示十进制有符号数
  * 参    数：X,Y 左上角坐标
  * 参    数：Number 要显示的数（-2147483648~2147483647）
  * 参    数：Length 长度（0~10）
  * 参    数：FontSize 字体大小
  * 返 回 值：无
  */
void Inf_oled_show_signed_num(uint8_t X, uint8_t Y, int32_t Number, uint8_t Length, uint8_t FontSize)
{
	uint8_t i;
	uint32_t Number1;

	if (Number >= 0)
	{
		Inf_oled_show_char(X, Y, '+', FontSize);
		Number1 = Number;
	}
	else
	{
		Inf_oled_show_char(X, Y, '-', FontSize);
		Number1 = -Number;
	}

	for (i = 0; i < Length; i++)
	{
		Inf_oled_show_char(X + (i + 1) * FontSize, Y, Number1 / Inf_oled_pow(10, Length - i - 1) % 10 + '0', FontSize);
	}
}

/**
  * 函    数：显示十六进制数
  * 参    数：X,Y 左上角坐标
  * 参    数：Number 要显示的数（0x00000000~0xFFFFFFFF）
  * 参    数：Length 长度（0~8）
  * 参    数：FontSize 字体大小
  * 返 回 值：无
  */
void Inf_oled_show_hex_num(uint8_t X, uint8_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
	uint8_t i, SingleNumber;

	for (i = 0; i < Length; i++)
	{
		SingleNumber = Number / Inf_oled_pow(16, Length - i - 1) % 16;

		if (SingleNumber < 10)
		{
			Inf_oled_show_char(X + i * FontSize, Y, SingleNumber + '0', FontSize);
		}
		else
		{
			Inf_oled_show_char(X + i * FontSize, Y, SingleNumber - 10 + 'A', FontSize);
		}
	}
}

/**
  * 函    数：显示二进制数
  * 参    数：X,Y 左上角坐标
  * 参    数：Number 要显示的数
  * 参    数：Length 长度（0~16）
  * 参    数：FontSize 字体大小
  * 返 回 值：无
  */
void Inf_oled_show_bin_num(uint8_t X, uint8_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
	uint8_t i;

	for (i = 0; i < Length; i++)
	{
		Inf_oled_show_char(X + i * FontSize, Y, Number / Inf_oled_pow(2, Length - i - 1) % 2 + '0', FontSize);
	}
}

/**
  * 函    数：显示浮点数
  * 参    数：X,Y 左上角坐标
  * 参    数：Number 要显示的数（-4294967295.0~4294967295.0）
  * 参    数：IntLength 整数位长度（0~10）
  * 参    数：FraLength 小数位长度（0~9，小数部分四舍五入）
  * 参    数：FontSize 字体大小
  * 返 回 值：无
  */
void Inf_oled_show_float_num(uint8_t X, uint8_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize)
{
	uint32_t PowNum, IntNum, FraNum;

	if (Number >= 0)
	{
		Inf_oled_show_char(X, Y, '+', FontSize);
	}
	else
	{
		Inf_oled_show_char(X, Y, '-', FontSize);
		Number = -Number;
	}

	IntNum = Number;
	Number -= IntNum;
	PowNum = Inf_oled_pow(10, FraLength);
	FraNum = round(Number * PowNum);
	IntNum += FraNum / PowNum;		/* 小数进位 */

	Inf_oled_show_num(X + FontSize, Y, IntNum, IntLength, FontSize);
	Inf_oled_show_char(X + (IntLength + 1) * FontSize, Y, '.', FontSize);
	Inf_oled_show_num(X + (IntLength + 2) * FontSize, Y, FraNum, FraLength, FontSize);
}

/**
  * 函    数：显示中文（内置12x12字模查表）
  * 参    数：X,Y 左上角坐标
  * 参    数：Chinese 中文字符串（全部为汉字或全角字符）
  * 返 回 值：无
  * 说    明：未找到字模时显示默认图形（一般为方框问号）
  */
void Inf_oled_show_chinese(uint8_t X, uint8_t Y, char* Chinese)
{
	uint8_t pChinese = 0;
	uint8_t pIndex;
	uint8_t i;
	char SingleChinese[OLED_CHN_CHAR_WIDTH + 1] = {0};

	for (i = 0; Chinese[i] != '\0'; i++)
	{
		SingleChinese[pChinese] = Chinese[i];
		pChinese++;

		if (pChinese >= OLED_CHN_CHAR_WIDTH)
		{
			pChinese = 0;

			for (pIndex = 0; strcmp(OLED_CF12x12[pIndex].Index, "") != 0; pIndex++)
			{
				if (strcmp(OLED_CF12x12[pIndex].Index, SingleChinese) == 0)
				{
					break;
				}
			}

			Inf_oled_show_image(X + ((i + 1) / OLED_CHN_CHAR_WIDTH - 1) * 16, Y, 12, 12, OLED_CF12x12[pIndex].Data);
		}
	}
}

/**
  * 函    数：显示图片
  * 参    数：X,Y 图片左上角坐标（支持负坐标，只显示屏幕内部分）
  * 参    数：Width,Height 图片宽高
  * 参    数：Image 图片数据（页式布局，1bit/像素）
  * 返 回 值：无
  */
void Inf_oled_show_image(int8_t X, int8_t Y0, uint8_t Width, uint8_t Height, const uint8_t* Image)
{
	int8_t Y = (int8_t)Y0;

	uint8_t i, j, p = 1;

	if (X + Width <= 0) {return;}
	else if (X > 127) {return;}

	uint8_t k = 0, l = 0, temp_Height;

	if (Y < 0)
	{
		temp_Height = Height;
		Height += Y;
		k = (-Y - 1) / 8 + 1;
		l = (-Y - 1) % 8 + 1;
		Y = 0;

		if ((int8_t)Height < 1)
		{
			return;
		}
	}
	else if (Y > 63)
	{
		return;
	}

	/* 先清空图片区域 */
	Inf_oled_clear_area(X, Y, Width, Height);

	uint8_t Height_ceil = (Height - 1) / 8 + 1;

	for (j = 0; j < Height_ceil; j++)
	{
		p = 1;

		for (i = 0; i < Width; i++)
		{
			if (p)
			{
				if (X < 0)
				{
					i = i + (0 - X);
					p = 0;
				}
			}

			if (X + i > 127)
			{
				break;
			}
			if (Y / 8 + j > 7)
			{
				return;
			}

			if (k)
			{
				/* 显示上一Byte图片在当前页的部分 */
				Inf_oled_display_buf[Y / 8 + j][X + i] |= Image[(j + k - 1) * Width + i] >> ((l));

				/* 使用continue：当前Byte越界时，下一Byte的剩余数据还需要显示 */
				if ((j + k) * 8 >= temp_Height)
				{
					continue;
				}

				/* 显示当前Byte图片在当前页的部分 */
				Inf_oled_display_buf[Y / 8 + j][X + i] |= Image[(j + k) * Width + i] << (8 - (l));
			}
			else
			{
				/* 显示图片在当前页的部分 */
				Inf_oled_display_buf[Y / 8 + j][X + i] |= Image[(j) * Width + i] << ((Y) % 8);

				if (Y / 8 + j + 1 > 7)
				{
					continue;
				}

				/* 显示图片在下一页的部分 */
				Inf_oled_display_buf[Y / 8 + j + 1][X + i] |= Image[(j) * Width + i] >> (8 - (Y) % 8);
			}
		}
	}
}


/******************************************************************************
 * 中英文混排显示与格式化打印
 * 说明：中文走W25Q128的GB2312全字库（12x12，每字24字节），ASCII用内置6x8字模
 ******************************************************************************/

/**
  * 函    数：显示字符串（中英文混排）
  * 参    数：X,Y 字符串左上角坐标
  * 参    数：String 字符串（GB2312编码，可混排中英文）
  * 参    数：FontSize ASCII字号（OLED_6X8/OLED_8X16），汉字固定12x12
  * 返 回 值：无
  * 说    明：支持换行符，超宽自动折行；行距随字号（6x8行距8，8x16行距16）；
  *           ASCII与12px汉字底部对齐（6x8下移4，8x16上移4）
  */
void Inf_oled_show_string(int8_t X, int8_t Y, char* String, uint8_t FontSize)
{
	uint8_t i;
	uint8_t xpos = 0;
	uint8_t SChinese[24];
	uint8_t GB_L, GB_H;

	for (i = 0; String[i] != '\0'; i++)
	{
		if (String[i] == '\n')             /* 换行 */
		{
			Y += (FontSize == OLED_8X16) ? 16 : 8;
			xpos = 0;
			continue;
		}

		if (String[i] > '~')                /* 双字节字符（GB2312编码） */
		{
			if (X + xpos + 12 > 128)        /* 超宽自动折行 */
			{
				Y += (FontSize == OLED_8X16) ? 16 : 8;
				xpos = 0;
			}

			GB_H = String[i];
			i++;
			GB_L = String[i];

			Inf_font_read_gb2312(GB_H, GB_L, SChinese);
			Inf_oled_show_image(X + xpos, Y, 12, 12, SChinese);

			xpos += 12;
		}
		else                                /* ASCII字符 */
		{
			if (X + xpos + FontSize > 128)  /* 超宽自动折行 */
			{
				Y += (FontSize == OLED_8X16) ? 16 : 8;
				xpos = 0;
			}

			Inf_oled_show_char(X + xpos, Y + ((FontSize == OLED_8X16) ? -4 : 4), String[i], FontSize);

			xpos += FontSize;
		}
	}
}

/**
  * 函    数：格式化字符串显示
  * 参    数：X,Y 左上角坐标
  * 参    数：FontSize ASCII字号
  * 参    数：format 格式化字符串
  * 返 回 值：无
  */
void Inf_oled_printf(uint8_t X, uint8_t Y, uint8_t FontSize, char* format, ...)
{
	char String[30];
	va_list arg;

	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);

	Inf_oled_show_string(X, Y, String, FontSize);
}
