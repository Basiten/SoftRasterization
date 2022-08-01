#pragma once
#include<iostream>
#include <fstream>

/* One bytes align. */
#pragma pack(push,1)
struct ImgHeader {
	char idlength;
	char colormaptype;
	char datatypecode;
	short colormaporigin;
	short colormaplength;
	char colormapdepth;
	short x_origin;
	short y_origin;
	short width;
	short height;
	char  bitsperpixel;
	char  imagedescriptor;
};
#pragma pack(pop)

struct ImgColor {
	union {
		struct {
			unsigned char b, g, r, a;
		};
		unsigned char raw[4];
		unsigned int val;
		float depth;
		
	};
	int bytespp;
	ImgColor() : val(0), bytespp(1) {}
	ImgColor(float depth) : depth(depth), bytespp(4) {}
	ImgColor(unsigned char R, unsigned char G, unsigned char B, unsigned char A) : b(B), g(G), r(R), a(A), bytespp(4) {
	}
	ImgColor(int v, int bpp) : val(v), bytespp(bpp) {
	}
	ImgColor(const ImgColor& c) : val(c.val), bytespp(c.bytespp) {
	}
	ImgColor(const unsigned char* p, int bpp) : val(0), bytespp(bpp) {
		for (int i = 0; i < bpp; i++) {
			raw[i] = p[i];
		}
	}
	ImgColor& operator =(const ImgColor& c) {
		if (this != &c) {
			bytespp = c.bytespp;
			val = c.val;
		}
		return *this;
	}
};


struct Image {
private:
	unsigned char* data;
	int width;
	int height;
	int bytespp;
	bool loadRLEData(std::ifstream& in);
	bool unloadRLEData(std::ofstream& out);

public:
	enum ImgFormat {
		GRAYSCALE = 1,
		RGB = 3,
		RGBA = 4
	};
	/* Default Constructor */
	Image();
	/* Constructor with Height, Width, BPP */
	Image(int width, int height, int bpp);
	/* Copy Constructor */
	Image(const Image & img);
	/* Destructor. */
	~Image();
	/* Read Image from a file, return boolean result. */
	bool readImage(const char * filename);
	/* Write Image to a file */
	bool writeImage(const char* filename, bool rle = true);
	/* Flip horizontally.*/
	bool flipHorizontally();
	/* Flip vertically. */
	bool flipVertically();
	/* Scale.*/
	bool scale(int w, int h);
	/* Return color of target pixel. */
	ImgColor get(int x, int y) const;
	/* Set color of a pixel. */
	bool set(int x, int y, ImgColor color);
	/* Assignment. */
	Image& operator=(const Image& img);
	/* Get height.*/
	int getHeight() const;
	/* Get width. */
	int getWidth() const;
	/* Get BPP. */
	int getBPP();
	/* */
	unsigned char* buffer();
	/* */
	void clear();
};