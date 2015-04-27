/*!
 *
 *	CS 595 Assignment 10
 *	Jie Jiang
 *	
 */
#ifndef __VEC_H
#define __VEC_H

namespace AJParallelRendering {

	struct int2 {
		int x, y;
	};
	
	class Vec3i
	{
	public:
		void setValue(int x, int y, int z) {_x = x; _y = y; _z = z; };
		void setValue(int* xyz);
		int _x;
		int _y;
		int _z;

		Vec3i operator + (const Vec3i & operand);
		Vec3i operator - (const Vec3i & operand);
		Vec3i& operator += (const Vec3i & operand);
		Vec3i& operator -= (const Vec3i & operand);
		int& operator [] (int index);
	protected:

	private:
		
	};
	
	class Vec3f
	{
	public:
		void setValue(float x, float y, float z) {_x = x; _y = y; _z = z; };
		Vec3f operator + (const Vec3f & operand);
		Vec3f operator - (const Vec3f & operand);
		Vec3f& operator += (const Vec3f & operand);
		Vec3f& operator -= (const Vec3f & operand);
		Vec3f& operator *= ( float scale );
		Vec3f operator * ( float scale );
		Vec3f operator / ( float scale );
		Vec3f operator / (const Vec3f & operand);
		Vec3f operator * (const Vec3f & operand);


		float dot ( const Vec3f& operand);
		Vec3f cross ( const Vec3f& operand);
		float length();
		float _x;
		float _y;
		float _z;
		float& operator [] (int index);

		void normalize();
	protected:

	private:

	};

	class Vec4f
	{
	public:
		void setValue(float x, float y, float z, float a) {_x = x; _y = y; _z = z; _a = a; };
		void setValue(float *xyza);
		float operator [] (int index);
		void getValue(float *rgba);
		float _x;
		float _y;
		float _z;
		float _a;
	protected:

	private:

	};

	struct Ray {
		Vec3f	origin;
		Vec3f	dir;
	};
};

#endif // __VEC_H
