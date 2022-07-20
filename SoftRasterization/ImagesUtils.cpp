#include"ImagesUtils.h"
#include <corecrt_memory.h>

Image::Image() :data(nullptr), width(0), height(0), bytespp(0) {}
Image::Image(int width, int height, int bpp) : data(nullptr), width(width), height(height), bytespp(bpp) {
	// total bytes.
	unsigned long nbytes = width * height * bytespp;
	data = new unsigned char[nbytes];
	// set 0 to all.
	memset(data, 0, nbytes);
}
Image::Image(const Image& img) {
	this->width = img.width;
	this->height = img.height;
	this->bytespp = img.bytespp;
	unsigned long nbytes = width * height * bytespp;
	data = new unsigned char[nbytes];
	// copy data.
	memcpy(data, img.data, nbytes);
}

Image::~Image() {
	if (data) { delete[] data; }
}

Image& Image::operator=(const Image& img) {
	if (this != &img) {
		if (data) { delete[] data; }
		this->width = img.width;
		this->height = img.height;
		this->bytespp = img.bytespp;
		unsigned long nbytes = width * height * bytespp;
		data = new unsigned char[nbytes];
		// copy data.
		memcpy(data, img.data, nbytes);
	}
	return *this;
}

bool Image::readImage(const char* filename) {
	if (data) delete[] data;
	data = NULL;
	std::ifstream in;
	in.open(filename, std::ios::binary);
	if (!in.is_open()) {
		std::cerr << "Can't open file " << filename << "\n";
		in.close();
		return false;
	}
	ImgHeader header;
	in.read((char*)&header, sizeof(header));
	if (!in.good()) {
		in.close();
		std::cerr << "Error occured while reading the header\n";
		return false;
	}
	width = header.width;
	height = header.height;
	// bytes = 8 bits.
	bytespp = header.bitsperpixel >> 3;
	if (width <= 0 || height <= 0 || (bytespp != GRAYSCALE && bytespp != RGB && bytespp != RGBA)) {
		in.close();
		std::cerr << "BPP/Width/Height value error.\n";
		return false;
	}
	unsigned long nbytes = bytespp * width * height;
	data = new unsigned char[nbytes];
	if (3 == header.datatypecode || 2 == header.datatypecode) {
		in.read((char*)data, nbytes);
		if (!in.good()) {
			in.close();
			std::cerr << "Error occured while reading the data\n";
			return false;
		}
	}
	else if (10 == header.datatypecode || 11 == header.datatypecode) {
		if (!loadRLEData(in)) {
			in.close();
			std::cerr << "an error occured while reading the data\n";
			return false;
		}
	}
	else {
		in.close();
		std::cerr << "unknown file format " << (int)header.datatypecode << "\n";
		return false;
	}
	if (!(header.imagedescriptor & 0x20)) {
		flipVertically();
	}
	if (header.imagedescriptor & 0x10) {
		flipHorizontally();
	}
	std::cerr << width << "x" << height << "/" << bytespp * 8 << "\n";
	in.close();
	return true;
}

bool Image::loadRLEData(std::ifstream& in) {
	unsigned long pixelcount = width * height;
	unsigned long currentpixel = 0;
	unsigned long currentbyte = 0;
	ImgColor colorbuffer;
	do {
		unsigned char chunkheader = 0;
		chunkheader = in.get();
		if (!in.good()) {
			std::cerr << "an error occured while reading the data\n";
			return false;
		}
		if (chunkheader < 128) {
			chunkheader++;
			for (int i = 0; i < chunkheader; i++) {
				in.read((char*)colorbuffer.raw, bytespp);
				if (!in.good()) {
					std::cerr << "an error occured while reading the header\n";
					return false;
				}
				for (int t = 0; t < bytespp; t++)
					data[currentbyte++] = colorbuffer.raw[t];
				currentpixel++;
				if (currentpixel > pixelcount) {
					std::cerr << "Too many pixels read\n";
					return false;
				}
			}
		}
		else {
			chunkheader -= 127;
			in.read((char*)colorbuffer.raw, bytespp);
			if (!in.good()) {
				std::cerr << "an error occured while reading the header\n";
				return false;
			}
			for (int i = 0; i < chunkheader; i++) {
				for (int t = 0; t < bytespp; t++)
					data[currentbyte++] = colorbuffer.raw[t];
				currentpixel++;
				if (currentpixel > pixelcount) {
					std::cerr << "Too many pixels read\n";
					return false;
				}
			}
		}
	} while (currentpixel < pixelcount);
	return true;
}

bool Image::writeImage(const char* filename, bool rle) {
	unsigned char developer_area_ref[4] = { 0, 0, 0, 0 };
	unsigned char extension_area_ref[4] = { 0, 0, 0, 0 };
	unsigned char footer[18] = { 'T','R','U','E','V','I','S','I','O','N','-','X','F','I','L','E','.','\0' };
	std::ofstream out;
	out.open(filename, std::ios::binary);
	if (!out.is_open()) {
		std::cerr << "can't open file " << filename << "\n";
		out.close();
		return false;
	}
	ImgHeader header;
	memset((void*)&header, 0, sizeof(header));
	header.bitsperpixel = bytespp << 3;
	header.width = width;
	header.height = height;
	header.datatypecode = (bytespp == GRAYSCALE ? (rle ? 11 : 3) : (rle ? 10 : 2));
	header.imagedescriptor = 0x20; // top-left origin
	out.write((char*)&header, sizeof(header));
	if (!out.good()) {
		out.close();
		std::cerr << "can't dump the tga file\n";
		return false;
	}
	if (!rle) {
		out.write((char*)data, width * height * bytespp);
		if (!out.good()) {
			std::cerr << "can't unload raw data\n";
			out.close();
			return false;
		}
	}
	else {
		if (!unloadRLEData(out)) {
			out.close();
			std::cerr << "can't unload rle data\n";
			return false;
		}
	}
	out.write((char*)developer_area_ref, sizeof(developer_area_ref));
	if (!out.good()) {
		std::cerr << "can't dump the tga file\n";
		out.close();
		return false;
	}
	out.write((char*)extension_area_ref, sizeof(extension_area_ref));
	if (!out.good()) {
		std::cerr << "can't dump the tga file\n";
		out.close();
		return false;
	}
	out.write((char*)footer, sizeof(footer));
	if (!out.good()) {
		std::cerr << "can't dump the tga file\n";
		out.close();
		return false;
	}
	out.close();
	return true;
}

bool Image::unloadRLEData(std::ofstream& out) {
	const unsigned char max_chunk_length = 128;
	unsigned long npixels = width * height;
	unsigned long curpix = 0;
	while (curpix < npixels) {
		unsigned long chunkstart = curpix * bytespp;
		unsigned long curbyte = curpix * bytespp;
		unsigned char run_length = 1;
		bool raw = true;
		while (curpix + run_length < npixels && run_length < max_chunk_length) {
			bool succ_eq = true;
			for (int t = 0; succ_eq && t < bytespp; t++) {
				succ_eq = (data[curbyte + t] == data[curbyte + t + bytespp]);
			}
			curbyte += bytespp;
			if (1 == run_length) {
				raw = !succ_eq;
			}
			if (raw && succ_eq) {
				run_length--;
				break;
			}
			if (!raw && !succ_eq) {
				break;
			}
			run_length++;
		}
		curpix += run_length;
		out.put(raw ? run_length - 1 : run_length + 127);
		if (!out.good()) {
			std::cerr << "can't dump the tga file\n";
			return false;
		}
		out.write((char*)(data + chunkstart), (raw ? run_length * bytespp : bytespp));
		if (!out.good()) {
			std::cerr << "can't dump the tga file\n";
			return false;
		}
	}
	return true;
}

ImgColor Image::get(int x, int y) const {
	if (!data || x < 0 || y < 0 || x >= width || y >= height) {
		return ImgColor();
	}
	// Data is aligned row by row.
	return ImgColor(data + (x + width * y) * bytespp, bytespp);
}

bool Image::set(int x, int y, ImgColor color){
	if (!data || x < 0 || y < 0 || x >= width || y >= height) {
		return false;
	}
	memcpy(data + (x + width * y) * bytespp, color.raw, bytespp);
	return true;
}

int Image::getHeight() const{
	return height;
}

int Image::getWidth() const{
	return width;
}

int Image::getBPP() {
	return bytespp;
}

bool Image::flipHorizontally() {
	if (!data) { return false; }
	int half = width / 2;
	for (int x = 0; x < half; x++) {
		for (int y = 0; y < height; y++) {
			ImgColor c1 = get(x, y);
			ImgColor c2 = get(width-x-1,y);
			set(x, y, c2);
			set(width-x-1, y, c1);
		}
	}
	return true;
}

bool Image::flipVertically() {
	if (!data) { return false; }
	int half = height / 2;
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < half; y++) {
			ImgColor c1 = get(x, y);
			ImgColor c2 = get(x, height-y-1);
			set(x, y, c2);
			set(x, height-y-1, c1);
		}
	}
	return true;
}

unsigned char* Image::buffer() {
	return data;
}

void Image::clear() {
	memset((void*)data, 0, width * height * bytespp);
}

bool Image::scale(int w, int h){
	if (!data) { return false; }
	unsigned long nbytes = w * h * bytespp;
	unsigned char * newdata = new unsigned char[nbytes];
 	// nearest
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			ImgColor color = get((int)x * (width / (float)w), (int)y * (height / (float)h));
			memcpy(newdata + (x + w * y) * bytespp, color.raw, bytespp);
		}
	}
	delete [] data;
	width = w;
	height = h;
	data = newdata;
	return true;
}
