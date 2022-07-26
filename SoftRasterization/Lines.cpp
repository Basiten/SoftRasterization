#include"Lines.h"
void line(glm::vec2 t0, glm::vec2 t1, Image& image, const ImgColor& color) {
    line(t0.x, t0.y, t1.x, t1.y, image, color);
}
void line(int x0, int y0, int x1, int y1, Image& image, const ImgColor& color) {
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1 - x0;
    int dy = y1 - y0;
    int derror2 = std::abs(dy) * 2;
    int error2 = 0;
    int y = y0;
    for (int x = x0; x <= x1; x++) {
        if (steep) {
            image.set(y, x, color);
        }
        else {
            image.set(x, y, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}

void wireFrame(Model& model, Image& img, const ImgColor &color){
    for (int i = 0; i < model.nfaces(); i++) {
        std::vector<int> face = model.face(i);
        for (int j = 0; j < 3; j++) {
            glm::vec3 v0 = model.vert(face[j]);
            glm::vec3 v1 = model.vert(face[(j + 1) % 3]);
            int x0 = (v0.x + 1.) * img.getWidth() / 2.;
            int y0 = (v0.y + 1.) * img.getHeight() / 2.;
            int x1 = (v1.x + 1.) * img.getWidth() / 2.;
            int y1 = (v1.y + 1.) * img.getHeight() / 2.;
            line(x0, y0, x1, y1, img, color);
        }
    }
};